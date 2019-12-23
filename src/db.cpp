#include <gstreamermm.h>
#include "config.h"
#include "db.h"

using namespace std;

db *db::m_instance = NULL;

db::db()
{
  queue_empty = true;
  thread t(&db::monitor, this);
  t.detach();
  config *conf = config::get_instance();
  sqlite3_open(conf->get<string>("db_file").c_str(), &sql);
  string query = (string) "CREATE TABLE IF NOT EXISTS db_catalog (path TEXT PRIMARY_KEY DEFAULT '', title TEXT DEFAULT '', " +
  "track_no INTEGER DEFAULT 0, album TEXT DEFAULT '', year INTEGER DEFAULT 0, artist TEXT DEFAULT '', favorite INTEGER DEFAULT 0, " +
  "duration REAL DEFAULT 0, spotify_id TEXT DEFAULT '', file_hash TEXT DEFAULT '-1');";
  push(query);
  query = (string) "CREATE TABLE IF NOT EXISTS _influences (artist TEXT NOT NULL, influence TEXT NOT NULL, PRIMARY KEY (artist, influence));";
  push(query);
  query = "CREATE TABLE IF NOT EXISTS _playlist (name TEXT PRIMARY KEY, source TEXT DEFAULT '', current INTEGER DEFAULT 0);";
  push(query);
}

void db::push(string q)
{
  unique_lock<mutex> lock(m_mutex);
  queries.push(q);
  lock.unlock();
  queue_empty = false;
  cv.notify_all();
}

void db::monitor(void)
{
  for (;;)
  {
    unique_lock<mutex> lock(m_mutex);
    cv.wait(lock, [this] {
            return (queries.size());
            });
    if (queries.size())
    {
      //auto op = move(queries.front());
      string q = queries.front();
      queries.pop();
      lock.unlock();
      int res = sqlite3_exec(sql, q.c_str(), 0, 0, 0);
      if (res != SQLITE_OK)
      {
        cout << "SQLITEERROR: " << sqlite3_errmsg(sql) << endl;
        cout << q <<endl;
      }
      if (queries.size() == 0)
        queue_empty = true;
      else
        queue_empty = false;
      lock.lock();
    }
  }
}

db* db::get_instance()
{
  return (!m_instance) ? m_instance = new db : m_instance;
}

db::~db()
{

}

void db::insert_path(const string path)
{
  string query = "INSERT INTO db_catalog (path) VALUES('" + escape(path) + "');";
  push(query);
}


void db::insert_track(const song s)
{
  song r = s;
  r.escape();
  string query = (string) "INSERT INTO db_catalog VALUES(" + r.to_query_string() + ");";
  push(query);
}

void db::insert_influence(const string artist, const string influence)
{
  string query = (string) "REPLACE INTO _influences VALUES('" + artist + "', '" + influence + "');";
  push(query);
}

void db::add_to_playlist(const string name, const string path)
{
  string query = (string) "REPLACE INTO [" + name + "] VALUES('" + escape(path) + "');";
  push(query);
}

void db::add_to_playlist(const string name, list<string> paths)
{
  if (paths.size() == 0) return;
  string prefix =  (string) "REPLACE INTO [" + name + "] VALUES ";
  string vals = "";
  for (string s : paths)
  {
    vals += "('" + escape(s) + "'),";
  }
  vals = vals.substr(0, vals.length() - 1) + ";";
  string query = (string) "CREATE TABLE IF NOT EXISTS [" + name + "] (path TEXT PRIMARY KEY);";
  push(query);
  query = prefix + vals;
  push(query);
}

void db::insert_playlist(const string name, const string source)
{
  string query = "REPLACE INTO _playlist (name, source) VALUES ('" + escape(name) + "','" + escape(source) + "');";
  push(query);
}

void db::update_track(const song s)
{
  string query = (string) "UPDATE db_catalog SET title='" + escape(s.title) + "'," +
    "track_no"=to_string(s.track_no) + "," +
    "album='" + escape(s.album) + "'," +
    "year=" + to_string(s.year) + "," +
    "artist='" + escape(s.artist) + "'," +
    "favorite=" + to_string(s.favorite) + "," +
    "duration=" + to_string(s.duration) + "," +
    "spotify_id='" + s.spotify_id + "'," +
    "file_hash='" + escape(s.md5) + "' " +
    "WHERE path = '" + escape(s.path) + "';";
  push(query);
}


