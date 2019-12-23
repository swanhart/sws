#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <limits.h>
#include <cstdlib>
#include <thread>
#include <algorithm>
#include <iterator>
#include <queue>
#include <experimental/filesystem>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "player.h"
#include "joystick.h"

using namespace std;

player* player::m_instance = NULL;
player::signal_t player::m_signal = player::signal_t();

player* player::get_instance()
{
  return (!m_instance) ? m_instance = new player : m_instance;
}

player::player()
{
  _config = config::get_instance();
  _db = db::get_instance();
  _dbq = db_queue::get_instance();
  db_catalog.tracks.clear();
  playlists.clear();
  playlist_tables.clear();
  while (playlist_tables.size() == 0)
  {
    playlist_tables = _db->tables();
  }
  populate_from_db();
  db_catalog = named("db_catalog");
  if (playlist_tables.size() < 1 || db_catalog.tracks.size() == 0)
  {
    populate(_config->get<string>("music_root"));
    cout << "waiting for empty q" << endl;
    while (!_db->empty_queue());
    populate_from_db();
  }
  thread t(&player::file_watcher, this);
  t.detach();
  it = playlist_tables.begin();
  string curr = _config->get<string>((const string) "current_playlist");
  current_playlist = named(curr);
  it_track = current_playlist.tracks.begin();
  long unsigned int adv = _db->get_playlist_current(current_playlist.name);
  if (adv < current_playlist.tracks.size())
    advance(it_track, _db->get_playlist_current(current_playlist.name));
  current_track.~track();
  Gst::init();
  playbin = Gst::PlayBin::create("playbin");
  joystick::listen(&joystick_event_hanlder);
  bus = playbin->get_bus();
  bus->add_watch(sigc::ptr_fun(&on_bus_message));
}

boost::signals2::connection player::listen(const player::signal_t::slot_type &subscriber)
{
  return m_signal.connect(subscriber);
}

void player::start()
{
  is_playing = false;
  play();
  loop = g_main_loop_new(NULL, false);
  g_main_loop_run(loop);
}
player::~player()
{
  populating = false;
  _config->save_config_settings();
}

void player::populate(string root)
{
  namespace fs = std::experimental::filesystem;
  if (root == "")
    root = _config->get<string>((const string) "music_root");
  cout << "Starting iterate " << root << endl;
  _dbq->monitor = false;

  for (const fs::directory_entry& p: std::experimental::filesystem::v1::__cxx11::recursive_directory_iterator(root, fs::directory_options::skip_permission_denied))
  {
    if (fs::is_regular_file(p.path()))
    {
      if (playlist::is_playlist_file(p.path().string()))
      {
        playlist pl(p.path());
        if (!exists(pl))
        {
          playlists.push_back(pl);
        }
        else
        {
          _db->add_to_playlist(pl.name, pl.tracks);
        }
        /*else
        {
          replace_if(playlists.begin(), playlists.end(), [&] (playlist x) {return x.name == pl.name;}, pl);
        }*/
        /*if (index_of(pl.name) == -1)
          playlists.push_back(pl);
        else
          playlists[index_of(pl.name)] = pl;*/
      }
      else if (track::is_audio_file(p.path().string()))
      {
        string s = p.path().string();
        if (find(db_catalog.tracks.begin(), db_catalog.tracks.end(), s) == db_catalog.tracks.end())
        {
          db_catalog.tracks.push_back(s);
          _db->insert_path(s);
        }
      }
    }
  }
  populating = false;
  _dbq->monitor = false;
  m_signal();
}

void player::populate_from_db(void)
{
  playlists.clear();
  for (string s : playlist_tables)
  {
    playlist p(s, _db->select_playlist(s));
    playlists.push_back(p);
  }
  m_signal();
}

playlist player::named(string s)
{
  for (playlist p : player::playlists)
  {
    if (s == p.name)
    {
      return p;
    }
  }
  cout << "NOTTT" << endl;
  return *playlists.begin();
}

bool player::exists(playlist s)
{
  for (playlist p: player::playlists)
  {
    if (p.name == s.name) return true;
  }
  return false;
}

