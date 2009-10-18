
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

#include <gr_io_signature.h>
#include <air_ms_types.h>
#include <air_ms_ppm_decode.h>

air_ms_ppm_decode_sptr air_make_ms_ppm_decode(int channel_rate)
{
    return air_ms_ppm_decode_sptr(new air_ms_ppm_decode(channel_rate));
}

air_ms_ppm_decode::air_ms_ppm_decode(int channel_rate) :
    gr_block ("ms_ppm_decode",
                   gr_make_io_signature2 (2, 2, sizeof(float), sizeof(ms_plinfo)),
                   gr_make_io_signature (1, 1, sizeof(ms_frame_raw)))
{
    d_channel_rate = channel_rate;
    d_reference = 0.0;
    d_high_limit = 0.0;
    d_low_limit = 0.0;
    d_low_energy_limit = 0.0;
    d_data_start = MS_PREAMBLE_TIME_US * channel_rate / 1000000; // in uS
    d_bit_width = MS_BIT_TIME_US * channel_rate / 1000000;
    d_chip_width = d_bit_width / 2;   // Two Chips per bit
    d_min_data_width =  MS_BIT_TIME_US * MS_SHORT_FRAME_LENGTH * channel_rate / 1000000; // in uS
    d_max_data_width =  MS_BIT_TIME_US * MS_LONG_FRAME_LENGTH * channel_rate / 1000000;
    d_sample_count = 0;

    //  Mode S frames are sent in a burst of one frame that occurs when the Mode S transponder is interrogated
    //  by the ground or other aircraft (ACAS/TCAS).  ADS-B Frames may also be sent once per second.
    //  This number represents a channel occupancy of about 50%  or over 4 thousand frames per second.
    //  It is unlikely that 700 to 2000 aircraft will be in range
    set_relative_rate((double)(d_max_data_width+d_data_start)*2.0+2.0);
    // set_output_multiple(2*(d_max_data_width+2));
}

void air_ms_ppm_decode::forecast (int noutput_items,
	       gr_vector_int &ninput_items_required)
{
	int size;
	size = noutput_items*d_max_data_width*2+2;  // do at least two frame width of data
	ninput_items_required[1] = ninput_items_required[0] = size;
}

int air_ms_ppm_decode::general_work(int noutput_items,
		                gr_vector_int &ninput_items,
		                gr_vector_const_void_star &input_items,
	                        gr_vector_void_star &output_items)

