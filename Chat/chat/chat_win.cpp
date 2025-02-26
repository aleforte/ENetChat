#include "chat_win.h"

ChatWindow::~ChatWindow()
{
    endwin();

    delete m_inputwin;
    delete m_userwin;
}

void ChatWindow::init()
{
    // curses setup
    initscr();

    // colors
    start_color();
    init_pair(WHITE, COLOR_WHITE, COLOR_BLACK);
    init_pair(RED, COLOR_RED, COLOR_BLACK);
    init_pair(GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(CYAN, COLOR_CYAN, COLOR_BLACK);
    
    refresh();
}

void ChatWindow::checkInputBox(char outMsg[80]) const
{    
    // Clear input box
    wclear(m_inputwin);
    box(m_inputwin, 0, 0);

    // Await user input
    mvwscanw(m_inputwin, 1, 1, "%[^\n]", outMsg);
}

void ChatWindow::print(const std::string& msg)
{
    color_set(WHITE, nullptr);
    mvprintw(m_msg_win_y, 40, "%s", msg.c_str());
    refresh();
    m_msg_win_y++;
}

void ChatWindow::print(const std::string& username, const std::string& msg, bool local)
{
    color_set((local ? CYAN : WHITE), nullptr);
    mvprintw(m_msg_win_y, 40, "%s: %s", username.c_str(), msg.c_str());
    refresh();
    m_msg_win_y++;
}

void ChatWindow::log(const std::string& msg)
{
    color_set(GREEN, nullptr);
    mvprintw(m_msg_win_y, 40, "[LOG] %s", msg.c_str());
    refresh();
    m_msg_win_y++;
}

void ChatWindow::error(const std::string& msg)
{
    color_set(RED, nullptr);
    mvprintw(m_msg_win_y, 40, "[ERROR] %s", msg.c_str());
    refresh();
    m_msg_win_y++;
}

void ChatWindow::clearAll()
{
    clearMessages();
    clearUserList();
}

void ChatWindow::clearMessages()
{
    clear();

    int yMax, xMax;
    getmaxyx(stdscr, yMax, xMax);
    
    // creating input box
    m_inputwin = newwin(3, xMax-10, yMax-4, 5);
    wcolor_set(m_inputwin, WHITE, nullptr);
    box(m_inputwin, 0, 0);

    refresh();
    wrefresh(m_inputwin);

    // resetting y-index var
    m_msg_win_y = 1;
}

void ChatWindow::clearUserList()
{
    wclear(m_userwin);

    // creating log box
    m_userwin = newwin(24, 30, 1, 5);
    wcolor_set(m_userwin, WHITE, nullptr);
    box(m_userwin, 0, 0);

    refresh();
    wrefresh(m_userwin);

    // resetting y-index var
    m_user_win_y = 1;
}

void ChatWindow::addUser(const std::string& username, bool is_local)
{
    wcolor_set(m_userwin, (is_local ? CYAN : WHITE), nullptr);
    mvwprintw(m_userwin, m_user_win_y, 2,  (is_local ? "*%s" : "%s"), username.c_str());
    wrefresh(m_userwin);
    m_user_win_y++;
}

