#ifndef DB_H
#define DB_H
#include <string>
#include <list>
#include <fstream>
#include <map>
#include <experimental/filesystem>
#include <boost/algorithm/string/replace.hpp>
#include <sqlite3.h>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

using namespace std;

class db
{
  public:
    struct song
    {
      std::string path;
      std::string title;
      int track_no;
      std::string album;
      unsigned short int year;
      std::string artist;
      bool favorite;
      std::string spotify_id;
      float rating;
      double duration;
      std::string md5;
      bool found = false;
      void escape()
      {
        boost::replace_all(path, "'", "''");
        boost::replace_all(title, "'", "''");
        boost::replace_all(album, "'", "''");
        boost::replace_all(artist, "'", "''");
      }
      string to_query_string()
      {
        return (string) "'" + path + "', '" + title + "', " + to_string(track_no) + ", '" + album + "', " + to_string(year) +
          ", '" + artist + "', " + to_string(favorite) + ", " + to_string(duration) + ", '"  + spotify_id + "', '" + md5 + "'";
      }
      song& operator=(song a)
      {
        path = a.path;
        title = a.title;
        track_no = a.track_no;
        album = a.album;
        year = a.year;
        artist = a.artist;
        favorite = a.favorite;
        spotify_id = a.spotify_id;
        rating = a.rating;
        duration = a.duration;
        md5 = a.md5;
        found = a.found;
        return *this;
      }
    };
    struct playlist
    {
      string name;
      string source;
      int last;
    };
    static db *get_instance();
    virtual ~db();

    void insert_path(const string path);
    void insert_track(const song s);
    void insert_influence(const string artist, const string influence);
    void add_to_playlist(const string name, const string path);
    void add_to_playlist(const string name, list<string> paths);
    void insert_playlist(const string name, const string source="");

    void update_track(const song s);
    void set_playlist_current(const string name, const int current);

    void remove_track(const string path);
    void remove_from_playlist(const string name, const string path);
    void remove_from_playlist(const string name, const list<string> paths);
    void remove_playlist(const string name);

    song select_track(const string path);
    list<string> select_playlist(const string path);
    list<string> select_where(const song s);
    list<string> select_like(const song s);
    string get_playlist_source(const string name);
    int get_playlist_current(const string name);

    list<pair<string, string>> influences();
    list<string> artists();
    void update_duration(song* s);
    void update_duration(string path, double d);
    list<string> tables();
    const bool empty_queue() { return queue_empty; };
  protected:

  private:
    db();
    static db *m_instance;
    sqlite3 *sql;
    queue<string> queries;
    void monitor(void);
    void push(string q);
    mutex m_mutex;
    condition_variable cv;
    static string escape(const string);
    static string more(bool b);
    bool queue_empty;
};

#endif // DB_H
