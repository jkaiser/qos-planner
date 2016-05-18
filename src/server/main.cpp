#include <iostream>

#include <gflags/gflags.h>

#include "../../../../../Library/Caches/CLion12/cmake/generated/a7b7e32b/a7b7e32b/Debug/src/common/rpc/proto/message.pb.h"

#include <zhelpers.hpp>

#include "Server.h"

using namespace std;


void executeServer(const string ipPort) {
    srandom ((unsigned) time (NULL));


    zmq::context_t context(1);
    zmq::socket_t server(context, ZMQ_REP);
    server.bind(ipPort);

    while (1) {
        std::string request = s_recv (server);

        std::cout << "I: normal request (" << request << ")" << std::endl;

        rpc::Message msg;
        msg.mutable_reply()->set_rc(1);

        s_send (server, msg.SerializeAsString());
    }
}


DEFINE_string(ip, "*", "IP to listen to.");
DEFINE_string(port, "5555", "Port to use.");

int main(int argc, char* argv[]) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    string ip_port = FLAGS_ip + ":" + FLAGS_port;

    cout << "Hello, World!" << endl;

//    executeServer(ipPort);

    Server s("");
    s.Init();
    s.Serve(ip_port)

    return 0;
}