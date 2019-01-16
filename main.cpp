#include <iostream>

#include "utility.h"
#include "track.h"
#include "joystick.h"

using namespace std;

void ding();



int main(int arvc, char **argv)
{
  //utility::init(argv[0]);
//  joystick::listen(&ding);
//joystick::load_config();
//  joystick::save_config();
  //while (true);
  //joystick::load_config();
  config *c = config::get_instance();
  cout << c->get<string>("db_file") << endl;
  joystick::listen(&ding);
  while(true);

  return 0;
}

void ding()
{
  cout << joystick::get_event() << endl;
}
