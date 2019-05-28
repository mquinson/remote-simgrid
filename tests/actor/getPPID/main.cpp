
#include "rsg/actor.hpp"
#include "rsg/mailbox.hpp"
#include "rsg/comm.hpp"
#include "rsg/host.hpp"
#include "rsg/engine.hpp"

#include <xbt.h>
#include <simgrid/s4u.hpp>

#include <iostream>
#define UNUSED(x) (void)(x)


XBT_LOG_NEW_CATEGORY(RSG_THRIFT_CLIENT, "Remote SimGrid");
XBT_LOG_NEW_DEFAULT_SUBCATEGORY(RSG_THRIFT_REMOTE_CLIENT, RSG_THRIFT_CLIENT , "RSG server (Remote SimGrid)");

using namespace ::simgrid;

int actor(void *) {
  XBT_INFO("My id is %d; my ppid is %d",rsg::this_actor::getPid(), rsg::this_actor::getPPid());
  rsg::this_actor::sleep(1);
  rsg::this_actor::quit();
  return 1;
}

int main() {
  rsg::HostPtr host1 = rsg::Host::by_name("host1");

  rsg::Actor *act = rsg::Actor::createActor("receiver" , host1 , actor, NULL);

  XBT_INFO("My id is %d; my ppid is %d",rsg::this_actor::getPid(), rsg::this_actor::getPPid());
  
  rsg::Actor *getByPid = rsg::Actor::byPid(act->getPid());
  if(getByPid) {
    XBT_INFO("getName: %s", getByPid->getName());
    getByPid->join();
  } else {
    act->join();
    XBT_INFO("No actor with pid '%d' could be found", act->getPid());
  }

  XBT_INFO("Main end at: %f", rsg::getClock());

  rsg::this_actor::quit();
  return 0;
}
