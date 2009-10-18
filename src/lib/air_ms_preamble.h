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

#ifndef INCLUDED_AIR_MS_PREAMBLE_H
#define INCLUDED_AIR_MS_PREAMBLE_H

#include <gr_sync_block.h>
#include <air_ms_consts.h>   // For Mode S const values

class air_ms_preamble;
typedef boost::shared_ptr<air_ms_preamble> air_ms_preamble_sptr;

air_ms_preamble_sptr air_make_ms_preamble(int channel_rate);

/*!
 * \brief mode select preamble detection
 * \ingroup block
 */
class air_ms_preamble : public gr_sync_block
{
private:
    friend air_ms_preamble_sptr air_make_ms_preamble(int channel_rate);
    air_ms_preamble(int channel_rate);

    float d_reference;   // current reference level
    int d_channel_rate;  // Sample rate of the streams
    int d_bit_positions[MS_PREAMBLE_PULSE_COUNT];  // Position of preample pulses in samples
    int d_data_start;        // When the data starts in samples
    int d_check_width;   // Width of Preamble checking in samples
    int d_chip_width;    // Width of chip (1/2 bit time) in samples
    int d_bit_width;     // Width of bit in samples
    int d_var_n;         // Number of samples to use after a leading edge
    int d_var_m;         // d_var_n plus trailing edge
public:
    int work (int noutput_items,
              gr_vector_const_void_star &input_items,
              gr_vector_void_star &output_items);
};

#endif /* INCLUDED_AIR_MS_PREAMBLE_H */
