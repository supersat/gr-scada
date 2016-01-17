/* -*- c++ -*- */
/* 
 * Copyright 2015 Regents of the University of California.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "mdsx710_decode_impl.h"

#include <stdio.h>

namespace gr {
  namespace scada {

    mdsx710_decode::sptr
    mdsx710_decode::make()
    {
      return gnuradio::get_initial_sptr
        (new mdsx710_decode_impl());
    }

    /*
     * The private constructor
     */
    mdsx710_decode_impl::mdsx710_decode_impl()
      : gr::block("mdsx710_decode",
              gr::io_signature::make(1, 1, sizeof(float)),
              gr::io_signature::make(1, 1, sizeof(char))),
        m_synced(false), m_in_frame(false), m_buffer_offset(0),
        m_sample_counter(0), m_last_bit(0), m_lfsr(0),
        m_outChar(0), m_charCount(0), m_dc_avg(0),
        m_syms_left_in_preamble(0)
    {
        memset(m_buffer, 0, sizeof(m_buffer));
        memset(m_preamble_corr, 0, sizeof(m_preamble_corr));

        // Generate the preamble signal to look for
        for (int i = 0; i < 312; i++) {
            m_preamble_signal[i] = cos(2 * M_PI * i / 32);
        }
    }

    /*
     * Our virtual destructor.
     */
    mdsx710_decode_impl::~mdsx710_decode_impl()
    {
    }

    void
    mdsx710_decode_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
        /* If we are synced, we need eight samples per bit. Otherwise, we request enough
           samples to detect a preamble. */
        if (m_synced) {
            ninput_items_required[0] = noutput_items * 8 * 9;
        } else {
            // Preamble is 320 samples long, but 240 samples will let us detect a correlation peak
            ninput_items_required[0] = 480;
        }
    }

    char
    mdsx710_decode_impl::slice(float sample)
    {
        if (sample > 0.75) {
            m_last_bit = 1;
            return 1;
        } else if (sample < -0.75) {
            m_last_bit = 0;
            return 0;
        } else {
            m_last_bit = !m_last_bit;
            return m_last_bit;
        }
    }

    int
    mdsx710_decode_impl::distance_to_preamble_corr_peak()
    {
        int max_idx = -1;
        float max_corr = 240; // preamble detection threshold

        for (int i = 0; i < 480; i++) {
            if (m_preamble_corr[i] > max_corr) {
                max_idx = i;
                max_corr = m_preamble_corr[i];
            }
        }

        if (max_idx == -1)
            return -1;
        if (max_idx > m_buffer_offset)
            return 480 - (max_idx - m_buffer_offset);
        else
            return m_buffer_offset - max_idx;
    }

    static int mod(int a, int b)
    {
        int res = a % b;
        if (res < 0)
            return res + b;
        else
            return res;
    }

    char mdsx710_decode_impl::lfsr(char c)
    {
        m_lfsr = (m_lfsr << 1) | c;
        return ((m_lfsr & 0x80) >> 7) ^ ((m_lfsr & 0x40) >> 6) ^ (m_lfsr & 1);
    }

    void mdsx710_decode_impl::process_bit(char c, char *out, int &noutput_items_generated)
    {
        unsigned nc = lfsr(c);
        m_outChar = (m_outChar << 1) | nc;

        // We initialize m_outChar to be 1. If the 1 in bit 0 gets shifted
        // to bit 9, then we've received an entire character.
        if (m_outChar & 0x200) {
            //printf("%03x\n", m_outChar);
            // If bit 8 is 0, then this is a standard in-band char
            if (!(m_outChar & 0x100)) {
                //m_outBuf[m_outCharCount++] = m_outChar & 0xff;
                out[noutput_items_generated++] = m_outChar & 0xff;
            } else if (m_outChar == 0x383) {
                m_in_frame = false;
                m_synced = false;
            }

            m_outChar = 1;
            m_charCount++;
            if (m_charCount == 60) {
                m_in_frame = false;
                m_synced = false;
            }
        }
    }

    int
    mdsx710_decode_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
        const float *in = (const float *) input_items[0];
        char *out = (char *) output_items[0];
        int noutput_items_generated = 0;

        for (int i = 0; i < ninput_items[0]; i++) {
            if (m_synced) {
                if (m_sample_counter++ == 0) {
					char bit = slice(m_buffer[mod(m_buffer_offset - 128, 480)] - m_dc_avg);
                    if (m_in_frame) {
                        if (m_syms_left_in_preamble > 0) {
                            m_outChar = (m_outChar << 1) | bit;
                            m_syms_left_in_preamble--;
                            if (!m_syms_left_in_preamble) {
                                // Did we detect the preamble too early?
                                if ((m_outChar & 0xf0) == 0xc0) {
                                    m_syms_left_in_preamble = 4;
                                } else {
                                    m_outChar = 1;
                                }
                            }
                        } else {
                            process_bit(bit, out, noutput_items_generated);
                        }
                    }
                }

                if (m_sample_counter == 8) {
                    m_sample_counter = 0;
                }
            }

            m_buffer[m_buffer_offset] = in[i];

            // Compute the average level over the correlation window to correct for
            // any tuning frequency offset (which becomes a DC value after FM demodulation).
            float avgLevelOverCorrWindow = 0;
            float minLevelOverCorrWindow = std::numeric_limits<float>::infinity();
            float maxLevelOverCorrWindow = -std::numeric_limits<float>::infinity();
            for (int j = 0; j < 312; j++) {
                float s = m_buffer[mod(m_buffer_offset + j + 168, 480)];
                if (s < minLevelOverCorrWindow)
                    minLevelOverCorrWindow = s;
                if (s > maxLevelOverCorrWindow)
                    maxLevelOverCorrWindow = s;
                avgLevelOverCorrWindow += s;
            }
            avgLevelOverCorrWindow /= 312;

            // Normalize samples over the preamble correlation window to [-1, +1]
            float normFactor = 4 / (maxLevelOverCorrWindow - minLevelOverCorrWindow);
            //printf("%f\n", normFactor);
            float corr = 0;
            for (int j = 0; j < 312; j++) {
                corr += m_preamble_signal[j] * normFactor *
                    (m_buffer[mod(m_buffer_offset + j + 168, 480)] -
                     avgLevelOverCorrWindow);
            }
            m_preamble_corr[m_buffer_offset] = corr;

            int dist = distance_to_preamble_corr_peak();
            if (dist == 127) {
                m_synced = true;
                m_sample_counter = 0;
                m_last_bit = 0;
                m_dc_avg = avgLevelOverCorrWindow;
                m_outChar = 0;
                m_in_frame = true;
                m_lfsr = 0;
                m_charCount = 0;
                m_syms_left_in_preamble = 8;
            }

            m_buffer_offset++;
            if (m_buffer_offset >= 480)
                m_buffer_offset = 0;
        }

        consume_each (ninput_items[0]);

        // Tell runtime system how many output items we produced.
        return noutput_items_generated;
    }

  } /* namespace scada */
} /* namespace gr */

