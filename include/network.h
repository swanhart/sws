#ifndef NETWORK_H
#define NETWORK_H


class network
{
  public:
    network();
    virtual ~network();
    network(const network& other);
    static bool is_connected();
  protected:
  struct header
  {
    char type;
    int length;
  };

  private:
};

#endif // NETWORK_H
