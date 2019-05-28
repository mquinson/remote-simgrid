#include <simgrid/s4u.hpp>
#include <xbt/string.hpp>
#include <iostream>
#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include <thrift/processor/TMultiplexedProcessor.h>

#include <simgrid/msg.h>

#include "../services.hpp"
#include "../../common.hpp"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using namespace  ::RsgService;

using namespace  ::simgrid;

#define UNUSED(x) (void)(x)

std::unordered_map<int, simgrid::s4u::ActorPtr> rsg::RsgActorHandler::pActors;
unsigned long long rsg::RsgActorHandler::pActorMapId = 0;

rsg::RsgActorHandler::RsgActorHandler()
    : server_exit(false)
{
}

void rsg::RsgActorHandler::close() {
    server_exit = true;
}

void  rsg::RsgActorHandler::sleep(const double duration) {
    s4u::this_actor::sleep_for(duration);
}

void rsg::RsgActorHandler::execute(const double flops) {
    s4u::this_actor::execute(flops);
}

void rsg::RsgActorHandler::send(const int64_t mbAddr, const std::string& content, const int64_t simulatedSize) {
    s4u::Mailbox* mbox = rsg::RsgMailboxHandler::pMailboxes.at(mbAddr);
    std::string *internalPtr = new std::string(content.data(), content.length());
    mbox->put((void*) internalPtr, simulatedSize);
}

void rsg::RsgActorHandler::recv(std::string& _return, const int64_t mbAddr) {
    s4u::Mailbox* mbox = rsg::RsgMailboxHandler::pMailboxes.at(mbAddr);
    std::string *content = (std::string*) mbox->get();
    _return.assign(content->data(), content->length());
    delete content;
}

void rsg::RsgActorHandler::getName(std::string& _return, const int64_t addr) {
    try {
        simgrid::s4u::ActorPtr actor = pActors.at(addr);
        simgrid::xbt::string c_name = actor->get_name();
        _return.assign(c_name);
    } catch(std::out_of_range& e) {
        std::cerr << "rsg::RsgActorHandler::getName no actors for this addr" << std::endl; 
    }
}


void rsg::RsgActorHandler::getHost(rsgHostCurrentResType& _return, const int64_t addr) {
    try {
        simgrid::s4u::ActorPtr actor = pActors.at(addr);
        s4u::Host *host = actor->get_host();
        _return.name = host->get_name();
        _return.addr = (unsigned long int) host;
    } catch(std::out_of_range& e) {
        std::cerr << "rsg::RsgActorHandler::getHost no actors for this addr" << std::endl; 
    }
}


int32_t rsg::RsgActorHandler::getPid(const int64_t addr) {
    try {
        simgrid::s4u::ActorPtr actor = pActors.at(addr);
        return actor->get_pid();
    } catch(std::out_of_range& e) {
        std::cerr << "rsg::RsgActorHandler::getPid no actors for this addr" << std::endl; 
    }
    return 0;
}


void rsg::RsgActorHandler::setAutoRestart(const int64_t addr, const bool autorestart) {
    try {
        simgrid::s4u::ActorPtr actor = pActors.at(addr);
        actor->set_auto_restart(autorestart);
    } catch(std::out_of_range& e) {
        std::cerr << "rsg::RsgActorHandler::setAutoRestart no actors for this addr" << std::endl; 
    }
}


void rsg::RsgActorHandler::setKillTime(const int64_t addr, const double time) {
    try {
        simgrid::s4u::ActorPtr actor = pActors.at(addr);
        actor->set_kill_time(time);
    } catch(std::out_of_range& e) {
        std::cerr << "rsg::RsgActorHandler::setKillTime no actors for this addr" << std::endl; 
    }
}


double rsg::RsgActorHandler::getKillTime(const int64_t addr) {
    try {
        simgrid::s4u::ActorPtr actor = pActors.at(addr);
        return actor->get_kill_time();
    } catch(std::out_of_range& e) {
        std::cerr << "rsg::RsgActorHandler::getKillTime no actors for this addr" << std::endl;
    }
    return -1;
}

void rsg::RsgActorHandler::killAll() {
    s4u::Actor::kill_all();
}

void rsg::RsgActorHandler::kill(const int64_t mbAddr) {
    try {
        simgrid::s4u::ActorPtr actor = pActors.at(mbAddr);
        actor->kill();
    } catch(std::out_of_range& e) {
        std::cerr << "rsg::RsgActorHandler::kill no actors for this addr" << std::endl;
    }
}

int32_t rsg::RsgActorHandler::killPid(const int32_t pid) {
  try {
    simgrid::s4u::Actor::by_pid(pid)->kill();
    return 0;
  } catch (std::out_of_range &e) {
    return 1;
  } catch (std::exception &e) {
    return 1;
  }
}

void rsg::RsgActorHandler::join(const int64_t addr) {
    try {
        simgrid::s4u::ActorPtr actor = pActors.at(addr);
        actor->join();
    } catch(std::out_of_range& e) {
        std::cerr << "rsg::RsgActorHandler::join no actors for this addr" << std::endl;
    }
}

void rsg::RsgActorHandler::createActorPrepare(std::string& _return) {
    TZmqServer::get_new_endpoint(_return);
    debug_server_print("Allocating a new RsgThriftServer instance");
    lastChildServer = new RsgThriftServer(_return);
}

int64_t rsg::RsgActorHandler::selfAddr() {
    simgrid::s4u::ActorPtr actor = s4u::Actor::self();
    unsigned long long newId = pActorMapId++;
    pActors.insert({newId, actor});
    return newId;
}



int64_t rsg::RsgActorHandler::createActor(const std::string& name, const int64_t hostaddr) {
    
    s4u::Host *host = (s4u::Host*) hostaddr;
    
    debug_server_print("createActor for sgname: %s", name.c_str());
    
    //we use a lambda because otherwise simgrid make unwanted copy of the class.
    simgrid::s4u::ActorPtr actor = simgrid::s4u::Actor::create(name.c_str(), host, [&]{
      RsgThriftServer* srv = lastChildServer;
       simgrid::s4u::this_actor::on_exit([srv](bool /*failed*/) {
               debug_server_print("deleteServerWhenActorIsKilled begin");
               delete srv;
               debug_server_print("deleteServerWhenActorIsKilled end");
       });
      (*srv)();
    });
    unsigned long long newId = pActorMapId++;
    pActors.insert({newId, actor});
    return newId;
}

void rsg::RsgActorHandler::deleteActor(const int64_t addr)  {
    try {
        pActors.erase(addr);
    } catch(std::out_of_range& e) {
        std::cerr << "rsg::RsgActorHandler::deleteActor no actors for this addr" << std::endl; 
    }
}

int32_t rsg::RsgActorHandler::this_actorGetPid() {
    return s4u::this_actor::get_pid();
}

int32_t rsg::RsgActorHandler::this_actorGetPPid() {
    return s4u::this_actor::get_ppid();
}


int32_t rsg::RsgActorHandler::getPPid(const int64_t addr) {
    simgrid::s4u::ActorPtr actor = pActors.at(addr);
    return actor->get_ppid();
}

int64_t rsg::RsgActorHandler::byPid(const int32_t pid) {
    simgrid::s4u::ActorPtr actor = s4u::Actor::by_pid(pid);
    if(actor != nullptr) {
        unsigned long long newId = pActorMapId++;
        pActors.insert({newId, actor});
        return newId;
    } else {
        return -1;
    }
}
