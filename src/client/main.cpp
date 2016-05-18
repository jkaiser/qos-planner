#include <iostream>

#include <gflags/gflags.h>

#include "Client.h"


using namespace std;


DEFINE_string(ip, "localhost", "IP to connect to.");
DEFINE_string(port, "5555", "Port to use.");

// ./client <ID> <TP> <Tstart> <Tend>
int main(int argc, char* argv[]) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    cout << "Hello, World!" << endl;
    string ipPort = FLAGS_ip + ":" + FLAGS_port;

    Client c(ipPort);
    if (!c.Init()) {
        std::cout << "Client initalization failed." << std::endl;
    }

    // Parse rest of commandline
    c.requestResources("");

    return 0;
}