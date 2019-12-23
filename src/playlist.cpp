#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <limits.h>
#include <cstdlib>
#include <fstream>
#include <locale>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/foreach.hpp>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>


#include "utility.h"
#include "playlist.h"

using namespace std;


playlist::playlist()
{
  conf = config::get_instance();
  _db = db::get_instance();
  db_q = db_queue::get_instance();
}

playlist::playlist(std::string playlist_name)
{
  conf = config::get_instance();
  _db = db::get_instance();
  db_q = db_queue::get_instance();
  name = playlist_name;
  _db->add_to_playlist(playlist_name, list<string>());
}

playlist::playlist(const experimental::filesystem::v1::__cxx11::path playlist_file)
{
  conf = config::get_instance();
  _db = db::get_instance();
  db_q = db_queue::get_instance();
  locale loc;
  list<string> paths;
  if (experimental::filesystem::exists(playlist_file) && is_playlist_file((const string) playlist_file.string()))
  {
    string extension = boost::algorithm::to_lower_copy(playlist_file.extension().string(), loc);
    if (extension == ".m3u" || extension == ".m3u8")
    {
      name = (const string) playlist_file.stem().string();
      ifstream ifs(playlist_file.string());
      string line;
      if (ifs.is_open())
      {
        while (getline(ifs, line))
        {
          if (line[0] != '#' && experimental::filesystem::v1::exists(experimental::filesystem::v1::__cxx11::path(line)))
          {
            tracks.push_back(line);
          }
        }
      }
    ifs.close();
    }
    _db->add_to_playlist(name, tracks);
    _db->insert_playlist(name, playlist_file.string());
  }
  else
  {
    name = "";
    tracks.clear();
  }
}


playlist::~playlist()
{
  //dtor
}

playlist::playlist(const playlist& other)
{
  //copy ctor
  name = other.name;
  tracks = other.tracks;
  _db = db::get_instance();
}

playlist::playlist(std::string playlist_name, std::list<string> track_list)
{
  name = playlist_name;
  for (string s: track_list)
  {
    tracks.push_back(s);
  }
  _db = db::get_instance();
}

playlist& playlist::operator=(const playlist& rhs)
{
  if (this == &rhs) return *this; // handle self assignment
  //assignment operator
  this->name = rhs.name;
  this->current = rhs.current;
  this->source = rhs.source;
  this->tracks = rhs.tracks;
  return *this;
}


bool playlist::is_playlist_file(const string filename)
{
  experimental::filesystem::path p(filename);
  if (p.has_extension())
  {
    string ext = p.extension().string();
    transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){return tolower(c); });
    if (ext == ".m3u" || ext == ".m3u8")
    {
      return true;
    }
  }
  return false;
}

void playlist::export_playlist()
{
  ofstream fout(name + ".m3u");
 // fout.imbue(loc);
  fout << "#EXTM3U";
  for (track s: tracks)
  {
    db::song t = _db->select_track(s.path);
    if (t.found)
    {
      fout << "#EXTINF:-1,";
      if (t.artist.length() > 0 ) fout << t.artist << " - ";
      if (t.album.length() > 0) fout << t.album << " - ";
      if (t.title.length() > 0) fout << t.title;
      fout << endl;
    }
    fout << s.path << endl;
  }
  fout.close();
}

const string playlist::json()
{
  using namespace rapidjson;
  StringBuffer sb;
  Writer<StringBuffer> w(sb);
  w.StartObject();
  w.Key("Name"); w.String(name.c_str());
  w.Key("Source"); w.String(source.c_str());
  w.Key("Tracks");
  w.StartArray();
  for (string p : tracks)
  {
    w.String(p.c_str());
  }
  w.EndArray();
  w.EndObject();
  return (sb.GetString());
}

const int playlist::index_of(const string path)
{
  int i = 0;
  for (string p : tracks)
  {
    if (path == p) return i;
      i++;
  }
  return 0;
}

const string playlist::value_of(const unsigned int i)
{

  if (i >= tracks.size())
    return *tracks.begin();
  list<string>::iterator iter = tracks.begin();
  advance(iter, i);
  return *iter;
}

void playlist::insert_after(unsigned int curr, list<string> paths, bool save)
{
  list<string>::iterator it = tracks.begin();
  if (curr > tracks.size())
    it = tracks.end();
  else
    advance(it, curr);
  for (list<string>::reverse_iterator rit = paths.rbegin(); rit != paths.rend(); ++rit)
  {
    tracks.emplace(it, *rit);
  }
  if (save)
  {
    _db->remove_playlist(name);
    _db->add_to_playlist(name, tracks);
  }

}
