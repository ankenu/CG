#include "GLFW/glfw3.h"
#include "vendor/Glad/include/glad/glad.h"

#include "vendor/stb_image/stb_image.h"

#include <array>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>

#include <vector>

#include <iostream>

using namespace std;

const int   WINDOW_WIDTH = 720, WINDOW_HEIGHT = 720;
struct scr_fl
{
    bool    death_check = false;
    bool    victory_check = false;
    bool    next_room = false;

    double  opacity = 0;
}screen_flag;

class Texture
{
    private:
        unsigned int    texture;
    public:
        double              new_time, old_time, delta_time;
        array<float, 2>     size, offset;
        array<int, 2>       frames;

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

            if (delta_time > time_speed)
            {
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

        float GetTexOffsetX()
        {
            return tex.offset[0];
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
    bool    status = false;
    float   step = 2.f;

    float   backup_coordinates[2];
    bool    may_go = true;
}keyboard;

void OnKeyboardPressed(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    switch (key)
	{
        case GLFW_KEY_ESCAPE:
            if (action == GLFW_PRESS)
                glfwSetWindowShouldClose(window, GL_TRUE);
            break;
        break;
        default:
            break;
	}
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

    if (keyboard.status && screen_flag.next_room == false && screen_flag.death_check == false && screen_flag.next_room == false)
    {
        if (keyboard.may_go == true)
        {
            if (keyboard.keys[GLFW_KEY_W] || keyboard.keys[GLFW_KEY_UP])
            {
                if ((keyboard.backup_coordinates[0] == player.coordinates[0]) && (keyboard.backup_coordinates[1] == player.coordinates[1]))
                    standart_transparent(player, 256.f/384.f, 0.09f);
                else
                    standart_transparent(player, 0.f, time_speed);
                WriteBackUpCoord(player.coordinates[0], player.coordinates[1]);
                player.coordinates[1] += keyboard.step * delta_time;
            }
            else if (keyboard.keys[GLFW_KEY_S] || keyboard.keys[GLFW_KEY_DOWN])
            {
                if ((keyboard.backup_coordinates[0] == player.coordinates[0]) && (keyboard.backup_coordinates[1] == player.coordinates[1]))
                    standart_transparent(player, 256.f/384.f, 0.09f);
                else
                    standart_transparent(player, 64.f/384.f, time_speed);
                WriteBackUpCoord(player.coordinates[0], player.coordinates[1]);
                player.coordinates[1] -= keyboard.step * delta_time;
            }
            else if (keyboard.keys[GLFW_KEY_A] || keyboard.keys[GLFW_KEY_LEFT])
            {
                if ((keyboard.backup_coordinates[0] == player.coordinates[0]) && (keyboard.backup_coordinates[1] == player.coordinates[1]))
                    standart_transparent(player, 256.f/384.f, 0.09f);
                else
                    standart_transparent(player, 128.f/384.f, time_speed);
                WriteBackUpCoord(player.coordinates[0], player.coordinates[1]);
                player.coordinates[0] -= keyboard.step * delta_time;
            }
            else if (keyboard.keys[GLFW_KEY_D] || keyboard.keys[GLFW_KEY_RIGHT])
            {
                if ((keyboard.backup_coordinates[0] == player.coordinates[0]) && (keyboard.backup_coordinates[1] == player.coordinates[1]))
                    standart_transparent(player, 256.f/384.f, 0.09f);
                else
                    standart_transparent(player, 192.f/384.f, time_speed);
                WriteBackUpCoord(player.coordinates[0], player.coordinates[1]);
                player.coordinates[0] += keyboard.step * delta_time;
            }
            else
                standart_transparent(player, 256.f/384.f, 0.09f);
        }
        else
        {
            player.coordinates[0] = keyboard.backup_coordinates[0];
            player.coordinates[1] = keyboard.backup_coordinates[1];
            keyboard.may_go = true;
            standart_transparent(player, 256.f/384.f, 0.09f);
        }
    }
    else
    {
        if (screen_flag.death_check == true && player.GetTexOffsetX() < 64.f/768.f * (12-2))
        {
            standart_transparent(player, 320.f/384.f, 0.09f);    
        }
        else
            if (screen_flag.death_check == true && player.GetTexOffsetX() >= 64.f/768.f * (12-2))
            {
                standart_transparent(player, 0.f, 0.f);
            }
            else
                standart_transparent(player, 256.f/384.f, 0.09f);
    }
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
        if(((player_coordinates[i][0] > map_block[0][0]) && (player_coordinates[i][0] < map_block[1][0]) && (player_coordinates[i][1] < map_block[0][1]) && (player_coordinates[i][1] > map_block[1][1])) == true)
        {
            return false;
        }
    }
    return true;
}

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

