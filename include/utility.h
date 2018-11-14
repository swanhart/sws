#ifndef UTILITY_H
#define UTILITY_H

#include <string>
#include <vector>
#include <random>
#include <list>
#include <fstream>
#include <libudev.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/thread.hpp>
#include <magic.h>

#include "config.h"
#include "md5.h"
#include "db.h"

namespace utility
{
    size_t utf8_leng(std::string);
    std::string file_hash(const std::string);
    std::string mime_info(const char*);
    static int fd_joystick;
    void populate_thread(boost::filesystem::path*);
    void populate();
    void monitor_udev_thread(void*);
    void monitor_udev();
    template < typename T > void shuffle(std::list<T>& );
}


#endif // UTILITY_H
