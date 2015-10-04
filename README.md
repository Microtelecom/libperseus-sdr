#Perseus on Linux/Unix: libperseus-sdr

0. The library was originally written by Nico Palermo, Microtelecom's founder and owner, 
I am only the maintainer, as passionate SDR user and OM.
I am not working for Microtelecom and I never did so.

1. The libperseus-sdr library is now suitable for both Linux/Unix systems and Windows. 
Please note though, that Microtelecom produce and supports a Winbdows only SDRDK, that one can use
accepting the  License Agreement (see Microtelecom site, http://microtelecom.it/sdrdk/sdrdk.php).
In any case, the libperseus-sdr is not suitable in order to run original Perseus SDR application 
from MicroTelecom under Linux (neither using WINE). 
By the way, I don't know whether just *trying* to do that, one breaks the Microtelecom's licence, so be careful.

2. For which use is libperseus-sdr (very) good ?
It allow to get control the Perseus hardware under Linux and, in general, on any recent Unix, BSDs, OSX and Windows 
where libusb1.0 is available.

3. There are a few SDR software available on Linux: if one wants to use Perseus as a communication receiver, 
I strongly suggest to use Linrad, maybe at first you find it a little bit steep, 
but it is worth the effort, believe me (http://www.sm5bsz.com/linuxdsp/usage/newco/newcomer.htm); 
in case you need to access remotely and/or share Perseus over the Internet, 
use ghspdr3-alex (http://napan.ca/ghpsdr3/index.php/Main_Page).

4. for everything that concerns bugs and request of enhancements about libperseus-sdr, 
please open an issue on Github.
In case of technical questions concerning ghpsdr3-alex/Linrad software, 
there are specific mailing lists on Google Groups.

5. libperseus-sdr is used as base for gr-microtelecom GNU Radio, see https://github.com/amontefusco/gr-microtelecom.
In order to build GNU Radio and gr-microtelecom, it is strongly advisable to use PyBOMBS 
(http://gnuradio.org/redmine/projects/pybombs/wiki) 

6. for build instructions, please read the README and READM.Windows files.

