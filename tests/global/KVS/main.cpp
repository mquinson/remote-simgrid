#include "rsg/actor.hpp"
#include "rsg/mailbox.hpp"
#include "rsg/comm.hpp"
#include "rsg/host.hpp"
#include "rsg/engine.hpp"
#include "rsg/kvs.hpp"

#include "../../print.hpp"

using namespace ::simgrid;

int actor(void *) {
  //We find the value
  std::string key("hello");
  std::string value;
  rsg::kvs::get(value, key);
  RSG_INFO("Retrieved value from KVS : %s", value.c_str());

  //We also try to get pre loaded values
  std::string preloaded_key("key_example");
  std::string preloaded_value;
  rsg::kvs::get(preloaded_value, preloaded_key);
  RSG_INFO("Retrieved preloaded value from KVS : %s", preloaded_value.c_str());

  //We also try another to get pre loaded values
  preloaded_key = "k";
  rsg::kvs::get(preloaded_value, preloaded_key);
  RSG_INFO("Retrieved preloaded value from KVS : %s", preloaded_value.c_str());

  //We update the key
  std::string new_value("univers");
  rsg::kvs::replace(key, new_value);
  rsg::this_actor::quit();
  return 1;
}

int main() {
  rsg::HostPtr host1 = rsg::Host::by_name("host1");
  //We first create a key and a value to be inserted in the KVS
  std::string key("hello");
  std::string value("WORLD!");
  // We insert it
  rsg::kvs::insert(key, value);
  // And then we create an actor, it will get the value and update it.
  rsg::Actor *act = rsg::Actor::createActor("receiver" , host1 , actor, NULL);
  act->join();

  //We try to find the new value.
  std::string updated_value;
  rsg::kvs::get(value, key);
  RSG_INFO("Retrieved updated value from KVS : %s", value.c_str());

  //Remove the value.
  rsg::kvs::remove(key);

  //And the try to get it.
  std::string removed_value;
  rsg::kvs::get(removed_value, key);
  RSG_INFO("Retrieved deleted value from KVS : %s", removed_value.c_str());

  //Remove it again ?
  rsg::kvs::remove(key);

  rsg::this_actor::quit();
  return 0;
}

