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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <air_ms_ec_brute.h>
#include <airi_ms_parity.h>
#include <gr_io_signature.h>
#include <air_ms_types.h>

// The recommendation is to do a convervative error correction of 12 bits and a brute force of 5 bits.
// The modern processor should be able to do 12 bits brute force :)
const int MAX_EC_CORRECTION = 12;


air_ms_ec_brute_sptr air_make_ms_ec_brute()
{
    return air_ms_ec_brute_sptr(new air_ms_ec_brute());
}

air_ms_ec_brute::air_ms_ec_brute() :
    gr_sync_block("ms_ec_brute",
	gr_make_io_signature(1, 1, sizeof(ms_frame_raw)),
	gr_make_io_signature(1, 1, sizeof(ms_frame_raw)))
{
}

int air_ms_ec_brute::work(int noutput_items,
    gr_vector_const_void_star &input_items,
    gr_vector_void_star &output_items)
{
    ms_frame_raw *data_in = (ms_frame_raw *)input_items[0];
    ms_frame_raw *data_out = (ms_frame_raw *)output_items[0];

    unsigned int error_syndrome = 0;
    unsigned int crc;
    int lcb_positions[MAX_EC_CORRECTION + 1];
    int i, j, index;
    int search_code;
    int offset;
    int found;
    int correction = 0;
    for (i = 0; i < noutput_items; i++) {
	data_out[i] = data_in[i];
	if((data_out[i].lcb_count() == 0) || (data_out[i].ec_quality() & (ms_frame_raw::crc_ok | ms_frame_raw::eq_ec_corrected)))
		continue;  // Nothing to do so continue on
        // The assumption is Mode A/C "Fruit" flipped some bits that were set as low confidence upstream.
        // This will only work for ADS-B and ACAS/TCAS Frames as the address overlayed on the parity is zero.
	if(data_out[i].lcb_count() <= MAX_EC_CORRECTION)
	{
		// Get the error
		error_syndrome = ms_check_parity(data_out[i]);
		index = 0;
		found = 0;
                // Offset into the parity table for short frames  The offsets are 0 for long frames and 56 for short frames
		offset = MS_LONG_FRAME_LENGTH - data_out[i].length();
 		// Assume all bits need to be flipped so start at the high end
		search_code = (1 << data_out[i].lcb_count()) - 1;
		// Get the low confidence bits
		lcb_positions[index++] = data_out[i].first_lcb();
		for (j= data_out[i].first_lcb() + 1; j <= data_out[i].last_lcb();j++)
		{
			if(data_out[i].flags(j))
			{
				lcb_positions[index++] =j;
			}
		}
		// Search down
		// Zero code means no correction which at this point is not possible
		while(search_code > 0)
		{
			crc = 0;
			// Calculate the syndrome for the search code
			for(j = 0; j < index; j++)
			{
				if((search_code >> j) & 1)
					crc ^= ms_parity_table[lcb_positions[j]+offset];
			}
			// A match
			if(crc == error_syndrome)
			{
				// Remember the correction
				correction = search_code;
				// If over one solution then it is no solution
				if(++found > 1)
					break;
			}
			search_code--;
		}
		if(found == 1) // Only one valid solution
		{
			// Flip the bits
			for(j = 0; j < index; j++)
			{
				if((correction >> j) & 1)
				{
					data_out[i].set_bit_flipped(lcb_positions[j]);
				}
			}
			// Correct the output
			crc = ms_check_parity(data_out[i]);
			data_out[i].set_address(crc);
			data_out[i].set_ec_quality(ms_frame_raw::eq_ec_corrected);
		}
		else if(found > 1)  // Indicate multiple solutions
			data_out[i].set_ec_quality(ms_frame_raw::eq_ec_multiple);
		else  // Nothing can be done
			data_out[i].set_ec_quality(ms_frame_raw::eq_ec_na);


	}
	else  // Too many lcbs
		data_out[i].set_ec_quality(ms_frame_raw::eq_ec_na);
    }
    return i;
}
