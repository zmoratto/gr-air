gr-air
======

This GNU Radio component implements common aircraft datalink protocols
such as Mode Select, ADS-B, and TIS-B. ACARS will be done in the future.

Current status (18/10/09)
-------------------------

This has been partially updated to use GNU Radio 3.2. Mode S decoding and 
dumping does work (USRP1 with DBSRX), although only with such a low gain 
that packets are almost invisible on a scope display.

Files
-----

    usrp_oscope_ms.py	    Display samples, reference level, and sample attributes on a oscilliscope. 
                            (doesn't work with GNU Radio 3.2 yet)
    usrp_mode_s_logfile.py  Dumps raw Mode S frames to a log file.
    ppm_demod.py		    Mode S Decoding Block
