#include <simgrid/s4u.hpp>
#include <iostream>

#include "../services.hpp"

using namespace ::apache::thrift::server;
using namespace  ::RsgService;

using namespace ::simgrid;

std::unordered_map<int, simgrid::s4u::Mailbox*> rsg::RsgMailboxHandler::pMailboxes;
unsigned long long rsg::RsgMailboxHandler::pMailboxesMapId = 0;

int64_t rsg::RsgMailboxHandler::mb_create(const std::string& name) {
    s4u::Mailbox* mbox = s4u::Mailbox::by_name(name);
    unsigned long long newId = pMailboxesMapId++;
    pMailboxes.insert({newId, mbox}); //TODO Maybe  we should check if the mailbox already exists (by register it by name)
    return newId;
};

void rsg::RsgMailboxHandler::setReceiver(const int64_t remoteAddrMailbox, const int64_t remoteAddrActor) {
    s4u::Mailbox* mbox = rsg::RsgMailboxHandler::pMailboxes.at(remoteAddrMailbox);
    try {
        s4u::ActorPtr actor = RsgActorHandler::pActors.at(remoteAddrActor);
        mbox->set_receiver(actor);
    } catch(std::out_of_range const& exc) {
        mbox->set_receiver(nullptr);
    }
}

int64_t rsg::RsgMailboxHandler::getReceiver(const int64_t remoteAddrMailbox) {
    s4u::Mailbox* mbox = rsg::RsgMailboxHandler::pMailboxes.at(remoteAddrMailbox);
    unsigned long long newId = rsg::RsgActorHandler::pActorMapId++;
    s4u::ActorPtr actor = mbox->get_receiver();
    if(actor == nullptr) {
        return 0;
    } 
    RsgActorHandler::pActors.insert({newId, actor});
    return newId;
}


bool rsg::RsgMailboxHandler::empty(const int64_t remoteAddrMailbox) {
    s4u::Mailbox* mbox = rsg::RsgMailboxHandler::pMailboxes.at(remoteAddrMailbox);
    return mbox->empty();
}
