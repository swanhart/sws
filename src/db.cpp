#include "db.h"

using namespace std;

sqlite3 *db::sql = NULL;
list<string> *db::artist_list = NULL;
queue<std::function<void()>> db::pending = queue<std::function<void()>>();

db::db()
{
  config* db_config = config::get_instance();
  sqlite3_open(db_config->get<string>("db_file").c_str(), &sql);
}

db::~db()
{

}

std::list<std::string> *db::genres()
{
  char* query = (char*) "CREATE TABLE IF NOT EXISTS db_genres (genre TEXT UNIQUE DEFAULT '');";
  sqlite3_exec(sql, query, 0, 0, 0);
  query = (char*) "SELECT genre from db_genres";
  sqlite3_stmt *stmt;
  sqlite3_prepare(sql, query, -1, &stmt, 0);
  int res;
  list<string> *gens = new list<string>;
  while (true)
    {
      res = sqlite3_step(stmt);
      if (res == SQLITE_ROW)
        {
          gens->push_back((string) (char*) sqlite3_column_text(stmt, 0));
        }
      else if (res == SQLITE_DONE)
        {
          break;
        }
    }
  stmt = NULL;
  return gens;
}

std::list<std::string> *db::influences()
{
  if (artist_list->size() == 0)
    {
      sqlite3_stmt *stmt;
      const char* query = "SELECT DISTINCT artist FROM db_catalog";
      sqlite3_prepare(sql, query, -1, &stmt, 0);
      int res;
      while (true)
        {
          res = sqlite3_step(stmt);
          if (res == SQLITE_ROW)
            {
              artist_list->push_back((string) (char*) sqlite3_column_text(stmt, 0));
            }
          if (res == SQLITE_DONE)
            {
              break;
            }
        }
    }
  return artist_list;
}

void db::get_track(const string* t, track* s)
{
  string md_hash;
  sqlite3_stmt *stmt;
  const string query = (string) "SELECT title, track_no, album, year, artist, favorite, file_hash from db_catalog where path = '" + *t + (string) "';";
  sqlite3_prepare(sql, query.c_str(), -1, &stmt, 0);
  int res = sqlite3_step(stmt);
  if (res == SQLITE_ROW)
    {
      md_hash = (string) (char*) sqlite3_column_text(stmt, 5);
      if (md_hash != s->md5)
      {
        pending.push(bind(update_track, new track(*t)));
      }
      else
      {
        s = new track();
        s->path = *t;
        s->title = (string) (char*) sqlite3_column_text(stmt, 0);
        s->track_no = sqlite3_column_int(stmt, 1);
        s->album = (string) (char*) sqlite3_column_text(stmt, 2);
        s->year = sqlite3_column_int(stmt, 3);
        s->artist = (string) (char*) sqlite3_column_text(stmt, 4);
        s->md5 = (string) (char*) sqlite3_column_text(stmt, 5);
      }
    }
  else
    {
      s = NULL;
    }

}

void db::update_track(const track *s)
{
  const string query = (string) "UPDATE db_catalog SET title = '" + s->title + (string) "', " + to_string(s->track_no) + (string) + ", '" + s->album + (string) "', "
                   + to_string(s->year) + (string) ", '" + s->artist + (string) "', " + to_string((int) s->favorite) + ", '" + s->md5 + (string) "' WHERE path = '" + s->path + "';";
  sqlite3_exec(sql, query.c_str(), 0, 0, 0);
}


void db::add_track(const string* s)
{
  pending.push(bind(static_cast<void(*)(const track*)>(&add_track), new track(*s)));
}

void db::add_track(const track* s)
{
  string query = (string) "CREATE TABLE IF NOT EXISTS db_catalog (path TEXT PRIMARY_KEY DEFAULT '' title TEXT, track_no INTEGER, album TEXT, year INTEGER, artist TEXT, favorite INTEGER, file_hash TEXT NOT NULL);";
  sqlite3_exec(sql, query.c_str(), 0, 0, 0);
  query = (string) "INSERT INTO TABLE db_catalog VALUES ('" + s->path + (string) "', '" + s->title + (string) "', " + to_string(s->track_no) + (string) ", '" + s->album + (string) "', "
      + to_string(s->year) + (string) ", '" + s->artist + (string) "', '" + s->md5 + "');";
  sqlite3_exec(sql, query.c_str(), 0, 0, 0);
}

void db::remove_track(const string* p)
{
  const string query = (string) "DELETE FROM db_catalog WHERE path = '" + *p + "';";
  sqlite3_exec(sql, query.c_str(), 0, 0, 0);
}

void db::add_influnce(const string* a, const string* b)
{
  string query = (string) "CREATE TABLE IF NOT EXISTS db_influence (artist TEXT NOT NULL, influence TEXT NOT NULL);";
  sqlite3_exec(sql, query.c_str(), 0, 0, 0);
  query = (string) "INSERT INTO db_influence VALUES('" + *a + (string) "', '" + *b + (string) "');";
  sqlite3_exec(sql, query.c_str(), 0, 0, 0);
}

