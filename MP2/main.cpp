#include <iostream>
#include <stdlib.h>  
#include <string>
#include <fstream>

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <assert.h>
#include <stdlib.h>

#include "peer.h"
#include "hash.h"
#include "peerMgr.h"
using namespace std;

int main(int argc, char *argv[])
{
    /* Make sure argv has a config path */
    assert(argc == 2);
    int ret;

    /* Load config file */
    Peer* cfg = new Peer();
    PeerManager* mgr = new PeerManager();
    if(!mgr->init_config(cfg, argv[1]))
        exit(EXIT_FAILURE);

    assert(cfg->getPeersNum() >= 0);
    // assert(cfg->getHostName() != NULL);
    // assert(cfg->getPeersName() != NULL);

    /* Create host UNIX socket */
    struct sockaddr_un sock_addr;
    int connection_socket;
    unlink(cfg->getHostName().c_str());
    connection_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (connection_socket < 0)
        exit(EXIT_FAILURE);
    memset(&sock_addr, 0, sizeof(struct sockaddr_un));
    // cout << "init socket:" << cfg->getHostName() << " at fd#" << connection_socket <<endl;
    
    /* Bind socket to a name. */
    sock_addr.sun_family = AF_UNIX;
    strncpy(sock_addr.sun_path, cfg->getHostName().c_str(), sizeof(sock_addr.sun_path) - 1);

    ret = bind(connection_socket,
               (const struct sockaddr *) &sock_addr,
               sizeof(struct sockaddr_un));

    if (ret == -1){
        cout << "binding error\n";
        exit(EXIT_FAILURE);
    }
    

    /* Prepare for accepting connections */
    ret = listen(connection_socket, 20);
    if (ret == -1)
        exit(EXIT_FAILURE);


    /* Enter the serving loop.
     * It calls select() to check if any file descriptor is ready.
     * You may look up the manpage select(2) for details.
     */

    int max_fd = sysconf(_SC_OPEN_MAX);

    fd_set read_set;
    fd_set write_set;

    FD_ZERO(&read_set);
    FD_ZERO(&write_set);

    FD_SET(STDIN_FILENO, &read_set);       /* check for user input */
    FD_SET(connection_socket, &read_set);  /* check for new peer connections */
    
    cfg->readRecord(cfg->getRepoName());
    cfg->communicate(true);
    while (1)
    {
        struct timeval tv;
        fd_set working_read_set, working_write_set;

        memcpy(&working_read_set, &read_set, sizeof(working_read_set));
        memcpy(&working_write_set, &write_set, sizeof(working_write_set));

        ret = select(max_fd, &working_read_set, &working_write_set, NULL, &tv);
        if (ret < 0)             /* We assume it doesn't happen. */
            exit(EXIT_FAILURE);

        if (ret == 0)            /* No fd is ready */
            continue;

        if (FD_ISSET(STDIN_FILENO, &working_read_set))
        {
            /* TODO Handle user commands */
            bool b = mgr->parseCommand(cin, cfg);
            cfg->needUpdate(b, "");
        }

        if (FD_ISSET(connection_socket, &working_read_set))
        {
            int peer_socket = accept(connection_socket, NULL, NULL);
            if (peer_socket < 0)
                exit(EXIT_FAILURE);
            cfg->recv_commit(peer_socket);
            // cfg->needUpdate("",);
            /* TODO Store the peer fd */
            close(peer_socket);
        }
        
        cfg->readRecord(cfg->getRepoName());
        cfg->communicate(false);
    }

    /* finalize */
    mgr->destroy_config(cfg);
    close(connection_socket);
    return 0;
}

