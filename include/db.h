#ifndef DB_H
#define DB_H

#include <fstream>
#include <string>
#include <list>
#include <queue>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <sqlite3.h>

#include "config.h"
#include "track.h"

class db
{
public:
  virtual ~db();
  static db* get_instance();
  static std::queue<std::function<void()>> pending;
  static boost::thread db_thread;
  static std::list<std::string> *genres();
  static std::list<std::string> *influences();
  static void get_track(const std::string*, track*);
  static void update_track(const track*);
  static void add_track(const std::string*);
  static void add_track(const track*);
  static void add_influnce(const std::string*, const std::string*);
  static void remove_influence(const std::string*, const std::string*);
  static void get_influence(const std::string*, std::list<std::string>*);
  static void get_influenced(const std::string*, std::list<std::string>*);
  static void remove_track(const std::string*);
  static void get_playlist(const std::string*, std::list<std::string>*);
  static void update_playlist(const std::string*, const std::list<std::string>*);
  static void add_to_playlist(const std::string*, const std::list<std::string>*);
  static void add_to_playlist(const std::string*, const std::string*);
  static void remove_from_playlist(const std::string*, const std::list<std::string>*);
  static void remove_from_playlist(const std::string*, const std::string*);
  static void add_playlist(const std::string*);
  static void get_favorites(const std::list<std::string>*);
  static void get_all(const std::list<std::string>*);

private:
  db();
  static db *db_instance;
  static bool is_alive;
  static sqlite3* sql;
  static std::list<std::string> *artist_list;
  unsigned long long get_duration(std::string);

};

#endif // DB_H
