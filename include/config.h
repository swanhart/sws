#ifndef CONFIG_H
#define CONFIG_H

#include <linux/limits.h>
#include <string>
#include <map>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "joystick.h"

class config
{
    public:
        static config* get_instance();
        std::map<std::string,std::string> get_config_settings();
        void save_config_settings();
        virtual ~config();
        joystick jstick;
    protected:
    private:
        config();
        static bool instance_flag;
        static config *m_instance;
        void set_default();
        std::map<std::string,std::string> config_settings;
        void set_program_name();
        std::string pname;
};

#endif // CONFIG_H
