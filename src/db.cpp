#include "db.h"

using namespace std;

sqlite3 *db::sql = NULL;
list<string> *db::artist_list = NULL;

db::db()
{
    sqlite3_open(db_config->get_config_settings().at("db_file").c_str(), &sql);
}

db::~db()
{

}

std::list<std::string> *db::genres()
{
    char* q = (char*) "CREATE TABLE IF NOT EXISTS db_genres (genre TEXT UNIQUE DEFAULT '');";
    sqlite3_exec(sql, q, 0, 0, 0);
    q = (char*) "SELECT genre from db_genres";
    sqlite3_stmt *stmt;
    sqlite3_prepare(sql, q, -1, &stmt, 0);
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
        const char* q = "SELECT DISTINCT artist FROM db_catalog";
        sqlite3_prepare(sql, q, -1, &stmt, 0);
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

void db::get_track(string* t, song* s)
{
    sqlite3_stmt *stmt;
    const string q = (string) "SELECT title, track_no, album, year, artist, file_hash from db_catalog where path = '" + *t + (string) "';";
    sqlite3_prepare(sql, q.c_str(), -1, &stmt, 0);
    int res = sqlite3_step(stmt);
    if (res == SQLITE_ROW)
    {
        s->path = *t;
        s->title = (string) (char*) sqlite3_column_text(stmt, 0);
        s->track_no = sqlite3_column_int(stmt, 1);
        s->album = (string) (char*) sqlite3_column_text(stmt, 2);
        s->year = sqlite3_column_int(stmt, 3);
        s->artist = (string) (char*) sqlite3_column_text(stmt, 4);
        s->file_hash = (string) (char*) sqlite3_column_text(stmt, 5);
    }
    else {
        s = NULL;
    }

}

void db::add_track(string* s)
{
    add_track(get_song(s));

}

void db::add_track(song* s)
{
    string q = (string) "CREATE TABLE IF NOT EXISTS db_catalog (path TEXT PRIMARY_KEY DEFAULT '' , track_no INTEGER, album TEXT, year INTEGER, artist TEXT, file_hash TEXT NOT NULL);";
    sqlite3_exec(sql, q.c_str(), 0, 0, 0);
    q = (string) "INSERT INTO TABLE db_catalog VALUES ('" + s->path + (string) "', " + to_string(s->track_no) + (string) ", '" + s->album + (string) "', "
                + to_string(s->year) + (string) ", '" + s->artist + (string) "', '" + s->file_hash + "');";
    sqlite3_exec(sql, q.c_str(), 0, 0, 0);
}

void db::remove_track(string* p)
{
    const string q = (string) "DELETE FROM db_catalog WHERE path = '" + *p + "';";
    sqlite3_exec(sql, q.c_str(), 0, 0, 0);
}

void db::add_influnce(string* a, string* b)
{
    string q = (string) "CREATE TABLE IF NOT EXISTS db_influence (artist TEXT NOT NULL, influence TEXT NOT NULL);";
    sqlite3_exec(sql, q.c_str(), 0, 0, 0);
    q = (string) "INSERT INTO db_influence VALUES('" + *a + (string) "', '" + *b + (string) "');";
    sqlite3_exec(sql, q.c_str(), 0, 0, 0);
}

void db::remove_influence(string* a, string *b)
{
    string q;
    if (b != NULL)
    {
        q = (string) "DELETE FROM db_influence WHERE artist = '" + *a + (string) "' AND influence = '" + *b + (string) "';";
    }
    else
    {
        q = (string) "DELETE FROM db_influence WHERE artist = '" + *a + (string) "';";
    }
    sqlite3_exec(sql, q.c_str(), 0, 0, 0);
}

void db::get_influence(string* a, list<string>* b)
{
    b = new list<string>;
    sqlite3_stmt *stmt;
    const string q = (string) "SELECT influence FROM db_influence WHERE artist = '" + *a + (string) "';";
    sqlite3_prepare(sql, q.c_str(), -1, &stmt, 0);
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

void db::get_influenced(string* a, list<string>* b)
{
    b = new list<string>;
    sqlite3_stmt *stmt;
    int res;
    const string q = (string) "SELECT artist from db_influence WHERE influence = '" + *a + (string) "';";
    sqlite3_prepare(sql, q.c_str(), -1, &stmt, 0);
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

void db::get_playlist(string* n, list<string> *p)
{
    p = new list<string>;
    sqlite3_stmt *stmt;
    int res;
    const string q = (string) "SELECT path from '" + *n + (string) "';";
    sqlite3_prepare(sql, q.c_str(), -1, &stmt, 0);
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

void db::set_playlist(string* n, list<string>* p)
{
    string q = (string) "DELETE FROM '" + *n + "';";
    sqlite3_exec(sql, q.c_str(), 0, 0, 0);
    for (list<string>::const_iterator it = p->begin(), end = p->end(); it != end; ++it)
    {
        q = (string) "INSERT INTO '" + *n + (string) "' (path) VALUES ('" + *it + (string) "');";
        sqlite3_exec(sql, q.c_str(), 0, 0 ,0);
    }
}

void db::add_to_playlist(string*n, list<string>* p)
{
    string q;
    for (list<string>::const_iterator it = p->begin(), end = p->end(); it != end; ++it)
    {
        q = (string) "INSERT INTO '" + *n + (string) "' (path) VALUES ('" + *it + (string) "');";
        sqlite3_exec(sql, q.c_str(), 0, 0, 0);
    }
}

void db::add_to_playlist(string *n, string *p)
{
    string q = (string) "INSERT INTO '" + *n + (string) "' (path) VALUES ('" + *p + "');";
    sqlite3_exec(sql, q.c_str(), 0, 0, 0);
}

void db::remove_from_playlist(string* n, list<string>* p)
{
    string q;
    for (list<string>::const_iterator it = p->begin(), end = p->end(); it != end; ++it)
    {
        q = (string) "DELETE FROM '" + *n + (string) "' WHERE path = '" + *it + "';";
        sqlite3_exec(sql, q.c_str(), 0, 0, 0);
    }
}

void db::remove_from_playlist(string *n, string *p)
{
    string q = (string) "DELETE FROM '" + *n + (string) "' WHERE path = '" + *p + "';";
    sqlite3_exec(sql, q.c_str(), 0, 0, 0);
}

void db::add_playlist(std::string* fn)
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
        db::add_to_playlist(&name, plist);
    plist = NULL;
}

db::song* db::get_song(std::string* p)
{
    db::song* s = new song;
    TagLib::FileRef f(p->c_str());
    if (f.tag())
    {
        s->title = (string) f.tag()->title().toCString(true);
        s->track_no = f.tag()->track();
        s->album = (string) f.tag()->album().toCString(true);
        s->year = f.tag()->year();
        s->artist = (string) f.tag()->artist().toCString(true);
        s->file_hash = utility::file_hash(*p);
    }
    return s;
}

