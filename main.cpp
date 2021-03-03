#include "GLFW/glfw3.h"
#include "vendor/Glad/include/glad/glad.h"

#include "vendor/stb_image/stb_image.h"

#include <array>
#include <fstream>
#include <string>
#include <cstdlib>

#include <vector>

#include <iostream>

using namespace std;

const int   WINDOW_WIDTH = 720, WINDOW_HEIGHT = 720;

class Texture
{
    private:
        unsigned int    texture;
    public:
        double              new_time, old_time, delta_time;
        array<float, 2>     size, offset;
        array<int, 2>       frames;

        // Texture(array<float, 2> s, array<float, 2> o, array<int, 2> f): size(s), offset(o), frames(f)
        // {
        //     if (frames[0] != frames[1])
        //         old_time = glfwGetTime ();
        // }

        void TexInstall(const char* texture_path)
        {
            int width, height, cnt;
            unsigned char* data = stbi_load (texture_path, &width, &height, &cnt, 0);

            if (!data)
                cout << "Failed to load texture" << endl;
            
            glEnable (GL_TEXTURE_2D);
            glGenTextures (1, &texture);
            glBindTexture (GL_TEXTURE_2D, texture);
                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, width, height,
                                            0, cnt == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }

        void TexSet(array<float, 2>s, array<float, 2> of, array<int, 2> fr)
        {
            size[0] = s[0];
            size[1] = s[1];
            offset[0] = of[0];
            offset[1] = of[1];
            frames[0] = fr[0];
            frames[1] = fr[1];
            if (frames[1] != frames[0])
                old_time = glfwGetTime ();
        }

        void TexBind()
        {
            glBindTexture (GL_TEXTURE_2D, texture);
        }

        void TexAnimation(float y_line, float time_speed)
        {
            new_time = glfwGetTime ();
            delta_time = new_time - old_time;

            offset[1] = y_line;
            //cout << delta_time <<endl;

            if (delta_time > time_speed)
            {
                //cout << "delta_time = " << delta_time << "; time_speed = " << time_speed << endl;
                if (frames[1] < frames[0])
                {
                    frames[1]++;
                    offset[0] += size[0];
                }
                else
                {
                    frames[1] = 1;
                    offset[0] = 0.f;
                }
                old_time = new_time;
            }
        }
};

class Object
{
    private:
        // float coordinates[2], sides[2];
        Texture         tex;
    public:
        array<float, 2> coordinates;
        array<float, 2> sides;

        Object(array<float, 2> c, array<float, 2> s): 
        coordinates(c), sides (s)
        {}
        Object()
        {}
        void TexInstall(const char* texture_path)
        {
            tex.TexInstall(texture_path);
        }

        void TexSet(array<float, 2> size, array<float, 2> offset, array<int, 2> frames)
        {
            tex.TexSet(size, offset, frames);
        }

        void StartObj(float y_line, float time_speed)
        {
            tex.TexBind();
            tex.TexAnimation(y_line, time_speed);
            DrawObj();
        }

        void StartObj()
        {
            tex.TexBind();
            DrawObj();
        }

        double GiveTexTime()
        {
            return (tex.old_time);
        }

        void ChangeTexTime(double time)
        {
            tex.old_time = time;
        }

        void DrawObj()
        {
            glBegin(GL_QUADS);
                glTexCoord2f (0.f + tex.offset[0], tex.size[1] + tex.offset[1]);
                    glVertex2f (coordinates[0] - sides[0]/2, coordinates[1] - sides[1]/2);
                glTexCoord2f (0.f + tex.offset[0], 0.f + tex.offset[1]);
                    glVertex2f (coordinates[0] - sides[0]/2, coordinates[1] + sides[1]/2);
                glTexCoord2f (tex.size[0] + tex.offset[0], 0.f + tex.offset[1]);
                    glVertex2f (coordinates[0] + sides[0]/2, coordinates[1] + sides[1]/2);
                glTexCoord2f (tex.size[0] + tex.offset[0], tex.size[1] + tex.offset[1]);
                    glVertex2f (coordinates[0] + sides[0]/2, coordinates[1] - sides[1]/2);
            glEnd();
        }
};

