#ifndef DB_QUEUE_H
#define DB_QUEUE_H

#include <queue>
#include <mutex>
#include <boost/thread.hpp>
#include <condition_variable>
#include "db.h"



class db_queue
{
  public:
    static db_queue* get_instance();
    ~db_queue();
    void push_song(db::song s);
    void push_duration(db::song* s);
    bool monitor;
  protected:

  private:
    db_queue();
    static db_queue* m_instance;
    condition_variable cv;
    std::mutex mut_song;
    std::mutex mut_duration;
    queue<db::song> song_queue;
    queue<db::song*> duration_queue;
    void monitor_update(void);
    void monitor_duration(void);
    db *_db;
};

#endif // DB_QUEUE_H
