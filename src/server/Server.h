//
// Created by jkaiser on 29.03.16.
//

#ifndef QOS_PLANNER_SERVER_H
#define QOS_PLANNER_SERVER_H

#include <memory>
#include <string>

#include <Scheduler.h>
#include <ScheduleState.h>
#include <ClusterState.h>
#include <JobMonitor.h>
#include <Lustre.h>
#include "../../../../../Library/Caches/CLion12/cmake/generated/a7b7e32b/a7b7e32b/Debug/src/common/rpc/proto/message.pb.h"

/**
 * Server s(...);
 * s.Init();
 * s.Serve(); // blocking call
 *
 * // in an interrupt handler;
 * <<Interrupt (SigTerm)>>
 * s.Teardown();    // stops all future requests.
 */
class Server {

private:
    std::string root_path;
    std::shared_ptr<common::Scheduler> scheduler;
    std::shared_ptr<common::ScheduleState> schedule_state;
    std::shared_ptr<common::ClusterState> cluster_state;
    std::shared_ptr<common::JobMonitor> job_monitor;
    std::shared_ptr<common::Lustre> lustre;

protected:
    bool ServeJobSubmission(const rpc::Message &msg);
    bool ServeJobRemove(const rpc::Message &msg);

public:

    /**
     * Constructor
     *
     * root_path:   The path where the server will store permanent data structures. If empty, it will only
     *              use in-memory data structures.
     */
    Server(const std::string &root_path);
    bool Init();
    bool TearDown();

    /**
     * ip_port: The ip-port pair to listen to. Example: "192.168.1.1:1234"
     */
    void Serve(const std::string ip_port);
};

#endif //QOS_PLANNER_SERVER_H
