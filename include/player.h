#ifndef PLAYER_H
#define PLAYER_H
#include <gstreamermm.h>
#include <glib.h>
#include <iostream>
#include <iomanip>
#include <glibmm/main.h>
#include <unistd.h>
#include <list>
#include <thread>
#include <boost/signals2/signal.hpp>
#include "config.h"
#include "playlist.h"
#include "db_queue.h"
#include "db.h"
#include <boost/signals2/signal.hpp>
#define MAX_EVENTS 1024
#define LEN_NAME 4096
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( MAX_EVENTS * ( EVENT_SIZE + LEN_NAME ))

class player
{
public:
  static player* get_instance();
  virtual ~player();
  void next();
  void previous();
  track peak_next();
  track peak_next(list<string>::iterator i);
  track peak_previous();
  track peak_previous(list<string>::iterator i);
  void stop();
  void start();
  void play();
  void playlist_up();
  void playlist_down();
  void shuffle();
  void pause();
  void sort();
  double position();
  double duratin();
  void say();
  void set_current_playlist(string name);
  track Search(std::string);
  list<playlist> playlists;
  playlist current_playlist;
  track current_track;
  playlist db_catalog;
  static const string json();
  static const string json_doc();
  const string position_json();
  typedef boost::signals2::signal<void ()> signal_t;
  static boost::signals2::connection listen(const signal_t::slot_type &subscriber);
  GMainLoop *loop;

protected:
private:
  player();
  player(track);
  player(playlist);
  static player* m_instance;
  bool is_playing;
  bool is_paused;
  char delay();
  void populate(string root="");
  void populate_from_db(void);
  playlist named(string s);
  bool exists(playlist s);
  config *_config;
  db *_db;
  db_queue *_dbq;
  bool populating;
  void file_watcher(void);
  list<string>::iterator it;
  list<string>::iterator it_track;
  list<string> playlist_tables;
  Glib::RefPtr<Gst::PlayBin> playbin;
  Glib::RefPtr<Gst::Bus> bus;
  static bool on_bus_message(const Glib::RefPtr<Gst::Bus>&, const Glib::RefPtr<Gst::Message>& message);
  static void on_decoder_pad_added(const Glib::RefPtr<Gst::Pad>& pad);
  static void joystick_event_hanlder();
  static signal_t m_signal;
};

#endif // PLAYER_H
