#pragma once

#include "chat_win.h"
#include "userinfo.h"
#include "network/enet_wrapper.h"
#include "network/protocol.h"

class ChatApp : net::NetworkListener
{
public:
    // Configuration set by local user 
    struct ChatConfig
    {
        bool conn_as_host;     ///< start connection as host?
        std::string nickname;  ///< local user's nickname

        ChatConfig() : conn_as_host(false) {}
    };
    
    ChatApp() : m_window(new ChatWindow()), m_enet(), m_quit(false) {}
    ~ChatApp();

    // Starts the application
    void run();
    
    // Quits the application
    void quit();

    // Start ENet as host
    void host(const int port, const int max_connections = 16);
        
    // Start ENet as client & connect to the provided address
    void connect(const std::string& address, const int port);
    
    // Controls the chat app's state, **always** use this to change state
    void goToState(State* state);
    
    void send(user_id_t user_id, protocol::UsernamePackage const& pkg) const;
    void send(user_id_t user_id, protocol::UsernameAckPackage const& pkg) const;
    void send(user_id_t user_id, protocol::AddUserPackage const& pkg) const;
    void send(user_id_t user_id, protocol::RemoveUserPackage const& pkg) const;
    void send(user_id_t user_id, protocol::MessagePackage const& pkg) const;
    
    void broadcast(protocol::AddUserPackage const& pkg) const;
    void broadcast(protocol::RemoveUserPackage const& pkg) const;
    void broadcast(protocol::MessagePackage const& pkg) const;

    void addUser(const UserInfo& user, bool is_local = false);
    void removeUser(user_id_t user_id);
    bool containsUser(UserInfo const& user) const;

    ChatWindow* getWindow() const { return m_window; }
    ChatConfig* getConfig() { return &m_config; }
 
    UserInfo* getUserInfoPtr(user_id_t user_id);
    UserMap& getUserMap() { return m_users; }

    UserInfo* getLocalUserPtr() const { return m_localuser_ptr; }
    void setLocalUserPtr(UserInfo* user) { m_localuser_ptr = user; }
    bool isLocalUser(const UserInfo* user) const;

    static net::peer_id_t toUserID(net::peer_id_t peer_id)
    {
        return peer_id + 1;
    }
    
    static net::peer_id_t toPeerID(user_id_t user_id)
    {
        return (user_id == 0) ? 0 : user_id - 1;
    }
    
private:
    // Polls for user input
    void pollForInput() const;

    // Refreshs user listing
    void refreshUserList() const;
    
protected:
    //~Begin NetworkListener interface
    void connectionEvent(net::NetworkTraffic const& e) override;
    void disconnectEvent(net::NetworkTraffic const& e) override;
    void receiveEvent(net::NetworkTraffic const& e) override;
    //~End NetworkListener interface
    
private:
    ChatWindow* m_window;      ///< chat window
    net::ENetWrapper* m_enet;  ///< ENet wrapper
    ChatConfig m_config;       ///< local chat configuration
    UserInfo* m_localuser_ptr; ///< pointer to local user
    UserMap m_users; ///< map of users by user ID
    State* m_state;  ///< window prompt state
    bool m_quit;     ///< flag used to control main thread
    mutable std::mutex m_mutex; ///< mutex
    std::jthread m_thread;      ///< chat window thread 
};

