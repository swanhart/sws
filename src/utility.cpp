#include "utility.h"

#include <iostream>

using namespace std;
namespace fs = boost::filesystem;

namespace utility
{

    size_t utf8_leng(const std::string& s)
    {
        unsigned char c;
        size_t i, ix, q;
        for (q=0, i=0, ix=s.length(); i < ix; i++, q++)
        {
            c = (unsigned char) s[i];
            if (c>=0 && c<=128) i+=0;
            else if ((c & 0xE0) == 0xC0) i+=1;
            else if ((c & 0xF0) == 0xE0) i+=2;
            else if ((c & 0XF8) == 0xF0) i+=3;
            else return 0;
        }
        return q;
    }

    string file_hash(const std::string s)
    {
        MD5 md5;
        return (string) md5.digestFile((char*) s.c_str());
    }

    std::string mime_info(const std::string* fn)
    {
        magic_t magt = magic_open(MAGIC_CONTINUE | MAGIC_ERROR | MAGIC_MIME);
        magic_load(magt, NULL);
        return  (string) magic_file(magt, (fn->c_str()));
    }

    void populate()
    {
        fs::path p("/media");
        boost::thread t(populate_thread, &p);
        t.join();
    }

    void populate_thread(fs::path* root_path)
    {
        fs::path p = *root_path;
        fs::recursive_directory_iterator itr(p), ieod;
        db m_db;
        BOOST_FOREACH(fs::path const &p, make_pair(itr, ieod))
        {
            if (fs::is_regular(p))
            {
                if (boost::algorithm::ends_with(p.c_str(), ".m3u"))
                {
                    boost::thread t(db::add_playlist, (string*) &p);
                    t.join();
                }
                else if (mime_info(&p.string()).find("audio") != string::npos)
                {
                    db::add_track((string*) &p);
                }
            }
        }
    }
    template < typename T > void shuffle(std::list<T>& lst )
    {
      vector<reference_wrapper<const T> > vec(lst.begin(), lst.end());
      shuffle(vec.begin()m vec.end(), mt19937{random_device()()});
      list<T> shuffled_list{vec.begin(), vec.end()};
      lst.swap(shuffled_list);
    }
}
