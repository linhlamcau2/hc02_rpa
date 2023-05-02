sudo apt install libmosquittopp-dev
sudo apt install libsqlite3-dev
sudo apt-get install libcurl4-openssl-dev

Install libubox, uci, ubus and procd on Ubuntu
Posted on 2020-02-16 by ClockworkBird
Requirements:

sudo apt install cmake
sudo apt install lua5.1
sudo apt install liblua5.1-0-dev
sudo apt install libjson-c-dev

- libubox
git clone git://git.openwrt.org/project/libubox.git
cd libubox
mkdir build
cd build
cmake ..
make
sudo make install
cd ../..

- ubus
git clone git://git.openwrt.org/project/ubus.git
cd ubus
mkdir build
cd build
cmake ..
make
sudo make install
cd ../..

- uci
git clone git://git.openwrt.org/project/uci.git
cd uci
mkdir build
cd build
cmake ..
make
sudo make install
cd ../..

- procd
git clone git://git.openwrt.org/project/procd.git
cd procd
mkdir build
cd build
cmake ..
make
sudo make install
cd ../..

- ldconfig
sudo ldconfig /usr/local/lib
