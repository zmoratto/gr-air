#
# Copyright 2007 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
#
from gnuradio import gr, gru, air

risetime_threshold_db = 48.0   # The minimum change for pulse leading edge in dB per bit time (Assume value for 8 MHz BW)
data_rate = 1000000.0        # Data rate in bits per second
chip_rate = data_rate*2.0     # Two chips to a bit so rate is double

class ppm_demod(gr.hier_block2):
    """
    Mode S protocol demodulation block.

    This block demodulates a complex down-converted baseband
    channel into Aviation Mode S protocol frames.

    Flow graph (so far):

    RESAMP   - Resample Input Stream (if needed)
    MAG      - Use Magitude function to get AM Pulses
    DETECT   - Detects Valid Pulses and Leading Edges
    SYNC     - Detects the Mode S Preamble
    FRAME    - Frames the data
    BIT      - Decodes the Pulse Position Modulation (PPM) to Mode S Data Frames
    PARITY   - Parity Checking (CRC)
    EC       - Brute Force Error Correction
    """
    def __init__(self, channel_rate, threshold):
        gr.hier_block2.__init__(self, "ppm_demod",
                              gr.io_signature(1, 1, gr.sizeof_gr_complex),
                              gr.io_signature(1, 1, 512))

        chan_rate = 8000000 # Minimum sample rate

        if channel_rate < chan_rate:
            raise ValueError, "Invalid channel rate %d. Must be 8000000 sps or higher" % (channel_rate)
        
        if channel_rate >= 10000000:
            chan_rate = 10000000    # Higher Performance Receiver

        # if rate is not supported then resample
        if channel_rate != chan_rate:
            interp = gru.lcm(channel_rate, chan_rate)/channel_rate
            decim  = gru.lcm(channel_rate, chan_rate)/chan_rate
            self.RESAMP = rational_resampler_ccf(self, interp, decim)

        # Calculate the leading edge threshold per sample time
        leading_edge = risetime_threshold_db/(chan_rate/data_rate)

        # Calculate the number of following samples above threshold needed to make a sample a valid pulse position
        valid_pulse_position = 2
        if chan_rate == 10000000:
            valid_pulse_position = 3

        # Demodulate AM with classic sqrt (I*I + Q*Q)
        self.MAG = gr.complex_to_mag()
        self.DETECT = air.ms_pulse_detect(leading_edge, threshold, valid_pulse_position) # Attack, Threshold, Pulsewidth
        self.SYNC = air.ms_preamble(chan_rate)
        self.FRAME = air.ms_framer(chan_rate)
        self.BIT   = air.ms_ppm_decode(chan_rate)
        self.PARITY = air.ms_parity()
        self.EC    =  air.ms_ec_brute()

        if channel_rate != chan_rate:
            self.connect(self, self.RESAMP, self.MAG, self.DETECT)
        else: 
            self.connect(self, self.MAG, self.DETECT)

        self.connect((self.DETECT, 0), (self.SYNC, 0))
        self.connect((self.DETECT, 1), (self.SYNC, 1))
        self.connect((self.SYNC, 0), (self.FRAME, 0))
        self.connect((self.SYNC, 1), (self.FRAME, 1))
        self.connect((self.FRAME, 0), (self.BIT, 0))
        self.connect((self.FRAME, 1), (self.BIT, 1))
        self.connect(self.BIT, self.PARITY, self.EC, self)