{
    float *data_in = (float *)input_items[0];
    ms_plinfo *attrib_in = (ms_plinfo *)input_items[1];
    ms_frame_raw *data_out = (ms_frame_raw *) output_items[0];  // sample data out

    int size = ninput_items[0] - (d_max_data_width+2); // Only search up to a frame from the end
    int i, j, k;
    int out_count;
    int bit_index = 0;
    int frame_end = 0;
    float f;
    out_count = 0;
    j = 0;
    for (i = 0; (i < size) && (out_count < noutput_items); i++)
    {
	// Ignore any preamble starts and look for data start
        // The upstream code puts a reference value at data start
	if(attrib_in[i].data_start())
	{
		// Calculate the reference and limits
		d_reference = attrib_in[i].reference();
		d_high_limit = d_reference * 1.41253;  // + 3 dB
		d_low_limit = d_reference * 0.70795;  // - 3 dB
                d_low_energy_limit = d_reference * 0.5012;  // -6 dB
                // Prep an output frame
		data_out[out_count].reset_all();
		data_out[out_count].set_rx_time(time(NULL));
		data_out[out_count].set_timestamp((d_sample_count + i));
		data_out[out_count].set_reference(d_reference);
		// Init variables used in loop
	        bit_index = 0;
		frame_end = 0;
		int multiplier = 1;
		int phase = 0;  // Data bit phase
		int chip_zero_ok = 0;
		int chip_zero_low_energy = 0;
		int chip_one_ok = 0;
		int chip_one_low_energy = 0;
		int score_zero = 0;
		int score_one = 0;
		int score_low_energy;
		// Go for up to the Maximum Frame length for the data bits
                // It is possible that noise could skew the phase enough that the buffer runs out so test input size as well
		for(bit_index = 0; (i < ninput_items[0]) && (bit_index < MS_LONG_FRAME_LENGTH); i++)
		{
			// If leading edge is a sample time late resync
			if((phase == 1) && attrib_in[i].leading_edge())
			{
                        	phase--;
				chip_one_ok = 0;   // Might as well reset
				chip_one_low_energy = 0;
			}
			// If leading edge is a sample time early resync
			if((phase == 3) && attrib_in[i].leading_edge())
			{
                        	phase++;
			}
			// If leading edge is a sample time late resync
			if((phase == (d_chip_width+1)) && attrib_in[i].leading_edge())
			{
                        	phase--;
				chip_zero_ok = 0;  // Might as well reset
				chip_zero_low_energy = 0;
			}
			// If leading edge is a sample time late resync
			if((phase == 7) && attrib_in[i].leading_edge())
			{
                        	phase++;
			}
			// the middle of the pulses have more weight than the edges
			if((phase == 0) ||(phase == (d_chip_width-1)) ||
			   (phase == d_chip_width) ||(phase >= (d_bit_width-1)))
				multiplier = 1;
			else
				multiplier = 2;
                        // Score the samples that represent a valid level and a low energy level
			if(phase < d_chip_width)   // the first chip represents a data value of one
			{
				f = data_in[i];
				if(f >= d_low_limit && f <= d_high_limit)
					chip_one_ok += multiplier;
				else if(f < d_low_energy_limit)
					chip_one_low_energy += multiplier;
			}
			else if(phase < d_bit_width)  // the second chip represents a data value of zero
			{
				f = data_in[i];
				if(f >= d_low_limit && f <= d_high_limit)
					chip_zero_ok += multiplier;
				else if(f < d_low_energy_limit)
					chip_zero_low_energy += multiplier;
			}
			if(++phase >= d_bit_width)
			{
				// Decide what the bit is.  Tie scores go to the zero
				score_one = chip_one_ok - chip_zero_ok + chip_zero_low_energy - chip_one_low_energy;
				score_zero = chip_zero_ok - chip_one_ok + chip_one_low_energy - chip_zero_low_energy;
				score_low_energy = chip_one_low_energy + chip_zero_low_energy - chip_one_ok - chip_zero_ok;
				k = abs(score_one - score_zero);
				if(k > 2)  // Bit has high confidence
				{
					data_out[out_count].set_bit_high_confidence(bit_index, (score_one > score_zero)?1:0);
				}
				else if(k > 0)  // Bit has a low confidence
				{
					data_out[out_count].set_bit_low_confidence(bit_index, (score_one > score_zero)?1:0);
				}
				else if(chip_zero_low_energy >= 6)  // both chips are equal as k == 0
				{
					data_out[out_count].set_bit_low_energy(bit_index, (score_one > score_zero)?1:0);
				}
				else
				{
					data_out[out_count].set_bit_low_confidence(bit_index, 0);
				}
				if(frame_end)
					break;  // Fall to data end block below
				phase = 0;
				chip_zero_ok = 0;
				chip_zero_low_energy = 0;
				chip_one_ok = 0;
				chip_one_low_energy = 0;
				score_zero = 0;
				score_one = 0;
				bit_index++;
			}
			if(attrib_in[i].data_end())
			{
				if(phase == 0)
					break;  // Fall to data end block below
				else  // complete the bit
					frame_end++;
			}
		}
	}
	// If frame has ended send the output
	if(attrib_in[i].data_end() || frame_end)
	{
		frame_end = 0;
		if(bit_index >= MS_LONG_FRAME_LENGTH)
		{
			data_out[out_count].set_long_frame();
		}
		else if (bit_index >= MS_SHORT_FRAME_LENGTH)
		{
			data_out[out_count].set_short_frame();
		}
		d_reference = 0.0; // Reset the reference
                out_count++;
	}
    }
    // Consumed the input with a output packet
    d_sample_count += i;
    consume_each(i);
    return out_count;
}
