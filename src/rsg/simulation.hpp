#pragma once

#include <string>

#include "interthread_messaging.hpp"

namespace rsg
{
    class TcpSocket;
}
struct ActorConnection;

class Actor
{
public:
    Actor(rsg::TcpSocket * socket, int expected_actor_id, rsg::message_queue * to_command);
    void operator()();

private:
    rsg::TcpSocket * _socket;
    int _id;
    rsg::message_queue * _to_command;
};

void start_simulation_in_another_thread(const std::string & platform_file,
    const std::vector<std::string> & simgrid_options,
    rsg::message_queue * to_command,
    const std::unordered_map<int, ActorConnection*> & actor_connections);