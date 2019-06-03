#include "rsg/mailbox.hpp"
#include "rsg/host.hpp"

#include "../../print.hpp"

#define nbWorker 10

using namespace ::simgrid;

int Worker(void* arg)
{
    int *msg = (int*) malloc(sizeof(int));
    int arg_int = (*(int*) arg);
    *msg = (arg_int + 1) * 10;
    rsg::this_actor::sleep(1);
    rsg::MailboxPtr mbox = rsg::Mailbox::byName("test");
    rsg::this_actor::send(*mbox, (char*) msg, sizeof(int));
    rsg::this_actor::quit();
    free(msg);
    return 0;
}

/*
 * This test test the wait_any function.
 * It creates a pool of actors and a new comm for each one. Each actor will send a sync on the
 */
int main()
{
    int *mboxbuff[10];
    std::vector<rsg::Comm*> comms;
    rsg::MailboxPtr mb = rsg::Mailbox::byName("test");

    //We create a pool of actors, and we associate a new Comm to each one of them
    for(int i = 0; i <  nbWorker; i++)
    {
        rsg::Comm *comm = &rsg::Comm::recv_init(*mb);
        comm->setDstData((void**)&(mboxbuff[i]));
        comms.push_back(comm);
        comm->start();

        rsg::Actor::createActor("worker", rsg::Host::by_name("host1"), Worker, (void*) &i);
    }

    for(int i = 0; i < nbWorker; i++)
    {
        auto it = rsg::Comm::wait_any(&comms);
        comms.erase(it);
        if(i == 0)
        {
            int pos = it - comms.begin();
            RSG_INFO("results %d at pos %d", *mboxbuff[pos], pos);
        }
    }

    for(int i = 0; i < nbWorker; i++)
    {
        RSG_INFO("%d", *mboxbuff[i]);
        free(mboxbuff[i]);
    }

    rsg::this_actor::quit();
    return 0;
}
