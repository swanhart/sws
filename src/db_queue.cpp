#include "utility.h"
#include "db_queue.h"

using namespace std;

db_queue* db_queue::m_instance = NULL;


db_queue::db_queue()
{
  monitor = true;
  _db = db::get_instance();
  thread d(&db_queue::monitor_duration, this);
  d.detach();
  thread u(&db_queue::monitor_update, this);
  u.detach();
}

db_queue::~db_queue()
{
  monitor = false;
}

db_queue* db_queue::get_instance()
{
  return (!m_instance) ? m_instance = new db_queue : m_instance;
}

void db_queue::push_song(db::song s)
{
  unique_lock<mutex> lock(mut_song);
  song_queue.push(s);
  lock.unlock();
  cv.notify_all();
}

void db_queue::push_duration(db::song* s)
{
  unique_lock<mutex> lock(mut_duration);
  duration_queue.push(s);
  lock.unlock();
  cv.notify_all();
}

void db_queue::monitor_update(void)
{
  unique_lock<mutex> lock(mut_song);
  while (true)
  {
    while (monitor)
    {
      cv.wait(lock, [this] {
            return (song_queue.size() || !monitor);
            });

      if (monitor && song_queue.size())
      {
        db::song s = song_queue.front();
        song_queue.pop();
        lock.unlock();
        _db->update_track(s);
        lock.lock();
      }
    }
  }
}

void db_queue::monitor_duration(void)
{
  unique_lock<mutex> lock(mut_duration);
  while (true)
  {
    do
    {
      cv.wait(lock, [this] {
              return (duration_queue.size() || !monitor);
              });

      if (monitor && duration_queue.size())
      {
        db::song* s = duration_queue.front();
        duration_queue.pop();
        lock.unlock();
        _db->update_duration(s);
        lock.lock();
      }
    } while (monitor);
  }
}
