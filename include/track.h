#ifndef TRACK_H
#define TRACK_H

#include <string>
#include <list>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <gstreamermm.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <magic.h>

#include "md5.h"

class track
{
public:
  track();
  track(const std::string);
  virtual ~track();
  track(const track& other);
  track& operator=(const track& other);
  std::string path;
  std::string title;
  unsigned char track_no;
  std::string album;
  unsigned short int year;
  std::string artist;
  std::string md5;
  std::string get_lyrics();
  void set_lyrics(std::string);
  std::list<std::string> influenced_by;
  std::list<std::string> influenced;
  bool favorite;
  float rating;
  std::string mime_type;
  double duration;
  static double get_duration(const std::string);
  static std::string file_hash(const std::string);
  static bool is_audio_file(const std::string);
  static std::string mime_info(const std::string);

protected:

private:

};

#endif // TRACK_H
