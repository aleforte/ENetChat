#pragma once

#include "state.h"
#include "quit_state.h"

/**
 * Chat State: Client
 *
 * Defines chat behaviors when connected as client. The client's responsibilities include:
 *   1) Sending the local user's messages to the host
 *   2) Receiving broadcasts from the hosts of *all* user messages (including the local clients)
 *   3) Manaing replicated session state, i.e. list of users as they join/leave 
 */
class ChatState_Client : public State
{
public:
    ChatState_Client(ChatApp* app) : State(app) {}

    void beginState() override
    {
        window()->log("Connected to session [client]...");
        m_app->connect("127.0.0.1", protocol::DEFAULT_PORT);
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
            m_app->send(0, protocol::MessagePackage(localUser->user_id, input));
        }
    }
    
    void receiveConnectionEvent(net::peer_id_t peer_id, const net::Address& address) override
    {
        // clients will instantly provide their username once connection is confirmed
        m_app->send(0, protocol::UsernamePackage(config()->nickname));
    }

    void receiveDisconnectEvent(net::peer_id_t peer_id, const net::Address& address) override
    {
        // terminating application for now (should cycle back to main prompt?)
        window()->log("Disconnected from host, exiting...");
        std::this_thread::sleep_for(std::chrono::seconds(3));
        m_app->goToState(new QuitState(m_app));
    }

    void receiveUsernameAckEvent(protocol::UsernameAckPackage& pkg) override
    {
        for (const UserInfo& user : pkg.users)
        {
            bool is_local = pkg.assigned_user_id == user.user_id;
            m_app->addUser(user, is_local);
        }
    }

    void receiveAddUserEvent(UserInfo* user, protocol::AddUserPackage& pkg) override
    {
        if (!m_app->containsUser(*user))
        {
            window()->log(user->name + " connected");
            m_app->addUser(*user);
        }
    }

    void receiveRemoveUserEvent(UserInfo* user, protocol::RemoveUserPackage& pkg) override
    {
        window()->log(user->name + " disconnected");
        m_app->removeUser(user->user_id);
    }

    void receiveMessageEvent(UserInfo* user, protocol::MessagePackage& pkg) override
    {
        if (!m_app->isLocalUser(user))
        {
            window()->print(user->name, pkg.message);
        }
    }
};