//Transparent settings
void standart_transparent(Object& obj, float y_line, float time_speed)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (time_speed)
    {
        //cout << "Here " << time_speed << endl;
        obj.StartObj(y_line, time_speed);
    }
    else
        obj.StartObj();
}

void rain_transparent(Object& rain, float y_line, float time_speed)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR);
    glBlendEquation(GL_FUNC_ADD);

    rain.StartObj(y_line, time_speed);
}

//keyboard start
struct keyboard_statistics
{
    int     keys[1024];
    bool    status;
    float   step = 3.f;

    float   backup_coordinates[2];
    bool    may_go = true;
}keyboard;

void OnKeyboardPressed(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (action == GLFW_PRESS)
        keyboard.keys[key] = keyboard.status = true;
	else 
        if (action == GLFW_RELEASE)
            keyboard.keys[key] = keyboard.status = false;
}

void WriteBackUpCoord (float x, float y)
{
    keyboard.backup_coordinates[0] = x;
    keyboard.backup_coordinates[1] = y;
}

void processPlayerMovement(Object &player, float time_speed, float time)
{
    double delta_time = glfwGetTime () - time;
    if (keyboard.status)
    {
        if (keyboard.may_go == true)
        {
            if (keyboard.keys[GLFW_KEY_W] || keyboard.keys[GLFW_KEY_UP])
            {
                if ((keyboard.backup_coordinates[0] == player.coordinates[0]) && (keyboard.backup_coordinates[1] == player.coordinates[1]))
                    standart_transparent(player, 256.f/320.f, 0.09f);
                else
                    standart_transparent(player, 0.f, time_speed);
                WriteBackUpCoord(player.coordinates[0], player.coordinates[1]);
                player.coordinates[1] += keyboard.step * delta_time;
            }
            else if (keyboard.keys[GLFW_KEY_S] || keyboard.keys[GLFW_KEY_DOWN])
            {
                if ((keyboard.backup_coordinates[0] == player.coordinates[0]) && (keyboard.backup_coordinates[1] == player.coordinates[1]))
                    standart_transparent(player, 256.f/320.f, 0.09f);
                else
                    standart_transparent(player, 64.f/320.f, time_speed);
                WriteBackUpCoord(player.coordinates[0], player.coordinates[1]);
                player.coordinates[1] -= keyboard.step * delta_time;
            }
            else if (keyboard.keys[GLFW_KEY_A] || keyboard.keys[GLFW_KEY_LEFT])
            {
                if ((keyboard.backup_coordinates[0] == player.coordinates[0]) && (keyboard.backup_coordinates[1] == player.coordinates[1]))
                    standart_transparent(player, 256.f/320.f, 0.09f);
                else
                    standart_transparent(player, 128.f/320.f, time_speed);
                WriteBackUpCoord(player.coordinates[0], player.coordinates[1]);
                player.coordinates[0] -= keyboard.step * delta_time;
            }
            else if (keyboard.keys[GLFW_KEY_D] || keyboard.keys[GLFW_KEY_RIGHT])
            {
                if ((keyboard.backup_coordinates[0] == player.coordinates[0]) && (keyboard.backup_coordinates[1] == player.coordinates[1]))
                    standart_transparent(player, 256.f/320.f, 0.09f);
                else
                    standart_transparent(player, 192.f/320.f, time_speed);
                WriteBackUpCoord(player.coordinates[0], player.coordinates[1]);
                player.coordinates[0] += keyboard.step * delta_time;
            }
            else
                standart_transparent(player, 256.f/320.f, 0.09f);
        }
        else
        {
            player.coordinates[0] = keyboard.backup_coordinates[0];
            player.coordinates[1] = keyboard.backup_coordinates[1];
            keyboard.may_go = true;
            standart_transparent(player, 256.f/320.f, 0.09f);
        }
    }
    else
        standart_transparent(player, 256.f/320.f, 0.09f);
}

