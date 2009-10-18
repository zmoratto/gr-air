
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
#include <air_ms_framer.h>

air_ms_framer_sptr air_make_ms_framer(int channel_rate)
{
    return air_ms_framer_sptr(new air_ms_framer(channel_rate));
}

air_ms_framer::air_ms_framer(int channel_rate) :
    gr_sync_block ("ms_framer",
                   gr_make_io_signature2 (2, 2, sizeof(float), sizeof(ms_plinfo)),
                   gr_make_io_signature2 (2, 2, sizeof(float), sizeof(ms_plinfo)))
{
    d_channel_rate = channel_rate;
    d_reference = 0.0;
    d_data_start = MS_PREAMBLE_TIME_US * channel_rate / 1000000;
    d_bit_width = MS_BIT_TIME_US * channel_rate / 1000000;
    d_chip_width = d_bit_width / 2;   // Two Chips per bit
    d_min_frame_width =  (MS_PREAMBLE_TIME_US + MS_BIT_TIME_US * MS_SHORT_FRAME_LENGTH)* channel_rate / 1000000;
    d_max_frame_width = (MS_PREAMBLE_TIME_US + MS_BIT_TIME_US * MS_LONG_FRAME_LENGTH)* channel_rate / 1000000;
    d_var_n = (channel_rate > 8000000)?3:2;  // Number of bits after leading edge to sample
    d_var_m = d_var_n + 1;

    set_output_multiple(2*(d_max_frame_width+2));
}


int air_ms_framer::work(int noutput_items,
                          gr_vector_const_void_star &input_items,
	                  gr_vector_void_star &output_items)

{
    float *data_in = (float *)input_items[0];
    ms_plinfo *attrib_in = (ms_plinfo *)input_items[1];
    float *data_out = (float *) output_items[0];  // sample data out
    ms_plinfo   *attrib_out = (ms_plinfo *) output_items[1];    // attribute data out

    int size = noutput_items - d_max_frame_width - 2; // Only search up to a frame from the end
    int i, j, k;
    int offset;
    int frame_size;
    float reference;
    float high_limit;
    float low_limit;
    float max_level;
    reference = 0.0;
    for (i = 0; i < size; i++)
    {
	data_out[i] = data_in[i]; // Direct Copy
        attrib_out[i] = attrib_in[i];
	// Look for the start of the extended part of the frame
	if(attrib_in[i].preamble_start())
	{
		// Calculate the reference and limits
		reference = attrib_in[i].reference();
                low_limit = reference * 0.5012;  // -6 dB
		// There is a short 56 bit frame and a long 112 bit frame
                // So figure out the frame size
                // Assume maximum frame size
		frame_size = d_max_frame_width;
		// Do a similar DF Valid technique for bits 57 through 62
        	offset = i + d_min_frame_width;
		for( j = 0; j < (5 * d_bit_width); j += d_bit_width)
		{
			int chips;
			int leflag;
                	int jj;
			max_level = 0.;
			chips = 0;
			leflag = 0;
			k = 0;
			if(attrib_in[offset+j].valid_pulse())
			{
				chips++;
				max_level = data_in[offset+j]; // init maximum level
				leflag = 1;
			}
			else // look at +/- 1 bit for valid pulse
			{
				for(k = -1; k < 2; k += 2)
				{
					if(attrib_in[offset+j+k].valid_pulse())
					{
						chips++;
						max_level = data_in[offset+j+k]; // init maximum level
						leflag = 1;
						break;
					}
				}
                	}
			if(leflag)
			{
				for(jj = 0; jj < d_var_m; jj++)
				{
					if(data_in[offset+j+k+jj] > max_level)
						max_level = data_in[offset+j+k+jj];
				}
			}
                	// Look at the second chip now
			leflag = 0;
                	k = d_chip_width;
			if(attrib_in[offset+j+k].valid_pulse())
			{
				chips++;
				max_level = data_in[offset+j+k]; // init maximum level
				leflag = 1;
			}
			else // look at +/- 1 bit for valid pulse
			{
				for(k = d_chip_width -1; k < d_chip_width+2; k += 2)
				{
					if(attrib_in[offset+j+k].valid_pulse())
					{
						chips++;
						max_level = data_in[offset+j+k];
						leflag = 1;
						break;
					}
				}
			}
			if(leflag)
			{
				for(jj = 0; jj < d_var_m; jj++)
				{
					if(data_in[offset+j+k+jj] > max_level)
						max_level = data_in[offset+j+k+jj];
				}
			}
			if ((chips == 0) || (max_level < low_limit))  // If no valid bits at 57 - 62 so 56 bit frame
			{
				frame_size = d_min_frame_width;
				break;
			}
		}
                // "Retrigger"  See if there is a preamble detected with a level that is more than 3 dB
                //              in the frame.  If so ignore current frame and move on
		high_limit = reference * 1.41253;  // + 3 dB
                for (j = 1; j <= frame_size; j++)
		{
			// Copy the frame data to output
			data_out[i+j] = data_in[i+j]; // Direct Copy
        		attrib_out[i+j] = attrib_in[i+j];
                        // If there is a preamble start see if it is + 3 dB stronger otherwise clear it
			if(attrib_in[i+j].preamble_start())
			{
				if (high_limit < attrib_in[i+j].reference())
				{
					// Ignore current preamble and any preamble up to the stronger one
					attrib_out[i].reset_preamble_start();
					break;
				}
				else // Make downstream processing ignore any following preambles within frame size
				{
					attrib_out[i+j].reset_preamble_start();
				}
			}
			attrib_out[i+j].set_reference(reference); // Keep setting the reference (mainly for scope display)
		}
                // If no stronger preamble in the frame then output
                if(j > frame_size)
		{
			attrib_out[i+d_data_start].set_data_start(reference);  // denote the start of data and indicate reference again
			attrib_out[i+frame_size].set_data_end();  // denote the end
		}
                // point to the sample after the frame or the stronger preamble
		i = i + j;
	}
    }
    return i;
}
