#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "simple_commands.hpp"
#include "../common/assert.hpp"
#include "../common/message.hpp"
#include "../common/network/tcp_socket.hpp"

#include "rsg.pb.h"

int add_actor(const std::string & server_hostname, int server_port,
    const std::string & actor_name, const std::string & vhost_name,
    bool autoconnect, const std::string & command_to_run,
    const std::vector<std::string> command_args)
{
    try
    {
        // Connect to the server.
        rsg::TcpSocket socket;
        socket.connect(server_hostname, server_port);

        // Generate message.
        rsg::Command command;
        auto add_actor = new(rsg::Command_AddActor);
        add_actor->set_actorname(actor_name);
        add_actor->set_hostname(vhost_name);
        command.set_allocated_addactor(add_actor);

        // Write message on socket.
        write_message(command, socket);

        // Read acknowledment.
        rsg::CommandAck command_ack;
        read_message(command_ack, socket);

        socket.close();

        if (!command_ack.success())
        {
            printf("add-actor refused from server\n");
            return 1;
        }

        // Create the requested process
        errno = 0;
        pid_t pid = fork();
        RSG_ENFORCE(pid != -1, "Cannot create a new process: %s", strerror(errno));

        if (pid == 0) // Child
        {
            // Prepare execv "argv".
            const size_t nb_args = command_args.size() + 1;
            char * args[nb_args];
            for (size_t i = 0; i < command_args.size(); i++)
                args[i] = strdup(command_args[i].c_str());
            args[command_args.size()] = nullptr;

            // Prepare environment variables.
            errno = 0;
            int ret = setenv("RSG_SERVER_HOSTNAME", server_hostname.c_str(), 1);
            RSG_ENFORCE(ret == 0, "Cannot setenv RSG_SERVER_HOSTNAME to '%s': %s",
                server_hostname.c_str(), strerror(errno));

            char port_string[16];
            ret = snprintf(port_string, 16, "%d", server_port);
            RSG_ENFORCE(ret > 0 && ret < 16, "Cannot convert port=%d to a 16-length string", server_port);
            ret = setenv("RSG_SERVER_PORT", port_string, 1);
            RSG_ENFORCE(ret == 0, "Cannot setenv RSG_SERVER_PORT to '%s': %s",
                port_string, strerror(errno));

            if (autoconnect)
            {
                ret = setenv("RSG_AUTOCONNECT", "1", 1);
                RSG_ENFORCE(ret == 0, "Cannot setenv RSG_AUTOCONNECT to '1': %s", strerror(errno));
            }
            else
            {
                ret = unsetenv("RSG_AUTOCONNECT");
                RSG_ENFORCE(ret == 0, "Cannot unsetenv RSG_AUTOCONNECT: %s", strerror(errno));
            }

            // Mute into the target process.
            errno = 0;
            ret = execv(command_to_run.c_str(), args);

            // This code is only executed if muting into the target process failed.
            printf("Could not mute into the target process: %s\n", strerror(errno));
            printf("execv arguments were:\n");
            printf("- path: %s\n", command_to_run.c_str());
            printf("- argv: ['%s'", args[0]);
            for (size_t i = 1; i < nb_args; i++)
                printf(", '%s'", args[i]);
            printf("]\n");

            // Tell the server we could not run the command by killing it.
            printf("Killing the server, as executing the process failed.\n");
            kill(server_hostname, server_port);

            return 1;
        }
        // Parent
        return 0;
    }
    catch (const rsg::Error & err)
    {
        printf("%s\n", err.what());
        return 1;
    }
}

void kill(const std::string & server_hostname, int server_port)
{
    try
    {
        // Connect to the server.
        rsg::TcpSocket socket;
        socket.connect(server_hostname, server_port);

        // Generate message.
        rsg::Command command;
        command.set_kill(true);

        // Write message on socket.
        write_message(command, socket);

        // Read acknowledment.
        rsg::CommandAck command_ack;
        read_message(command_ack, socket);

        if (!command_ack.success())
            printf("kill failed\n");
    }
    catch (const rsg::Error & err)
    {
        printf("%s\n", err.what());
    }
}

void start(const std::string & server_hostname, int server_port)
{
    try
    {
        // Connect to the server.
        rsg::TcpSocket socket;
        socket.connect(server_hostname, server_port);

        // Generate message.
        rsg::Command command;
        command.set_start(true);

        // Write message on socket.
        write_message(command, socket);

        // Read acknowledment.
        rsg::CommandAck command_ack;
        read_message(command_ack, socket);

        if (!command_ack.success())
            printf("start failed\n");
    }
    catch (const rsg::Error & err)
    {
        printf("%s\n", err.what());
    }
}

void status(const std::string & server_hostname, int server_port)
{
    try
    {
        // Connect to the server.
        rsg::TcpSocket socket;
        socket.connect(server_hostname, server_port);

        // Generate message.
        rsg::Command command;
        command.set_status(true);

        // Write message on socket.
        write_message(command, socket);

        // Read acknowledment.
        rsg::CommandAck command_ack;
        read_message(command_ack, socket);

        if (!command_ack.success())
            printf("status failed\n");
    }
    catch (const rsg::Error & err)
    {
        printf("%s\n", err.what());
    }
}
