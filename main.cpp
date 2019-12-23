#include <iostream>
#include <list>
#include <signal.h>
#include <thread>

#include "console.h"
#include "utility.h"
#include "track.h"
#include "joystick.h"
#include "bluetooth.h"
#include "playlist.h"
#include "db.h"
#include "player.h"
#include "player.h"
#include "wsocket.h"


using namespace std;

void ding(int s);
void wss();
void pl();

int main(int arvc, char **argv)
{
  //utility::init(argv[0]);
//joystick::load_config();
//  joystick::save_config();
  //while (true);
  //joystick::load_config();
  //out{} << c->get<string>("db_file") << endl;
  //joystick::listen(&ding);
  //bluetooth *bt = bluetooth::get_instance();
 // while(true);
//  signal(SIGINT, ding);
  //pid_t pid = fork();
  //thread w(wss);
  //w.detach();
  //thread p(pl);
  //p.detach();
  //cout << "FF" << eloop = g_main_loop_new(NULL, true);
  //GMainLoop *loop;
  //g_main_loop_run(loop);ndl;
  //thread w(wss);
  //w.detach();
  //player *p = player::get_instance();
  //p->start();
  player::get_instance()->start();
  wsocket::GetInstance()->start();
  for (;;)
  {
    sleep(5);
    cout << player::get_instance()->position_json() << endl;
  }


  //GMainLoop *loop = g_main_loop_new(NULL, FALSE);
  //wsocket *ws = wsocket::GetInstance();
  //ws->start();
  //thread t(&ding, 1);
  //t.detach();
  //g_main_loop_run(loop);
  return 0;
}

void ding(int s)
{
  wsocket *w = wsocket::GetInstance();
  w->~wsocket();
}

void wss()
{
  wsocket *w = wsocket::GetInstance();
  w->start();
}

void pl()
{
  player *p = player::get_instance();
  p->start();
}