void player::file_watcher(void)
{
  cout << "watcher" << endl;
  int length, i, wd;
  int fd;
  char buffer[BUF_LEN];
  string root = _config->get<string>((const string) "music_root");
  cout << "watching " << root << endl;
  fd = inotify_init();
  if (fd < 0)
  {
    cout << "Could not initialize inotify" << endl;
    return;
  }
  wd = inotify_add_watch(fd, root.c_str(), IN_ALL_EVENTS);
  while (true)
  {
    i = 0;
    length = read(fd, buffer, BUF_LEN);
    if (length < 0)
    {
      cout << "Could not add watcher to " << root << endl;
      return;
    }
    while(i < length)
    {
      struct inotify_event *event = (struct inotify_event*) &buffer[i];
      if (event->len)
      {

        cout << hex << event->mask << " " << event->name << endl;
        if (event->mask & IN_CREATE)
        {
          cout << hex << event->mask << " " << event->name << endl;
          if (event->mask & IN_ISDIR)
          {
            cout <<"DIRECTORY: " << event->name << endl;
            string d = root + (string) event->name + "/";
            cout << d << endl;
            sleep(1);
            populate(d);
          }
        }
      }
      i += EVENT_SIZE + event->len;
    }
  }
  (void) inotify_rm_watch(fd, wd);
  (void) close(fd);
}

void player::next()
{
  cout << "Next : ";
  ++it_track;
  if (it_track == current_playlist.tracks.end())
    it_track = current_playlist.tracks.begin();
  is_playing = false;
  play();
  m_signal();
}

void player::previous()
{
    if (it_track == current_playlist.tracks.begin())
      it_track = current_playlist.tracks.end();
    --it_track;
    is_playing = false;
    play();
    m_signal();
}

void player::play()
{
  //if (!is_playing)
  {
    playbin->set_state(Gst::STATE_NULL);
    current_track = track(*it_track);
    Glib::RefPtr<Gst::FileSrc> source;
    string uri = (string) "file://" + current_track.path;
    if (!playbin)
    {
      cout << "playererr: playbin not created" << endl;
      return;
    }
    playbin->set_property("uri", uri);
    playbin->set_state(Gst::STATE_PLAYING);
    _db->set_playlist_current(current_playlist.name, distance(current_playlist.tracks.begin(), it_track));
    is_paused = false;
    is_playing = true;
    cout << current_track.path << endl;
    m_signal();
  }
  bus->enable_sync_message_emission();
}

void player::pause()
{
    if (is_paused)
    {
      playbin->set_state(Gst::STATE_PLAYING);
      is_paused = false;
    }
    else
    {
      playbin->set_state(Gst::STATE_PAUSED);
      is_paused = true;
    }
    m_signal();
}

void player::stop()
{
  if (playbin)
  {
    playbin->set_state(Gst::STATE_NULL);
  }
  if (current_track.path!="")
    current_track.~track();
  is_playing = false;
  m_signal();
}

double player::position()
{
  gint64 pos;
  if (playbin->query_position(Gst::FORMAT_TIME, pos))
    return pos/1000000000;
  return 0;
}

double player::duratin()
{
  gint64 dur;
  if (playbin->query_duration(Gst::FORMAT_TIME, dur))
  {
    if (dur != current_track.duration)
      _db->update_duration(current_track.path, dur);
    return dur/1000000000;
  }
  return 0;
}

void player::playlist_up()
{
  if (it == playlist_tables.begin())
  {
    it = playlist_tables.end();
  }
  --it;
  current_playlist = named(*it);
  _config->put("current_playlist", current_playlist.name);
  is_playing = false;
  play();
  m_signal();
}

void player::playlist_down()
{
  ++it;
  if (it == playlist_tables.end())
  {
    it = playlist_tables.begin();
  }
  current_playlist = named(*it);
  _config->put("current_playlist", current_playlist.name);
  is_playing = false;
  play();
  m_signal();
}

void player::set_current_playlist(string name)
{
  for (playlist p : playlists)
  {
    if (p.name == name)
      current_playlist = p;
  }
  is_playing = false;
  play();
}
//TODO Make delegate so that static method not necessary for CB
bool player::on_bus_message(const Glib::RefPtr<Gst::Bus>&, const Glib::RefPtr<Gst::Message>& message)
{
  player *plr = player::get_instance();
  switch(message->get_message_type())
  {
  case Gst::MESSAGE_EOS:
    cout << "EOS" << endl;
    plr->next();
    return false;
  case Gst::MESSAGE_ERROR:
    cout << "buserror: " << Glib::RefPtr<Gst::MessageError>::cast_static(message)->parse_debug() << std::endl;
    return false;
  default:
    break;
  }
  return true;
}

