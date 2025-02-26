#pragma once

#include "state.h"
#include "quit_state.h"

/**
 * Chat State: Host
 *
 * Defines chat behaviors when connected as host. The host has two main responsibilities:
 *   1) Broadcasting all messages as they are received from clients
 *   2) Managing the current session's state for clients, providing updates as users join/leave
 */
class ChatState_Host : public State
{
public:
    ChatState_Host(ChatApp* app) : State(app) {}

    void beginState() override
    {
        window()->log("Started new session [hosting]...");
        m_app->addUser(UserInfo(0, config()->nickname), true);
        m_app->host(protocol::DEFAULT_PORT, 16);
    }

    void handleInput(char input[80]) override
    {
        if (strcmp(EXIT, input) == 0)
        {
            m_app->goToState(new QuitState(m_app));
        }
        else if (UserInfo* localUser = m_app->getLocalUserPtr())
        {
            window()->print(localUser->name, input, true);
            m_app->broadcast(protocol::MessagePackage(0, input)); // always zero for host
        }
    }

    void receiveConnectionEvent(net::peer_id_t peer_id, const net::Address& address) override
    {
        // wait until user sends name before adding to user list & broadcasting to clients
    }

    void receiveDisconnectEvent(net::peer_id_t peer_id, const net::Address& address) override
    {
        if (UserInfo* user = m_app->getUserInfoPtr(ChatApp::toUserID(peer_id)))
        {
            window()->log(user->name + " disconnected [" + user->address.str() + "]");
            m_app->broadcast(protocol::RemoveUserPackage(user->user_id));
            m_app->removeUser(user->user_id);
        }
        else
        {
            window()->error("Disconnect from user not found in records");
        }
    }

    void receiveUsernameEvent(UserInfo* user, protocol::UsernamePackage& pkg) override
    {
        if (user)
        {
            m_app->addUser(*user);
            window()->log(user->name + " connected [" + user->address.str() + "]");
            m_app->send(user->user_id, protocol::UsernameAckPackage(user->user_id, m_app->getUserMap()));
            m_app->broadcast(protocol::AddUserPackage(*user));
        }
        else
        {
            window()->error("Username received from missing user");
        }
    }

    void receiveMessageEvent(UserInfo* user, protocol::MessagePackage& pkg) override
    {
        window()->print(user->name, pkg.message);
        m_app->broadcast(pkg);
    }
};
