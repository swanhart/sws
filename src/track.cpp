#include "track.h"

track::track()
{
    //ctor
}

track::~track()
{
    //dtor
}

track::track(const track& other)
{
    //copy ctor
}

track& track::operator=(const track& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    //assignment operator
    return *this;
}
