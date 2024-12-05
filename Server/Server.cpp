
#include <iostream>
#include <enet/enet.h>

int main(int argc, char* argv[])
{
    if (enet_initialize() != 0)
    {
        fprintf(stderr, "An error occured while initializing ENet!\n");
        return EXIT_FAILURE;
    }
    atexit(enet_deinitialize);
    
    ENetEvent event;     // holds event server receives from clients
    ENetAddress address; // for server, this is the ip/ports we allow connections from 
    address.host = ENET_HOST_ANY;
    address.port = 7777;

    ENetHost* server;
    server = enet_host_create(&address, 32, 1, 0, 0);

    if (server == NULL)
    {
        fprintf(stderr, "An error occured while trying to create an ENet server host!\n");
        return EXIT_FAILURE;
    }
    
    // PROGRAM LOOP START
    while (true) // TODO (aleforte) fix this
    {
        while (enet_host_service(server, &event, 1000) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_CONNECT:
                printf("A new client connected from %x:%u.\n",
                        event.peer -> address.host,
                        event.peer -> address.port);
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                printf("A packet of length %u containing %s was received from %x:%u on channel %u.\n",
                        event.packet -> dataLength,
                        event.packet -> data,
                        event.peer -> address.host,
                        event.peer -> address.port,
                        event.channelID);
                enet_packet_destroy(event.packet); // Clean up packet now that we're done using it.
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                printf("%x:%u disconnect.\n",
                        event.peer -> address.host,
                        event.peer -> address.port);
                break;
            }
        }
    }
    // PROGRAM LOOP END

    enet_host_destroy(server);

    return EXIT_SUCCESS;
}
