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


#ifndef INCLUDED_SCADA_MDSX710_DECODE_H
#define INCLUDED_SCADA_MDSX710_DECODE_H

#include <scada/api.h>
#include <gnuradio/block.h>

namespace gr {
  namespace scada {

    /*!
     * \brief Converts a stream of soft duobinary symbols to a stream of bits.
     * \ingroup scada
     *
     */
    class SCADA_API mdsx710_decode : virtual public gr::block
    {
     public:
      typedef boost::shared_ptr<mdsx710_decode> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of scada::mdsx710_decode.
       *
       * To avoid accidental use of raw pointers, scada::mdsx710_decode's
       * constructor is in a private implementation
       * class. scada::mdsx710_decode::make is the public interface for
       * creating new instances.
       */
      static sptr make();
    };

  } // namespace scada
} // namespace gr

#endif /* INCLUDED_SCADA_MDSX710_DECODE_H */

