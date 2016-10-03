//
// Created by jkaiser on 08.03.16.
//

#include "Client.h"

Client::Client(std::string ipPort) {
    this->ipPort = ipPort;
}

bool Client::Init() {
    context = new zmq::context_t(1);
    std::cout << "I: connecting to server…" << std::endl;

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

    std::cout << "will send: " << msg.DebugString() << std::endl;
    std::string reply;

    std::string raw_msg = msg.SerializeAsString();
    if (!sendAndReceiveRequest(raw_msg, reply)) {
        std::cout << "getting the request the server failed" << std::endl;
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
        std::cout << "I: sending msg..." << std::endl;

        s_send (*client, raw_msg);
        std::cout << "I: sent msg" << std::endl;

        while (1) {
            //  Poll socket for a reply, with timeout
            zmq::pollitem_t items[] = { { (void *) *client, 0, ZMQ_POLLIN, 0 } };
            zmq::poll (&items[0], 1, request_timeout);

            //  If we got a reply, process it
            if (items[0].revents & ZMQ_POLLIN) {
                reply = s_recv (*client);
                std::cout << "I: server replied OK (" << reply << ")" << std::endl;
                return true;
            } else if (--retries_left == 0) {
                std::cout << "E: server seems to be offline, abandoning" << std::endl;
                return false;
            } else {
                std::cout << "W: no response from server, retrying..." << std::endl;
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
    msg.ParseFromString(reply);

    std::cout << "got answer from server:" << std::endl;
    std::cout << msg.DebugString() << std::endl;
}

