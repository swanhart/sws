#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <asm/types.h>
#include <map>

class joystick
{
    public:
        joystick();
        joystick(const joystick& other);
        virtual ~joystick();
        struct js_event
        {
            __u32 time_length;
            __s16 value;
            __u8 type;
            __u8 button;
        };
        std::map<std::string,js_event> js_map;
        void js_set();
        void js_clear();
        void js_save();
    protected:

    private:
};

#endif // JOYSTICK_H
