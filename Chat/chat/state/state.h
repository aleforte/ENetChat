#pragma once

#define EXIT "/exit"

#include "chat/chat_app.h"

/**
 * Basic interface to implement app behaviors
 */
class State
{
public:
    State(ChatApp* app) : m_app(app) {}
    ~State() = default;
    
    virtual void beginState() {}
    virtual void endState() { window()->clearAll(); }
    
    // User provided input
    // @param input string (non-empty)
    virtual void handleInput(char input[80]) {}
    
    // Peer connected
    // @param peer_id Peer ID assigned by ENet
    // @param address Peer Address provided by ENet
    virtual void receiveConnectionEvent(net::peer_id_t peer_id, const net::Address& address) {}
    // Peer disconnected
    // @param peer_id Peer ID assigned by ENet
    // @param address Peer Address provided by ENet
    virtual void receiveDisconnectEvent(net::peer_id_t peer_id, const net::Address& address) {}

    // The following are called when data is exchanged between host/client. Each package
    // requires specific handling depending on application is host or client
    // @param user user associated to the package
    // @param pkg the package, see protocol
        
    virtual void receiveUsernameEvent(UserInfo* user, protocol::UsernamePackage& pkg) {}
    virtual void receiveUsernameAckEvent(protocol::UsernameAckPackage& pkg) {}
    virtual void receiveAddUserEvent(UserInfo* user, protocol::AddUserPackage& pkg) {}
    virtual void receiveRemoveUserEvent(UserInfo* user, protocol::RemoveUserPackage& pkg) {}
    virtual void receiveMessageEvent(UserInfo* user, protocol::MessagePackage& pkg) {}

protected:
    ChatApp* m_app; ///< pointer to the owning chat window

    // helpers to access app class members
    ChatWindow* window() const { return m_app->getWindow(); }
    ChatApp::ChatConfig* config() const { return m_app->getConfig(); }
};
