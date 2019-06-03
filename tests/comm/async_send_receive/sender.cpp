#include "rsg/actor.hpp"
#include "rsg/mailbox.hpp"
#include "rsg/comm.hpp"
#include "rsg/host.hpp"

using namespace ::simgrid;

int main()
{
    const char *msg = "Hello.";
    rsg::MailboxPtr mbox = rsg::Mailbox::byName("toto");
    rsg::Comm &comm = rsg::Comm::send_init(*mbox);
    comm.setSrcData((void*)msg, strlen(msg) + 1);
    comm.start();
    comm.wait();

    const char *msg2 = "It is 'Arthur', King of the Britons.";
    rsg::Comm &comm2 = rsg::Comm::send_async(*mbox, (void*) msg2, strlen(msg2) + 1);
    comm2.wait();

    const char *msg3 = "To seek the Holy Grail.";
    rsg::Comm &comm3 = rsg::Comm::send_async(*mbox, (void*) msg3, strlen(msg2) + 1);
    comm3.wait();

    const char *msg4 = "What do you mean? An African or European swallow?";
    rsg::Comm &comm4 = rsg::Comm::send_init(*mbox);
    comm4.setSrcData((void*)msg4, strlen(msg4) + 1);
    comm4.start();
    comm4.wait();

    rsg::this_actor::quit();
    return 0;
}
