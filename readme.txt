Install libubox, uci, ubus and procd on Ubuntu
Posted on 2020-02-16 by ClockworkBird
Requirements:

sudo apt install lua5.1
sudo apt install liblua5.1-0-dev
sudo apt install libjson-c-dev
libubox
$ git clone git://git.openwrt.org/project/libubox.git
$ cd libubox
$ mkdir build
$ cd build
$ cmake ..
$ make
$ sudo make install

ubus
$ git clone git://git.openwrt.org/project/ubus.git
$ cd ubus
$ mkdir build
$ cmake ..
$ make
$ sudo make install

uci
$ git clone git://git.openwrt.org/project/uci.git
$ cd uci
$ mkdir build
$ cmake ..
$ make
$ sudo make install

procd
$ git clone git://git.openwrt.org/project/procd.git
$ cd procd
$ mkdir build
$ cmake ..
$ make
$ sudo make install

ldconfig
$ sudo ldconfig /usr/local/lib
