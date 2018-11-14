#ifndef DB_H
#define DB_H

#include <fstream>
#include <string>
#include <list>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <sqlite3.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>

#include "utility.h"
#include "config.h"

class db
{
    public:
        typedef struct song{
            std::string path;
            std::string title;
            unsigned char track_no;
            std::string album;
            unsigned short int year;
            std::string artist;
            std::string file_hash;
        } song;
        db();
        virtual ~db();
        static std::list<std::string> *genres();
        static std::list<std::string> *influences();
        static void get_track(std::string*, song*);
        static void set_track(song*);
        static void add_track(std::string*);
        static void add_track(song*);
        static void add_influnce(std::string*, std::string*);
        static void remove_influence(std::string*, std::string*);
        static void get_influence(std::string*, std::list<std::string>*);
        static void get_influenced(std::string*, std::list<std::string>*);
        static void remove_track(std::string*);
        static void get_playlist(std::string*, std::list<std::string>*);
        static void set_playlist(std::string*, std::list<std::string>*);
        static void add_to_playlist(std::string*, std::list<std::string>*);
        static void add_to_playlist(std::string*, std::string*);
        static void remove_from_playlist(std::string*, std::list<std::string>*);
        static void remove_from_playlist(std::string*, std::string*);
        static void add_playlist(std::string*);
    private:
        config* db_config;
        static sqlite3* sql;
        static std::list<std::string> *artist_list;
        static song* get_song(std::string*);
};

#endif // DB_H
