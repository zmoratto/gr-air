/*
 * Copyright 2007 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_AIR_MS_PPM_DECODE_H
#define INCLUDED_AIR_MS_PPM_DECODE_H

#include <gr_block.h>

class air_ms_ppm_decode;
typedef boost::shared_ptr<air_ms_ppm_decode> air_ms_ppm_decode_sptr;

air_ms_ppm_decode_sptr air_make_ms_ppm_decode(int channel_rate);

/*!
 * \brief mode select framer
 * \ingroup block
 */
class air_ms_ppm_decode : public gr_block
{
private:
    friend air_ms_ppm_decode_sptr air_make_ms_ppm_decode(int channel_rate);
    air_ms_ppm_decode(int channel_rate);

    float d_reference;   // current reference level
    float d_high_limit;
    float d_low_limit;
    float d_low_energy_limit;
    int d_channel_rate;  // Sample rate of the streams
    int d_data_start;
    int d_chip_width;
    int d_bit_width;
    int d_min_data_width;
    int d_max_data_width;
    int d_sample_count;

public:
    void forecast (int noutput_items,
		   gr_vector_int &ninput_items_required);

    int general_work (int noutput_items,
		      gr_vector_int &ninput_items,
		      gr_vector_const_void_star &input_items,
		      gr_vector_void_star &output_items);
};

#endif /* INCLUDED_AIR_MS_PPM_DECODE_H */