void db::set_playlist_current(const string name, const int current)
{
  string query = (string) "UPDATE _playlist SET current=" + to_string(current) + " WHERE name='" + escape(name) + "';";
  push(query);
}

void db::remove_track(const string path)
{
  string query = (string) "DELETE FROM db_catalog WHERE path='" + escape(path) + "';";
  push(query);
}

void db::remove_from_playlist(const string name, const string path)
{
  string query = (string) "DELETE FROM [" + name + "] WHERE path='" + escape(path) + "';";
  push(query);
}

void db::remove_from_playlist(const string name, const list<string> paths)
{
  for (const string & path : paths)
  {
    string query = (string) "DELETE FROM [" + name + "] WHERE path='" + escape(path) + "';";
    push(query);
  }
}

void db::remove_playlist(const string name)
{
  string query = (string) "DROP TABLE IF EXISTS [" + name + "];";
  push(query);
}

db::song db::select_track(const string path)
{
  int res = -1;
  sqlite3_stmt *stmt = NULL;
  song s;
  string query = (string) "SELECT title, track_no, album, year, artist, favorite, spotify_id, file_hash from db_catalog "+
  "WHERE path='" + escape(path) + "';";
  string c_query = (string) "CREATE TABLE IF NOT EXISTS db_catalog (path TEXT PRIMARY_KEY DEFAULT '', title TEXT DEFAULT '', " +
  "track_no INTEGER DEFAULT 0, album TEXT DEFAULT '', year INTEGER DEFAULT 0, artist TEXT DEFAULT '', favorite INTEGER DEFAULT 0, " +
  "duration REAL DEFAULT 0, spotify_id TEXT DEFAULT '', file_hash TEXT DEFAULT '-1');";
  res = sqlite3_exec(sql, c_query.c_str(), 0, 0, 0);
  if (res != SQLITE_OK)
  {
    cout << "SQLSELECTTRACK: " << sqlite3_errmsg(sql) << endl;
    cout << query << endl;
  }
  res = sqlite3_prepare(sql, query.c_str(), -1, &stmt, 0);
  if (res != SQLITE_OK)
  {
    cout << "SQLSELECTTRACK: " << sqlite3_errmsg(sql) << endl;
    cout << query << endl;
    s.found = false;
    s.title = "";
    s.track_no = -1;
    s.album = "";
    s.year = 0;
    s.artist = "";
    s.favorite = false;
    s.spotify_id = "";
    s.md5 = "-1";
  }
  else
  {
    res = sqlite3_step(stmt);
    if (res == SQLITE_ROW)
    {
      s.found = true;
      s.path = path;
      s.title = (string) (char*) sqlite3_column_text(stmt, 0);
      s.track_no = sqlite3_column_int(stmt, 1);
      s.album = (string) (char*) sqlite3_column_text(stmt, 2);
      s.year = (unsigned short int) sqlite3_column_int(stmt, 3);
      s.artist = (string) (char*) sqlite3_column_text(stmt, 4);
      s.favorite = (bool) sqlite3_column_int(stmt, 5);
      s.spotify_id = (string) (char*) sqlite3_column_text(stmt, 6);
      s.md5 = (string) (char*) sqlite3_column_text(stmt, 7);
    }
    else
    {
      s.found = false;
      s.title = "";
      s.track_no = -1;
      s.album = "";
      s.year = 0;
      s.artist = "";
      s.favorite = false;
      s.spotify_id = "";
      s.md5 = "-1";
    }
  }
  sqlite3_finalize(stmt);
  return s;
}

list<string> db::select_playlist(const string name)
{
  int res = -1;
  sqlite3_stmt *stmt = NULL;
  list<string> result;
  string query = (string) "SELECT DISTINCT path FROM [" + name + "];";
  res = sqlite3_prepare(sql, query.c_str(), -1, &stmt, 0);
  if (res != SQLITE_OK)
  {
    cout << "SQLSELECTPLAYLIST: " << sqlite3_errmsg(sql) << endl;
    cout << query << endl;
    return result;
  }
  while (true)
  {
    res = sqlite3_step(stmt);
    if (res == SQLITE_ROW)
    {
      result.push_back((string) (char*) sqlite3_column_text(stmt, 0));
    }
    else
    {
      break;
    }
  }
  sqlite3_finalize(stmt);
  return result;
}

