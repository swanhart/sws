#include "playlist.h"

playlist::playlist()
{
    //ctor
}

playlist::~playlist()
{
    //dtor
}

playlist::playlist(const playlist& other)
{
    //copy ctor
}

playlist& playlist::operator=(const playlist& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    //assignment operator
    return *this;
}