void db::remove_influence(const string* a, const string *b)
{
  string query;
  if (b != NULL)
    {
      query = (string) "DELETE FROM db_influence WHERE artist = '" + *a + (string) "' AND influence = '" + *b + (string) "';";
    }
  else
    {
      query = (string) "DELETE FROM db_influence WHERE artist = '" + *a + (string) "';";
    }
  sqlite3_exec(sql, query.c_str(), 0, 0, 0);
}

void db::get_influence(const string* a, list<string>* b)
{
  b = new list<string>;
  sqlite3_stmt *stmt;
  const string query = (string) "SELECT influence FROM db_influence WHERE artist = '" + *a + (string) "';";
  sqlite3_prepare(sql, query.c_str(), -1, &stmt, 0);
  int res;
  while (true)
    {
      res = sqlite3_step(stmt);
      if (res == SQLITE_DONE)
        {
          break;
        }
      else if (res == SQLITE_ROW)
        {
          b->push_back((string)(char*) sqlite3_column_text(stmt, 0));
        }
    }
  stmt = NULL;
}

void db::get_influenced(const string* a, list<string>* b)
{
  b = new list<string>;
  sqlite3_stmt *stmt;
  int res;
  const string query = (string) "SELECT artist from db_influence WHERE influence = '" + *a + (string) "';";
  sqlite3_prepare(sql, query.c_str(), -1, &stmt, 0);
  while (true)
    {
      res = sqlite3_step(stmt);
      if (res == SQLITE_DONE)
        {
          break;
        }
      else if (res == SQLITE_ROW)
        {
          b->push_back((string)(char*) sqlite3_column_text(stmt, 0));
        }
    }
  stmt = NULL;
}

void db::get_playlist(const string* n, list<string> *p)
{
  p = new list<string>;
  sqlite3_stmt *stmt;
  int res;
  const string query = (string) "SELECT path from '" + *n + (string) "';";
  sqlite3_prepare(sql, query.c_str(), -1, &stmt, 0);
  while (true)
    {
      res = sqlite3_step(stmt);
      if (res == SQLITE_DONE)
        {
          break;
        }
      else if (res == SQLITE_ROW)
        {
          p->push_back((string)(char*) sqlite3_column_text(stmt, 0));
        }
    }
  stmt = NULL;
}

void db::update_playlist(const string* n, const list<string>* p)
{
  string query = (string) "DELETE FROM '" + *n + "';";
  sqlite3_exec(sql, query.c_str(), 0, 0, 0);
  for (list<string>::const_iterator it = p->begin(), end = p->end(); it != end; ++it)
    {
      query = (string) "INSERT INTO '" + *n + (string) "' (path) VALUES ('" + *it + (string) "');";
      sqlite3_exec(sql, query.c_str(), 0, 0,0);
    }
}

void db::add_to_playlist(const string*n, const list<string>* p)
{
  string query;
  for (list<string>::const_iterator it = p->begin(), end = p->end(); it != end; ++it)
    {
      query = (string) "INSERT INTO '" + *n + (string) "' (path) VALUES ('" + *it + (string) "');";
      sqlite3_exec(sql, query.c_str(), 0, 0, 0);
    }
}

void db::add_to_playlist(const string *n, const string *p)
{
  string query = (string) "INSERT INTO '" + *n + (string) "' (path) VALUES ('" + *p + "');";
  sqlite3_exec(sql, query.c_str(), 0, 0, 0);
}

void db::remove_from_playlist(const string* n, const list<string>* p)
{
  string query;
  for (list<string>::const_iterator it = p->begin(), end = p->end(); it != end; ++it)
    {
      query = (string) "DELETE FROM '" + *n + (string) "' WHERE path = '" + *it + "';";
      sqlite3_exec(sql, query.c_str(), 0, 0, 0);
    }
}

void db::remove_from_playlist(const string *n, const string *p)
{
  string query = (string) "DELETE FROM '" + *n + (string) "' WHERE path = '" + *p + "';";
  sqlite3_exec(sql, query.c_str(), 0, 0, 0);
}

void db::add_playlist(const string* fn)
{
  string name = boost::filesystem::path(*fn).stem().string();
  ifstream ifs (fn->c_str());
  string line;
  list<string>* plist = new list<string>;
  if (ifs.is_open())
    {
      while (getline(ifs, line))
        {
          if (line[0] != '#')
            {
              plist->push_back(line);
            }
        }
    }
  ifs.close();
  if (plist->size() > 0)
    pending.push(bind(static_cast<void(*)(const string*, const list<string>*)>(add_to_playlist), &name, plist));
  plist = NULL;
}


