/*
 * Copyright (c) 2017 Jürgen Kaiser
 */
/*
 * GPL HEADER START
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 only,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details (a copy is included
 * in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU General Public License
 * version 2 along with this program; If not, see <http://www.gnu.org/licenses/>.
 *
 * GPL HEADER END
 */


#include <iostream>

#include <gflags/gflags.h>
#include <spdlog/spdlog.h>

#include "Client.h"


DEFINE_string(ip, "localhost", "IP to connect to.");
DEFINE_string(port, "5555", "Port to use.");
DEFINE_string(id, "", "ID of the job");
DEFINE_string(filenames, "", "comma seperated list of files");
DEFINE_int32(throughput, 0, "requested throughput in MB/s");
DEFINE_int32(duration, 0, "duration of the reservation in seconds");
DEFINE_string(storage_req_file, "", "file that includes the storage requirements definitions");
DEFINE_bool(v, false, "verbose");

using namespace std;

void setupLogging(bool verbose) {

    auto console = spdlog::stdout_logger_mt("console");
    spdlog::set_pattern("[%H:%M:%S] [%l] %v");

    if (verbose)
        spdlog::set_level(spdlog::level::debug);
    else
        spdlog::set_level(spdlog::level::info);

}

void setupUsageString(const std::string app_name) {

    stringstream ss;
    ss << app_name << " <cmd> <options>" << std::endl;
    ss << "valid commands are:" << std::endl;
    ss << "    - reserve: Try to reserve the resources as defined in the options" << std::endl;
    ss << "    - remove: Try to remove the resources reservation (use -id <ID> to specify)" << std::endl;
    ss << "    - ls: List all reservations" << std::endl;

    gflags::SetUsageMessage(ss.str());
}

int execute(Client &client, const std::string &cmd) {

    bool success = false;
    if (cmd.compare("reserve") == 0) {
        success = client.TryReserveResources(FLAGS_id, FLAGS_filenames, FLAGS_throughput, FLAGS_duration, FLAGS_storage_req_file);
    } else if (cmd.compare("remove") == 0) {
        success = client.RemoveReservation(FLAGS_id);
    } else if (cmd.compare("ls") == 0) {
        success = client.ListReservations();
    } else {
        spdlog::get("console")->critical("unknown command: {}", cmd);
    }

    if (success)
        return 0;
    else
        return -1;
}


// ./client -filenames /foo/bar,/foo/bar2 -throughput 10 -Tend 10
// ./client <ID> <TP> <Tstart> <Tend>
int main(int argc, char *argv[]) {

    setupUsageString(std::string(argv[0]));
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    setupLogging(FLAGS_v);

    if (argc <2) {
        spdlog::get("console")->error("no command given");
        return -1;
    }

    string ipPort = FLAGS_ip + ":" + FLAGS_port;
    Client c(ipPort);
    if (!c.Init()) {
        spdlog::get("console")->critical("initialization failed");
    }

    std::string command = argv[1];
    int ret = execute(c, command);

    gflags::ShutDownCommandLineFlags();
    return ret;
}