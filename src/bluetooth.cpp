#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <string>
#include <boost/filesystem.hpp>
#include <fstream>
#include <regex>
#include <glib/gprintf.h>

#include "utility.h"
extern "C" {
  #include "bluez_util.h" // for uuid to string
}
#include "bluetooth.h"


using namespace std;
namespace fs = boost::filesystem;

bluetooth* bluetooth::m_instance = NULL;

bluetooth::bluetooth()
{
  read();
}

bluetooth::~bluetooth()
{
  //dtor
}

bluetooth* bluetooth::get_instance()
{
  return (!m_instance) ? m_instance = new bluetooth : m_instance;
}

void bluetooth::scan()
{

  /*unsigned int ptype      = HCI_DM1 | HCI_DM3 | HCI_DM5 | HCI_DH1 | HCI_DH3 | HCI_DH5;
  int adaptor_id = 0;
  adaptor_id = hci_get_route(NULL);
  if (adaptor_id < 0)
  {
    //Printouterrormessage
    printf("Error getting adaptor id");
  }
  printf("Device: %d\n", adaptor_id);

  int len = 8;
  int max_rsp = 255;
  int flags = IREQ_CACHE_FLUSH;
  inquiry_info* li = (inquiry_info*)malloc(max_rsp * sizeof(inquiry_info));
  int num_rsp = hci_inquiry(adaptor_id, len, max_rsp, NULL, &li, flags);
  if (num_rsp < 0 )
  {
    //error out
    printf("Cannot inquiry\n");
    perror("hci_inquiry");
  }
  printf ("Found %d device\n", num_rsp);

  int socket_id = hci_open_dev(adaptor_id);
  if (socket_id < 0)
  {
    printf("Cannot open socket\n");
  }
  int i;
  for (i = 0; i < num_rsp; i++)
  {
    printf("%d\n", i);
    char addr[20], name[300];
    inquiry_info* device = li + i;
    ba2str(&(device->bdaddr), addr);
    memset(name, 0, sizeof(name));
    if (hci_read_remote_name(socket_id, &(li+i)->bdaddr, sizeof(name), name, 0) < 0)
    {
      strcpy(name, "[Unknown]");
    }
    printf("%s %s\n", addr, name);
    bt_device bt;
    bt.id = addr;
    bt.name = name;
    struct hci_conn_info_req *cr;
    cr = (struct hci_conn_info_req*) malloc(sizeof(*cr) + sizeof(struct hci_conn_info));
    if (!cr)
    {
      perror("Cannot allocat hci conn info");
    }
    cr->type = ACL_LINK;

    int dd = hci_open_dev(i);
    if (dd < 0)
    {
      perror("Cannot get conn info");
     }
      bdaddr_t     bdaddr;
      str2ba(addr, &bdaddr);
     uint16_t handle;
    int err = hci_create_connection(dd, &bdaddr, htobs(ptype), 0, 0, &handle, 0);
    if (err < 0)
    {
      perror("CreateConnection");
    }
    err = ioctl(dd, HCIGETCONNINFO, (unsigned long) cr);
    if (err < 0)
    {
      perror("ioctl");
    }
    sleep(10);
    err = hci_authenticate_link(dd, handle, 10000);
    if (err < 0)
    {
      perror("Authentication:");
    }


    err = hci_encrypt_link(dd, handle, 1, 0);
    if (err < 0)
    {
      perror("Ecryption");
    }


  }
  delete(li);
  close(socket_id);*/
}

string bluetooth::uuid_to_str(string uuid)
{
  const char* u = uuid.c_str();
  const char* c = bt_uuidstr_to_str(u);
  string s(c);
  c = NULL;
  u =NULL;
  return s;
}

void bluetooth::read()
{
  GError *err = NULL;
  GDBusConnection *conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &err);
   GDBusProxy *proxy = g_dbus_proxy_new_sync(conn, G_DBUS_PROXY_FLAGS_NONE, NULL, "org.bluez", "/", "org.freedesktop.DBus.ObjectManager", NULL, &err);
   g_autoptr(GVariant) result;
   g_autoptr(GVariant) bluez;
   g_autoptr(GVariantIter) iter1 = NULL, iter2 = NULL, iter3 = NULL;
   const gchar *name;
   result = g_dbus_proxy_call_sync(proxy, "GetManagedObjects", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, &err);
   g_assert_no_error(err);

   g_variant_get(result, "(a{oa{sa{sv}}})", &iter1);

    while (g_variant_iter_next(iter1, "{&o@a{sa{sv}}}", &name, &bluez))
      {
        string s(name);
        regex e ("^/org/bluez/hci[0-9]/dev_");
        smatch m;
        if (regex_search(s, m, e))
        {
          cout << name << " Adding " << endl;
          bt_device bt_dev;
          const gchar *prop_name;
          g_autoptr(GVariant) properties;
          g_variant_get(bluez, "a{sa{sv}}", &iter2);
          while (g_variant_iter_next(iter2, "{&s@a{sv}}", &prop_name, &properties))
          {
            //g_message("%s", prop_name);
            if (g_str_equal(prop_name, "org.bluez.Device1"))
            {
              g_variant_get(properties, "a{sv}", &iter3);
              const char *key;

              g_autoptr(GVariant) val;
              while (g_variant_iter_next(iter3, "{&sv}", &key, &val))
              {
                const gchar *type = g_variant_get_type_string(val);
                if (*type == 's')
                {
                    if (key == (string) "Address")
                    {
                      bt_dev.id = g_variant_get_string(val, NULL);
                      cout << "Id: " << bt_dev.id;
                    }
                    else if (key == (string) "Name")
                    {
                      bt_dev.name = g_variant_get_string(val, NULL);
                      cout << "Name: " << bt_dev.name;
                    }
                    //g_message("%s: %s", key, g_variant_get_string(val, NULL));
                }
                else if (*type == 'u')
                {
                  if (key == (string) "Class")
                  {
                    bt_dev.bclass = g_variant_get_uint32(val);
                  }
                }
                else if (*type == 'b')
                {
                  bool b = g_variant_get_boolean(val);
                  if (key == (string) "Paired")
                  {
                    bt_dev.paired = b;
                  }
                  if (key == (string) "Connected")
                  {
                    bt_dev.connected = b;
                  }
                  if (key == (string) "Trusted")
                  {
                    bt_dev.trusted = b;
                  }
                }
                else if (g_str_equal(key, "UUIDs"))
                {
                  g_autoptr(GVariantIter) it;
                  const gchar *uuidchar;
                  g_variant_get(val, "as", &it);
                  while (g_variant_iter_next(it, "&s", &uuidchar))
                    {
                      //g_message("%s: %s", uuidchar, bt_uuidstr_to_str(uuidchar));
                      string s(uuidchar);
                      cout << s << ": " << uuid_to_str(s) << endl;
                    }

                }
                else
                {
                }
                type = NULL;
              }
              prop_name = NULL;
            }
          }
          bt_devices.push_back(bt_dev);
        }
      }
  conn = NULL;
  proxy = NULL;
  name = NULL;
}

