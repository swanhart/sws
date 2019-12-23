#ifndef TRACK_H
#define TRACK_H

#include <string>
#include <list>
#include "db_queue.h"
#include "db.h"
#include "md5.h"

class track
{
public:
  track();
  track(const std::string, bool details = false);
  track(db::song s);
  static track get_detail(string path);
  virtual ~track();
  std::string path;
  std::string title;
  unsigned char track_no;
  std::string album;
  unsigned short int year;
  std::string artist;
  bool favorite;
  float rating;
  double duration;
  std::string spotify_id;
  std::string md5;
  std::string get_lyrics();
  void set_lyrics(std::string);
  std::list<std::string> influenced_by;
  std::list<std::string> influenced;
  std::string mime_type;
  static std::string file_hash(const std::string);
  static bool is_audio_file(const std::string);
  static std::string mime_info(const std::string);
  db::song to_song();
  void get_tags();
  const string json();
  const string json_doc();
protected:

private:
  db_queue *db_q;
  db *_db;
};

#endif // TRACK_H
