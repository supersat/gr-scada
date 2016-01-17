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

#ifndef INCLUDED_SCADA_MDSX710_DECODE_IMPL_H
#define INCLUDED_SCADA_MDSX710_DECODE_IMPL_H

#include <scada/mdsx710_decode.h>

namespace gr {
  namespace scada {

    class mdsx710_decode_impl : public mdsx710_decode
    {
     private:
      // TODO(supersat): Are these the same?
      bool m_synced;
      bool m_in_frame;
      float m_buffer[480];
      float m_preamble_signal[312];
      float m_preamble_corr[480];
      int m_buffer_offset;
      int m_sample_counter;
      char m_last_bit;
      unsigned char m_lfsr;
      int m_outChar;
      int m_charCount;
      int m_syms_left_in_preamble;
      float m_dc_avg;

      char slice(float sample);
      int distance_to_preamble_corr_peak();

      char lfsr(char c);
      void process_bit(char c, char *out, int &noutput_items_generated);

     public:
      mdsx710_decode_impl();
      ~mdsx710_decode_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
		       gr_vector_int &ninput_items,
		       gr_vector_const_void_star &input_items,
		       gr_vector_void_star &output_items);
    };

  } // namespace scada
} // namespace gr

#endif /* INCLUDED_SCADA_MDSX710_DECODE_IMPL_H */

