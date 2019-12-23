#include <vector>
#include <random>
#include <list>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/foreach.hpp>
#include <thread>
#include <magic.h>
#include <gstreamermm.h>

#include "utility.h"



using namespace std;
using namespace Gst;
using Glib::RefPtr;

namespace utility
{


void init(std::string program_name)
{
  proc_name = program_name;
  //create joystick config if first time run
}


size_t utf8_leng(const std::string& s)
{
  unsigned char c;
  size_t i, ix, q;
  for (q=0, i=0, ix=s.length(); i < ix; i++, q++)
    {
      c = (unsigned char) s[i];
      if (c>=0 && c<=128) i+=0;
      else if ((c & 0xE0) == 0xC0) i+=1;
      else if ((c & 0xF0) == 0xE0) i+=2;
      else if ((c & 0XF8) == 0xF0) i+=3;
      else return 0;
    }
  return q;
}


std::string mime_info(const string fn)
{
  magic_t magt = magic_open(MAGIC_CONTINUE | MAGIC_ERROR | MAGIC_MIME);
  magic_load(magt, NULL);
  string s =  (string) magic_file(magt, fn.c_str());
  magic_close(magt);
  return s;
}

bool starts_with(string const h, string const n)
{
  return boost::algorithm::starts_with(h, n);
}

bool ends_with(const string h, const string n)
{
  return boost::algorithm::ends_with(h, n);
}


bool is_playlist_file(const string fn)
{
  vector<string> pf = {(const string) ".m3u"};
  for (auto i: pf)
    {
      if (ends_with(fn, i)) return true;
    }
  return false;
}

void on_decoder_pad_added(const RefPtr<Pad> pad)
{
  Glib::ustring caps = pad->get_current_caps()->to_string().substr(0,5);
  RefPtr<Bin> parent = parent.cast_dynamic(pad->get_parent()->get_parent());
  if (!parent)
    {
      cerr << "cannot get parent bin" << endl;;
      return;
    }
  Glib::ustring factory_name;
  if (caps == "video" || caps == "audio")
    {
      factory_name = "autoaudiosink";
    }
  else
    {
      cerr << "unsupported media type " << pad->get_current_caps()->to_string() << endl;
    }
  RefPtr<Element> element = ElementFactory::create_element(factory_name);

}

}
