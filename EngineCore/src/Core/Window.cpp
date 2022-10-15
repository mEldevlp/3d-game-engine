#include "Core/Window.hpp"
#include "Core/Log.hpp"
#include "Core/Rendering/OpenGL/ShaderProgram.hpp"
#include "Core/Rendering/OpenGL/VertexBuffer.hpp"
#include "Core/Rendering/OpenGL/VertexArray.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_glfw.h>

#include <memory>


namespace EngineCore {

    static bool s_GLFW_initialized = false;

    GLfloat points[] = {
        0.0f,   0.5f,  0.0f,
        0.5f,  -0.5f,  0.0f,
       -0.5f,  -0.5f,  0.0f
    };

    GLfloat colors[] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };

    const char* vertex_shader =
        "#version 460\n"
        "layout(location = 0) in vec3 vertex_position;"
        "layout(location = 1) in vec3 vertex_color;"
        "out vec3 color;"
        "void main() {"
        "   color = vertex_color;"
        "   gl_Position = vec4(vertex_position, 1.0);"
        "}";

    const char* fragment_shader =
        "#version 460\n"
        "in vec3 color;"
        "out vec4 frag_color;"
        "void main() {"
        "   frag_color = vec4(color, 1.0);"
        "}";

    GLuint shader_program;
    GLuint vao;

    std::unique_ptr<ShaderProgram> g_pShader_program;
    std::unique_ptr<VertexBuffer> g_pPoints_vbo;
    std::unique_ptr<VertexBuffer> g_pColors_vbo;

    std::unique_ptr<VertexArray> g_pVAO;

	Window::Window(std::string title, const unsigned width, const unsigned height)
        : m_data({ std::move(title), width, height })
	{
		int resultCode = init();

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui_ImplOpenGL3_Init();

        ImGui_ImplGlfw_InitForOpenGL(m_pWindow, true);

	}

	Window::~Window()
	{
		shutdown();
	}


	int Window::init()
	{
        LOG_INFO("Creating window '{0}' width size {1}x{2}", m_data.title, m_data.width, m_data.height);

        if (!s_GLFW_initialized)
        {
            if (!glfwInit())
            {
                LOG_CRITICAL("Can't initialize GLFW!");
                return -1;
            }
            s_GLFW_initialized = true;
        }
       

        m_pWindow = glfwCreateWindow(m_data.width, m_data.height, m_data.title.c_str(), nullptr, nullptr);

        if (!m_pWindow)
        {
            LOG_CRITICAL("Can't create window {0} width size {1}x{2}!", m_data.title, m_data.width, m_data.height);
            glfwTerminate();
            return -2;
        }

        glfwMakeContextCurrent(m_pWindow);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            LOG_CRITICAL("Failed to initialized GLAD library");
            return -3;
        }

        glfwSetWindowUserPointer(m_pWindow, &m_data);

        glfwSetWindowSizeCallback(m_pWindow,
            [](GLFWwindow* pWindow, int width, int height)
            {
                WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(pWindow));
                data.width = width;
                data.height = height;

                EventWindowResize event(width, height);

                data.eventCallbackFn(event);
            }
        );

        glfwSetCursorPosCallback(m_pWindow, [](GLFWwindow* pWindow, double x, double y)
            {
                WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(pWindow));
                EventMouseMoved event(x, y);
                data.eventCallbackFn(event);
            }
        );

        glfwSetWindowCloseCallback(m_pWindow, [](GLFWwindow* pWindow)
            {
                WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(pWindow));

                EventWindowClose event;

                data.eventCallbackFn(event);
            }
        );

        glfwSetFramebufferSizeCallback(m_pWindow, [](GLFWwindow* pWindow, int width, int height)
            {
                glViewport(0, 0, width, height);
            }
        );


        g_pShader_program = std::make_unique<ShaderProgram>(vertex_shader, fragment_shader);
        if (!g_pShader_program->isCompiled()) return -1;


        g_pPoints_vbo = std::make_unique<VertexBuffer>(points, sizeof(points));
        g_pColors_vbo = std::make_unique<VertexBuffer>(colors, sizeof(colors));
        g_pVAO        = std::make_unique<VertexArray>();


        g_pVAO->add_buffer(*g_pPoints_vbo);
        g_pVAO->add_buffer(*g_pColors_vbo);

        return 0;  
	}

	void Window::shutdown()
	{
        glfwDestroyWindow(m_pWindow);
        glfwTerminate();
	}

    void Window::on_update()
    {
        glClearColor(m_background_color[0], m_background_color[1], m_background_color[2], m_background_color[3]);
        glClear(GL_COLOR_BUFFER_BIT);

        g_pShader_program->bind();
        g_pVAO->bind();
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize.x = static_cast<float>(get_width());
        io.DisplaySize.y = static_cast<float>(get_height());
        
        ImGui_ImplOpenGL3_NewFrame();
        
        ImGui_ImplGlfw_NewFrame();
        
        ImGui::NewFrame();

        ImGui::Begin("Background Color Window");
            ImGui::ColorEdit4("Background Color", m_background_color);
        ImGui::End();

        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(m_pWindow);
        glfwPollEvents();
    }
}