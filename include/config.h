#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <map>
#include <boost/filesystem.hpp>

#include "joystick.h"

class config
{
    public:
        static config* get_instance();
        std::map<std::string,std::string> config_settings;
        virtual ~config();
        joystick jstick;
    protected:
    private:
        config();
        static bool instance_flag;
        static config *m_instance;
};

#endif // CONFIG_H
