#include <iostream>

#include <gflags/gflags.h>

#include "../common/rpc/proto/message.pb.h"

#include <spdlog/spdlog.h>
#include <zhelpers.hpp>

#include "Server.h"
#include <Planner.h>

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


void setupLogging(bool verbose) {

    auto console = spdlog::stdout_logger_mt("console", false);
    spdlog::set_pattern("[%H:%M:%S] [%l] %v");

    if (verbose)
        spdlog::set_level(spdlog::level::debug);
    else
        spdlog::set_level(spdlog::level::info);

}

DEFINE_string(ip, "*", "IP to listen to.");
DEFINE_string(port, "5555", "Port to use.");
DEFINE_string(ost_limits, "", "Config file defining the max MB/s per ost.");
DEFINE_bool(v, false, "verbose");

int main(int argc, char* argv[]) {
//    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);


    setupLogging(FLAGS_v);
    string ip_port = FLAGS_ip + ":" + FLAGS_port;

    spdlog::get("console")->info("loggers can be retrieved from a global registry using the spdlog::get(logger_name) function");

    cout << "Hello, World!" << endl;
//    LOG(ERROR) << "Hello World from the logger";

    std::string root_path = "";
    std::shared_ptr<common::Planner> planner(new common::Planner(root_path, FLAGS_ost_limits));
    if (!planner->Init()) {
        spdlog::get("console")->info("Initializing planner failed");
        return -1;
    }

//    executeServer(ipPort);

    Server s(ip_port,"", planner);
    s.Init();
    s.Serve();

    if (!planner->TearDown()) {
        return -1;
    }


    return 0;
}