//keyboard end
bool MayGo(Object &map, Object &player, array <float, 8> player_offset)
{
    float p_x = player.coordinates[0], p_y = player.coordinates[1], p_half_side_x = player.sides[0]/2.f, p_half_side_y = player.sides[1]/2.f;
    float player_coordinates[4][2] = {{p_x - p_half_side_x + player_offset[0], p_y + p_half_side_y + player_offset[1]}, {p_x + p_half_side_x + player_offset[2], p_y + p_half_side_y + player_offset[3]}, {p_x - p_half_side_x + player_offset[4], p_y - p_half_side_y + player_offset[5]}, {p_x + p_half_side_x + player_offset[6], p_y - p_half_side_y + player_offset[7]}};

    float m_x = map.coordinates[0], m_y = map.coordinates[1], m_half_side_x = map.sides[0]/2.f, m_half_side_y = map.sides[1]/2.f;
    float map_block[2][2] = {{m_x - m_half_side_x, m_y + m_half_side_y}, {m_x + m_half_side_x, m_y - m_half_side_y}};

    for (int i = 0; i < 4; i++)
    {
        //cout << "Player" << endl << player_coordinates[i][0] << "; " << player_coordinates[i][1] << ";" << endl;
        if(((player_coordinates[i][0] > map_block[0][0]) && (player_coordinates[i][0] < map_block[1][0]) && (player_coordinates[i][1] < map_block[0][1]) && (player_coordinates[i][1] > map_block[1][1])) == true)
        {
            // cout << "Map " << map.coordinates[0] << map.coordinates[1] << endl;
            // cout << "Map" << endl << "1) map_block"<< map_block[0][0] << "; "<< map_block[0][1] << "; 2nd:" << map_block[1][0] << ";" << map_block[1][1] << "; " << endl;
            // cout << "2) Sides "<< map.sides[0] << "; "<< map.sides[1] << endl;
            // cout << "3) M_coord "<< map.coordinates[0] << "; "<< map.coordinates[1] << endl;
            // cout << "4) P_coord "<< player.coordinates[0] << "; "<< player.coordinates[1] << endl;
            // cout << "5) player_block"<< player_coordinates[0][0] << "; "<< player_coordinates[0][1] << "; 2nd:" << player_coordinates[1][0] << ";" << player_coordinates[1][1] << "; " << endl;
            return false;
        }
    }
    return true;
}
    
    // return true;

    // float player_coordinates[1][2] = {{player.coordinates[0], player.coordinates[1]}};
    // for (int i = 0; i < 1; i++)
    // {
    //     if (((player.coordinates[0] > map.coordinates[0]) && (player.coordinates[0] < (map.coordinates[0] + map.sides[0]/2.f)) && (player.coordinates[1] > (map.coordinates[1] - map.sides[1]/2.f)) && (player.coordinates[1] < map.coordinates[1])) == true)
    //         return false;
    //     return true;
    // }

struct frame_rate
{
    double t = 0.0;
    double dt = 1.0 / 30.0;
}fps;

typedef struct
{
    array <float, 4>    grass_coord_x;
    array <float, 2>    grass_coord_y;
    vector<int>         grass_index;

    array <float, 5>    way_coord_x;
    vector<int>         way_index;

    float               darkness_step = 200.f/12000.f;
    array<int, 2>       darkness_frames = {60, 2};
    array<int, 2>       darkness_block_ij = {0, 0};
    array<float, 2>     darkness_offset = {0.f, 0.f};
    bool                darkness_status = false;  
    array<float, 3>     darkness_time = {0.f, 0.f, 0.09f};  //new_time, time, time_frames
}MapGen;


class Mapper
{
private:
    Object              map;
    array<float, 2>     map_size;
    array<string, 16>   symbols;
    bool                player_start;

    Object              darkness;
    Object              chest;

    MapGen              mapg;    
public:
    Mapper(array<float, 2> c, array<float, 2> s, const char* texture_path, array<float, 2> m_s): 
    map(c, s), map_size(m_s)
    {
        player_start = false;

        darkness.sides = {0.125f, 0.125f};
        darkness.TexInstall("resources/eye.png");
        darkness.TexSet({200.f/12000.f, 200.f/200.f}, {0.f, 0.f}, {60, 1});

        chest.sides = {0.125f * 1.2, 0.125f * 1.2};
        chest.TexInstall("resources/chest.png");
        chest.TexSet({72.f/288.f, 72.f/288.f}, {0.f, 0.f}, {4, 1});

        //Для генерации
        srand(clock());
        mapg.grass_coord_x = {96.f/map_size[0], 112.f/map_size[0], 128.f/map_size[0], 144.f/map_size[0]};
        mapg.grass_coord_y = {48.f/map_size[1], 64.f/map_size[1]};
        
        mapg.way_coord_x = {0.f, 16.f/map_size[0], 0.f, 32.f/map_size[0], 0.f};

        mapg.darkness_time[1] = glfwGetTime();
        //Установка тектуры
        map.TexInstall(texture_path);
    }

