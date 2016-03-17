/* Copyright (c) 2015. The SimGrid Team. All rights reserved.              */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero Licence (see in file LICENCE).        */

#include "xbt.h"
#include "simgrid/s4u.h"
#include <stdlib.h>

using namespace simgrid;

XBT_LOG_NEW_CATEGORY(RSG,"Remote SimGrid");
XBT_LOG_NEW_DEFAULT_SUBCATEGORY(RSG_SERVER, RSG, "RSG server (Remote SimGrid)");

int serverSocket;
int serverPort;
#include "rsg/socket.hpp"
#include "rsg.pb.h"

/* Serves a remote process (connected onto mysock), until a CMD_QUIT message is received */
static void representative_loop(int mysock) {
	simgrid::rsg::Request request;
	// TODO: recv_message(Register): The client will send a Register message declaring its identity

	// TODO: We should somehow assert that the RSG_HOST sent by the remote host is the one that I just passed by environment variable
	// TODO: On mismatch (ie, if the accepted socket is not from my remote host but another,
	// TODO:      we should move to the host that the representative thinks it is on.
	// TODO:      we should also change the representative name to RSG_ACTORNAME[+tid] if it happens. (needs S4U support)
	// TODO: Another representative will take care of my remote (and move back to this host)

	s4u::Actor &self = s4u::Actor::self();
	s4u::Host *self_host = NULL;
	bool done = false;
	while (!done) {
		simgrid::rsg::Answer ans;

		XBT_VERB("Wait for incoming data on fd %d",mysock);

		rsg_sock_maybeaccept(serverSocket);// if non-blocking accept retrieves something, start a new relative for the incoming process
		if (!recv_message(mysock, &request))
			xbt_die("Cannot receive message!");
		XBT_VERB("Got a message of type %d: %s", request.type(), request.ShortDebugString().c_str());

		ans.set_type(request.type());

		switch (request.type()) {
		case simgrid::rsg::CMD_SLEEP: {
			double duration = request.sleep().duration();
			XBT_INFO("sleep(%f)",duration);
			self.sleep(duration);
			break;
		}
		case simgrid::rsg::CMD_EXEC: {
			double flops = request.exec().flops();
			XBT_INFO("execute(%f)",flops);
			self.execute(flops);
			break;
		}
		case simgrid::rsg::CMD_SEND: {
			s4u::Mailbox *mbox = (s4u::Mailbox*)request.send().mbox();
			char *content = (char*)request.send().content().c_str();
			XBT_INFO("send(%s,%s)",mbox->getName(),content);
			self.send(*mbox, xbt_strdup(content), request.send().simulatedsize());
			break;
		}
		case simgrid::rsg::CMD_RECV: {
			s4u::Mailbox *mbox = (s4u::Mailbox*)request.recv().mbox();
			XBT_VERB("block on recv(%p)", mbox);
			char *content = (char*)self.recv(*mbox);
			XBT_INFO("recv(%s) ~> %s",mbox->getName(), content);
			ans.mutable_recv()->set_content((const char*)content);
			ans.mutable_recv()->set_contentsize(strlen(content));
			free(content);
			break;
		}
		case simgrid::rsg::CMD_ENGINE_GETCLOCK: {
			XBT_INFO("getClock()");
			break;
		}
		case simgrid::rsg::CMD_MB_CREATE: {
			const char *name = request.mbcreate().name().c_str();
			s4u::Mailbox *mbox = s4u::Mailbox::byName(name);
			XBT_VERB("mailbox_create(%s)",name);
			ans.mutable_mbcreate()->set_remoteaddr((google::protobuf::uint64)mbox);
			break;
		}
		case simgrid::rsg::CMD_QUIT: {
			XBT_INFO("quit()");
			done = true;
			break;
		}
		case simgrid::rsg::CMD_HOST_CURRENT: {
			XBT_INFO("CMD_HOST_CURRENT");
			if(NULL == self_host) {
				self_host = s4u::Host::current();
			}
			ans.mutable_hostgetcurrent()->set_hostname(self_host->name());
			ans.mutable_hostgetcurrent()->set_remoteaddr((google::protobuf::uint64)self_host);
			break;
		}
		case simgrid::rsg::CMD_HOST_GETBYNAME: {
			XBT_INFO("CMD_HOST_GETBYNAME");
			const char *name = request.hostgetbyname().name().c_str();
			s4u::Host *host = s4u::Host::by_name(name);

			if(host == nullptr) {
				XBT_INFO("No such Host (%s)", name);
			}

			XBT_INFO("host get by name (%s)",host->name().c_str());
			ans.mutable_hostgetbyname()->set_remoteaddr((google::protobuf::uint64)host);
			ans.mutable_hostgetbyname()->set_hostname(host->name());
			break;
		}
		case simgrid::rsg::CMD_HOST_GETSPEED: {
			XBT_INFO("CMD_HOST_GETSPEED");
			s4u::Host *host = (s4u::Host*)request.hostgetspeed().host();
			double speed = host->speed();
			ans.mutable_hostgetspeed()->set_speed(speed);
			break;
		}
		default:
			xbt_die("Received an unknown command: %d. Did you implement the answer of your command in %s?",
					request.type(),__FILE__);
		} // switch request->type()
		request.Clear();
		ans.set_clock(simgrid::s4u::Engine::getClock());
		send_message(mysock, &ans);
		ans.Clear();
	}

}

/* Fork a remote process, and serve it until it CMD_QUITs */
static int rsg_representative(int argc, char **argv) {

	XBT_VERB("Launching %s",argv[1]);

	if (! fork()) {
		// child. I'm not in the mood of parsing the command line, so have bash do it for me.
		putenv(bprintf("RSG_HOST=%s",s4u::Host::current()->name().c_str()));
		putenv(bprintf("RSG_ACTORNAME=%s", s4u::Actor::self().getName()));
		putenv(bprintf("RSG_PORT=%d",serverPort));
		int newargc = argc-1+2+1;
		char **newargv = (char**)calloc(newargc, sizeof(char*));
		newargv[0] = (char*)"/usr/bin/env";
		newargv[1] = (char*)"--";
		for(int i=1; i < argc; i++)
			newargv[1+i] = argv[i];
		newargv[newargc-1] = NULL;
		execv(newargv[0], newargv);
	}
	int mysock = rsg_sock_accept(serverSocket);

	representative_loop(mysock);
	return 0;
}


int main(int argc, char **argv) {
	// Verify that the version of the library that we linked against is
	// compatible with the version of the headers we compiled against.
	GOOGLE_PROTOBUF_VERIFY_VERSION;


	s4u::Engine *e = new s4u::Engine(&argc,argv);

	if (argc < 4) {
		fprintf(stderr,"Usage: rsg platform.xml deploy.xml port\n");
		exit(1);
	}
	serverPort = atoi(argv[3]);
	if (serverPort < 1024)
		xbt_die("You should not run RSG on lower port %d.",serverPort);

	/* Create a server socket onto the forked applications will connect */
	serverSocket = rsg_createServerSocket(serverPort);

	/* Initialize the SimGrid world */
	e->loadPlatform(argv[1]);
	e->registerDefault(rsg_representative);
	e->loadDeployment(argv[2]);
	e->run();
	XBT_INFO("Simulation done");
	close(serverPort);
	delete e;

	// Delete all global objects allocated by libprotobuf.
	google::protobuf::ShutdownProtobufLibrary();

	return 0;
}
