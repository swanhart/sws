#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <string>
#include <list>
#include <vector>
#include <thread>
#include <random>
#include <experimental/filesystem>

#include "config.h"
#include "db_queue.h"
#include "db.h"
#include "track.h"

using namespace std;

class playlist
{
public:
  template < typename T > void shuffle(std::list<T>& lst )
{
  vector<reference_wrapper<const T> > vec(lst.begin(), lst.end());
  shuffle(vec.begin(), vec.end(), mt19937{random_device()()});
  list<T> shuffled_list{vec.begin(), vec.end()};
  lst.swap(shuffled_list);
}

  playlist();
  playlist(std::string playlist_name);
  playlist(experimental::filesystem::v1::__cxx11::path path);
  playlist(std::string playlist_name, std::list<string> track_list);
  virtual ~playlist();
  playlist(const playlist& other);
  playlist& operator=(const playlist& other);
  std::list<string> tracks;
  std::string name;
  std::string source;
  void insert_after(unsigned int curr, list<string> paths, bool save = false);
  int current;
  void export_playlist();
  static bool is_playlist_file(const std::string filename);
  const string json();
  const string json_doc();
  const int index_of(const string path);
  const string value_of(const unsigned int i);

protected:

private:
  config *conf;
  db_queue *db_q;
  db *_db;

};
#endif // PLAYLIST_H