    void TexSetAndDo(array<float, 2> size, array<float, 2> offset, array<int, 2> frames)
    {
        map.TexSet(size, offset, frames);
        standart_transparent(map, 0.f, 0);
    }

    void DrawMap(Object& player)
    {
        int grass_num = 0, way_num = 0, wall_num = 0;

        for (int i = 0; i < 16; i++)
        {

            for (int j = 0; j < 16; j++)
            {
                switch (symbols[j][i])
                {
                case '.':
                    if (keyboard.status == true && (MayGo(map, player, {player.sides[0]/2.f, -player.sides[1], -player.sides[0]/2.f, -player.sides[1], (player.sides[0]/2.f - 0.01f), 0.f, -(player.sides[0]/2.f - 0.01f), 0.f}) == false))
                        cout << "Dark " << i <<endl;
                    darkness.coordinates = {map.coordinates[0], map.coordinates[1]};
                    darkness.TexSet({200.f/12000.f, 200.f/200.f}, {0.f, 0.f}, {1, 1});
                    standart_transparent(darkness, 0.f/147.f, 0.f);
                    if (mapg.darkness_status == false)
                    {
                        mapg.darkness_time[0] = glfwGetTime();
                        if (((mapg.darkness_time[0] -  mapg.darkness_time[1]) > mapg.darkness_time[2]) && ((float)rand()/RAND_MAX * 10000) > 9999)
                        {
                            //cout<< "here"<<endl;
                            mapg.darkness_status = true;
                            mapg.darkness_block_ij = {i, j};
                            mapg.darkness_offset = {200.f/12000.f, 0.f};
                            //mapg.darkness_frames = {60, 2};
                            
                            darkness.TexSet({200.f/12000.f, 200.f/200.f}, {mapg.darkness_offset[0], mapg.darkness_offset[1]}, {60, 2});
                            standart_transparent(darkness, 0.f/200.f, mapg.darkness_time[2]);

                            mapg.darkness_time[2] = (3 + rand()%15)/100.f;
                            mapg.darkness_time[1] = mapg.darkness_time[0];
                        }
                        else
                        {
                            darkness.TexSet({200.f/12000.f, 200.f/200.f}, {0.f, 0.f}, {1, 1});
                            standart_transparent(darkness, 0.f/147.f, 0.f);
                        }
                    }
                    else
                    {
                        if (mapg.darkness_block_ij[0] == i && mapg.darkness_block_ij[1] == j)
                        {
                            if (mapg.darkness_offset[0] < 1.f)
                            {
                                //cout << mapg.darkness_offset[0]<<endl;
                                //darkness.ChangeTexTime(mapg.darkness_old_time);
                                mapg.darkness_time[0] = glfwGetTime();
                                if ((mapg.darkness_time[0] -  mapg.darkness_time[1]) > mapg.darkness_time[2])
                                {
                                    mapg.darkness_offset[0] += mapg.darkness_step;
                                    mapg.darkness_frames[1]++;
                                    mapg.darkness_time[1] = mapg.darkness_time[0];
                                }

                                darkness.TexSet({200.f/12000.f, 200.f/200.f}, {mapg.darkness_offset[0], mapg.darkness_offset[1]}, {60, 2});
                                standart_transparent(darkness, 0.f/200.f, mapg.darkness_time[2]);
                            }
                            else
                            {
                                mapg.darkness_status = false;
                                mapg.darkness_time[2] *= 30;
                                darkness.TexSet({200.f/12000.f, 200.f/200.f}, {0.f, 0.f}, {1, 1});
                                standart_transparent(darkness, 0.f/147.f, 0.f);
                            }
                        }
                    }
                    break;
                case '@':
                    if (player_start == false)
                    {
                        player.coordinates[0] = map.coordinates[0] - 0.01f;
                        player.coordinates[1] = map.coordinates[1] + map.sides[1]/2.f;
                        player_start = true;
                    }
                    if (mapg.way_index.size() < (way_num + 1))
                            mapg.way_index.push_back(((float)rand()/RAND_MAX * 5));
                        TexSetAndDo({16.f/map_size[0], 16.f/map_size[1]}, {mapg.way_coord_x[mapg.way_index[way_num]], 48.f/map_size[1]}, {1, 1});
                        way_num++;
                    break;
                case '#':
                    //cout << "Map" << endl << map.coordinates[0] << "; " << map.coordinates[1] << endl;
                    if (keyboard.status == true && (MayGo(map, player, {0.04f, 0.f, -0.03f, 0.f, 0.04f, 0.f, -0.03f, 0.f}) == false))
                        keyboard.may_go = false;
                    //if (keyboard.may_go == false)
                        //cout << keyboard.may_go <<endl;
                    TexSetAndDo({16.f/map_size[0], 16.f/map_size[1]}, {64.f/map_size[0], 0.f/map_size[1]}, {1, 1});
                    break;
                case '!':
                    if (!wall_num)
                    {
                        wall_num++;
                        if (keyboard.status == true && (MayGo(map, player, {-0.03f, -0.03f, -0.03f, -0.03f, -0.03f, 0.f, -0.3f, 0.f}) == false))
                            keyboard.may_go = false;
                    }
                    else
                        wall_num = 0;
                    TexSetAndDo({16.f/map_size[0], 16.f/map_size[1]}, {24.f/map_size[0], 16.f/map_size[1]}, {1, 1});
                    break;
                case 'a':
                    TexSetAndDo({16.f/map_size[0], 16.f/map_size[1]}, {96.f/map_size[0], 0.f/map_size[1]}, {1, 1});
                    break;
                case 'b':
                    TexSetAndDo({16.f/map_size[0], 16.f/map_size[1]}, {112.f/map_size[0], 0.f/map_size[1]}, {1, 1});
                    break;
                case 'c':
                    TexSetAndDo({16.f/map_size[0], 16.f/map_size[1]}, {128.f/map_size[0], 0.f/map_size[1]}, {1, 1});
                    break;
                case 'd':
                    TexSetAndDo({16.f/map_size[0], 16.f/map_size[1]}, {96.f/map_size[0], 16.f/map_size[1]}, {1, 1});
                    break;
                case 'x':
                    TexSetAndDo({16.f/map_size[0], 16.f/map_size[1]}, {112.f/map_size[0], 16.f/map_size[1]}, {1, 1});
                    break;
                case 'e':
                    TexSetAndDo({16.f/map_size[0], 16.f/map_size[1]}, {128.f/map_size[0], 16.f/map_size[1]}, {1, 1});
                    break;
                case '~':
                    if (mapg.way_index.size() < (way_num + 1))
                        mapg.way_index.push_back(((float)rand()/RAND_MAX * 5));
                    TexSetAndDo({16.f/map_size[0], 16.f/map_size[1]}, {mapg.way_coord_x[mapg.way_index[way_num]], 48.f/map_size[1]}, {1, 1});
                    way_num++;
                    break;
                case ')':
                    TexSetAndDo({16.f/map_size[0], 16.f/map_size[1]}, {48.f/map_size[0], 64.f/map_size[1]}, {1, 1});
                    break;
                case '(':
                    TexSetAndDo({16.f/map_size[0], 16.f/map_size[1]}, {64.f/map_size[0], 64.f/map_size[1]}, {1, 1});
                    break;
                case 'o':
                    TexSetAndDo({16.f/map_size[0], 16.f/map_size[1]}, {0.f/map_size[0], 64.f/map_size[1]}, {1, 1});
                    break;
                case '=':
                    TexSetAndDo({16.f/map_size[0], 16.f/map_size[1]}, {192.f/map_size[0], 48.f/map_size[1]}, {1, 1});
                    break;
                case '*':
                {
                    int grass_index_y = 0;

                    if (mapg.grass_index.size() < (grass_num + 1))
                        mapg.grass_index.push_back(((float)rand()/RAND_MAX * 4));
                    if (mapg.grass_index[grass_num] > 1)
                        grass_index_y = 1;

                    TexSetAndDo({16.f/map_size[0], 16.f/map_size[1]}, {mapg.grass_coord_x[mapg.grass_index[grass_num]], mapg.grass_coord_y[grass_index_y]}, {1, 1});
                    grass_num++;
                    break;
                }
                case 'G':
                {
                    if (keyboard.status == true && (MayGo(map, player, {player.sides[0]/2.f, -player.sides[1] + 0.02f, -player.sides[0]/2.f, -player.sides[1] + 0.02f, (player.sides[0]/2.f - 0.02f), 0.02f, -(player.sides[0]/2.f - 0.03f), 0.02f}) == false))
                        keyboard.may_go = false;
                    if (keyboard.status == true && keyboard.keys[GLFW_KEY_E] && (MayGo(map, player, {0.f,0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}) == false))
                        cout << "Open" << endl;
                    chest.coordinates[0] = map.coordinates[0];
                    chest.coordinates[1] = map.coordinates[1] + 0.02;
                    chest.TexSet({72.f/288.f, 72.f/72.f}, {0.f, 0.f}, {1, 1});
                    standart_transparent(chest, 0.f, 0.f);
                    break;
                }
                default:
                    break;
                }
                map.coordinates[1] -= 0.125f;
            }
            map.coordinates[0] += 0.125f;
            map.coordinates[1] = 1.f - 0.125/2;
        }
        map.coordinates[0] = -1.f + 0.125/2;
        map.coordinates[1] = 1.f - 0.125/2;
    }

