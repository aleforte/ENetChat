#include "chat_app.h"

#include "network/enet_wrapper.h"
#include "state/prompt_state_conn.h"
#include "util/byte_stream.h"

ChatApp::~ChatApp()
{
    delete m_enet;
    delete m_state;
    delete m_window;
}

void ChatApp::run()
{
    m_thread = std::jthread(&ChatApp::pollForInput, std::ref(*this));

    m_window->init();
    goToState(new PromptState_Conn(this));

    // join thread here so application quits immediately
    m_thread.join();
}

void ChatApp::quit()
{
    if (m_enet) m_enet->disconnectAll();
    m_quit = true;
}

void ChatApp::host(const int port, const int max_connections)
{
    m_enet = new net::ENetWrapper(*this, true, port, NULL, max_connections);
}

void ChatApp::connect(const std::string& address, const int port)
{
    m_enet = new net::ENetWrapper(*this, false, port, NULL, 1);
    m_enet->connect(address, port);
}

void ChatApp::goToState(State* state)
{
    if (m_state)
    {
        m_state->endState();
        delete m_state;
    }
    m_state = state;
    m_state->beginState();
}

void ChatApp::send(user_id_t user_id, protocol::UsernamePackage const& pkg) const
{
    ByteStream s;
    pkg.serialize(s);
    m_enet->send(toPeerID(user_id), s.getBuf());    
}

void ChatApp::send(user_id_t user_id, protocol::UsernameAckPackage const& pkg) const
{
    ByteStream s;
    pkg.serialize(s);
    m_enet->send(toPeerID(user_id), s.getBuf());
}

void ChatApp::send(user_id_t user_id, protocol::AddUserPackage const& pkg) const
{
    ByteStream s;
    pkg.serialize(s);
    m_enet->send(toPeerID(user_id), s.getBuf());
}

void ChatApp::send(user_id_t user_id, protocol::RemoveUserPackage const& pkg) const
{
    ByteStream s;
    pkg.serialize(s);
    m_enet->send(toPeerID(user_id), s.getBuf());
}

void ChatApp::send(user_id_t user_id, protocol::MessagePackage const& pkg) const
{
    ByteStream s;
    pkg.serialize(s);
    m_enet->send(toPeerID(user_id), s.getBuf());
}

void ChatApp::broadcast(protocol::AddUserPackage const& pkg) const
{
    ByteStream s;
    pkg.serialize(s);
    m_enet->broadcast(s.getBuf());
}

void ChatApp::broadcast(protocol::RemoveUserPackage const& pkg) const
{
    ByteStream s;
    pkg.serialize(s);
    m_enet->broadcast(s.getBuf());
}

void ChatApp::broadcast(protocol::MessagePackage const& pkg) const
{
    ByteStream s;
    pkg.serialize(s);
    m_enet->broadcast(s.getBuf());
}

void ChatApp::addUser(const UserInfo& user, bool is_local)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_users.insert(std::pair(user.user_id, user));
    if (is_local) m_localuser_ptr = &m_users[user.user_id];
    refreshUserList();
}

void ChatApp::removeUser(user_id_t user_id)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_users.erase(user_id);
    refreshUserList();
}

bool ChatApp::containsUser(UserInfo const& user) const
{
    return m_users.contains(user.user_id);
}

UserInfo* ChatApp::getUserInfoPtr(user_id_t user_id)
{
    UserMap::iterator it = m_users.find(user_id);
    if (it != m_users.end()) return &it->second;
    return NULL;
}

bool ChatApp::isLocalUser(const UserInfo* user) const
{
    if (user && m_localuser_ptr)
    {
        return user->user_id == m_localuser_ptr->user_id;
    }
    return false;
}

void ChatApp::pollForInput() const
{
    char input[80];
    while (!m_quit)
    {
        input[0] = 0; // ensure empty string
        m_window->checkInputBox(input);
        if (m_state && strlen(input) > 0)
        {
            m_state->handleInput(input);
        }
    }
}

void ChatApp::refreshUserList() const
{
    m_window->clearUserList();
    for (auto it = m_users.begin(); it != m_users.end(); ++it)
    {
        const UserInfo* user = &it->second;
        const bool is_local = m_localuser_ptr && m_localuser_ptr->user_id == user->user_id;
        m_window->addUser(user->name, is_local);
    }
}

// network callback
void ChatApp::connectionEvent(net::NetworkTraffic const& e)
{
    if (m_state) m_state->receiveConnectionEvent(e.peer_id, e.peer_address);
}

// network callback
void ChatApp::disconnectEvent(net::NetworkTraffic const& e)
{
    if (m_state) m_state->receiveDisconnectEvent(e.peer_id, e.peer_address);
}

// network callback
void ChatApp::receiveEvent(net::NetworkTraffic const& e)
{
    ByteStream s(reinterpret_cast<const char*>(e.packet_data), e.packet_length);
    switch (s.peekInt8())
    {
        case protocol::USERNAME: {
                protocol::UsernamePackage pckt(s);
                UserInfo user(toUserID(e.peer_id), pckt.username, e.peer_address);
                if (m_state) m_state->receiveUsernameEvent(&user, pckt);
                break;
        } case protocol::USERNAME_ACK: {
                protocol::UsernameAckPackage pckt(s);
                if (m_state) m_state->receiveUsernameAckEvent(pckt);
                break;
        } case protocol::STATE_ADD_USER: {
                protocol::AddUserPackage pckt(s);
                if (m_state) m_state->receiveAddUserEvent(&pckt.user, pckt);
                break;
        } case protocol::STATE_REM_USER: {
                protocol::RemoveUserPackage pckt(s);
                UserInfo* user = getUserInfoPtr(pckt.user_id);
                if (m_state) m_state->receiveRemoveUserEvent(user, pckt);
                break;                
        } case protocol::MESSAGE: {
                protocol::MessagePackage pckt(s);
                UserInfo* user = getUserInfoPtr(pckt.user_id);
                if (m_state) m_state->receiveMessageEvent(user, pckt);
                break;                
        } default: { /* do nothing */ }
    }
}
