#include "Core/Application.hpp"
#include "Core/Log.hpp"
#include "Core/Window.hpp"

#include <iostream>

namespace EngineCore
{
	Application::Application()
	{
		LOG_INFO("Starting Application");
	}
	Application::~Application()
	{
		LOG_INFO("Closing Application");
	}

	int Application::start(unsigned window_width, unsigned window_height, const char* title)
	{
		m_pWindow = std::make_unique<Window>(title, window_width, window_height);

		while (true)
		{
			m_pWindow->on_update();
			on_update();
		}

        return 0;
	}
}

