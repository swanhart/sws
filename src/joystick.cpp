#include "joystick.h"


using namespace std;
std::string joystick::default_joystick_config = "{\"joysticks\":{\"default\":{\"noop\":-1,\"next\":204,\"previous\":304,\"stop\":8,\"play\":9,\"pause\":109,\"rewind\":0,\"fastforward\":2,\"shuffle\":7,\"favorite\":1,\"add_to_playlist\":3,\"volume_up\":301,\"volume_down\":201,\"get_suggestion\":103,\"download\":101,\"menu\":12,\"say\":5,\"confirm\":112,\"playlist_up\":305,\"playlist_down\":205,\"same_artist\":6,\"eof\":-1}}}";
joystick *joystick::m_joystick = NULL;
string joystick::dev_path = "/dev/input/js0";
int joystick::fd_joystick = open("/dev/input/js0", O_RDONLY| O_NONBLOCK);
joystick::signal_t joystick::m_signal = joystick::signal_t();
joystick::ACTION_TYPE joystick::joystick_event = joystick::ACTION_TYPE::noop;
//vector<joystick::ACTION_TYPE> joystick::settings(1024, joystick::noop);
std::map<int, joystick::ACTION_TYPE> joystick::settings;
bool joystick::is_monitoring = true;
boost::thread joystick::m_thread = boost::thread(&joystick::monitor);
boost::thread joystick::m_udev_thread = boost::thread(boost::bind(&joystick::monitor_udev_thread, udev_new()));
unsigned long joystick::vendor_id = 0;
unsigned long joystick::product_id = 0;
std::map<std::string, joystick::ACTION_TYPE> joystick::actions = boost::assign::map_list_of("noop", noop)("next", next)("previous", previous)("stop", stop)("play", play)\
    ("pause", pause)("rewind", rewind)("fastforward", fastforward)("shuffle", shuffle)("favorite", favorite)("add_to_playlist", add_to_playlist)("volume_up", volume_up)\
    ("volume_down", volume_down)("get_suggestion", get_suggestion)("download", download)("menu", menu)("say", say)("playlist_up", playlist_up)("playlist_down", playlist_down)\
    ("same_artist", same_artist)("confirm", confirm)("eof", eof);

joystick::joystick()
{

}

joystick::~joystick()
{

}

joystick *joystick::get_instance()
{
  return (!m_joystick) ? m_joystick = new joystick : m_joystick;
}

void joystick::reconnect()
{
  if (is_monitoring)
  {
    is_monitoring = false;
    cout << "ending monitor thread" << endl;
    m_thread.join();
    is_monitoring = true;
    m_thread = boost::thread(&monitor);
  }
}

void joystick::js_plug(struct udev_device* dev)
{
  const char* action = udev_device_get_action(dev);
  string s(udev_device_get_devnode(dev));
  if (strcmp(action, "add") == 0 && s.substr(0,13) == "/dev/input/js") //c++20 starts_with()
  {
    dev_path = s;
    close(fd_joystick);
    fd_joystick = open(dev_path.c_str(), O_RDONLY | O_NONBLOCK);
    cout << dev_path <<endl;
    reconnect();
    cout << "DevType: " << udev_device_get_devtype(dev) << endl;
    struct udev_device *parent = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");
    unsigned long ven = strtoul(udev_device_get_sysattr_value(parent, "idVendor"), NULL, 16 );
    unsigned long pro = strtoul(udev_device_get_sysattr_value(parent, "idProduct"), NULL, 16);
    cout << ven << " " << pro << endl;
    if ( ven != 0 && pro != 0)
    {
      if (ven != vendor_id && pro != product_id)
      {
        vendor_id = ven;
        product_id = pro;
        load_config();
      }
    }
  }
}

void joystick::process_udev(struct udev_device* dev)
{
  if (dev)
  {
    if (udev_device_get_devnode(dev))
    {
      js_plug(dev);
    }
    udev_device_unref(dev);
  }
}


void joystick::monitor_udev_thread(udev* udev)
{
  struct udev_monitor* mon = udev_monitor_new_from_netlink(udev, "udev");
  udev_monitor_filter_add_match_subsystem_devtype(mon, SUBSYSTEM, NULL);
  udev_monitor_enable_receiving(mon);
  int fd = udev_monitor_get_fd(mon);
  while (1)
  {
      fd_set fds;
      FD_ZERO(&fds);
      FD_SET(fd, &fds);
      int ret = select(fd+1, &fds, NULL, NULL, NULL);
      if (ret <= 0)
          break;
      if (FD_ISSET(fd, &fds)) {
          struct udev_device* dev = udev_monitor_receive_device(mon);
          process_udev(dev);
      }
  }
}

boost::signals2::connection joystick::listen(const joystick::signal_t::slot_type &subscriber)
{
  load_config();
  return m_signal.connect(subscriber);
}

const joystick::ACTION_TYPE joystick::get_event()
{
  return joystick_event;
}

void joystick::set_event(ACTION_TYPE t)
{
  joystick_event = t;
  m_signal();
}

