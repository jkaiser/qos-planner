#include <iostream>

#include <gflags/gflags.h>
#include <spdlog/spdlog.h>

#include "Client.h"


using namespace std;

void setupLogging(bool verbose) {

    auto console = spdlog::stdout_logger_mt("console", false);
    spdlog::set_pattern("[%H:%M:%S] [%l] %v");

    if (verbose)
        spdlog::set_level(spdlog::level::debug);
    else
        spdlog::set_level(spdlog::level::info);

}

DEFINE_string(ip, "localhost", "IP to connect to.");
DEFINE_string(port, "5555", "Port to use.");
DEFINE_bool(v, true, "verbose");


// ./client <ID> <TP> <Tstart> <Tend>
int main(int argc, char *argv[]) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    setupLogging(FLAGS_v);

    string ipPort = FLAGS_ip + ":" + FLAGS_port;
    Client c(ipPort);
    if (!c.Init()) {
        std::cout << "Client initalization failed." << std::endl;
    }

    // Parse rest of commandline
    c.requestResources("");

    return 0;
}