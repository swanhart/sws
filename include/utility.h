#ifndef UTILITY_H
#define UTILITY_H

#include <string>
#include <vector>
#include <random>
#include <list>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <thread>
#include <magic.h>

#include "config.h"

#include "db.h"


namespace utility
{
  static std::string proc_name;
  void init(std::string program_name);
  size_t utf8_leng(std::string);
//  std::string file_hash(const std::string);
  std::string mime_info(const std::string);
  void populate_thread(boost::filesystem::path*);
  void populate();
  template <typename T> void shuffle(std::list<T>& );
  bool starts_with(const std::string, const std::string);
  bool ends_with(const std::string, const std::string);
  bool is_audio_file(const std::string);
  bool is_playlist_file(const std::string);



}


#endif // UTILITY_H
