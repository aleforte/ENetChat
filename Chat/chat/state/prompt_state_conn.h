#pragma once

#include "state.h"
#include "prompt_state_name.h"
#include "quit_state.h"

#define HOST "1"
#define JOIN "2"
#define QUIT "3"

/**
 * Prompt State: Connection
 *
 * Prompts user to configure their connection, e.g. host or client
 * (This is the initial prompt on startup, essentially a main menu)
 */
class PromptState_Conn : public State
{
public:
    PromptState_Conn(ChatApp* app) : State(app) {}

    void beginState() override
    {
        window()->clearAll();
        window()->print("Please select an option:");
        window()->print("1. Host");
        window()->print("2. Join");
        window()->print("3. Exit");
    }

    void handleInput(char input[80]) override
    {
        if (strcmp(HOST, input) == 0)
        {
            config()->conn_as_host = true;
            m_app->goToState(new PromptState_Name(m_app));
        }
        else if (strcmp(JOIN, input) == 0)
        {
            m_app->goToState(new PromptState_Name(m_app));
        }
        else if (strcmp(QUIT, input) == 0)
        {
            m_app->goToState(new QuitState(m_app));
        }
    }
};
