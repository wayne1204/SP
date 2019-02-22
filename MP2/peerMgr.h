#include <string>
#include <vector>
#include "peer.h"
using namespace std;
/* This header provides example code to handle configurations.
 * The config struct stores whole socket path instead of names.
 * You are free to modifiy it to fit your needs.
 */

class PeerManager{
public:
    PeerManager(){}
    bool init_config(Peer* cfg, string path);
    void destroy_config(Peer* cfg);
    
    bool parseCommand(istream& ifs, Peer* cfg);
    bool copy(string& src, string& dest, Peer* cfg);
    bool move(string& src, string& dest, Peer* cfg);
    bool removeFile(string& src, Peer* cfg);

    bool split(string& line, string& token, size_t& pos);
    
private:
    vector<string> current_files;
    
};