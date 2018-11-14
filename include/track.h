#ifndef TRACK_H
#define TRACK_H

#include <string>
#include <list>

#include "db.h"

class track
{
    public:
        track();
        virtual ~track();
        track(const track& other);
        track& operator=(const track& other);
        db::song tags;
        std::string path;
        std::string title;
        unsigned char track_no;
        std::string album;
        unsigned short int year;
        std::string artist;
        std::string get_lyrics();
        void set_lyrics(std::string);
        std::list<std::string> influenced_by;
        std::list<std::string> influenced;

        bool favorite;
        float rating;
        std::string mime_type;

    protected:

    private:
};

#endif // TRACK_H
