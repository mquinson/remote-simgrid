#include "../services.hpp"

#include "xbt.h"
#include "simgrid/s4u.hpp"

#include <iostream>

using namespace ::apache::thrift::server;
using namespace  ::RsgService;

using namespace ::simgrid;



double rsg::RsgEngineHandler::getClock() {
    return SIMIX_get_clock();
}

/*
void rsg::RsgEngineHandler::setKeepAliveOnNextClientDisconnect(const bool newValue) {
    std::cerr<<"setKeepAliveOnNextClientDisconnect NOT IMPLEMENTED"<<std::endl;
    assert(false);
}
*/