list<string> db::select_where(const song s)
{
  list<string> result;
  int res = -1;
  sqlite3_stmt *stmt = NULL;
  bool where = false;
  string where_clause = "";
  string query = "SELECT path from db_catalog WHERE";
  if (s.album != "")
  {
    where_clause += more(where) + " album='" + escape(s.album) + "'";
    where = true;
  }
  if (s.artist != "")
  {
    where_clause += more(where) + " artist='" + escape(s.artist) + "'";
    where = true;
  }
  if (s.favorite)
  {
    where_clause += more(where) + " favorite=" + to_string(s.favorite);
    where = true;
  }
  if (s.title != "")
  {
    where_clause += more(where) + " title='" + escape(s.title) + "'";
    where = true;
  }
  if (s.year > 0)
  {
    where_clause += more(where) + " year=" + to_string(s.year);
    where = true;
  }
  if (where)
  {
    query = query + where_clause + ";";
    res = sqlite3_prepare(sql, query.c_str(), -1, &stmt, 0);
    if (res != SQLITE_OK)
    {
      cout << "SQLSELECTWHERE: " << sqlite3_errmsg(sql) << endl;
      cout << query << endl;
      sqlite3_finalize(stmt);
      return result;
    }
    while (true)
    {
      res = sqlite3_step(stmt);
      if (res == SQLITE_ROW)
      {
        result.push_back((string) (char*) sqlite3_column_text(stmt, 0));
      }
      else
      {
        break;
      }
    }
  }
  sqlite3_finalize(stmt);
  return result;
}

list<string> db::select_like(const song s)
{
  list<string> result;
  int res = -1;
  sqlite3_stmt *stmt = NULL;
  bool where = false;
  string where_clause = "";
  string query = "SELECT path from db_catalog WHERE";
  if (s.album != "")
  {
    where_clause += more(where) + " album LIKE '%" + escape(s.album) + "%'";
    where = true;
  }
  if (s.artist != "")
  {
    where_clause += more(where) + " artist LIKE '%" + escape(s.artist) + "%'";
    where = true;
  }
  if (s.favorite)
  {
    where_clause += more(where) + " favorite=" + to_string(s.favorite);
    where = true;
  }
  if (s.title != "")
  {
    where_clause += more(where) + " title LIKE '%" + escape(s.title) + "%'";
    where = true;
  }
  if (s.year > 0)
  {
    where_clause += more(where) + " year=" + to_string(s.year);
    where = true;
  }
  if (where)
  {
    query = query + where_clause + ";";
    res = sqlite3_prepare(sql, query.c_str(), -1, &stmt, 0);
    if (res != SQLITE_OK)
    {
      cout << "SQLSELECTWHERE: " << sqlite3_errmsg(sql) << endl;
      cout << query << endl;
      sqlite3_finalize(stmt);
      return result;
    }
    while (true)
    {
      res = sqlite3_step(stmt);
      if (res == SQLITE_ROW)
      {
        result.push_back((string) (char*) sqlite3_column_text(stmt, 0));
      }
      else
      {
        break;
      }
    }
  }
  sqlite3_finalize(stmt);
  return result;
}

string db::get_playlist_source(const string name)
{
  int res = -1;
  sqlite3_stmt *stmt;
  string result = "";
  string query = (string) "SELECT source from _playlist WHERE name='" + escape(name) + "';";
  res = sqlite3_prepare(sql, query.c_str(), -1, &stmt, 0);
  if (res != SQLITE_OK)
  {
    cout << "SQLGET_PLAYLIST: " << sqlite3_errmsg(sql) << endl;
    cout << query << endl;
  }
  else
  {
    res = sqlite3_step(stmt);
    if (res == SQLITE_ROW)
    {
      result = (string)(char*) sqlite3_column_text(stmt, 0);
    }
  }
  sqlite3_finalize(stmt);
  return result;
}

int db::get_playlist_current(const string name)
{
  int res = -1;
  sqlite3_stmt *stmt;
  int result = 0;
  string query = (string) "SELECT current from _playlist WHERE name='" + escape(name) + "';";
  res = sqlite3_prepare(sql, query.c_str(), -1, &stmt, 0);
  if (res != SQLITE_OK)
  {
    cout << "SQLGETPLAYLISTCURRENT: " << sqlite3_errmsg(sql) << endl;
    cout << query << endl;
  }
  else
  {
    res = sqlite3_step(stmt);
    if (res == SQLITE_ROW)
    {
      result = sqlite3_column_int(stmt, 0);
    }
  }
  sqlite3_finalize(stmt);
  cout << "result: " << result << endl;
  return result;
}

