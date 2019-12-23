#ifndef SPOTIFY_H
#define SPOTIFY_H

#include <string>

#include "config.h"
#include "track.h"

class spotify
{
  public:
    spotify();
    virtual ~spotify();
    track suggest();
    void download();
    std::list<track> get_favoirites();
    std::list<track> get_album(std::string name);
    std::list<track> get_tracks(std::string search_str, std::string search_type="track"); //comma delimited album, artist, playlist, track
    std::list<track> get_artits(std::string name);
    track get_track(std::string id);
  protected:

  private:
    config *conf;
    void renew_token();
    std::string token;


};

#endif // SPOTIFY_H
