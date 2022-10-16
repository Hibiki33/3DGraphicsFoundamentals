#include <UGL/UGL>
#include <UGM/UGM>

#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "../../tool/Camera.h"
#include "../../tool/SimpleLoader.h"

#include <iostream>

using namespace Ubpa;
using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void processRelease(GLFWwindow* window);
gl::Texture2D loadTexture(char const* path);

// ��Ļ����
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
bool have_normal_and_displacement = true;
bool isPressingKeySpace = false;

// ����������Camera.cpp
Camera camera(pointf3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// ��ʱ
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

int main()
{
    // glfw��ʼ��
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // ��������
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "HW2.1 - Displacement & Normal", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    //�趨�¼�������Ļص�����������Qt�е�connect
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: ����OpenGL����ָ��
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // ����Openglȫ��״̬�������UGLʵ��
    gl::Enable(gl::Capability::DepthTest);

    // ���ض�����ɫ��
    // ------------------------------------
    gl::Shader vertex_shader(gl::ShaderType::VertexShader, "../data/shaders/dn.vert");
    // ����ƬԪ��ɫ��
    gl::Shader fragment_shader(gl::ShaderType::FragmentShader, "../data/shaders/light_dn.frag");
    gl::Program program(&vertex_shader, &fragment_shader);

    float ambient = 0.2f;
    float specular = 0.8f;
    program.SetTex("color_texture", 0);
    program.SetTex("normalmap", 1);
    program.SetTex("displacementmap", 2);
    program.SetFloat("displacement_coefficient", 0.2f);
    program.SetVecf3("point_light_pos", { 0,10,0 });
    program.SetVecf3("point_light_radiance", { 200,200,200 });
    program.SetFloat("ambient", ambient);
    program.SetFloat("specular", specular);

    // ����ƽ��ģ��
    // ------------------------------------------------------------------
    vector<pointf3> positions;
    vector<pointf2> texcoords;
    vector<normalf> normals;
    vector<vecf3> tangents;
    vector<GLuint> indices;
    constexpr size_t N = 200;
    const pointf3 LBCorner{ -1,0,1 };
    const vecf3 right{ 2,0,0 };
    const vecf3 up{ 0,0,-2 };
    for (size_t j = 0; j <= N; j++) {
        float v = j / static_cast<float>(N);
        for (size_t i = 0; i <= N; i++) {
            float u = i / static_cast<float>(N);
            positions.push_back(LBCorner + u * right + v * up);
            texcoords.emplace_back(u, v);
            normals.emplace_back(0, 1, 0);
            tangents.emplace_back(1, 0, 0);
        }
    }
    for (size_t j = 0; j < N; j++) {
        for (size_t i = 0; i < N; i++) {
            size_t two_tri[6] = {
                j * (N + 1) + i, j * (N + 1) + i + 1, (j + 1) * (N + 1) + i,
                (j + 1) * (N + 1) + i + 1,(j + 1) * (N + 1) + i,j * (N + 1) + i + 1
            };
            for (auto idx : two_tri)
                indices.push_back(static_cast<GLuint>(idx));
        }
    }
    gl::VertexBuffer vb_pos(positions.size() * sizeof(pointf3), positions[0].data());
    gl::VertexBuffer vb_uv(texcoords.size() * sizeof(pointf2), texcoords[0].data());
    gl::VertexBuffer vb_n(normals.size() * sizeof(normalf), normals[0].data());
    gl::VertexBuffer vb_t(tangents.size() * sizeof(vecf3), tangents[0].data());
    gl::ElementBuffer eb(gl::BasicPrimitiveType::Triangles, indices.size() / 3, indices.data());

    gl::VertexArray::Format format;
    format.attrptrs.push_back(vb_pos.AttrPtr(3, gl::DataType::Float, false, sizeof(pointf3)));
    format.attrptrs.push_back(vb_uv.AttrPtr(2, gl::DataType::Float, false, sizeof(pointf2)));
    format.attrptrs.push_back(vb_n.AttrPtr(3, gl::DataType::Float, false, sizeof(normalf)));
    format.attrptrs.push_back(vb_t.AttrPtr(3, gl::DataType::Float, false, sizeof(vecf3)));
    format.eb = &eb;
    gl::VertexArray plane({ 0,1,2,3 }, format);

    // λ��
    pointf3 instancePositions[] = {
        pointf3(0.0f,  0.0f,  0.0f),
        pointf3(2.0f,  5.0f, -15.0f),
        pointf3(-1.5f, -2.2f, -2.5f),
        pointf3(-3.8f, -2.0f, -12.3f),
        pointf3(2.4f, -0.4f, -3.5f),
        pointf3(-1.7f,  3.0f, -7.5f),
        pointf3(1.3f, -2.0f, -2.5f),
        pointf3(1.5f,  2.0f, -2.5f),
        pointf3(1.5f,  0.2f, -1.5f),
        pointf3(-1.3f,  1.0f, -1.5f)
    };

    // ���ز���������
    // -------------------------
    gl::Texture2D checkerboard = loadTexture("../data/textures/checkerboard.png");
    gl::Texture2D normalmap = loadTexture("../data/textures/cg_normalmap.jpg");
    gl::Texture2D displacementmap = loadTexture("../data/textures/cg_displacementmap.jpg");

    // ��Ⱦѭ��
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // Ϊÿ֡����ʱ��
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // ����
        // -----
        processInput(window);
        processRelease(window);

        // �����￪ʼ�ǻ�������
        // ------
        gl::ClearColor({ ambient, ambient, ambient, 1.0f });
        gl::Clear(gl::BufferSelectBit::ColorBufferBit | gl::BufferSelectBit::DepthBufferBit); // also clear the depth buffer now!

        program.SetVecf3("camera_pos", camera.Position);

        // ������󶨵�OpenGL id��
        program.Active(0, &checkerboard);
        program.Active(1, &normalmap);
        program.Active(2, &displacementmap);

        // ����ͶӰ���󲢷��͵���ɫ��
        transformf projection = transformf::perspective(to_radian(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.f);
        program.SetMatf4("projection", projection);

        // �ӽǾ���
        program.SetMatf4("view", camera.GetViewMatrix());

        // ������Ӱ�������ڳ�������ʱ�ÿո����
        program.SetBool("have_normal_and_displacement", have_normal_and_displacement);

        // ��Ⱦƽ��
        for (size_t i = 0; i < 10; i++)
        {
            // ����ģ�;���ģ�Ͳ�����ת
            float angle = 20.0f * i + 10.f * (float)glfwGetTime();
            transformf model(instancePositions[i], quatf{ vecf3(1.0f, 0.3f, 0.5f), to_radian(angle) });
            program.SetMatf4("model", model);
            plane.Draw(&program);
        }

        // ��buffer�л��Ƶ����ݻ�������չʾ����Ļ��
        glfwSwapBuffers(window);

        // ǰ��Ļص����������������
        glfwPollEvents();
    }

    // ������Զ����δ�ͷ���Դ�����ݿ�������������ͷ�
    // ------------------------------------------------------------------------

    // ���������ͷ�����GLFW��Դ
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// ����GLFW API���ƶ��ӽ�λ�ã��ӽǳ����������仯��
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::Movement::FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::Movement::BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::Movement::LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::Movement::RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::Movement::UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::Movement::DOWN, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        isPressingKeySpace = true;
}

void processRelease(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE && isPressingKeySpace) {
        isPressingKeySpace = false;
        have_normal_and_displacement = !have_normal_and_displacement;
    }
}

