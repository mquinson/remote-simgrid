// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.


#include "rsg/actor.hpp"
#include "rsg/mailbox.hpp"
#include "rsg/comm.hpp"
#include "rsg/host.hpp"

#include <xbt.h>
#include <simgrid/s4u.hpp>

#include <iostream>

XBT_LOG_NEW_CATEGORY(RSG_THRIFT_CLIENT, "Remote SimGrid");
XBT_LOG_NEW_DEFAULT_SUBCATEGORY(RSG_THRIFT_REMOTE_CLIENT, RSG_THRIFT_CLIENT , "RSG server (Remote SimGrid)");

using boost::shared_ptr;
using namespace ::RsgService;
using namespace ::simgrid;

#define UNUSED(x) (void)(x)

int simpleReceiver(void * ) {
    rsg::MailboxPtr mbox = rsg::Mailbox::byName("toto");
    rsg::this_actor::recv(*mbox);
    rsg::this_actor::quit();
    return 1;
}

int main(int argc, char **argv) {
    const char *msg = "Do you copy ? ";
    rsg::HostPtr host1 = rsg::Host::by_name("host1");
    
    
    rsg::MailboxPtr mbox = rsg::Mailbox::byName("toto");
    rsg::Actor* actor = rsg::Actor::createActor("receiver" , host1 , simpleReceiver, NULL);
    
    rsg::this_actor::send(*mbox,msg, strlen(msg) + 1);
    char *actorname = actor->getName();
    XBT_INFO("actor name -> %s", actorname);
    XBT_INFO("actor pid -> %d", actor->getPid());
    XBT_INFO("host name accessing by actor -> %s", actor->getHost()->getName().c_str());
    
    delete actorname;
    rsg::this_actor::quit();
    return 0;
}
