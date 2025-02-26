
#include <iostream>
#include <enet/enet.h>

#include "chat/chat_app.h"
#include "util/byte_stream.h"

int main(int argc, char* argv[])
{
    net::ENetContainer enet; // initialize ENet
    
    ChatApp app = ChatApp(); 
    app.run();

    return EXIT_SUCCESS;
}
