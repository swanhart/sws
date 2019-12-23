#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <list>
#include <json/json.h>
#include <gio/gio.h>

#include "config.h"
#include "bt_device.h"


using namespace std;

class bluetooth
{
public:
  static bluetooth* get_instance();

  virtual ~bluetooth();
  list<bt_device> bt_devices;
  void scan();
  Json::Value json_bt_devices;

protected:

private:
  void read();
  bluetooth();
  static bluetooth* m_instance;
  string bt_config_file;
  string uuid_to_str(string uuid);
};

#endif // BLUETOOTH_H