    void LoadFile(const char* file_name)
    {
        ifstream    file;
        int         i = 0;
        
        file.open(file_name);
        if (file)
        {
            for (string line; getline (file, line);)
            {
                symbols[i] = line.c_str();
                i++;
            }
        }
        else
            cout << "Failed to open file." << endl;
        for (int i = 0; i < 16; i++)
        {
            cout << symbols[i]<< endl;
        }        
    }
    
};

int main()
{
    GLFWwindow* window;

    if (!glfwInit ())
        return -1;
    window = glfwCreateWindow (WINDOW_WIDTH, WINDOW_HEIGHT, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate ();
        return -1;
    }
    glfwMakeContextCurrent (window);
    if(gladLoadGLLoader ((GLADloadproc) glfwGetProcAddress) == 0)
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }
    glfwSetKeyCallback (window, OnKeyboardPressed);

    //координаты объекта, стороны объекта
    Object      player({0.f, 0.f}, {0.2f, 0.2f});
    Object      rain({0.f, 0.f}, {2.f, 2.f});
    //Object      m({-0.91f, 0.91f}, {0.16f, 0.16f});
    //Object      map({0.f, 0.f}, {2.f, 2.f}, {720.f/720.f, 720.f/720.f}, {0.f, 0.f}, {1, 1}, "resources/t.png");

    Mapper      map({(-1.f + 0.125/2), (1.f - 0.125/2)}, {0.125f, 0.125f}, "resources/map.png", {256.f, 96.f});

    map.LoadFile("resources/m1.txt");

    //путь к текстуре
    player.TexInstall("resources/player.png");
    rain.TexInstall("resources/rain.png");
    //m.TexInstall("resources/map.png");

    //размер текстуры, смещение текстуры, кадры(всего, начальный)
    player.TexSet({64.f/768.f, 64.f/320.f}, {0.f, 0.f}, {12, 1});
    rain.TexSet({240.f/14400.f, 240.f/240.f}, {0.f, 0.f}, {60, 1});
    //m.TexSet({16.f/256.f, 16.f/96.f}, {64.f/256.f, 0.f}, {1, 1});

    glfwSwapInterval(1);

    float time;
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        // if (fps.t >= 1)
        // {
            time = glfwGetTime();
            glClear(GL_COLOR_BUFFER_BIT);

            //processPlayerMovement (player_loc);
            glClearColor(32.0f/255.0f, 39.0f/255.0f, 44.0f/255.0f, 1.0f);
            //player.StartObj(0.f);
            //map.StartObj(0.f, 5.f);

            //Размер кадра из текстуры
            //standart_transparent(m, 0.f, 0);
            map.DrawMap(player);
            processPlayerMovement (player, 0.06f, time);
            rain_transparent(rain, 0.f, 0.06);

        //     fps.t = 0.0;
        // }
        // else
        //     fps.t += fps.dt;

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}