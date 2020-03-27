[![release](https://github-basic-badges.herokuapp.com/release/Microtelecom/libperseus-sdr.svg)](https://github.com/Microtelecom/libperseus-sdr/releases/latest)

# Perseus on Linux/Unix: libperseus-sdr

## Build instructions are below, for more detailed instructions, please read the README and README.Windows files.

1. This library was originally written by Nico Palermo, Microtelecom's founder and owner.

2. Albeit the libperseus-sdr library was originally targeted to Linux, it is now suitable for both Linux/Unix systems and Windows.
Please note though, that Microtelecom produces and supports a Windows only SDRDK, that one can use
accepting the License Agreement (see Microtelecom site, http://microtelecom.it/sdrdk/sdrdk.php).
In any case, the library described here (libperseus-sdr) is not suitable to run the original Perseus SDR application from Microtelecom under Linux (neither using WINE).
By the way, I don't know whether just *trying* to do that, one breaks the Microtelecom's licence, so be careful.

3. For which use is libperseus-sdr (very) good ?
It allows us to get control the Perseus hardware under Linux and on any recent Unix, BSDs, OSX, Windows and, in general, on any POSIX compliant system where libusb1.0 and pthreads are available.

4. There are a few SDR software available on Linux: if one wants to use Perseus as a communication receiver,
I strongly suggest to use Linrad, maybe at first you find it a little bit steep,
but it is worth the effort, believe me (http://www.sm5bsz.com/linuxdsp/usage/newco/newcomer.htm);
in case you need to access remotely and/or share Perseus over the Internet,
use OpenWebRx (https://github.com/jketterl/openwebrx).
For other usages, GNU Radio is the preferred choice.

5. For everything that concerns bugs and request of enhancements about libperseus-sdr,
please open an issue on Github.
In case of technical questions concerning ghpsdr3-alex and Linrad software,
there are specific mailing lists on Google Groups.

6. libperseus-sdr is used as base for gr-microtelecom GNU Radio, see https://github.com/amontefusco/gr-microtelecom.
In order to build GNU Radio and gr-microtelecom, it is strongly advisable to use PyBOMBS
(http://gnuradio.org/redmine/projects/pybombs/wiki)

7. For an example on how to use this library on Windows, please see my ExtIO module in https://github.com/amontefusco/extio-iw0hdv

8. On my own web site there is a page where updates of libpersues-sdr integrations are posted. See https://www.montefusco.com/perseus/

9. I am only the maintainer, as passionate SDR user and OM.
I am not working for Microtelecom and I never did so.



## libperseus-sdr: how to build on Ubuntu

Below you find the instructions for copy, compile, install, test; copy them verbatim in a shell (tested on U14.04).

```
sudo apt-get install libusb-1.0-0-dev
cd /tmp
wget https://github.com/Microtelecom/libperseus-sdr/releases/download/v0.8.2/libperseus_sdr-0.8.2.tar.gz
tar -zxvf libperseus_sdr-0.8.2.tar.gz
cd libperseus_sdr-0.8.2/
./configure
make
sudo make install
sudo ldconfig
perseustest
```

If you prefer to run it without a full installation

```
./examples/perseustest
```

In either case, the output should be as follows (supposing the hardware for now is detached):

```
Revision: 0.8.2
SAMPLE RATE: 95000
NBUF: 6 BUF SIZE: 1024 TOTAL BUFFER LENGTH: 6144
perseus: perseus_init()
perseus: Executable: perseustest
perseus: path: [] name: []
perseus: Found device with VID/PID 8087:8000 on BUS1 ADDR2
perseus: Found device with VID/PID 1D6B:0002 on BUS1 ADDR1
perseus: Found device with VID/PID 1D6B:0003 on BUS3 ADDR1
perseus: Found device with VID/PID 04F2:B39A on BUS2 ADDR4
perseus: Found device with VID/PID 046D:C52F on BUS2 ADDR2
perseus: Found device with VID/PID 1D6B:0002 on BUS2 ADDR1
0 Perseus receivers found
No Perseus receivers detected
perseus: perseus_exit(): poll_libusb_thread_flag=0
```

Once you achieve the above output, connect the hardware and restart the program: now it should acquire samples form the radio (don't expect to hear any audio though, it is just storing samples in a file named ```perseusdata```).
If it is the first time you are using liberseus-sdr on your system, a logout (not reboot) may be needed in order to get the USB configurations active.

