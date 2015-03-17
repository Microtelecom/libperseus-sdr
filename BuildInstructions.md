# Introduction #

If you want just compile the package, and are not interested to contribute, install the prerequisite packages and build from sources as follows:

```
sudo apt-get install wget gcc g++ make libusb-1.0-0-dev
wget http://www.montefusco.com/perseus/libperseus_sdr-0.7.46.tar.gz
tar -zxvf libperseus_sdr-0.7.46.tar.gz
cd libperseus_sdr-0.7.46/
./configure && make
./perseustest -s 1600000
```

If instead do you want build as package maintainer, please do refer to the [README](http://code.google.com/p/libperseus-sdr/source/browse/trunk/README) file in trunk for detailed build instructions.