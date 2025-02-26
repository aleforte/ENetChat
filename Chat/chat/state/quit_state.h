#pragma once

#include "state.h"

/**
 * Quit state
 *
 * Stop polling for input & quit application
 */
class QuitState : public State
{
public:
    QuitState(ChatApp* app) : State(app) {}

    void beginState() override
    {
        m_app->quit();
        window()->print("Press Enter to close window...");
    }
};
