#include <linux/limits.h>
#include <unistd.h>

#include "utility.h"
#include "config.h"

using namespace std;
namespace fs = boost::filesystem;

config *config::m_instance = NULL;


config::config()
{
  set_program_name();
  const char* home_directory;
  home_directory = getenv("HOME");
  root = (string) home_directory + (string) "/." + prog_name + (string) "/";
  config_file = root + "settings.conf";
  boost::filesystem::path p(config_file);
  if (boost::filesystem::exists(p))
  {
    boost::property_tree::read_json(p.string(), settings);
  }
  else
  {
    put("root", root);
    put("db_file", root + prog_name + (string) ".sqlite");
    put("confirm", true);
    put("music_root", "/media/");
    put("current_playlist", "db_catalog");
    save_config_settings();
  }
}

config* config::get_instance()
{
  return (!m_instance) ? m_instance = new config : m_instance;
}

config::~config()
{
  //dtor
}

void config::save_config_settings()
{
  boost::property_tree::write_json(config_file, settings);
}

void config::set_program_name()
{
  char *p = (char*)malloc(PATH_MAX);
  if (readlink("/proc/self/exe", p, PATH_MAX) < 0)
  {
     perror("lstat");
     p = { 0 };
  }
  boost::filesystem::path pn(p);
  if (!boost::filesystem::exists(pn)) set_program_name();
  else
  {
    prog_name = pn.filename().string();
  }
}

