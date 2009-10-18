
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
#include <air_ms_preamble.h>

air_ms_preamble_sptr air_make_ms_preamble(int channel_rate)
{
    return air_ms_preamble_sptr(new air_ms_preamble(channel_rate));
}

air_ms_preamble::air_ms_preamble(int channel_rate) :
    gr_sync_block ("ms_preamble",
                   gr_make_io_signature2 (2, 2, sizeof(float), sizeof(ms_plinfo)),
                   gr_make_io_signature2 (2, 2, sizeof(float), sizeof(ms_plinfo)))
{
    d_channel_rate = channel_rate;
    d_reference = 0.0;
    d_bit_positions[0] = 0 * channel_rate / 10000000;  // Figure out sample positions of preamble pulses (0.1 us units)
    d_bit_positions[1] = 10 * channel_rate / 10000000;
    d_bit_positions[2] = 35 * channel_rate / 10000000;
    d_bit_positions[3] = 45 * channel_rate / 10000000;
    d_data_start = MS_PREAMBLE_TIME_US * channel_rate / 1000000;
    d_bit_width = MS_BIT_TIME_US * channel_rate / 1000000;
    d_chip_width = d_bit_width / 2;   // Two Chips per bit
    d_check_width = ((MS_PREAMBLE_TIME_US+(5*MS_BIT_TIME_US)) * channel_rate / 1000000)+2;
    d_var_n = (channel_rate > 8000000)?3:2;  // Number of bits after leading edge to sample
    d_var_m = d_var_n + 1;
    set_output_multiple(2*d_check_width);
}


