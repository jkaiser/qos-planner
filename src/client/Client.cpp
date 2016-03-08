//
// Created by jkaiser on 08.03.16.
//

#include "Client.h"


#define REQUEST_TIMEOUT     10000    //  msecs, (> 1000!)
#define REQUEST_RETRIES     3       //  Before we abandon
zmq::socket_t static *s_client_socket(const std::string ipPort, zmq::context_t & context) {
    std::cout << "I: connecting to server…" << std::endl;
    zmq::socket_t * client = new zmq::socket_t (context, ZMQ_REQ);
    client->connect(std::string("tcp://" + ipPort).data());

    //  Configure socket to not wait at close time
    int linger = 0;
    client->setsockopt (ZMQ_LINGER, &linger, sizeof (linger));
    return client;
}

bool static sendAndReceive(const std::string ipPort, const std::string &rawMsg, std::string reply) {

    std::string repMsg;
    zmq::context_t context (1);

    zmq::socket_t * client = s_client_socket(ipPort, context);

    int retries_left = REQUEST_RETRIES;

    while (retries_left) {
        s_send (*client, rawMsg.data());
        sleep (1);

        bool expect_reply = true;
        while (expect_reply) {
            //  Poll socket for a reply, with timeout
            zmq::pollitem_t items[] = { { (void *) *client, 0, ZMQ_POLLIN, 0 } };
            zmq::poll (&items[0], 1, REQUEST_TIMEOUT);


            //  If we got a reply, process it
            if (items[0].revents & ZMQ_POLLIN) {
                //  We got a reply from the server, must match sequence
                reply = s_recv (*client);
                std::cout << "I: server replied OK (" << reply << ")" << std::endl;
                repMsg = reply.data();
                retries_left = 0;
                expect_reply = false;
            } else if (--retries_left == 0) {
                std::cout << "E: server seems to be offline, abandoning" << std::endl;
                expect_reply = false;
                break;
            } else {
                std::cout << "W: no response from server, retrying..." << std::endl;
                //  Old socket will be confused; close it and open a new one
                delete client;
                client = s_client_socket (ipPort, context);
                //  Send request again, on new socket
                s_send (*client, rawMsg.data());
            }
        }
    }

    delete client;
    return true;
}

Client::Client(std::string ipPort) {
    this->ipPort = ipPort;
}

bool Client::requestResources(std::string request) {

    rpc::Message msg;
    msg.set_type(rpc::Message::REQUEST);
    auto req = msg.mutable_request();
    req->set_type(rpc::Request_Type_RESERVE);

    auto res1 = req->mutable_resourcerequest()->add_requestedresources();
    res1->set_file("foo");
    res1->set_throughputmb(100);
    res1->set_tstart(0);
    res1->set_tstop(42);

    std::cout << "will send: " << msg.DebugString() << std::endl;
    std::string reply;
    if (!sendAndReceive(this->ipPort, msg.SerializeAsString(), reply)) {
        std::cout << "getting the request the server failed" << std::endl;
        return false;
    }

    msg.Clear();
    msg.ParseFromString(reply);

    std::cout << "got answer from server:" << std::endl;
    std::cout << msg.DebugString() << std::endl;

    return true;
}

