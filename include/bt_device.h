#ifndef BT_DEVICE_H
#define BT_DEVICE_H

#include <string>
#include <list>

class bt_device
{
  public:
    enum class cap : short { file, audio, interface, network };
    bt_device();
    std::string id;
    std::string name;
    std::list<cap> capabilities;
    void pair();
    void disconnect(const cap);
    void disconnect();
    bool is_capable(const cap);
    unsigned int bclass;
    bool paired;
    bool connected;
    bool trusted;
    virtual ~bt_device();
    void network_connect();
    void copy_to(std::string rel_path);
    void copy_from(std::string rel_path);
    void audio_in();
    void audio_out();
  protected:

  private:
    bool file_capable, audio_capable, interface_capable, network_capable;
    void connect_file();
    void connect_audio();
    void connect_interface();
    void connect_network();
};

#endif // BT_DEVICE_H
