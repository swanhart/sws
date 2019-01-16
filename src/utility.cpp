#include "utility.h"



using namespace std;
using namespace Gst;
using Glib::RefPtr;
namespace fs = boost::filesystem;

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

void populate()
{
  fs::path p("/media");
  boost::thread t(populate_thread, &p);
}

void populate_thread(fs::path* root_path)
{
  fs::path p = *root_path;
  fs::recursive_directory_iterator itr(p), ieod;
  BOOST_FOREACH(fs::path const &p, make_pair(itr, ieod))
  {
    if (fs::is_regular(p))
      {
        if (is_playlist_file(p.string()))
          {
            //db::db_action a;
            //a.id = 1;
            //a.fptr = db::add_playlist;
            //a.fptr(&p);
            //boost::thread t(a);
            //t.join();
            const string *s = &p.string();
            db::pending.push(bind(db::add_playlist, s));
          }
        else if (track::is_audio_file(p.string()))
          {
            db::add_track((string*) &p);
          }
      }
  }
}
template < typename T > void shuffle(std::list<T>& lst )
{
  vector<reference_wrapper<const T> > vec(lst.begin(), lst.end());
  shuffle(vec.begin(), vec.end(), mt19937{random_device()()});
  list<T> shuffled_list{vec.begin(), vec.end()};
  lst.swap(shuffled_list);
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

