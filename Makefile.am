bin_PROGRAMS = myapp
myapp_SOURCES = src/bluetooth.cpp src/console.cpp src/db_sqlite.cpp src/playlist.cpp src/util.c src/bluez_util.cdb.cpp src/joystick.cpp src/server.cpp src/utility.cpp src/bt_device.cpp src/db_mysql.cpp src/network.cpp src/spotify.cpp src/wsocket.cpp src/config.cpp src/db_queue.cpp src/player.cpp src/track.cpp
CFLAGS="-g -I include -02 -Wall -std=c++17 -fexceptions "`pkg-config --cflags gstreamermm-1.0` `pkg-config --cflags sqlite3` `pkg-config --cflags libudev` `pkg-config --cflags taglib` `pkg-config --cflags libcurl` `pkg-config --cflags jsoncpp` `pkg-config --cflags dbus-c++-1` `pkg-config --cflags glibmm-2.4` -DRAPIDJSON_HAS_STDSTRING"
AM_FLAGS=$CFLAGS