    int                 chest_open = 0; // 0 - не открыто, 1 - открытие, -1 - открыто
    array<int, 2>       chest_frames = {4, 1};
    double              chest_frame_time = 0.09f;

    int                 lever_open = 0;
    array<int, 2>       lever_frames = {20, 1};
    double              lever_frame_time = 0.09f;

    double              opacity_step;
}MapGen;

class Boss
{
private:
    Object              bs;
    Object              space;

    Object              bosslock;
    int                 bosslock_open; // 0 - не работает, -1 - начал работу, 1 - закончил
    array<int, 2>       bosslock_frames;
    double              bosslock_frame_time;

public:
    array <int, 150>    array_elements;
    array <int, 2>      array_num; // max, now
    array <float, 3>    time; //new_time, time, gen_interval
    array <float, 3>    block_time;
    bool                boss_flag;
    int                 boss_count;
    
    Boss()
    {
        boss_count = 0;

        bosslock_frame_time = 0.09f;
        bosslock_frames = {12, 1};
        bosslock_frame_time = 0.048f;
        bosslock_open = -1;

        time = {0.f, 0.f, 6.f};
        block_time = {0.f, 0.f, 2.f};

        array_num = {150, 0};
        boss_flag = false;

        bs.sides = {0.25f, 0.25f};
        bs.coordinates = {-3.f, -3.f};

        space.sides = {0.39f, 0.25f};
        space.coordinates = {-3.f, -3.f};

        bosslock.sides = {0.39f, 0.25f};
        bosslock.coordinates = {-3.f, -3.f};

        bs.TexInstall("resources/finalb.png");
        bs.TexSet({240.f/14400.f, 240.f/240.f}, {0.f, 0.f}, {60, 1});

        bosslock.TexInstall("resources/bosslock.png");
        bosslock.TexSet({144.f/1728.f, 144.f/144.f}, {0.f, 0.f}, {12, 1});
    }
    void GenerateNumbers()
    {
        for (int i = 0; i < 150; i++)
            array_elements[i] = ((float)rand()/RAND_MAX * 2);
    }
    void DrawBoss(Object& map, float bosslock_go)
    {
        if (bs.coordinates[0] == -3)
            bosslock.coordinates = bs.coordinates = {map.coordinates[0] + map.sides[0], map.coordinates[1] - map.sides[1]/2};

        if (bosslock_open != 1)
            bs.StartObj(0.f, 0.09);

        if (bosslock_go == -1 && bosslock_open == -1)
        {
            standart_transparent(bosslock, 0.f, bosslock_frame_time);

            if (bosslock.GetTexOffsetX() >= 144.f/2040.f * (bosslock_frames[0] - 1))
            {
                bosslock_open = 1;
                boss_flag = false;
            }
        }
        else
            standart_transparent(bosslock, 0.f, 0.f);
    }
};

class Mapper
{
private:
    Object              map;
    array<float, 2>     map_size;
    array<string, 16>   symbols;
    bool                player_start;

    string              labyrinth;
    int                 room_counter;

    Object              darkness;
    Object              chest;
    Object              lever;
    Boss                mrdark;

    //Interface
    Object              key;

    MapGen              mapg;    
public:
    Mapper(array<float, 2> c, array<float, 2> s, const char* texture_path, array<float, 2> m_s): 
    map(c, s), map_size(m_s)
    {
        room_counter = 0;
        player_start = false;

        darkness.sides = {0.125f, 0.125f};
        darkness.TexInstall("resources/eye.png");
        darkness.TexSet({200.f/12000.f, 200.f/200.f}, {0.f, 0.f}, {60, 1});

        chest.sides = {0.125f * 1.2, 0.125f * 1.2};
        chest.coordinates = {-3, -3};
        chest.TexInstall("resources/chest.png");
        chest.TexSet({72.f/288.f, 72.f/288.f}, {0.f, 0.f}, {mapg.chest_frames[0], mapg.chest_frames[1]});

        key.sides = {0.125f, 0.125f};
        key.coordinates = {-3.f, -3.f};
        key.TexInstall("resources/keys.png");

        lever.sides = {0.125f, 0.125f};
        lever.coordinates = {-3.f, -3.f};
        lever.TexInstall("resources/lever.png");

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
        file.close();    
    }

