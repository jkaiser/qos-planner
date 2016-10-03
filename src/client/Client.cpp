//
// Created by jkaiser on 08.03.16.
//

#include "Client.h"

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
    client.reset(new zmq::socket_t (*context, ZMQ_REQ));
    std::string full_address = "tcp://" + ipPort;
    client->connect (full_address.data());

    //  Configure socket to not wait at close time
    int linger = 0;
    client->setsockopt (ZMQ_LINGER, &linger, sizeof (linger));

}

bool Client::requestResources(std::string request) {

    rpc::Message msg = buildMessage();
    spdlog::get("console")->debug("will send: {}", msg.DebugString());

    std::string reply;
    std::string raw_msg = msg.SerializeAsString();
    if (!sendAndReceiveRequest(raw_msg, reply)) {
        spdlog::get("console")->error("getting the request the server failed");
        return false;
    }

    ProcessReply(reply);
    return true;
}

rpc::Message Client::buildMessage() const {
    rpc::Message msg;
    msg.set_type(rpc::Message::REQUEST);
    auto req = msg.mutable_request();
    req->set_type(rpc::Request_Type_RESERVE);

    auto res1 = req->mutable_resourcerequest()->add_requestedresources();
    res1->set_file("foo");
    res1->set_throughputmb(100);
    res1->set_tstart(0);
    res1->set_tstop(42);
    return msg;
}

bool Client::sendAndReceiveRequest(std::string &raw_msg, std::string &reply) {

    int retries_left = request_retries;

    while (retries_left) {
        s_send (*client, raw_msg);

        while (1) {
            //  Poll socket for a reply, with timeout
            zmq::pollitem_t items[] = { { (void *) *client, 0, ZMQ_POLLIN, 0 } };
            zmq::poll (&items[0], 1, request_timeout);

            //  If we got a reply, process it
            if (items[0].revents & ZMQ_POLLIN) {
                reply = s_recv (*client);
                return true;
            } else if (--retries_left == 0) {
                spdlog::get("console")->error("server seems to be offline, abandoning");
                return false;
            } else {
                spdlog::get("console")->warn("no response from server, retrying...");
                //  Old socket will be confused; close it and open a new one
                InitializeZMQSocket();
                s_send (*client, raw_msg.data());
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

