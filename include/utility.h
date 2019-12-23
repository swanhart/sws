#ifndef UTILITY_H
#define UTILITY_H

#include <string>
#include <boost/filesystem.hpp>

#include "config.h"
#include "track.h"


namespace utility
{
  static std::string proc_name;
  void init(std::string program_name);
  size_t utf8_leng(std::string);
//  std::string file_hash(const std::string);
  std::string mime_info(const std::string);
  bool starts_with(const std::string, const std::string);
  bool ends_with(const std::string, const std::string);
  bool is_audio_file(const std::string);
  bool is_playlist_file(const std::string);
}



#endif // UTILITY_H
