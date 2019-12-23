#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include "network.h"

using namespace std;
using boost::asio::ip::tcp;

class server : public boost::enable_shared_from_this<server>
{
public:
  typedef boost::shared_ptr<server> pointer;
  static pointer pointer_create(boost::asio::io_service &io_service);
  tcp::socket &socket();
  void start();

protected:

private:
server(boost::asio::io_service& io_service) : _socket (io_service) {};
void handle_write(const boost::system::error_code&);
tcp::socket _socket;
string message;

};

#endif // SERVER_H
