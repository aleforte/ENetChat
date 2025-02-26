#pragma once

#include <string>
#include "curses.h"
#include "userinfo.h"

class State;

/**
 * Simple Chat Window using PDCurses, includes
 * operations to post/display messages
 *
 * TODO (aleforte) nice to have:
 *  - scrolling to text display window
 *  - fix column alignment on window resize
 */
class ChatWindow
{
public:
    
    enum TextColor : uint8_t
    {
        WHITE = 0,
        RED = 1,
        GREEN = 2,
        CYAN = 3
    };

    ChatWindow() = default;
    ~ChatWindow();

    // Initializes chat window
    void init();
    
    // Checks input box for user input and copies to buffer
    void checkInputBox(char outMsg[80]) const;

    // Prints message to the main chat window
    void print(const std::string& msg);

    // Prints message, prefixed with username, to the main chat window
    void print(const std::string& username, const std::string& msg, bool local = false);

    // Prints log message to window
    void log(const std::string& msg);

    // Prints error message to window
    void error(const std::string& msg);

    // Refreshes all window panels
    void clearAll();

    // Refreshes message panel
    void clearMessages();

    // Refreshes user list panel
    void clearUserList();

    // Adds user to user list panel
    void addUser(const std::string& username, bool is_local);
    
private:
    // Message window Y-axis, used to slot messages as they are posted
    int m_msg_win_y = 1;

    // User window Y-axis, used to slot usernames as they join session
    int m_user_win_y = 1;

    WINDOW* m_inputwin = nullptr; ///< Textbox for user input
    WINDOW* m_userwin = nullptr;  ///< Window listing connected users
};
