#include <iostream>

#include <gflags/gflags.h>

#include "../common/rpc/proto/message.pb.h"

#include <spdlog/spdlog.h>
#include <zhelpers.hpp>

#include <Planner.h>
#include "Server.h"


using namespace std;

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
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    setupLogging(FLAGS_v);
    string ip_port = FLAGS_ip + ":" + FLAGS_port;


    std::string root_path = "";
    std::shared_ptr<common::Planner> planner(new common::Planner(root_path, FLAGS_ost_limits));
    if (!planner->Init()) {
        spdlog::get("console")->info("Initializing failed");
        return -1;
    }

    Server s(ip_port, "", planner);
    s.Init();
    s.Serve();

    if (!planner->TearDown()) {
        return -1;
    }


    return 0;
}