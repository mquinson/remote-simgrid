#include "rsg/RsgServiceImpl.h"

#include "xbt.h"
#include "simgrid/s4u.h"

#include <iostream>

XBT_LOG_NEW_CATEGORY(RSG_SERVICE, "Remote SimGrid");
XBT_LOG_NEW_DEFAULT_SUBCATEGORY(RSG_THRIFT_SERVICE, RSG_SERVICE , "RSG server (Remote SimGrid)");

RsgServiceHandler::RsgServiceHandler()  : pSelf(s4u::Actor::self()), pServer(NULL) {
}

void RsgServiceHandler::setServer(TServerFramework *server) {
  pServer = server;
}

void RsgServiceHandler::close() {
  pServer->stop();
}

void  RsgServiceHandler::sleep(const double duration) {
  pSelf.sleep(duration);
  XBT_INFO("slept %f secondes", duration);
}

void RsgServiceHandler::execute(const double flops) {
  XBT_INFO(" before execute %f flops", flops);
  pSelf.execute(flops);
  XBT_INFO("after execute %f flops", flops);
}

void RsgServiceHandler::send(const int64_t mbAddr, const std::string& content, const int64_t simulatedSize) {
  s4u::Mailbox *mbox = (s4u::Mailbox*)mbAddr;
  char *contentChar = (char*) content.c_str();
  pSelf.send(*mbox, xbt_strdup(contentChar), simulatedSize);
}

void RsgServiceHandler::recv(std::string& _return, const int64_t mbAddr) {
  s4u::Mailbox *mbox = (s4u::Mailbox*) mbAddr;
  char *content = (char*)pSelf.recv(*mbox);
  XBT_INFO("recv(%s) ~> %s", mbox->getName(), content);
  _return.assign(content);
}

int64_t RsgMailboxHandler::mb_create(const std::string& name) {
  s4u::Mailbox *mbox = s4u::Mailbox::byName(name.c_str());
  return (int64_t) mbox;
};


RsgHostHandler::RsgHostHandler() : pSelf(*s4u::Host::current()) {
}

int64_t RsgHostHandler::by_name(const std::string& name) {

  XBT_INFO("CMD_HOST_GETBYNAME");
  const char *c_name = name.c_str();
  s4u::Host *host = s4u::Host::by_name(c_name);

  if(host == nullptr) {
    XBT_INFO("No such Host (%s)", name);
    return 0;
  }

  XBT_INFO("host get by name (%s)",host->name().c_str());

  return (int64_t)host;
}

void RsgHostHandler::current(rsgHostCurrentResType& _return) {
  XBT_INFO("CMD_HOST_CURRENT");
  s4u::Host *host = s4u::Host::current();
  _return.name = host->name();
  _return.addr = (unsigned long int) host;
}


int64_t RsgHostHandler::speed(const int64_t addr) {
  s4u::Host *host = (s4u::Host*) addr;
  return (int64_t) host->speed();
}

void RsgHostHandler::turnOn(const int64_t addr) {
  s4u::Host *host = (s4u::Host*) addr;
  XBT_INFO("turn On host -> %s ", host->name().c_str());
  host->turnOn();
}

void RsgHostHandler::turnOff(const int64_t addr) {
  s4u::Host *host = (s4u::Host*) addr;
  XBT_INFO("turn Off host -> %s", host->name().c_str());
  host->turnOff();
}


bool RsgHostHandler::isOn(const int64_t addr) {
	s4u::Host *host = (s4u::Host*) addr;
    return host->isOn();
}