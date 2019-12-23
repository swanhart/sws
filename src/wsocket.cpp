#include <iostream>
#include <rapidjson/document.h>

#include "joystick.h"
#include "wsocket.h"

using namespace std;

wsocket* wsocket::m_instance = NULL;

wsocket* wsocket::GetInstance()
{
  return ! m_instance ? m_instance = new wsocket() : m_instance;
}

wsocket::wsocket()
{
  update = false;
  is_listening = false;
  m_player = player::get_instance();
  m_player->listen(bind(&wsocket::player_update, this));
  m_endpoint.set_error_channels(websocketpp::log::elevel::all);
  m_endpoint.set_access_channels(websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload);
  m_endpoint.init_asio();
  m_endpoint.set_open_handler(bind(&wsocket::onOpen, this, websocketpp::lib::placeholders::_1));
  m_endpoint.set_close_handler(bind(&wsocket::onClose, this, websocketpp::lib::placeholders::_1));
  m_endpoint.set_message_handler(bind (&wsocket::onMessage, this, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));
  while (!is_listening)
  {
    try
    {
      m_endpoint.listen(9002);
      is_listening = true;
    }
    catch (websocketpp::exception const &e)
    {
      cout << "Error Listening: " << e.what() << endl;
      sleep(3);
    }
  }
}

void wsocket::start()
{
  while (!is_listening);
  m_endpoint.start_accept();
  m_endpoint.run();
}

void wsocket::player_update()
{
  update = true;
}

void wsocket::player_handler(websocketpp::connection_hdl hdl)
{
  cout << "hdl: " << hdl.lock().get() << endl;
  for (;;)
  {
    if (update)
    {
      websocketpp::lib::error_code ec;
      m_endpoint.send(hdl, m_player->json(), websocketpp::frame::opcode::TEXT, ec);
      if (ec)
      {
        cout << "Error sending on websocket: " << ec.message() << endl;
      }
      update = false;
    }
  }
}

wsocket::~wsocket()
{
  m_endpoint.stop_listening();
  m_endpoint.stop();
  //m_endpoint.~endpoint();
}

void wsocket::send(websocketpp::connection_hdl hdl, string msg, websocketpp::frame::opcode::value opcode)
{
  try
  {
    m_endpoint.send(hdl, msg.c_str(), websocketpp::frame::opcode::TEXT);
  }
  catch (websocketpp::exception const & e)
  {
    cout << "Error sending: " << e.what() << endl;
    //cout << msg << endl;

  }
  catch (...)
  {
    cout << "Error Sending: Unknown" << endl;
    //cout << msg << endl;
  }
}

void wsocket::onOpen(websocketpp::connection_hdl hdl)
{
  thread t(&wsocket::player_handler, this, hdl);
  t.detach();
}

void wsocket::onClose(websocketpp::connection_hdl hdl)
{
  m_connections.erase(hdl);
}

void wsocket::onMessage(websocketpp::connection_hdl hdl, server::message_ptr msg)
{
  using namespace rapidjson;
  websocketpp::lib::error_code ec;
  player *p = player::get_instance();
  Document d;
  cout << "Opcode: " << msg->get_opcode() << endl;
  string rcv = msg->get_payload();
  cout << "Message received " << hdl.lock().get() << " " << endl;
  d.Parse(rcv.c_str());
  if (rcv.length() == 0 || d.HasParseError())
  {
    cout << "Error Parsing: " << rcv << endl;
    m_endpoint.send(hdl, p->json(), websocketpp::frame::opcode::TEXT, ec);
    if (ec)
    {
      cout << "Error sending on websocket: " << ec.message() << endl;
    }
    //cout << p->current_track.json() << endl;
  }
  else if (d.HasMember("Action"))
  {
    if (d["Action"].IsString())
    {
      string action = d["Action"].GetString();
      if (joystick::actions.find(action) != joystick::actions.end())
      {
        switch(joystick::actions.at(action))
        {
        case joystick::play:
          p->play();
          break;
        case joystick::stop:
          p->stop();
          break;
        case joystick::pause:
          p->pause();
          break;
        case joystick::next:
          p->next();
          break;
        case joystick::previous:
          p->previous();
          break;
        case joystick::playlist_down:
          p->playlist_down();
          break;
        case joystick::playlist_up:
          p->playlist_up();
          break;
        default:
          break;
        }
      }
      else if (action == (string) "Position")
      {
        m_endpoint.send(hdl, p->position_json(), websocketpp::frame::opcode::TEXT ,ec);
        if (ec)
        {
          cout << "Error sending on websocket: " << ec.message() << endl;
        }
      }
    }
  }
}

void wsocket::get_details(int first, int last, playlist pl)
{

}

void wsocket::send(websocketpp::connection_hdl hdl, string str)
{
}
