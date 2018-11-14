#include "config.h"

using namespace std;
namespace fs = boost::filesystem;

config *config::m_instance = NULL;

config::config()
{
  config_settings.at("config_root") = "`/.sws";
  config_settings.at("db_file") = config_settings.at("config_root") + (string) "/db.sqlite";
}

config* config::get_instance()
{
    return (!m_instance) ? m_instance = new config : m_instance;
}

config::~config()
{
    //dtor
}
