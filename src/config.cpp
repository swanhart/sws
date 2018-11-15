#include "config.h"

using namespace std;
namespace fs = boost::filesystem;

config *config::m_instance = NULL;

config::config()
{
  set_program_name();
  config_settings.at("config_root") = (string) "`/." + pname;
}

config* config::get_instance()
{
    return (!m_instance) ? m_instance = new config : m_instance;
}

config::~config()
{
    //dtor
}

map<string,string> config::get_config_settings()
{
  string fname = config_settings.at("config_root") + (string) "/" + fname + (string) ".config";
  boost::property_tree::ptree pt;
  try
  {
    boost::property_tree::ini_parser::read_ini(fname.c_str(), pt);
  }
  catch(exception &ex)
  {
    set_default();
  }

  for (auto it: pt)
  {

  }

  return config_settings;
}

void config::set_default()
{
  config_settings.at("db_file") = config_settings.at("config_root") + (string) "/" + pname + (string) ".db";
  save_config_settings();
}

void config::save_config_settings()
{

}

void config::set_program_name()
{
  char *p = (char*)malloc(PATH_MAX);
  if (p != NULL)
  {
    if (readlink("/proc/self/exe", p, PATH_MAX) == -1)
    {
      free(p);
      p = NULL;
    }
  }
  pname = (string) p;
}