    void LoadLabyrinth(const char* file_name)
    {
        ifstream    file;
        
        file.open(file_name);
        if (file)
        {
            for (string line; getline (file, line);)
            {
                labyrinth = line.c_str();
            }
        }
        else
            cout << "Failed to open file." << endl;       
        file.close();
    }

    void ResetAllParameters()
    {
        player_start = false;

        chest.coordinates = {-3, -3};
        mapg.chest_open = 0;
        mapg.way_index.resize(0);
        mapg.grass_index.resize(0);

        key.coordinates = {-3.f, -3.f};
    }

    void DrawMap(Object& player)
    {
        int grass_num = 0, way_num = 0, wall_num = 0;

        if (mrdark.boss_flag == true)
            mrdark.array_num[1] = 0;

        if (!room_counter)
        {
            LoadFile(&(string("resources/Maps/") + labyrinth.substr(room_counter, 1) + ".txt")[0]);
            room_counter++;
        }

        for (int i = 0; i < 16; i++)
        {

            for (int j = 0; j < 16; j++)
            {
                if (mrdark.boss_flag == true)
                {
                    mrdark.time[0] = glfwGetTime();
                    if ((mrdark.time[0] - mrdark.time[1]) > mrdark.time[2])
                    {
                        mrdark.block_time[1] = glfwGetTime ();
                        mrdark.time[1] = mrdark.time[0];
                        mrdark.GenerateNumbers();
                    }
                }
                switch (symbols[j][i])
                {
                case '.':
                    if (keyboard.status == true && player_start == true && (MayGo(map, player, {player.sides[0]/2.f, -player.sides[1] + 0.02f, -player.sides[0]/2.f, -player.sides[1] + 0.02f, (player.sides[0]/2.f - 0.01f), 0.02f, -(player.sides[0]/2.f - 0.01f), 0.02f}) == false))
                    {
                        screen_flag.death_check = true;
                    }
                    darkness.coordinates = {map.coordinates[0], map.coordinates[1]};
                    darkness.TexSet({200.f/12000.f, 200.f/200.f}, {0.f, 0.f}, {1, 1});
                    standart_transparent(darkness, 0.f/147.f, 0.f);
                    if (mapg.chest_open == -1 && i == 0 && j == 1)
                        break;
                    if (mapg.darkness_status == false)
                    {
                        mapg.darkness_time[0] = glfwGetTime();
                        if (((mapg.darkness_time[0] -  mapg.darkness_time[1]) > mapg.darkness_time[2]) && ((float)rand()/RAND_MAX * 10000) > 9999)
                        {
                            mapg.darkness_status = true;
                            mapg.darkness_block_ij = {i, j};
                            mapg.darkness_offset = {200.f/12000.f, 0.f};
                            
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
                    if (keyboard.status == true && (MayGo(map, player, {0.04f, 0.f, -0.03f, 0.f, 0.04f, 0.f, -0.03f, 0.f}) == false))
                        keyboard.may_go = false;
                    TexSetAndDo({16.f/map_size[0], 16.f/map_size[1]}, {64.f/map_size[0], 0.f/map_size[1]}, {1, 1});
                    break;
                case '!':
                    if (!wall_num)
                    {
                        wall_num++;
                        if (keyboard.status == true && (MayGo(map, player, {0.03f, -0.03f, -0.03f, -0.03f, 0.03f, 0.f, -0.3f, 0.f}) == false))
                        {
                            keyboard.may_go = false;
                        }
                    }
                    else
                        wall_num = 0;
                    TexSetAndDo({16.f/map_size[0], 16.f/map_size[1]}, {16.f/map_size[0], 16.f/map_size[1]}, {1, 1});
                    break;
                case 'a':
                    if (keyboard.status == true && (MayGo(map, player, {-0.03f, -0.03f, -0.03f, -0.03f, -0.03f, 0.f, -0.3f, 0.f}) == false))
                        keyboard.may_go = false;
                    TexSetAndDo({16.f/map_size[0], 16.f/map_size[1]}, {96.f/map_size[0], 0.f/map_size[1]}, {1, 1});
                    break;
                case 'b':
                    if (keyboard.status == true && (MayGo(map, player, {-0.03f, -0.03f, -0.03f, -0.03f, -0.03f, 0.f, -0.3f, 0.f}) == false))
                        keyboard.may_go = false;
                    TexSetAndDo({16.f/map_size[0], 16.f/map_size[1]}, {112.f/map_size[0], 0.f/map_size[1]}, {1, 1});
                    break;
                case 'c':
                    if (keyboard.status == true && (MayGo(map, player, {-0.03f, -0.03f, -0.03f, -0.03f, -0.03f, 0.f, -0.3f, 0.f}) == false))
                        keyboard.may_go = false;
                    TexSetAndDo({16.f/map_size[0], 16.f/map_size[1]}, {128.f/map_size[0], 0.f/map_size[1]}, {1, 1});
                    break;
                case 'd':
                    TexSetAndDo({16.f/map_size[0], 16.f/map_size[1]}, {96.f/map_size[0], 16.f/map_size[1]}, {1, 1});
                    break;
                case 'x':
                    TexSetAndDo({16.f/map_size[0], 16.f/map_size[1]}, {112.f/map_size[0], 16.f/map_size[1]}, {1, 1});
                    if (screen_flag.opacity != 0)
                    {
                        if (screen_flag.opacity > 1 && screen_flag.next_room == true)
                        {
                            LoadFile(&(string("resources/Maps/") + labyrinth.substr(room_counter, 1) + ".txt")[0]);
                            map.coordinates[0] = -1.f + 0.125/2;
                            map.coordinates[1] = 1.f - 0.125/2;
                            player.coordinates = {-3.f, -3.f};
                            i = 16;
                            j = 16;
                            ResetAllParameters();
                            room_counter++;
                            mapg.opacity_step = - 0.005;
                            screen_flag.next_room = false;
                            break;
                        }
                        screen_flag.opacity += mapg.opacity_step;
                        if (screen_flag.opacity <= 0)
                        {
                            mapg.opacity_step = 0.005;
                            screen_flag.opacity = 0;
                        }
                    }

                    if ((screen_flag.next_room == false) && (room_counter == 1 || symbols[j+1][i] != '@') && keyboard.status == true && keyboard.keys[GLFW_KEY_E] && (MayGo(map, player, {-0.05f, -0.05f, -0.05f, -0.05f, -0.05f, 0.f, -0.5f, 0.f}) == false))
                    {
                        if (room_counter < labyrinth.length())
                        {
                            screen_flag.next_room = true;
                            screen_flag.opacity = mapg.opacity_step = 0.002;
                            cout << "Open" << endl;
                        }
                        else
                            if (mapg.chest_open == -1)
                                screen_flag.victory_check = true;
                    }
                    break;
                case 'e':
                    TexSetAndDo({16.f/map_size[0], 16.f/map_size[1]}, {128.f/map_size[0], 16.f/map_size[1]}, {1, 1});
                    break;
                case '~':
                    if (mapg.way_index.size() < (way_num + 1))
                        mapg.way_index.push_back(((float)rand()/RAND_MAX * 5));
                    
                    if (mrdark.boss_flag == true)
                    {
                        if (mrdark.array_num[1] < mrdark.array_num[0] && !mrdark.array_elements[mrdark.array_num[1]])
                        {
                            mrdark.block_time[0] = glfwGetTime ();
                            if (mrdark.block_time[0] - mrdark.block_time[1] < mrdark.block_time[2])
                            {
                                glColor4f(1.0f, 1.0f, 1.0f, 0.4f);
                                TexSetAndDo({16.f/map_size[0], 16.f/map_size[1]}, {mapg.way_coord_x[mapg.way_index[way_num]], 48.f/map_size[1]}, {1, 1});
                                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
                            }
                            else
                            {
                                if ((MayGo(map, player, {player.sides[0]/2.f, -player.sides[1] + 0.02f, -player.sides[0]/2.f, -player.sides[1] + 0.02f, (player.sides[0]/2.f - 0.01f), 0.02f, -(player.sides[0]/2.f - 0.01f), 0.02f}) == false))
                                {
                                    screen_flag.death_check = true;
                                }
                                darkness.coordinates = {map.coordinates[0], map.coordinates[1]};
                                if (mapg.darkness_frames[1] >= mapg.darkness_frames[0])
                                {
                                    mapg.darkness_offset[0] = 200.f/12000.f;
                                    mapg.darkness_frames[1] = 2;
                                }
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
                        }
                        else
                            TexSetAndDo({16.f/map_size[0], 16.f/map_size[1]}, {mapg.way_coord_x[mapg.way_index[way_num]], 48.f/map_size[1]}, {1, 1});
                        mrdark.array_num[1]++;
                    }
                    else
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
                case 'B':
                {
                    if (keyboard.status == true && (MayGo(map, player, {0.04f, 0.f, -0.03f, 0.f, 0.04f, 0.f, -0.03f, 0.f}) == false))
                    {
                        keyboard.may_go = false;
                        if (mrdark.boss_flag == true)
                        {
                            screen_flag.death_check = true;
                        }
                    }
                    if (mrdark.boss_flag == false && !mrdark.boss_count)
                    {
                        mrdark.boss_flag = true;
                        mrdark.block_time[1] = glfwGetTime ();
                        mrdark.boss_count++;
                    }
                    mrdark.DrawBoss(map, mapg.lever_open);
                    break;
                }
                case 'L':
                {
                    if (lever.coordinates[0] == -3)
                    {
                        lever.coordinates[0] = map.coordinates[0];
                        lever.coordinates[1] = map.coordinates[1];
                        lever.TexSet({102.f/2040.f, 102.f/102.f}, {0.f, 0.f}, {20, 1});
                    }

                    if (keyboard.status == true && (MayGo(map, player,{0.06f, 0.f, -0.03f, 0.f, 0.06f, 0.f, -0.03f, 0.f}) == false))
                        keyboard.may_go = false;

                    if (mapg.lever_open == 0 && keyboard.status == true && keyboard.keys[GLFW_KEY_E] && (MayGo(map, player, {0.f,  0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}) == false))
                    {
                        standart_transparent(lever, 0.f, mapg.lever_frame_time);
                        mapg.lever_open = 1;
                    }
                    else
                    {
                        if (mapg.lever_open == 1)
                        {
                            standart_transparent(lever, 0.f, mapg.lever_frame_time);

                            if (lever.GetTexOffsetX() >= 102.f/2040.f * (mapg.lever_frames[0] -1))
                            {
                                mapg.lever_open = -1;
                            }
                        }
                        else
                            standart_transparent(lever, 0.f, 0.f);
                    }
                    break;
                }
                case 'G':
                {
                    if (chest.coordinates[0] == -3)
                    {
                        chest.coordinates[0] = map.coordinates[0];
                        chest.coordinates[1] = map.coordinates[1] + 0.02;
                        chest.TexSet({72.f/288.f, 72.f/72.f}, {0.f, 0.f}, {4, 1});
                    }
                    if (keyboard.status == true && (MayGo(map, player, {player.sides[0]/2.f, -player.sides[1] + 0.02f, -player.sides[0]/2.f, -player.sides[1] + 0.02f, (player.sides[0]/2.f - 0.02f), 0.02f, -(player.sides[0]/2.f - 0.03f), 0.02f}) == false))
                        keyboard.may_go = false;
                    
                    if (mrdark.boss_flag == false)
                    {  
                        if (mapg.chest_open == 0 && keyboard.status == true && keyboard.keys[GLFW_KEY_E] && (MayGo(chest, player, {0.04f,  -player.sides[1] + 0.1f, -0.03f, -player.sides[1] + 0.1f, 0.04f, 0.02f, -0.03f, 0.02f}) == false))
                        {
                            standart_transparent(chest, 0.f, mapg.chest_frame_time);
                            mapg.chest_open = 1;
                        }
                        else
                        {
                            if (mapg.chest_open == 1)
                            {
                                standart_transparent(chest, 0.f, mapg.chest_frame_time);

                                if (chest.GetTexOffsetX() > chest.sides[0] * mapg.chest_frames[0])
                                        mapg.chest_open = -1;
                            }
                            else
                                standart_transparent(chest, 0.f, 0.f);
                        }
                    }
                    else
                    {
                        if ((MayGo(map, player, {player.sides[0]/2.f, -player.sides[1] + 0.02f, -player.sides[0]/2.f, -player.sides[1] + 0.02f, (player.sides[0]/2.f - 0.02f), 0.02f, -(player.sides[0]/2.f - 0.03f), 0.02f}) == false))
                        {    
                            screen_flag.death_check = true;
                            cout << "G" << endl;
                        }
                        standart_transparent(chest, 0.f, 0.f);
                    }
                    break;
                }
                default:
                    break;
                }
                if (mapg.chest_open == -1 && i == 0 && j == 1)
                {
                    if (key.coordinates[0] == -3)
                    {
                        key.coordinates[0] = map.coordinates[0];
                        key.coordinates[1] = map.coordinates[1];
                        key.TexSet({16.f/32.f, 16.f/16.f}, {0.f, 0.f}, {1, 1});
                    }
                    standart_transparent(key, 0.f, 0.f);
                }
                map.coordinates[1] -= 0.125f;
            }
            map.coordinates[0] += 0.125f;
            map.coordinates[1] = 1.f - 0.125/2;
        }
        map.coordinates[0] = -1.f + 0.125/2;
        map.coordinates[1] = 1.f - 0.125/2;
    }
};

float Smooth(float opacity, float step)
{
    glColor4f(1.0f, 1.0f, 1.0f, opacity);
    opacity += step;
    return opacity;
}

int main()
{
    GLFWwindow* window;

    if (!glfwInit ())
        return -1;
    window = glfwCreateWindow (WINDOW_WIDTH, WINDOW_HEIGHT, "The Dreams of Darkness", NULL, NULL);
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
    Object      dark({0.f, 0.4f}, {0.4f, 0.4f});
    Object      ds({0.f, 0.f}, {2.f, 2.f});
    Object      vs({0.f, 0.f}, {2.f, 2.f});
    Object      nrs({0.f, 0.f}, {2.f, 2.f});

    Mapper      map({(-1.f + 0.125/2), (1.f - 0.125/2)}, {0.125f, 0.125f}, "resources/map.png", {256.f, 96.f});

    map.LoadLabyrinth("resources/map.txt");

    //путь к текстуре
    player.TexInstall("resources/player.png");
    dark.TexInstall("resources/finalb.png");
    ds.TexInstall("resources/Death.jpg");
    nrs.TexInstall("resources/eyenrs.jpg");
    vs.TexInstall("resources/Victory.png");
    rain.TexInstall("resources/rain.png");

    //размер текстуры, смещение текстуры, кадры(всего, начальный)
    player.TexSet({64.f/768.f, 64.f/384.f}, {0.f, 0.f}, {12, 1});
    dark.TexSet({240.f/14400.f, 240.f/240.f}, {0.f, 0.f}, {60, 1});
    ds.TexSet({720.f/720.f, 720.f/720.f}, {0.f, 0.f}, {1, 1});
    nrs.TexSet({720.f/720.f, 720.f/720.f}, {0.f, 0.f}, {1, 1});
    vs.TexSet({477.f/5724.f, 477.f/477.f}, {0.f, 0.f}, {12, 1});
    rain.TexSet({240.f/14400.f, 240.f/240.f}, {0.f, 0.f}, {60, 1});

    glfwSwapInterval(1);

    float time;
    float opacity = 0;
    while (!glfwWindowShouldClose(window))
    {
            time = glfwGetTime();
            glClear(GL_COLOR_BUFFER_BIT);

            glClearColor(32.0f/255.0f, 39.0f/255.0f, 44.0f/255.0f, 1.0f);

            if (screen_flag.death_check == false && screen_flag.victory_check == false)
            {
                map.DrawMap(player);
                processPlayerMovement (player, 0.06f, time);
                if (screen_flag.next_room == true || screen_flag.opacity)
                {
                    glColor4f(1.f, 1.f, 1.f, screen_flag.opacity);
                    nrs.StartObj();
                    glColor4f(1.0f, 1.0f, 1.0f, 1.f);
                }
                else
                    rain_transparent(rain, 0.f, 0.09);
            }
            else
            {
                if (screen_flag.death_check == true && player.GetTexOffsetX() >= 64.f/768.f * (12-2))
                {
                    if (opacity <= 1)
                    {
                        glColor4f(1.0f, 1.0f, 1.0f, opacity);
                        opacity += 0.001;
                    }
                    
                    ds.StartObj();

                    if (opacity > 1)
                        standart_transparent(dark, 0.f, 0.09);
                }
                else
                {
                    if (screen_flag.death_check == true && player.GetTexOffsetX() < 64.f/768.f * (12-2))
                    {
                        map.DrawMap(player);
                        processPlayerMovement (player, 0.06f, time);
                    }
                    if (screen_flag.victory_check == true)
                    {
                        if (opacity <= 1)
                        {
                            glColor4f(1.0f, 1.0f, 1.0f, opacity);
                            opacity += 0.001;
                        }
                        vs.StartObj(0.f, 0.09f);
                    }
                }
            }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}