int air_ms_preamble::work(int noutput_items,
                          gr_vector_const_void_star &input_items,
		                  gr_vector_void_star &output_items)
{
    float *data_in = (float *)input_items[0];
    ms_plinfo *attrib_in = (ms_plinfo *)input_items[1];
    float *data_out = (float *) output_items[0];  // sample data out
    ms_plinfo *attrib_out = (ms_plinfo *) output_items[1];    // attribute data out

    int size = noutput_items - d_check_width; // Only search up to a check width from the end
    int i, j, k;
    float f;
    for (i = 0; i < size; i++)
    {
	float reference = 0.0;
    	int   lateness[MS_PREAMBLE_PULSE_COUNT];  // How late a bit is in samples
    	int   pcount = 0;
    	int   lcount = 0;
        int maxcount = 0;
        int multiflag = 0;
    	float levels[d_var_n*MS_PREAMBLE_PULSE_COUNT];
        int rcount[d_var_n*MS_PREAMBLE_PULSE_COUNT];
        float high_limit = 0.0;
        float low_limit = 0.0;
        float min_level = 0.0;
        float max_level = 0.0;
        for (j = 0; j < MS_PREAMBLE_PULSE_COUNT; j++)
		lateness[j] = -1;
	data_out[i] = data_in[i]; // Direct Copy
        attrib_out[i] = attrib_in[i];
	// look for valid pulses at 0 1 3.5 and 4.5 uS
        // also collect samples for later processing
	for(j = 0; j < MS_PREAMBLE_PULSE_COUNT; j++)
	{
		int pos = i + d_bit_positions[j]; // Position to the sample
		if(attrib_in[pos+1].leading_edge())
		{
			lateness[j] = 1;
			for(k = 1; k <= d_var_n; k++)
				levels[lcount++] = data_in[pos+k+lateness[j]];
    			pcount++;
		}
		else if(attrib_in[pos].leading_edge())
		{
			lateness[j] = 0;
			for(k = 1; k <= d_var_n; k++)
				levels[lcount++] = data_in[pos+k+lateness[j]];
    			pcount++;
		}
		else if(attrib_in[pos].valid_pulse())
		{
			lateness[j] = 0;
    			pcount++;
		}
		else if(attrib_in[pos+1].valid_pulse())
		{
			lateness[j] = 1;
    			pcount++;
		}
		else
			break;  // No Valid Pulse in time slot
	}
	if((pcount < MS_PREAMBLE_PULSE_COUNT) || (lcount < (d_var_n*(MS_PREAMBLE_PULSE_COUNT/2))))
		continue;
        // Plus or minus one sample is okay but not samples at both plus and minus
        // This code only looks ahead one sample so it is possible that all four samples are late
        // If all samples are late then just continue and it will be picked up next time
	if((lateness[0] + lateness[1] + lateness[2] + lateness[3]) == 4)
		continue;
        // The Mode S specifications say the amplitude levels of the pulses must be within 2 dB
	// Take the samples after the leading edges and figure out the reference level
        // The Reference Level is also used downstream for framing and decoding
        multiflag = 0;
	for(j = 0; j < lcount; j++)
	{
		// Count the number of other samples that are within 2 db of the level
		rcount[j] = 0;
		high_limit = levels[j] * 1.25893;  // 2 db
		low_limit =  levels[j] * 0.79433;  // - 2 db
		for(k = 0; k < lcount; k++)
		{
			if(j == k) // Do not count itself
				continue;
			if(levels[k] >= low_limit && levels[k] <= high_limit)
				rcount[j]++;
		}
                // if the count is the higher than previous high level then assume it is the only highest
		if(rcount[j] > maxcount)
		{
			maxcount = rcount[j];
			multiflag = 0;
                        // This is the reference candidate
			min_level = reference = levels[j];
		}
                // else if there is a tie more processing is needed unless a higher level comes along later
		else if(rcount[j] == maxcount)
		{
			multiflag++;
                        // find the minimum power of the maximum count samples
			if(levels[j] < min_level)
			{
				min_level = levels[j];
			}
		}
	}
        // If there are 2 or more values with the same maximum count then average out the samples
	if(multiflag)
	{
		max_level = min_level * 1.25893; // + 2 dB
		reference = 0.0;
		k = 0;
		for(j = 0; j < lcount; j++)
		{
			// Sum up samples with the maximum count and within 2 dB of minimum power
			if((rcount[j] == maxcount) && (levels[j] <= max_level))
			{
				reference += levels[j];
				k++;
			}
		}
                // This should never happen so if it does then give up
		if ((k == 0) || (reference == 0))
			continue;
                // Average the samples
		reference = reference / (float)k;
	}
        // Mode S Frames can overlap (FRUIT) and if the later frame is 3 dB or stronger it can be decoded.
        // Later processing can retrigger the start of a data frame but it has problems when
        // the later preamble values are used to calculate the current preamble reference level.
        //
        // Look for overlapping preambles
        // Only the bit after the preamble start + the bit time is used
        // Start with the 1.0 uS position and find the minimum level at 1.0 2.0 4.5 5.5
        int offset = i + lateness[0] + d_bit_positions[1] + 1;
        min_level = data_in[offset + d_bit_positions[0]];
        for (j = 1; j < MS_PREAMBLE_PULSE_COUNT; j++)
	{
		f = data_in[offset+d_bit_positions[j]];
		if(f < min_level)
			min_level = f;
	}
        // calculate the -3 dB point
        min_level *= 0.70795;  // -3 dB
        // Find maximum of 0 and 3.5
	offset = i + lateness[0] + 1;
	max_level = data_in[offset + d_bit_positions[0]];
        f =  data_in[offset + d_bit_positions[2]];
	if(f > max_level)
		max_level = f;
        // If maximum of 0 and 3.5 is below the -3 dB point of the minimum level at 1.0 2.0 4.5 5.5 then reject
	if(max_level < min_level)
		continue;
        // Go to the 3.5 position and find the minimum level at 3.5 4.5 7.0 8.0
        offset = i + lateness[0] + d_bit_positions[2] + 1;
        min_level = data_in[offset + d_bit_positions[0]];
        for(j = 1; j < MS_PREAMBLE_PULSE_COUNT; j++)
	{
		f = data_in[offset+d_bit_positions[j]];
		if(f < min_level)
			min_level = f;
	}
       // calculate the -3 dB point
        min_level *= 0.70795;  // - 3 dB
        // Find maximum of 0 and 1.0
	offset = i + lateness[0] + 1;
	max_level = data_in[offset + d_bit_positions[0]];
        f =  data_in[offset + d_bit_positions[1]];
	if(f > max_level)
		max_level = f;
        // If maximum of 0 and 1.0 is below the -3 dB point of the minimum level at 3.5 4.5 7.0 8.0 then reject
	if(max_level < min_level)
		continue;
        // Go to the 4.5 position and find the minimum level at 4.5 5.5 8.0 9.0
        offset = i + lateness[0] + d_bit_positions[3] + 1;
        min_level = data_in[offset + d_bit_positions[0]];
        for(j = 1; j < MS_PREAMBLE_PULSE_COUNT; j++)
	{
		f = data_in[offset+d_bit_positions[j]];
		if(f < min_level)
			min_level = f;
	}
       // calculate the -3 dB point
        min_level *= 0.70795;  // -3 dB
       // Find maximum of 0 1.0 3.5
	offset = i + lateness[0] + 1;
	max_level = data_in[offset + d_bit_positions[0]];
        f =  data_in[offset + d_bit_positions[1]];
	if(f > max_level)
		max_level = f;
        f =  data_in[offset + d_bit_positions[2]];
	if(f > max_level)
		max_level = f;
        // If maximum of 0 1.0 3.5 is below the -3 dB point of the minimum level at 4.5 5.5 8.0 9.0 then reject
	if(max_level < min_level)
		continue;
	// Consistent Power Test
        // Two out of the 4 preambles must be within 3 dB of the reference
	maxcount = 0;
	high_limit = reference * 1.41253;  // + 3 dB
	low_limit = reference * 0.70795;  // - 3 dB
	offset = i + lateness[0] + 1;
        for(j = 0; j < MS_PREAMBLE_PULSE_COUNT; j++)
	{
		f = data_in[offset+d_bit_positions[j]];
		if (f >= low_limit && f <= high_limit)
			maxcount++;
	}
	if(maxcount < (MS_PREAMBLE_PULSE_COUNT/2))
		continue;  // No preamble here so return
        // DF Validation
        // Look for valid pulses in one or both of the chip positions for 5 data bits
        // Pulses must be -6 dB or greater of the reference level
        low_limit = reference * 0.5012;  // -6 dB
        offset = i + lateness[0] + d_data_start;
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
		if ((chips == 0) || (max_level < low_limit))
			break;  // No preamble here so break out
	}
	if(j < (5 * d_bit_width)) // If Data Field is not valid then search
		continue;
	// There is a possible preamble
        // Preamble detection is always running so it is up to the downstream to
        // not act on preamble starts that occur because of data in the Mode S frame
	d_reference = reference;
	attrib_out[i].set_preamble_start(d_reference);
    }
    return i;
}
