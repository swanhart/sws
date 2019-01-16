#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <asm/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <iostream>
#include <libudev.h>
#include <boost/thread.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/assign.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/foreach.hpp>

#include "config.h"

#define SUBSYSTEM "input"
#define MIN_PRESSED 100
#define LONG_PRESS 2000
#define MAX_PRESSED 10000

class joystick
{
public:
  struct js_value
  {
    __u32 time;
    __s16 value;
    __u8 type;
    __u8 button;
  };

  enum ACTION_TYPE {noop = 1, next, previous, stop, play, pause, rewind, fastforward, shuffle, favorite, add_to_playlist, volume_up, volume_down, get_suggestion, download, menu, say, playlist_up, playlist_down, same_artist, confirm, eof};
  static std::map<std::string, ACTION_TYPE> actions;
  static joystick* get_instance();
  typedef boost::signals2::signal<void ()> signal_t;
  static boost::signals2::connection listen(const signal_t::slot_type &subscriber);
  const static ACTION_TYPE get_event();
  static void load_config();
  static void save_config();
  int get_config_key_from_action(const ACTION_TYPE value);
  ACTION_TYPE get_config_action(const int key);
  void set_action_by_id(const int key, const ACTION_TYPE value);
  void set_action_by_action(const ACTION_TYPE value, const int key);
  virtual ~joystick();


protected:

private:
  joystick();
  static joystick *m_joystick;
  static void monitor_udev_thread(struct udev* udev);
  static void js_plug(struct udev_device* dev);
  static void process_udev(struct udev_device* dev);
  static int fd_joystick;
  static signal_t m_signal;
  static ACTION_TYPE joystick_event;
  static std::map<int, ACTION_TYPE> settings;
  static void set_event(ACTION_TYPE t);
  static void create_map();
  static void monitor();
  static boost::thread m_thread;
  static boost::thread m_udev_thread;
  static std::string default_config;
  static bool is_monitoring;
  static void reconnect();
  static void get_hwids_by_path(std::string path, unsigned long *vend_id, unsigned long *prod_id);
  static unsigned long get_product_id();
  static unsigned long vendor_id;
  static unsigned long product_id;
  static std::string dev_path;
  static void save_config(std::string hwid);
  static std::string enum_to_string(ACTION_TYPE a);
  static std::string default_joystick_config;
  static std::string joystick_config_file;
};

#endif // JOYSTICK_H
