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

// 屏幕设置
unsigned int scr_width = 800;
unsigned int scr_height = 600;
bool have_shadow = false;
bool isPressingKeySpace = false;

//  摄像机，详见Camera.cpp
Camera camera(pointf3(0.0f, 0.0f, 3.0f));
float lastX = scr_width / 2.0f;
float lastY = scr_height / 2.0f;
bool firstMouse = true;

// 计时
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

int main()
{
    // glfw初始化
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口
    // --------------------
    GLFWwindow* window = glfwCreateWindow(scr_width, scr_height, "HW2.2- Shadow", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    //设定事件发生后的回调函数，类似Qt中的connect
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: 加载OpenGL函数指针
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 配置Opengl全局状态，可详见UGL实现
    // -----------------------------
    gl::Enable(gl::Capability::DepthTest);

    // 加载顶点着色器
    // ------------------------------------
    gl::Shader p3t2n3_vs(gl::ShaderType::VertexShader, "../data/shaders/p3t2n3.vert");
    gl::Shader p3_vs(gl::ShaderType::VertexShader, "../data/shaders/p3.vert");


    // 加载片元着色器
    gl::Shader light_shadow_fs(gl::ShaderType::FragmentShader, "../data/shaders/light_shadow.frag");
    // 用于生成阴影映射的fragment shader
    gl::Shader empty_fs(gl::ShaderType::FragmentShader, "../data/shaders/empty.frag");

    gl::Program light_shadow_program(&p3t2n3_vs, &light_shadow_fs);
    gl::Program shadow_program(&p3_vs, &empty_fs);

    float ambient = 0.2f;
    light_shadow_program.SetTex("color_texture", 0);
    light_shadow_program.SetTex("shadowmap", 1);
    light_shadow_program.SetVecf3("point_light_pos", { 0,10,0 });
    light_shadow_program.SetVecf3("point_light_radiance", { 200,200,200 });
    light_shadow_program.SetFloat("ambient", ambient);

    // 加载小牛模型
    // ------------------------------------------------------------------
    auto spot = SimpleLoader::LoadObj("../data/models/spot_triangulated_good.obj");
    gl::Texture2D spot_color = loadTexture("../data/textures/spot_albedo.png");

    // 加载平面
    auto plane = SimpleLoader::LoadObj("../data/models/plane.obj");
    gl::Texture2D plane_texture = loadTexture("../data/textures/checkerboard.png");
    pointf3 plane_pos(0.0f, -3.0f, -8.0f);
    scalef3 plane_size(20.0f, 1.0f, 20.0f);
    transformf plane_model(plane_pos, plane_size);
    
    // 位置
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

    

    // 为阴影映射创建缓冲区 （这个很重要）
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

    //  渲染循环
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // 为每帧计算时间
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 输入
        // -----
        processInput(window);
        processRelease(window);

        //  从这里开始是绘制部分
        // ------

        // 阴影部分
        // 绑定缓冲区，从而使后续绘制是在阴影映射图上
        shadowFB.Bind();
        gl::Viewport({ 0,0 }, SHADOW_TEXTURE_SIZE, SHADOW_TEXTURE_SIZE);
        gl::Clear(gl::BufferSelectBit::DepthBufferBit);

        // TODO: HW2.2 绘制阴影映射
        // 计算绘制阴影映射所需的View和Projection矩阵
        // -------
        // ref: https://learnopengl-cn.github.io/05%20Advanced%20Lighting/03%20Shadows/01%20Shadow%20Mapping/
        float near = 0.1f, far = 100.0f;
        transformf lightProjection = transformf::perspective(to_radian(90.0f), (float)scr_width / (float)scr_height, near, far);
        transformf lightView = transformf::look_at(pointf3(0.0f, 10.0f, 0.0f), {0.0f, -3.0f, -8.0f}, vecf3(0.0, 0.0, -1.0));
        transformf lightSpaceMatrix = lightProjection * lightView;
        shadow_program.SetMatf4("projection", lightProjection);
        shadow_program.SetMatf4("view", lightView);

        // 阴影映射图渲染
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

        // 将帧缓冲区绑定回默认缓冲区，后续绘制内容将绘制到屏幕上
        gl::FrameBuffer::BindReset(); 
        gl::Viewport({ 0,0 }, scr_width, scr_height);
        gl::ClearColor({ ambient, ambient, ambient, 1.0f });
        gl::Clear(gl::BufferSelectBit::ColorBufferBit | gl::BufferSelectBit::DepthBufferBit);

        light_shadow_program.SetVecf3("camera_pos", camera.Position);

        // 绑定shadowmap
        light_shadow_program.Active(0, &spot_color);
        light_shadow_program.Active(1, &shadowmap);

        // 计算投影矩阵并发送到着色器
        transformf projection = transformf::perspective(to_radian(camera.Zoom), (float)scr_width / (float)scr_height, 0.1f, 100.f);
        light_shadow_program.SetMatf4("projection", projection);

        // 视角矩阵
        light_shadow_program.SetMatf4("view", camera.GetViewMatrix());
        // 有无阴影，可以在程序运行时用空格控制
        light_shadow_program.SetBool("have_shadow", have_shadow);

        // TODO: 2.2:将在着色器中绘制阴影相关的内容发送到着色器
        light_shadow_program.SetMatf4("lightSpaceMatrix", lightSpaceMatrix);

        // 渲染模型
        for (unsigned int i = 0; i < 10; i++)
        {
            // 计算模型矩阵，模型不断旋转
            float angle = 20.0f * i + 10.f * (float)glfwGetTime();
            transformf model(instancePositions[i], quatf{ vecf3(1.0f, 0.3f, 0.5f), to_radian(angle) });
            light_shadow_program.SetMatf4("model", model);
            spot->va->Draw(&light_shadow_program);
        }
        light_shadow_program.Active(0, &plane_texture);
        light_shadow_program.SetMatf4("model", plane_model);
        plane->va->Draw(&light_shadow_program);

        // 在buffer中绘制的内容会在这里展示到屏幕上
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 如果有自定义的未释放资源的数据可以在这里调用释放
    // ------------------------------------------------------------------------
    delete spot;
    delete plane;


    // 结束程序，释放所有GLFW资源
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// 调用GLFW API，移动视角位置，空格控制阴影显示
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

// glfw: 屏幕大小被改变时调用
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    gl::Viewport({ 0, 0 }, width, height);
    scr_width = width;
    scr_height = height;
}


// glfw: 鼠标移动时调用
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

// glfw: 鼠标滚轮移动时调用
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}


// 加载纹理的函数
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
