// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include <thrift/protocol/TBinaryProtocol.h>

#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

#include "rsg/RsgServiceImpl.h"
#include "client/RsgClientEngine.hpp"
#include "rsg/actor.hpp"
#include "rsg/mailbox.hpp"
#include "rsg/comm.hpp"
#include "rsg/host.hpp"

#include "xbt.h"
#include "simgrid/s4u.h"

#include <iostream>

XBT_LOG_NEW_CATEGORY(RSG_THRIFT_CLIENT, "Remote SimGrid");
XBT_LOG_NEW_DEFAULT_SUBCATEGORY(RSG_THRIFT_NODE_1, RSG_THRIFT_CLIENT , "RSG server (Remote SimGrid)");

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;
using namespace ::RsgService;

int main(int argc, char **argv) {
  const char *msg = "Do you copy ? ";
  rsg::Mailbox *mbox = rsg::Mailbox::byName("toto");
  rsg::Actor &self = rsg::Actor::self();
  self.send(*mbox,msg);
  XBT_INFO("send %s with size : %d", msg, strlen(msg));
  
  char *rec;
  rec = self.recv(*mbox);
  XBT_INFO("Received -> %s ", rec);

  rsg::Mailbox *fooMb = rsg::Mailbox::byName("foo");
  self.send(*fooMb, msg);
  XBT_INFO("send %s with size : %d", msg, strlen(msg));

  char *fooRec;
  fooRec = self.recv(*fooMb);
  XBT_INFO("Received -> %s ", fooRec);

  self.quit();
  return 0;
}