void player::on_decoder_pad_added(const Glib::RefPtr<Gst::Pad>& pad)
{
   Glib::ustring caps_format = pad->get_current_caps()->to_string().substr(0, 5);
  Glib::RefPtr<Gst::Bin> parent = parent.cast_dynamic(pad->get_parent()->get_parent());
  cout << "Pad added" << endl;
  if (!parent)
  {
    std::cerr << "cannot get parent bin" << std::endl;
    return;
  }
  Glib::ustring factory_name;

  if (caps_format == "video")
  {
    factory_name = "autovideosink";
  }
  else if (caps_format == "audio")
  {
    factory_name = "autoaudiosink";
  }
  else
  {
    std::cerr << "unsupported media type: " << pad->get_current_caps()->to_string() << std::endl;
    return;
  }

  Glib::RefPtr<Gst::Element> element = Gst::ElementFactory::create_element(factory_name);

  if (!element)
  {
    std::cerr << "cannot create element " << factory_name << std::endl;
    return;
  }

  try
  {
    parent->add(element);
    //element->set_state(STATE_PLAYING);
    //pad->link(element->get_static_pad("sink"));
  }
  catch (const std::runtime_error& err)
  {
    std::cerr << "cannot add element to a bin: " << err.what() << std::endl;
  }
  Glib::RefPtr<Gst::Caps> caps = pad->get_current_caps();
  caps = caps->create_writable();
}

const string player::json()
{
  player *p = player::get_instance();
  using namespace rapidjson;
  Document d, curr_t, curr_p, next_t, prev_t;
  curr_t.Parse<0>(p->current_track.json().c_str());
  curr_t.AddMember("IsPaused", p->is_paused, curr_t.GetAllocator());
  next_t.Parse<0>(p->peak_next().json());
  prev_t.Parse<0>(p->peak_previous().json());
  d.SetObject();
  Document::AllocatorType& alloc = d.GetAllocator();
  d.AddMember("CurrentTrack", curr_t, alloc);
  d.AddMember("NextTrack", next_t, alloc);
  d.AddMember("PreviousTrack", prev_t, alloc);
  d.AddMember("CurrentPlaylist", p->current_playlist.name, alloc);
  Value plists(kArrayType);
  for (playlist pls : p->playlists)
  {
    Value v;
    v.SetString(pls.name.c_str(), alloc);
    plists.PushBack(v, alloc);
  }
  d.AddMember("Playlists", plists, alloc);
  StringBuffer sb;
  Writer<StringBuffer> writer(sb);
  d.Accept(writer);
  return sb.GetString();
}

const string player::position_json()
{
  using namespace rapidjson;
  StringBuffer sb;
  Writer<StringBuffer> w(sb);
  w.StartObject();
  w.Key("Position"); w.Double(position());
  w.Key("Duration"); w.Double(duratin());
  w.EndObject();
  return string(sb.GetString());
}


void player::joystick_event_hanlder()
{
  player *p = player::get_instance();
  switch (joystick::get_event())
  {
  case joystick::ACTION_TYPE::next:
    p->next();
    break;
  case joystick::ACTION_TYPE::previous:
    p->previous();
    break;
  case joystick::ACTION_TYPE::playlist_up:
    p->playlist_up();
    break;
  case joystick::ACTION_TYPE::playlist_down:
    p->playlist_down();
  case joystick::ACTION_TYPE::stop:
    p->stop();
    break;
  case joystick::ACTION_TYPE::play:
    p->play();
    break;
  default:
    break;
  }
}

track player::peak_next()
{
  list<string>::iterator iter = current_playlist.tracks.begin();
  advance(iter, current_playlist.index_of(*it_track));
  iter++;
  if (iter == current_playlist.tracks.end())
    return track(*current_playlist.tracks.begin());
  else
    return track(*iter);
}

track player::peak_previous()
{
  list<string>::iterator iter = current_playlist.tracks.begin();
  advance(iter, current_playlist.index_of(*it_track));
  if (iter == current_playlist.tracks.begin())
    iter = current_playlist.tracks.end();
  iter--;
  return track(*iter);
}