void joystick::monitor()
{
  int bytes = 0;
  struct js_value j;
  bool p[24] = { 0 };
  __u32 time_pressed[24] = { 0 };
  while (is_monitoring)
  {
    bytes = read(fd_joystick, &j, sizeof(j));
    if (bytes > 0)
    {
      if (j.type == 1) // button press
      {
        if (j.value == 1)
        {
          p[j.button] = true;
          time_pressed[j.button] = j.time;
        }
        else if (j.value == 0 && p[j.button] == true)
        {
          p[j.button] = false;
          if (j.time - time_pressed[j.button] > MAX_PRESSED)
            cout << "TOOLONG " << endl;
          else if (j.time - time_pressed[j.button] > LONG_PRESS)
            set_event(settings.at(j.button + 100));
          else if (j.time - time_pressed[j.button] > MIN_PRESSED)
            set_event(settings.at(j.button));
        }
      }
      else if (j.type == 2) //Axis Change
      {
        if (j.value == 32767 || j.value == -32767)
        {
          if (j.value > 0)
            set_event(settings.at(j.button + 200));
          else
            set_event(settings.at(j.button + 300));
        }
      }
    }
  }
}

void joystick::get_hwids_by_path(std::string path, unsigned long *vend_id, unsigned long *prod_id)
{
  *vend_id = 0;
  *prod_id = 0;
  struct udev *udev = udev_new();
  struct udev_enumerate *enumerate = udev_enumerate_new(udev);
  udev_enumerate_add_match_subsystem(enumerate, SUBSYSTEM);
  udev_enumerate_scan_devices(enumerate);
  struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
  struct udev_list_entry *entry;

  udev_list_entry_foreach(entry, devices)
  {
    const char* p = udev_list_entry_get_name(entry);
    struct udev_device *dev = udev_device_new_from_syspath(udev, p);
    const char *c = udev_device_get_devnode(dev);
    if (c != NULL && strcmp(c, dev_path.c_str()) == 0)
    {
      cout << p << "\t" << c << endl;
      cout << "DevType: " << udev_device_get_devtype(udev_device_get_parent(dev)) << endl;
      struct udev_device *parent = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");
      unsigned long ven = strtoul(udev_device_get_sysattr_value(parent, "idVendor"), NULL, 16 );
      unsigned long pro = strtoul(udev_device_get_sysattr_value(parent, "idProduct"), NULL, 16);
      cout << ven << " " << pro << endl;
      if (ven > 0 && pro > 0)
      {
        *vend_id = ven;
        *prod_id = pro;
      }
    }
  }
}

void joystick::load_config()
{
  cout << "Looading joystick config" << endl;
  config *c = config::get_instance();
  boost::filesystem::path p(c->root + (string) "joystick.conf");
  using namespace boost::property_tree;
  settings.clear();
  ptree parent, pt, js;
  if (vendor_id == 0 || product_id == 0)
  {
    get_hwids_by_path(dev_path, &vendor_id, &product_id);
  }
  std::stringstream ss;
  ss << std::setfill('0') << std::setw(4) << std::hex << vendor_id << std::setfill('0') << std::setw(4) << std::hex << product_id;
  read_json(p.string(), parent);
  pt = parent.get_child("joysticks");
  ptree::const_assoc_iterator itp = pt.find(ss.str());
  if (itp == pt.not_found())
  {
    js = pt.get_child("default");
  }
  else
  {
    js = pt.get_child(ss.str());
  }
  cout << js.data() << endl;
  for (auto it: js)
  {
    cout << it.first << ": " << it.second.get_value<int>() << endl;
    //settings.at(it->second.get_value<int>()) = actions.at(it->first);
    if (actions.find(it.first) != actions.end())
    {
      int i = it.second.get_value<int>();
      settings.insert(pair<int, ACTION_TYPE>(i, actions.at(it.first)));
    }
    else
      cerr << "Setting enum not in map" << endl;
    //TODO Check for out of bounds
  }
}


void joystick::save_config()
{
  using namespace boost::property_tree;
  config *c = config::get_instance();
  const boost::filesystem::path p(c->root + (string) "joystick.conf");
  ifstream ifs;
  ofstream ofs;
  unsigned long vid = 0, pid = 0;
  get_hwids_by_path(dev_path, &vid, &pid);
  std::string hwid = "";
  if (vid == 0  || pid == 0)
  {
    hwid = "default";
  }
  else
  {
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(4) << std::hex << vid << std::setfill('0') << std::setw(4) << std::hex << pid;
    hwid = ss.str();
  }

  ptree newtree;
  for(map<int, ACTION_TYPE>::iterator it = settings.begin(); it != settings.end(); it++)
  {
    cout << it->first << endl;
    newtree.put(enum_to_string(it->second), it->first);
  }
  ptree parent;
  read_json(p.string(), parent);
  cout << "Closed files.... " << endl;
  ptree js = parent.get_child("joysticks");
  ptree::const_assoc_iterator it = js.find(hwid);
  if (it == js.not_found())
  {
    js.add_child(hwid, newtree);
    parent.put_child("joysticks", js);
  }
  else
  {
    js.put_child(hwid, newtree);
    parent.put_child("joysticks", js);
  }
  if (hwid != "default")
  {
    js.put_child("default", newtree);
    parent.put_child("joysticks", js);
  }
  write_json(p.string(), parent);
}

std::string joystick::enum_to_string(ACTION_TYPE a)
{
  for (auto v: actions)
  {
    if (v.second == a)
    {
      return v.first;
    }
  }
  return "noop";
}
