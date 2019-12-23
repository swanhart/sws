#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/thread.hpp>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/flacfile.h>
#include <taglib/flacpicture.h>
#include <magic.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "utility.h"
#include "track.h"

using namespace std;
namespace fs = boost::filesystem;

track::track()
{
  _db = db::get_instance();
  path = "";
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
  spotify_id = "";
  mime_type = "";
  duration = 0;
}

track::track(const string s, bool details)
{
  details = true;
  _db = db::get_instance();
  db_q = db_queue::get_instance();

  path = (string) s;
  fs::path p(path);
  if (!fs::exists(p))
  {
    cout << "TrackError: File not exist " << path << endl;
    title = "";
    artist = "";
    track_no = 0;
    album = "";
    year = 0;
    artist = "";
    md5 = "";
    favorite = false;
    spotify_id = "";
    rating = 0;
    influenced.clear();
    influenced_by.clear();
    mime_type = "";
    duration = 0;
  }
  else
  {
    db::song s = _db->select_track(path);
    if (!s.found)
    {
      _db->insert_path(path);
    }
    string file_md5 = file_hash(path);
    mime_type = mime_info(path);
    if (!s.found)
    {
      _db->insert_path(path);
    }
    else if (details && s.md5 != file_md5 && is_audio_file(path))
    {
      get_tags();
      _db->insert_track(s);
      db_q->push_duration(&s);
     // db_q->pending.push(p);
    }
    else if (details && duration <= 0)
    {
      db_q->push_duration(&s);
    }
    else
    {
      title = s.title;
      track_no = s.track_no;
      album = s.album;
      year = s.year;
      artist = s.artist;
      spotify_id = s.spotify_id;
      duration = s.duration;
      md5 = s.md5;
    }
  }
}

track::track(db::song s)
{
  _db = db::get_instance();
  path = s.path;
  title = s.title;
  track_no = s.track_no;
  album = s.album;
  year = s.year;
  artist = s.artist;
  md5 = s.md5;
  spotify_id = s.spotify_id;
  favorite = s.favorite;
  rating = s.rating;
  mime_type = mime_info(path);
  duration = s.duration;
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
  spotify_id = "";
  favorite = false;
  rating = 0;
  influenced.clear();
  influenced_by.clear();
  mime_type = "";
}


db::song track::to_song()
{
  db::song s;
  s.path = path;
  s.title = title;
  s.track_no = track_no;
  s.album = album;
  s.year = year;
  s.artist = artist;
  s.md5 = md5;
  s.duration = duration;
  s.favorite = favorite;
  s.spotify_id = spotify_id;
  s.rating = rating;
  return s;
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
    if (boost::algorithm::starts_with(m, i))
    {
      return true;
    }
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

void track::get_tags()
{
  TagLib::FileRef f(path.c_str());
  if (!f.isNull() && f.tag())
  {
    TagLib::Tag *tags = f.tag();
    title = tags->title().toCString(true);
    track_no = (unsigned int) tags->track();
    album = tags->album().toCString(true);
    year = tags->year();
    artist = tags->artist().toCString(true);
  }
}

track track::get_detail(string path)
{
  return track(path, true);
}

const string track::json()
{
  using namespace rapidjson;
  StringBuffer sb;
  Writer<StringBuffer> w(sb);
  w.StartObject();
  w.Key("Id"); w.String(path.c_str());
  w.Key("Path"); w.String(path.c_str());
  w.Key("Title"); w.String(title.c_str());
  w.Key("TrackNo"); w.Uint(track_no);
  w.Key("Album"); w.String(album.c_str());
  w.Key("Year"); w.Uint(year);
  w.Key("Artist"); w.String(artist.c_str());
  w.Key("Favorite"); w.Bool(favorite);
  w.Key("Rating"); w.Double(rating);
  w.Key("Duration"); w.Double(duration);
  w.Key("SporitfyId"); w.String(spotify_id);
  w.EndObject();
  return string(sb.GetString());
}

const string track::json_doc()
{
  string str = (string) "{\"Track\":" + json() + "}";
  return str;
}

//char * track::image()
//{
  //flac
  /*
  if (mime_type == "flac")
  {
    TagLib::FLAC::File file = File(path);
    const TagLib::List<TagLib::FLAC::Picture>& picList = file.pictureList();
    TagLib::FLAC::Picture* pic = picList[0];
    return pic.data();
  }
  else if (mime_type == "asf")
  {
    TagLib::ASF::File file = File(path);
    const TagLib::ASF::AttributeListMap& attrListMap = file->tag()->attributeListMap();
    const TagLib::ASF::AttributeList& attrList = attrListMap["WM/Picture"];
    TagLib::ASF::Picture pic = attrList[0].toPicture();
  }*/
//}
