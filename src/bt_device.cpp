#include <algorithm>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include "bt_device.h"

using namespace std;


bt_device::bt_device()
{
  file_capable = false;
  audio_capable = false;
  interface_capable = false;
  capabilities.clear();
}



bt_device::~bt_device()
{
  //dtor
}

bool bt_device::is_capable(const cap c)
{
  return false;
}