// glfw: ��Ļ��С���ı�ʱ����
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    gl::Viewport({ 0, 0 }, width, height);
}


// glfw: ����ƶ�ʱ����
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos); // reversed since y-coordinates go from bottom to top

    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);

    camera.ProcessMouseMovement(static_cast<float>(xoffset), static_cast<float>(yoffset));
}

// glfw: �������ƶ�ʱ����
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}


// ��������ĺ���
gl::Texture2D loadTexture(char const* path)
{
    gl::Texture2D tex;
    tex.SetWrapFilter(gl::WrapMode::Repeat, gl::WrapMode::Repeat, gl::MinFilter::Linear, gl::MagFilter::Linear);
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    gl::PixelDataFormat c2f[4] = {
        gl::PixelDataFormat::Red,
        gl::PixelDataFormat::Rg,
        gl::PixelDataFormat::Rgb,
        gl::PixelDataFormat::Rgba
    };
    gl::PixelDataInternalFormat c2if[4] = {
        gl::PixelDataInternalFormat::Red,
        gl::PixelDataInternalFormat::Rg,
        gl::PixelDataInternalFormat::Rgb,
        gl::PixelDataInternalFormat::Rgba
    };
    if (data)
    {
        tex.SetImage(0, c2if[nrChannels - 1], width, height, c2f[nrChannels - 1], gl::PixelDataType::UnsignedByte, data);
        tex.GenerateMipmap();
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    return tex;
}
