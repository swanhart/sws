#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <map>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <iostream>

class config
{
public:
  template< typename T >
  void put (std::string a, T b)
  {
    settings.put(a, b);
    save_config_settings();
  }
  template< typename T >
  T get (const std::string a)
  {
    T val;
    val = settings.get<T>(a);
    return (val);
  }
  static config* get_instance();
  void save_config_settings();
  std::string root;
  virtual ~config();
protected:
private:
  config();
  static bool instance_flag;
  static config *m_instance;
  void set_default();
  std::string config_file;
  boost::property_tree::ptree settings;
  void set_program_name();
  std::string prog_name;
};

#endif // CONFIG_H
