#ifndef WSOCKET_H
#define WSOCKET_H

#include <set>
#include <boost/bind.hpp>
#include <boost/bind/arg.hpp>
#include <boost/bind/placeholders.hpp>
#include <websocketpp/config/core.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/common/connection_hdl.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/message_buffer/alloc.hpp>
#include <websocketpp/message_buffer/message.hpp>
#include <websocketpp/server.hpp>
#include <functional>

#include "player.h"
#include "playlist.h"
#include "track.h"

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
//using websocketpp::lib::bind;
typedef server::message_ptr message_ptr;

class wsocket
{
  public:
    static wsocket* GetInstance();
    ~wsocket();
     void start();
    static void send(websocketpp::connection_hdl hdl, string str);
  protected:

  private:
    struct connection_data {
      int sessionid;
      std::string name;
    };
    wsocket();
    static wsocket* m_instance;
    server m_endpoint;
    void onOpen(websocketpp::connection_hdl);
    void onClose(websocketpp::connection_hdl);
    void onMessage(websocketpp::connection_hdl hdl, server::message_ptr msg);
    void send(websocketpp::connection_hdl hdl, string msg, websocketpp::frame::opcode::value opcode);
    static void get_details(int first, int last, playlist pl);
    player *m_player;
    bool update;
    void player_update();
    void player_handler(websocketpp::connection_hdl hdl);
    typedef std::map<websocketpp::connection_hdl,connection_data,std::owner_less<websocketpp::connection_hdl>> con_list;
    con_list m_connections;
    connection_data& get_data_from_hdl(websocketpp::connection_hdl hdl);
    bool is_listening;

};

#endif // WSOCKET_H
