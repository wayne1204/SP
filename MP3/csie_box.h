#include <string>
using namespace std;
#define BUFSIZE 256

class csie_box{
public:
    void parse(const char* fname);
    void setPid(int id) { peer_pid = id; }
    int getPid() { return peer_pid; }
    void sig_handler(int sig);
    void init_client();
    
    void createDir(string dir_name);
    void removeDir(string dir_name);
    string getFifoPath() {return fifo_path; }
    string getDirectory() {return directory; }

    void sendFiles(int fd, string subpath);
    void readFiles(int fd);
    
private:
    int peer_pid;
    string fifo_path;
    string directory;
};