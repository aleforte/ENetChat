#pragma once

#include "state.h"
#include "chat_state_client.h"
#include "chat_state_host.h"
#include "quit_state.h"

/**
 * Prompt State: Name/Nickname
 *
 * Prompts user to configure their nickname.
 * (This is the last bit of configuration before a connection is opened)
 */
class PromptState_Name : public State
{
public:
    PromptState_Name(ChatApp* app) : State(app) {}

    void beginState() override
    {
        window()->print("Please Enter Your Username...");
    }

    void handleInput(char input[80]) override
    {
        if (strcmp(EXIT, input) == 0)
        {
            m_app->goToState(new QuitState(m_app));
        }
        else
        {
            config()->nickname = input;
            if (config()->conn_as_host)
            {
                m_app->goToState(new ChatState_Host(m_app));
            }
            else
            {
                m_app->goToState(new ChatState_Client(m_app));
            }
        }
    }
};
