#include <UGL/UGL>
#include <UGM/UGM>

#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "../../tool/Camera.h"
#include "../../tool/SimpleLoader.h"

#include <iostream>

using namespace Ubpa;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void processRelease(GLFWwindow* window);
gl::Texture2D loadTexture(char const* path);

// ��Ļ����
unsigned int scr_width = 800;
unsigned int scr_height = 600;
bool have_shadow = false;
bool isPressingKeySpace = false;

//  ����������Camera.cpp
Camera camera(pointf3(0.0f, 0.0f, 3.0f));
float lastX = scr_width / 2.0f;
float lastY = scr_height / 2.0f;
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
    GLFWwindow* window = glfwCreateWindow(scr_width, scr_height, "HW2.2- Shadow", NULL, NULL);
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
    // -----------------------------
    gl::Enable(gl::Capability::DepthTest);

    // ���ض�����ɫ��
    // ------------------------------------
    gl::Shader p3t2n3_vs(gl::ShaderType::VertexShader, "../data/shaders/p3t2n3.vert");
    gl::Shader p3_vs(gl::ShaderType::VertexShader, "../data/shaders/p3.vert");


    // ����ƬԪ��ɫ��
    gl::Shader light_shadow_fs(gl::ShaderType::FragmentShader, "../data/shaders/light_shadow.frag");
    // ����������Ӱӳ���fragment shader
    gl::Shader empty_fs(gl::ShaderType::FragmentShader, "../data/shaders/empty.frag");

    gl::Program light_shadow_program(&p3t2n3_vs, &light_shadow_fs);
    gl::Program shadow_program(&p3_vs, &empty_fs);

    float ambient = 0.2f;
    light_shadow_program.SetTex("color_texture", 0);
    light_shadow_program.SetTex("shadowmap", 1);
    light_shadow_program.SetVecf3("point_light_pos", { 0,10,0 });
    light_shadow_program.SetVecf3("point_light_radiance", { 200,200,200 });
    light_shadow_program.SetFloat("ambient", ambient);

    // ����Сţģ��
    // ------------------------------------------------------------------
    auto spot = SimpleLoader::LoadObj("../data/models/spot_triangulated_good.obj");
    gl::Texture2D spot_color = loadTexture("../data/textures/spot_albedo.png");

    // ����ƽ��
    auto plane = SimpleLoader::LoadObj("../data/models/plane.obj");
    gl::Texture2D plane_texture = loadTexture("../data/textures/checkerboard.png");
    pointf3 plane_pos(0.0f, -3.0f, -8.0f);
    scalef3 plane_size(20.0f, 1.0f, 20.0f);
    transformf plane_model(plane_pos, plane_size);
    
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

    

    // Ϊ��Ӱӳ�䴴�������� ���������Ҫ��
    // -------------------------
    const size_t SHADOW_TEXTURE_SIZE = 1024;
    gl::Texture2D shadowmap;
    shadowmap.SetImage(0, gl::PixelDataInternalFormat::DepthComponent, SHADOW_TEXTURE_SIZE, SHADOW_TEXTURE_SIZE,
        gl::PixelDataFormat::DepthComponent, gl::PixelDataType::Float, 0);
    shadowmap.SetWrapFilter(gl::WrapMode::ClampToBorder, gl::WrapMode::ClampToBorder,
        gl::MinFilter::Nearest, gl::MagFilter::Nearest);
    rgbaf borderColor{ 1.f,1.f,1.f,1.f };
    gl::TexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor.data());
    gl::FrameBuffer shadowFB;
    shadowFB.Attach(gl::FramebufferAttachment::DepthAttachment, &shadowmap);

    //  ��Ⱦѭ��
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

        //  �����￪ʼ�ǻ��Ʋ���
        // ------

        // ��Ӱ����
        // �󶨻��������Ӷ�ʹ��������������Ӱӳ��ͼ��
        shadowFB.Bind();
        gl::Viewport({ 0,0 }, SHADOW_TEXTURE_SIZE, SHADOW_TEXTURE_SIZE);
        gl::Clear(gl::BufferSelectBit::DepthBufferBit);

        // TODO: HW2.2 ������Ӱӳ��
        // ���������Ӱӳ�������View��Projection����
        // -------
        // ref: https://learnopengl-cn.github.io/05%20Advanced%20Lighting/03%20Shadows/01%20Shadow%20Mapping/
        float near = 0.1f, far = 100.0f;
        transformf lightProjection = transformf::perspective(to_radian(90.0f), (float)scr_width / (float)scr_height, near, far);
        transformf lightView = transformf::look_at(pointf3(0.0f, 10.0f, 0.0f), {0.0f, -3.0f, -8.0f}, vecf3(0.0, 0.0, -1.0));
        transformf lightSpaceMatrix = lightProjection * lightView;
        shadow_program.SetMatf4("projection", lightProjection);
        shadow_program.SetMatf4("view", lightView);

        // ��Ӱӳ��ͼ��Ⱦ
        //=================================
        for (unsigned int i = 0; i < 10; i++)
        {
            float angle = 20.0f * i + 10.f * (float)glfwGetTime();
            transformf model(instancePositions[i], quatf{ vecf3(1.0f, 0.3f, 0.5f), to_radian(angle) });
            shadow_program.SetMatf4("model", model);
            spot->va->Draw(&shadow_program);
        }
        shadow_program.SetMatf4("model", plane_model);
        plane->va->Draw(&shadow_program);

        // ��֡�������󶨻�Ĭ�ϻ������������������ݽ����Ƶ���Ļ��
        gl::FrameBuffer::BindReset(); 
        gl::Viewport({ 0,0 }, scr_width, scr_height);
        gl::ClearColor({ ambient, ambient, ambient, 1.0f });
        gl::Clear(gl::BufferSelectBit::ColorBufferBit | gl::BufferSelectBit::DepthBufferBit);

        light_shadow_program.SetVecf3("camera_pos", camera.Position);

        // ��shadowmap
        light_shadow_program.Active(0, &spot_color);
        light_shadow_program.Active(1, &shadowmap);

        // ����ͶӰ���󲢷��͵���ɫ��
        transformf projection = transformf::perspective(to_radian(camera.Zoom), (float)scr_width / (float)scr_height, 0.1f, 100.f);
        light_shadow_program.SetMatf4("projection", projection);

        // �ӽǾ���
        light_shadow_program.SetMatf4("view", camera.GetViewMatrix());
        // ������Ӱ�������ڳ�������ʱ�ÿո����
        light_shadow_program.SetBool("have_shadow", have_shadow);

        // TODO: 2.2:������ɫ���л�����Ӱ��ص����ݷ��͵���ɫ��
        light_shadow_program.SetMatf4("lightSpaceMatrix", lightSpaceMatrix);

        // ��Ⱦģ��
        for (unsigned int i = 0; i < 10; i++)
        {
            // ����ģ�;���ģ�Ͳ�����ת
            float angle = 20.0f * i + 10.f * (float)glfwGetTime();
            transformf model(instancePositions[i], quatf{ vecf3(1.0f, 0.3f, 0.5f), to_radian(angle) });
            light_shadow_program.SetMatf4("model", model);
            spot->va->Draw(&light_shadow_program);
        }
        light_shadow_program.Active(0, &plane_texture);
        light_shadow_program.SetMatf4("model", plane_model);
        plane->va->Draw(&light_shadow_program);

        // ��buffer�л��Ƶ����ݻ�������չʾ����Ļ��
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ������Զ����δ�ͷ���Դ�����ݿ�������������ͷ�
    // ------------------------------------------------------------------------
    delete spot;
    delete plane;


    // ���������ͷ�����GLFW��Դ
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// ����GLFW API���ƶ��ӽ�λ�ã��ո������Ӱ��ʾ
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
        have_shadow = !have_shadow;
    }
}

// glfw: ��Ļ��С���ı�ʱ����
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    gl::Viewport({ 0, 0 }, width, height);
    scr_width = width;
    scr_height = height;
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
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
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
