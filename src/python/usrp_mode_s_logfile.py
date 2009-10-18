#!/usr/bin/env python

from gnuradio import gr, gru, usrp, optfir, eng_notation, air
from gnuradio.eng_option import eng_option
from grc_gnuradio import usrp as grc_usrp
from optparse import OptionParser
import time, os, sys
from string import split, join
from usrpm import usrp_dbid
import ppm_demod

"""
This example application demonstrates receiving and demodulating the
Aviation Mode Select (Mode S) Transponder protocol.  The decoded Mode
Select frames are outputted to a log file

A receive chain is built up of the following signal processing
blocks:

USRP  - Daughter board source generating complex baseband signal.
MODE_S - Mode S transponder protocol decoder
FORMAT - Format Mode S Frames for logging

The following are optional command line parameters:

-R SUBDEV    Daughter board specification, defaults to first found
-f FREQ         USRP receive frequency (1090 MHz Default)
-g GAIN      Daughterboard gain setting. Defaults to mid-range.
-d DECIM     USRP decimation rate
-t THRESH    Receiver valid pulse threshold
-a           Output all frames. Defaults only output frames

Once the program is running, ctrl-break (Ctrl-C) stops operation.
"""

def pick_subdevice(u):
    """
    The user didn't specify a subdevice on the command line.
    Try for one of these, in order: DBS_RX, BASIC_RX, whatever is on side A.

    @return a subdev_spec
    """
    return usrp.pick_subdev(u, (usrp_dbid.DBS_RX,
                                usrp_dbid.BASIC_RX))

class app_flow_graph(gr.top_block):
    def __init__(self, options, args, queue):
        gr.top_block.__init__(self)
        self.options = options
        self.args = args

        self.u = usrp.source_c(which=0, decim_rate=self.options.decim)
        if self.options.rx_subdev_spec is None:
            self.options.rx_subdev_spec = pick_subdevice(self.u)

        self.u.set_mux(usrp.determine_rx_mux_value(self.u, self.options.rx_subdev_spec))
        self.subdev = usrp.selected_subdev(self.u, self.options.rx_subdev_spec)

        if options.gain is None:
            g = self.subdev.gain_range()
            options.gain = float(g[0]+g[1])/2
        self.subdev.set_gain(options.gain)

        r = self.u.tune(0, self.subdev, options.freq)
        if_rate = self.u.adc_freq() / self.u.decim_rate()
        pass_all = 0
        if options.output_all :
            pass_all = 1
            MODE_S = ppm_demod.ppm_demod(if_rate, options.thresh)
        FORMAT = air.ms_fmt_log(pass_all, queue)

        self.connect(self.u, MODE_S, FORMAT)

def main():
    usage="%prog: [options] output_filename"
    parser = OptionParser(option_class=eng_option, usage=usage)
    parser.add_option("-R", "--rx-subdev-spec", type="subdev",
                      help="select USRP Rx side A or B", metavar="SUBDEV")
    parser.add_option("-f", "--freq", type="eng_float", default=1090.0,
                      help="set receive frequency to MHz [default=%default]", metavar="FREQ")
    parser.add_option("-g", "--gain", type="int", default=None,
                      help="set RF gain", metavar="dB")
    parser.add_option("-d", "--decim", type="int", default=8,
                      help="set fgpa decimation rate to DECIM [default=%default]")
    parser.add_option("-T", "--thresh", type="int", default=10,
                      help="set valid pulse threshold to THRESH [default=%default]")
    parser.add_option("-a","--output-all", action="store_true", default=False,
                          help="output all frames")
    (options, args) = parser.parse_args()
    if len(args) != 1:
        parser.print_help()
        raise SystemExit, 1
    filename = args[0]

    options.freq *= 1e6

    queue = gr.msg_queue()

    fg = app_flow_graph(options, args, queue)
    print "Running"
    try:
        fileHandle = open(filename, "w")
        fg.start()
        while 1:
            msg = queue.delete_head() # Blocking read
            fileHandle.write( msg.to_string()+"\n")
            fileHandle.flush()
    except KeyboardInterrupt:
        fg.stop()
        fileHandle.close()

if __name__ == "__main__":
    main()
