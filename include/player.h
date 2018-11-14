#ifndef PLAYER_H
#define PLAYER_H

#include <gstreamermm-1.0/gstreamermm.h>
#include <iostream>
#include <glibmm-2.4/glibmm/main.h>
#include <gstreamermm/playbin.h>


#include "config.h"
#include "playlist.h"
#include "track.h"

class player
{
    public:
        player();
        player(track);
        player(playlist);
        virtual ~player();
        player(const player& other);
        void next();
        void previous();
        void stop();
        void play();
        void shuffle();
        void sort();
        void say();
        void Add(track);
        void Remove(track);
        void Search(std::string);
        playlist current_playlist;
        track current_track;

    protected:
    private:
        bool connected_to_network;
        Glib::RefPtr<Glib::MainLoop> player_loop;
        bool on_bus_message(const Glib::RefPtr<Gst::Bus>&, const Glib::RefPtr<Gst::Message>&);
        Glib::ustring uri;
        Glib::RefPtr<Gst::Bus> bus;
        bool is_playing;
        bool is_paused;
};

#endif // PLAYER_H