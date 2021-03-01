#include "GLFW/glfw3.h"
#include "vendor/Glad/include/glad/glad.h"

#include "vendor/stb_image/stb_image.h"

#include <iostream>
#include <ctime>

using namespace std;

const int WINDOW_WIDTH = 720, WINDOW_HEIGHT = 720;

class player_texture
{
    private:
        float animation_interval, tex_x, tex_y;
    public:
        float tex_width, tex_height, tex_size;
};

struct player
{
    float           x = 0.f;
    float           y = 0.f;
    float           side = 0.2f;
    bool            keys[1024]{};
    float           step = 0.4f/WINDOW_WIDTH;
    float           tex_step[2] = {0, 0};
    float           player_size[2] = {64.f/768.f, 64.f/320.f};
    int             tex_counter = 1;
    GLfloat         key_timer = 0;
    player_texture  tex;
}player_loc;

unsigned int texture;

void GameTexture()
{
    int width, height, cnt;
    unsigned char* data = stbi_load("resources/player.png", &width, &height, &cnt, 0);

    if (!data)
        std::cout << "Failed to load texture" << std::endl;

    glEnable (GL_TEXTURE_2D);
    glGenTextures (1, &texture);
    glBindTexture (GL_TEXTURE_2D, texture);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, width, height,
                                     0, cnt == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
    //glBindTexture (GL_TEXTURE_2D, 0);
    stbi_image_free(data);
}

void OnKeyboardPressed(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (action == GLFW_PRESS)
        player_loc.keys[key] = true;
	else 
        if (action == GLFW_RELEASE)
        {
            player_loc.keys[key] = false;
        }
}

void PlayerTextureAnimation (player &player_loc, float y_step, int textures_count)
{
    double new_time = glfwGetTime ();
    player_loc.tex_step[1] = y_step;
        //cout << player_loc.tex_counter[0] <<endl;
    if ((new_time - player_loc.key_timer) > 0.09)
    {
        if (player_loc.tex_counter < textures_count)
        {
            player_loc.tex_counter++;
            player_loc.tex_step[0] += (player_loc.player_size[0]);
        }
        else
        {
            player_loc.tex_counter = 1;
            player_loc.tex_step[0] = 0.f;
        }
        player_loc.key_timer = new_time;
    }
}

void processPlayerMovement(player &player_loc)
{
    bool status = false;
    double new_time = glfwGetTime ();

    if (player_loc.keys[GLFW_KEY_W] || player_loc.keys[GLFW_KEY_UP])
    {
        status = true;
        PlayerTextureAnimation(player_loc, 0, 12);
        player_loc.y += player_loc.step;
    }
    if (player_loc.keys[GLFW_KEY_S] || player_loc.keys[GLFW_KEY_DOWN])
    {
        status = true;
        PlayerTextureAnimation(player_loc, 64.f/320.f, 12);
        player_loc.y -= player_loc.step;
    }
    if (player_loc.keys[GLFW_KEY_A] || player_loc.keys[GLFW_KEY_LEFT])
    {
        status = true;
        PlayerTextureAnimation(player_loc, 128.f/320.f, 12);
        player_loc.x -= player_loc.step;
    }
    if (player_loc.keys[GLFW_KEY_D] || player_loc.keys[GLFW_KEY_RIGHT])
    {
        status = true;
        PlayerTextureAnimation(player_loc, 192.f/320.f, 12);
        player_loc.x += player_loc.step;
    }
    
    if (status == false)
    {
        player_loc.tex_step[1] = 256.f/320.f;
        if (player_loc.tex_counter > 2)
        {
            player_loc.tex_counter = 1;
            player_loc.tex_step[0] = 0;
        }
        if ((new_time - player_loc.key_timer) > 0.7)
        {
            player_loc.tex_step[0] += player_loc.player_size[0];
            player_loc.tex_counter++;
            player_loc.key_timer = new_time;
        }
    }
}

void PlayerDraw()
{
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    //glDisable(GL_DITHER);
    glBegin(GL_QUADS);
        glTexCoord2f (0.f + player_loc.tex_step[0], player_loc.player_size[1] + player_loc.tex_step[1]);
            glVertex2f (player_loc.x - player_loc.side/2, player_loc.y - player_loc.side/2);
        glTexCoord2f (0.f + player_loc.tex_step[0], 0.f + player_loc.tex_step[1]);
            glVertex2f (player_loc.x - player_loc.side/2, player_loc.y + player_loc.side/2);
        glTexCoord2f (player_loc.player_size[0] + player_loc.tex_step[0], 0.f + player_loc.tex_step[1]);
            glVertex2f (player_loc.x + player_loc.side/2, player_loc.y + player_loc.side/2);
        glTexCoord2f (player_loc.player_size[0] + player_loc.tex_step[0], player_loc.player_size[1] + player_loc.tex_step[1]);
            glVertex2f (player_loc.x + player_loc.side/2, player_loc.y - player_loc.side/2);
    glEnd();
}

int main(void)
{
    GLFWwindow* window;
    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(WINDOW_WIDTH, 720, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    //glfwSwapInterval(1);

    if(gladLoadGLLoader((GLADloadproc) glfwGetProcAddress) == 0)
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }
 
    glfwSetKeyCallback (window, OnKeyboardPressed);  
    /* Loop until the user closes the window */
    player_loc.key_timer = glfwGetTime();
    
    GameTexture();
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);
        processPlayerMovement (player_loc);
        PlayerDraw();
        
        //glClearColor(32.0f/255.0f, 39.0f/255.0f, 44.0f/255.0f, 1.0f);

        
        //glClear(GL_COLOR_BUFFER_BIT);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}