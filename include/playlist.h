#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <string>
#include <list>

#include "track.h"

class playlist
{
    public:
        playlist();
        virtual ~playlist();
        playlist(const playlist& other);
        playlist& operator=(const playlist& other);
        std::list<track>* tracks;

    protected:

    private:
};

#endif // PLAYLIST_H
