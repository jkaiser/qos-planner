//
// Created by jkaiser on 08.03.16.
//

#include "Client.h"

#include <iostream>
#include <fstream>

#include "ReserveRequestBuilder.h"
#include "RemoveReservationRequestBuilder.h"
#include "ListReservationsRequestBuilder.h"
#include "StorageReqParser.h"

#include <spdlog/spdlog.h>

Client::Client(std::string ipPort) {
    this->ipPort = ipPort;
}

bool Client::Init() {
    context = new zmq::context_t(1);
    spdlog::get("console")->debug("connecting to server...");

    InitializeZMQSocket();
    return true;
}

void Client::InitializeZMQSocket() {
    client.reset(new zmq::socket_t(*context, ZMQ_REQ));
    std::string full_address = "tcp://" + ipPort;
    client->connect(full_address.data());

    //  Configure socket to not wait at close time
    int linger = 0;
    client->setsockopt(ZMQ_LINGER, &linger, sizeof(linger));

}

bool Client::TryReserveResources(const std::string &id, const std::string &filenames, int throughput, int duration_sec,
                                 const std::string &storage_req_file) {


    std::string filenames_to_use = filenames;
    int throughput_to_use = throughput;
    int duration_to_use = duration_sec;

    std::shared_ptr<rpc::Message> msg(new rpc::Message());
    if (!storage_req_file.empty()) {
        if (!TryParseReqFile(storage_req_file, filenames_to_use, throughput_to_use, duration_to_use)) {
            return false;
        }

    }

    spdlog::get("console")->debug("check for valid input");
    if (!IsInputValid(id, filenames_to_use, duration_to_use)) {
        spdlog::get("console")->critical("input is invalid {}, {}, {}", id, filenames_to_use, duration_to_use);
        return false;
    }

    spdlog::get("console")->debug("try to build message");

    if (!TryBuildReserveMessage(id, filenames_to_use, throughput_to_use, duration_to_use, msg)) {
        return false;
    };

    spdlog::get("console")->debug("will send: {}", msg->DebugString());

    std::string reply;
    if (!TrySendRequestAndReceiveReply(msg, reply)) {
        return false;
    }

    ProcessReply(reply);
    return true;
}

bool Client::TryBuildReserveMessage(const std::string &id, const std::string &filenames, int throughput, int duration,
                                    std::shared_ptr<rpc::Message> &msg) const {
    msg->set_type(rpc::Message::REQUEST);

    std::shared_ptr<rpc::Request> request(new rpc::Request);
    ReserveRequestBuilder rrb;
    if (!rrb.BuildRequest(id, filenames, throughput, duration, *request)) {
        spdlog::get("console")->error("couldn't parse parameters: {}", request->DebugString());
        return false;
    }

    msg->mutable_request()->CopyFrom(*request);
    return true;
}

bool Client::IsInputValid(const std::string &id, const std::string &filenames, int duration) const {
    if (filenames.empty()) {
        spdlog::get("console")->error("no id given");
        return false;
    } else if (filenames.empty()) {
        spdlog::get("console")->error("at least one file must be given");
        return false;
    } else if (duration <= 0) {
        spdlog::get("console")->error("an end date must be given");
        return false;
    }
    return true;
}

bool Client::TrySendRequestAndReceiveReply(std::shared_ptr<rpc::Message> &request, std::string &reply) {
    std::string raw_msg = request->SerializeAsString();
    if (!SendAndReceiveRequest(raw_msg, reply)) {
        spdlog::get("console")->error("getting the request the server failed");
        return false;
    }
    return true;
}

bool Client::SendAndReceiveRequest(std::string &raw_msg, std::string &reply) {

    int retries_left = request_retries;

    while (retries_left) {
        s_send(*client, raw_msg);

        while (1) {
            //  Poll socket for a reply, with timeout
            zmq::pollitem_t items[] = {{(void *) *client, 0, ZMQ_POLLIN, 0}};
            zmq::poll(&items[0], 1, request_timeout);

            //  If we got a reply, process it
            if (items[0].revents & ZMQ_POLLIN) {
                reply = s_recv(*client);
                return true;
            } else if (--retries_left == 0) {
                spdlog::get("console")->error("server seems to be offline, abandoning");
                return false;
            } else {
                spdlog::get("console")->warn("no response from server, retrying...");
                //  Old socket will be confused; close it and open a new one
                InitializeZMQSocket();
                s_send(*client, raw_msg.data());
            }
        }
    }

    return true;
}

void Client::ProcessReply(std::string &reply) {

    rpc::Message msg;
    if (!msg.ParseFromString(reply))
        spdlog::get("console")->error("got invalid answer");
    else
        spdlog::get("console")->debug("got answer: {}", msg.DebugString());
}

bool Client::RemoveReservation(const std::string &reservation_id) {
    std::shared_ptr<rpc::Message> msg(new rpc::Message());

    if (reservation_id.empty()) {
        return false;
    }

    rpc::Request request;
    RemoveReservationRequestBuilder rb;
    if (!rb.BuildRequest(reservation_id, request)) {
        return false;
    }

    msg->mutable_request()->CopyFrom(request);

    spdlog::get("console")->debug("will send: {}", msg->DebugString());

    std::string reply;
    if (!TrySendRequestAndReceiveReply(msg, reply)) {
        return false;
    }

    return true;
}

bool Client::ListReservations() {
    std::shared_ptr<rpc::Message> msg(new rpc::Message());

    rpc::Request request;
    ListReservationsRequestBuilder rb;
    if (!rb.BuildRequest(request)) {
        return false;
    }

    msg->mutable_request()->CopyFrom(request);

    spdlog::get("console")->debug("will send: {}", msg->DebugString());

    std::string reply;
    if (!TrySendRequestAndReceiveReply(msg, reply)) {
        return false;
    }

    return ProcessListReply(reply);

}

bool Client::ProcessListReply(const std::string &reply) const {
    std::shared_ptr<rpc::Message> msg(new rpc::Message());
    if (!msg->ParseFromString(reply)) {
        spdlog::get("console")->error("couldn't parse reply from server");
        return false;
    }

    if (!msg->has_reply()) {
        spdlog::get("console")->error("reply doesn't have any content!");
        return false;
    }

    if (msg->reply().rc() != 0) {
        spdlog::get("console")->error("server returned error: {}",
                                      rpc::Error::errorType_Name(msg->reply().error().error()));
        return false;
    }

    std::cout << msg->reply().return_msg() << std::endl;
    return true;
}

bool Client::TryParseReqFile(const std::string &filename, std::string &filenames_to_use, int &throughput_to_use,
                             int &duration_to_use) {


    std::ifstream is(filename, std::ifstream::in);
    if (!is.is_open()) {
        spdlog::get("console")->critical("storage req file \"{}\" doesn't exist!", filename);
        return false;
    }

    common::StorageReqParser p;
    if (!p.Parse(is)) {
        spdlog::get("console")->critical("failed to parse \"{}\"", filename);
        return false;
    }

    spdlog::get("console")->info("parsing successful");
    filenames_to_use = p.getRead_files();
    throughput_to_use = p.getThroughput_mbs();
    duration_to_use = p.getDuration();

    return true;
}

