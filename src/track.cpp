#include "track.h"

using namespace std;
namespace fs = boost::filesystem;

track::track()
{
  title = "";
  track_no = 0;
  album = "";
  year = 0;
  artist = "";
  md5 = "";
  favorite = false;
  rating = 0;
  influenced.clear();
  influenced_by.clear();
  mime_type = "";
  duration = 0;
}

track::track(const string s)
{
  path = s;
  fs::path p(path);
  if (!fs::exists(p))
    {
      title = "";
      track_no = 0;
      album = "";
      year = 0;
      artist = "";
      md5 = "";
      favorite = false;
      rating = 0;
      influenced.clear();
      influenced_by.clear();
      mime_type = "";
      duration = 0;
    }
  md5 = file_hash(path);
  if (is_audio_file(path))
    {
      duration = get_duration(path);
      TagLib::FileRef f(path.c_str());
      if (!f.isNull() && f.tag())
      {
        TagLib::Tag *tags = f.tag();
        mime_type = mime_info(path);
        title = tags->title().to8Bit();
        track_no = tags->track();
        album = tags->album().to8Bit();
        year = tags->year();
        artist = tags->artist().to8Bit();

      }
    }
}

track::~track()
{
  path = "";
  title = "";
  track_no = 0;
  album = "";
  year = 0;
  artist = "";
  duration = 0;
  md5 = "";
  favorite = false;
  rating = 0;
  influenced.clear();
  influenced_by.clear();
  mime_type = "";
}

track::track(const track& other)
{
  path = other.path;
  title = other.title;
  track_no = other.track_no;
  album = other.album;
  year = other.year;
  artist = other.artist;
  md5 = other.md5;
  duration = other.duration;
  favorite = other.favorite;
  rating = other.rating;
  influenced = other.influenced;
  influenced_by = other.influenced_by;
  mime_type = other.mime_type;
}

track& track::operator=(const track& rhs)
{
  if (this == &rhs) return *this; // handle self assignment
  //assignment operator
  return *this;
}

double track::get_duration(const string fn)
{
  gint64 len;
  int diff;
  string s;
  len = 0;
  diff = 0;
  Gst::Format fmt = Gst::FORMAT_TIME;
  Glib::RefPtr<Gst::PlayBin> playbin;
  //Glib::RefPtr<Gst::Pipeline> pipeline;
#ifndef GSTREAMERMM_DISABLE_DEPRECATED
  Glib::RefPtr<Gst::FileSrc> source;
#else
  Glib::RefPtr<Gst::Element> source;
#endif
  Gst::init();
#ifndef GSTREAMERMM_DISABLE_DEPRECATED
  playbin = Gst::PlayBin::create("playbin");
#else
  Glib::RefPtr<Gst::Element> playbin = Gst::ElementFactory::create_element("playbin");
#endif
  if (!playbin)
    {
      std::cerr << "The playbin could not be created." << std::endl;
      return 0;
    }

  if (!fn.find("://"))
    {
      s = (string) "file://" + fn;
    }

  playbin->set_property("uri", fn);
  playbin->set_state(Gst::STATE_PAUSED);
  auto start_time = chrono::high_resolution_clock::now();
  while (!playbin->query_duration(fmt, len) && diff < 3)
    {
      diff = chrono::duration_cast<chrono::seconds>(chrono::high_resolution_clock::now() - start_time).count();
    }
  playbin->set_state(Gst::STATE_NULL);
  playbin->unreference();
  return len/1000000000;

}

string track::file_hash(const string s)
{
  MD5 md5;
  return (string) md5.digestFile((char*) s.c_str());
}

bool track::is_audio_file(const string fn)
{
  vector<string> af = {(const string) "audio", (const string) "video"};
  const string m = mime_info(fn);
  for (auto i: af)
    {
      if (boost::algorithm::starts_with(m, i)) return true;
    }
  return false;
}

string track::mime_info(const string fn)
{
  magic_t magt = magic_open(MAGIC_CONTINUE | MAGIC_ERROR | MAGIC_MIME);
  magic_load(magt, NULL);
  string s =  (string) magic_file(magt, fn.c_str());
  magic_close(magt);
  return s;
}