list<pair<string, string>> db::influences()
{
  list<pair<string, string>> result;
  int res = -1;
  sqlite3_stmt *stmt;
  string query = (string) "SELECT artist, influence from _influences ORDER BY artist;";
  res = sqlite3_prepare(sql, query.c_str(), -1, &stmt, 0);
  if (res != SQLITE_OK)
  {
    cout << "SQLINFLUENCES: " << sqlite3_errmsg(sql) << endl;
    cout << query << endl;
  }
  else
  {
    while (true)
    {
      res = sqlite3_step(stmt);
      if (res == SQLITE_ROW)
      {
        result.push_back(pair((string)(char*) sqlite3_column_text(stmt, 0), (string)(char*) sqlite3_column_text(stmt, 0)));
      }
      else
      {
        break;
      }
    }
  }
  sqlite3_finalize(stmt);
  return result;
}

list<string> db::artists()
{
  list<string> result;
  int res = -1;
  sqlite3_stmt *stmt = NULL;
  string query = (string) "SELECT DISTINCT artist from db_catalog ORDER BY ARTIST;";
  res = sqlite3_prepare(sql, query.c_str(), -1, &stmt, 0);
  if (res != SQLITE_OK)
  {
    cout << "SQLARTIST: " << sqlite3_errmsg(sql) << endl;
    cout << query << endl;
  }
  else
  {
    while (true)
    {
      res = sqlite3_step(stmt);
      if (res == SQLITE_ROW)
      {
        result.push_back((string)(char*) sqlite3_column_text(stmt, 0));
      }
      else
      {
        break;
      }
    }
  }
  sqlite3_finalize(stmt);
  return result;
}



string db::escape(const string s)
{
  string r = (string) s;
  boost::replace_all(r, "'", "''");
  return r;
}

string db::more(bool b)
{
  if (b)
    return " AND";
  else
    return "";
}

void db::update_duration(song* s)
{
  string p = s->path;
  namespace fs = experimental::filesystem;
  cout << "Getting duration " << endl;
  fs::path pth(p);
  if (!fs::exists(pth))
  {
    cout << p << " Not found" << endl;
    return;
  }
  if (!s->found)
  {
    cout << "Not in database yet" << endl;
    return;
  }
  gint64 len;
  int diff;
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
    return;
  }
  if (p.find("://") == string::npos)
  {
    p = (string) "file://" + p;
  }
  playbin->set_property("uri", p.c_str());
  playbin->set_state(Gst::STATE_PAUSED);
  auto start_time = chrono::high_resolution_clock::now();
  while (!playbin->query_duration(fmt, len) && diff < 3)
  {
    diff = chrono::duration_cast<chrono::seconds>(chrono::high_resolution_clock::now() - start_time).count();
  }
  cout << "diff = " << diff << endl;
  playbin->set_state(Gst::STATE_NULL);
//  playbin->unreference();
  double duration = len/1000000000;
  const string query = (string) "UPDATE db_catalog SET duration = " + to_string(duration) + " WHERE path='" + escape(p)+ "';";
  push(query);
  s->duration = duration;

}

void db::update_duration(string p, double d)
{
  const string query = (string) "UPDATE db_catalog SET duration = " + to_string(d) + " WHERE path='" + escape(p) + "';";
  push(query);
}

list<string> db::tables()
{
  list<string> l;
  sqlite3_stmt *stmt = NULL;
  string query = "SELECT name FROM sqlite_master WHERE type='table' AND SUBSTR(name,1,1)!='_';";
  int res = sqlite3_prepare(sql, query.c_str(), -1, &stmt, 0);
  if (res != SQLITE_OK)
  {
    cout << "SQLLISTTABLES: " << sqlite3_errmsg(sql) << endl;
    cout << query << endl;
  }
  else
  {
    while (true)
    {
      res = sqlite3_step(stmt);
      if (res == SQLITE_ROW)
      {
        l.push_back((string)(char*) sqlite3_column_text(stmt, 0));
        cout << "r: " << sqlite3_column_text(stmt, 0) << endl;
      }
      else
      {
        break;
      }
    }
  }
  sqlite3_finalize(stmt);
  for (string s : l)
  {
    cout << "LIST: " << s << endl;
  }
  return l;
}
