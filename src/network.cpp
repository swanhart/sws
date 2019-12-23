#include <curl/curl.h>

#include "network.h"

network::network()
{
  //ctor
}

network::~network()
{
  //dtor
}

network::network(const network& other)
{
  //copy ctor
}

bool network::is_connected()
{
  CURL *curl;
  CURLcode res;
  bool internet = false;

  curl = curl_easy_init();
  if (curl)
  {
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 2);
    curl_easy_setopt(curl, CURLOPT_URL, "www.google.com");
    curl_easy_setopt (curl, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt (curl, CURLOPT_CONNECT_ONLY, 1L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    res = curl_easy_perform(curl);
    if (res == CURLE_OK)
    {

      internet = true;
    }
  }
  curl_easy_cleanup(curl);
  return internet;
}
