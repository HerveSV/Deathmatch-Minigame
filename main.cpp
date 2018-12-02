//
//  main.cpp
//  space invaders
//
//  Created by Hervé on 14/10/2018.
//  Copyright © 2018 Hervé. All rights reserved
//  Credits to Nick Tasios' amazing guide on OpenGL games, to be found at http://nicktasios.nl/posts/space-invaders-from-scratch-part-1.html

#include <cstdio>
#include <iostream>
//#include "Sprites.hpp"
#include <cstdint>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <chrono> // for std::chrono functions 2
#include <ctime>
#include <vector>
#include <random>
#include <string>
#include <array>
#include <sstream>
#include <cmath>



//int64_t timesecs = 0;
std::string score_text = "SCORE";
bool game_running = true;
//bool diag_move = false;
//bool left_shift = false;
int move_direction[2] {};

bool jump1 = false;
bool jump2 = false;
const int gravity = -3;
bool shoot = false;
const int bullet_speed = 4;
int max_ammo = 3;
const int death_time = 10*60;
const int idle_activ_time = 10*60;
//const int num_bullets = 1;
//bool bullet_exist = false;
//bool init_bullet = false;
//int player_score[2];
size_t jump_surface_y = 0;
bool press_w = false;
bool press_up = false;
bool moving_left1 = true;
bool moving1 = false;
bool moving_left2 = true;
bool moving2 = false;
bool shoot_hold = false;
int air_time1 = 0;
int air_time2 = 0;
bool press_shoot1 = false;
bool press_shoot2 = false;
const int max_jump_time = 35;
const int max_slide_time = 50;
const int slide_speed = 3;
const int jump_dim = 10;
bool can_jump;
const int jump_height = 6;
const int move_speed = 1;
bool fall = false;
bool press_reload1 = false;
bool press_reload2 = false;
bool reset = false;
bool press_slide1 = false;
bool press_slide2 = false;
const int reset_time = 20;
int player1_score = 0;
int player2_score = 0;
int player_scores[2];

time_t prev_reload = 0;
size_t player_lives = 4;

//const size_t PLATFORM_NUM = 4;

#define GL_ERROR_CASE(glerror)\
case glerror: snprintf(error, sizeof(error), "%s", #glerror)

inline void gl_debug(const char *file, int line) {
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR){
        char error[128];
        switch(err) {
                GL_ERROR_CASE(GL_INVALID_ENUM); break;
                GL_ERROR_CASE(GL_INVALID_VALUE); break;
                GL_ERROR_CASE(GL_INVALID_OPERATION); break;
                GL_ERROR_CASE(GL_INVALID_FRAMEBUFFER_OPERATION); break;
                GL_ERROR_CASE(GL_OUT_OF_MEMORY); break;
            default: snprintf(error, sizeof(error), "%s", "UNKNOWN_ERROR"); break;
        }
        fprintf(stderr, "%s - %s: %d\n", error, file, line);
    }
}
typedef void(*GLFWkeyfun)(GLFWwindow*, int, int, int, int); //key callback events

#undef GL_ERROR_CASE
void validate_shader(GLuint shader, const char *file = 0){
    static const unsigned int BUFFER_SIZE = 512;
    char buffer[BUFFER_SIZE];
    GLsizei length = 0;
    glGetShaderInfoLog(shader, BUFFER_SIZE, &length, buffer);
    if(length>0){
        printf("Shader %d(%s) compile error: %s\n", shader, (file? file: ""), buffer);
    }
}
bool validate_program(GLuint program){
    static const GLsizei BUFFER_SIZE = 512;
    GLchar buffer[BUFFER_SIZE];
    GLsizei length = 0;
    glGetProgramInfoLog(program, BUFFER_SIZE, &length, buffer);
    if(length>0){
        printf("Program %d link error: %s\n", program, buffer);
        return false;
    }
    return true;
}
void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

class Timer
{
    
private:
    // Type aliases to make accessing nested type easier
    using clock_t = std::chrono::high_resolution_clock;
    using second_t = std::chrono::duration<double, std::ratio<1> >;
    std::chrono::time_point<clock_t> m_beg;
public:
    Timer() : m_beg(clock_t::now())
    {
    }
    void reset()
    {
        m_beg = clock_t::now();
    }
    double elapsed() const
    {
        return std::chrono::duration_cast<second_t>(clock_t::now() - m_beg).count();
    }
};



struct Buffer
{
    size_t width, height;
    uint32_t* data;
};

struct Sprite
{
    size_t width, height;
    uint8_t* data;
};

struct Alien
{
    size_t x, y;
    uint8_t type;
};


struct Bullet
{
    size_t x, y;
    int dir;
};

struct Platform
{
    size_t x, y;
};

struct Standing
{
    int standing_on_num;
    bool standing = false;
    
};

enum Player_state
{
    PLAYER_STATE_ALIVE,
    PLAYER_STATE_DEAD,
};

enum Player_Num
{
    PLAYER_1,
    PLAYER_2,
};

struct Grave
{
    size_t x, y;
    bool exist = false;
    Player_Num player;
    Standing grounded;
};



struct Player
{
    size_t x, y;
    size_t life;
    size_t ammo;
    int air_time = 0;
    int slide_time = 0;
    Standing standing;
    bool can_jump = true;
    bool jump = false;
    bool moving = false;;
    bool moving_left = true;
    bool shoot = false;
    bool fall = false;
    bool slide = false;
    bool sliding = false;
    bool reload = false;
    bool reloading = false;
    int reload_time = 10;
    bool alive = true;
    int death_counter = death_time;
    int idle_counter = 0;
    bool idling = false;
    Sprite current_sprite;
    Player_state state = PLAYER_STATE_ALIVE;
};


enum alien_state
{
    ALIEN_STATE_ALIVE,
    ALIEN_STATE_DEAD,
};

struct SpriteSheet
{
    Sprite fragment;
    uint32_t colour;
    size_t x,y;
};

enum Player_Position
{
    PLAYER_POSITION_A,
    PLAYER_POSITION_B,
    PLAYER_POSITION_C,
    PLAYER_POSITION_D,
    PLAYER_POSITION_E,
    PLAYER_POSITION_F,
    PLAYER_POSITION_G,
    POSITION_NUMS,
};

typedef std::vector<SpriteSheet> LayerSprite;

/*class LayerSprites
{
private:
    unsigned m_layers;

public:
    LayerSprites(unsigned layers);
    std::vector<SpriteSheet> spriteSheets;
};

LayerSprites::LayerSprites(unsigned layers)
{
    
}*/




#define GAME_MAX_BULLETS 10
#define NUM_PLATFORMS 10
#define NUM_PLAYERS 2
#define MAX_RELOAD 20
struct Game
{
    size_t width, height;
    size_t num_aliens;
    size_t num_bullets;
    size_t num_platforms;
    Alien* aliens;
    //Player player1;
    //Player player2;
    Standing standing;
    Bullet bullets[GAME_MAX_BULLETS];
    Platform platforms[NUM_PLATFORMS];
    Player player[NUM_PLAYERS];
    std::vector<Grave> grave;
    
    
};

struct SpriteAnimation
{
    bool loop;
    size_t num_frames;
    size_t frame_duration;
    size_t time;
    Sprite** frames;
};
//enum Direction {
//    DIRECTION_LEFT,
//    DIRECTION_RIGHT,
//};
int rngGen(int min, int max)
{
    //const int min{1};
    //const int max{100};
    
    
    
    //static const double fraction = 1.0 / (RAND_MAX + 1.0);  // static used for efficiency, so we only calculate this value once
    // evenly distribute the random number across our range
    //return min + static_cast<int>((max - min + 1) * (rand() * fraction));
    std::random_device rd;
    std::mt19937 mersenne(rd());
    std::uniform_int_distribution<> rngGenerator(min, max);
    int randNum = rngGenerator(mersenne);
    return randNum;
}


void buffer_clear(Buffer* buffer, uint32_t colour)
{
    for(size_t i = 0; i < buffer->width * buffer->height; ++i)
    {
        buffer->data[i] = colour;
    }
}
bool sprite_overlap_check(
                          const Sprite& sp_a, size_t x_a, size_t y_a,
                          const Sprite& sp_b, size_t x_b, size_t y_b
                          )
{
    if(x_a < x_b + sp_b.width && x_a + sp_a.width > x_b &&
       y_a < y_b + sp_b.height && y_a + sp_a.height > y_b)
    {
        return true;
    }
    
    return false;
}

void buffer_draw_sprite(Buffer* buffer, const Sprite& sprite, size_t x, size_t y, uint32_t colour)
{
    for(size_t xi = 0; xi < sprite.width; ++xi)
    {
        for(size_t yi = 0; yi < sprite.height; ++yi)
        {
            if(sprite.data[yi * sprite.width + xi] &&
               (sprite.height - 1 + y - yi) < buffer->height &&
               (x + xi) < buffer->width)
            {
                buffer->data[(sprite.height - 1 + y - yi) * buffer->width + (x + xi)] = colour;
            }
        }
    }
}

void draw_layer_sprite(LayerSprite folder,unsigned layers, Buffer* buffer, size_t m_x, size_t m_y)
{
    for(int li = 0; li < layers; ++li)
    {
        const Sprite &sprite = folder[li].fragment;
        size_t x = m_x + folder[li].x;
        size_t y = m_y + folder[li].y;
        uint32_t colour = folder[li].colour;
        buffer_draw_sprite(buffer, sprite, x, y, colour);
    }
}

void buffer_write_letters(std::array<Sprite, 26> &alphabet, Buffer &buffer, size_t x, size_t y, std::string text, int letter_spacing, uint32_t colour)
{
    for(int i = 0; i < text.size(); ++i)
    {
        int letterNum = static_cast<int>(text[i])-65;
        if(letterNum >= 32)
        {
            letterNum -= 32;
        }
        size_t x_position = x + (i*5) + (i*letter_spacing);
        size_t y_position = y;
        buffer_draw_sprite(&buffer, alphabet[letterNum], x_position, y_position, colour);
    }
}

void buffer_write_numbers(std::array<Sprite, 10> &numbers, Buffer &buffer, size_t x, size_t y, int display_num, uint32_t colour)
{
    if(display_num > 999)
        display_num = 999;
    
    if(display_num < 10)
    {
        buffer_draw_sprite(&buffer, numbers[display_num], x, y, colour);
    }
    else if(display_num < 100)
    {
        int temp_var = display_num;
        int digits[2];
        digits[0] = temp_var % 10;
        temp_var = temp_var / 10;
        digits[1] = temp_var % 10;
        
        buffer_draw_sprite(&buffer, numbers[digits[1]], x, y, colour);
        buffer_draw_sprite(&buffer, numbers[digits[0]], x + 5+1, y, colour);
    }
    else if(display_num < 1000)
    {
        int temp_var = display_num;
        int digits[3];
        digits[0] = temp_var % 10;
        temp_var = temp_var / 10;
        digits[1] = temp_var % 10;
        temp_var = temp_var / 10;
        digits[2] = temp_var % 10;
        
        buffer_draw_sprite(&buffer, numbers[digits[2]], x, y, colour);
        buffer_draw_sprite(&buffer, numbers[digits[1]], x + 5+1, y, colour);
        buffer_draw_sprite(&buffer, numbers[digits[0]], x + 10+2, y, colour);
    }
    /*if(display_num < 10)
    {
        number_index = display_num;
        buffer_draw_sprite(&buffer, numbers[number_index], x, y, colour);
    }
    else if(display_num < 100)
    {
        int temp_var = display_num;
        bool flag1 = true;
        bool flag2 = true;
        for(int i = 0; temp_var > 9 && flag1; ++i)
        {
            temp_var -= (i * 10);
            if(temp_var < 10)
            {
                buffer_draw_sprite(&buffer, numbers[i], x, y, colour);
                flag1 = false;
            }
        }
        for(int i = 0; temp_var > 0 && flag2; ++i)
        {
            temp_var -= i;
            if(temp_var <= 0)
            {
                buffer_draw_sprite(&buffer, numbers[i], x, y, colour);
                flag2 = false;
            }
        }
    }
    else if(display_num < 1000)
    {
        int temp_var = display_num;
        bool flag1 = true;
        bool flag2 = true;
        bool flag3 = true;
        for(int i = 0; temp_var > 99 && flag1; ++i)
        {
            temp_var -= i * 100;
            if(temp_var < 100)
            {
                buffer_draw_sprite(&buffer, numbers[i], x, y, colour);
                flag1 = false;
            }
        }
        for(int i = 0; temp_var > 9 && flag2; ++i)
        {
            temp_var -= i * 10;
            if(temp_var < 10)
            {
                buffer_draw_sprite(&buffer, numbers[i], x+5+1, y, colour);
                flag2 = false;
            }
        }
        for(int i = 0; temp_var > 0 && flag3; ++i)
        {
            temp_var -= i;
            if(temp_var == 0)
            {
                buffer_draw_sprite(&buffer, numbers[i], x+10+2, y, colour);
                flag3 = false;
            }
        }
        
    }*/
    
    /*int number_index = 0;
    if(display_num > 999)
        display_num = display_num/10;
    if(display_num < 10)
    {
        number_index = display_num;
        buffer_draw_sprite(&buffer, numbers[number_index], x, y, colour);
    }
    else if(display_num < 100)
    {
        int temp_index = 10;
        int i1 = 0;
        int i2 = 0;
        for(i1 = temp_index; i1 <= display_num; i1 += 10)
        {
            number_index = i1/10;
        }
        if(number_index > 9)
            number_index = 9;
        buffer_draw_sprite(&buffer, numbers[number_index], x, y, colour);
        temp_index += i1-10;
        number_index = 0;
        for(int i2 = temp_index; i2 <= display_num; ++i2)
        {
            number_index = i2;
        }
        if(number_index > 9)
            number_index = 9;
        buffer_draw_sprite(&buffer, numbers[number_index], x+5+1, y, colour);
        temp_index += i2-1;
        number_index = 0;
    }
    else if(display_num < 1000)
    {
        int temp_index = 100;
        int i1 = 0;
        int i2 = 0;
        int i3 = 0;
        for(int i1 = temp_index; i1 <= display_num; i1 += 100)
        {
            number_index = i1/100;
        }
        if(number_index > 9)
            number_index = 9;
        buffer_draw_sprite(&buffer, numbers[number_index], x, y, colour);
        temp_index += i1-100;
        number_index = 0;
        for(int i2 = temp_index; i2 <= display_num; i2 += 10)
        {
            number_index = i2/10;
        }
        if(number_index > 9)
            number_index = 9;
        buffer_draw_sprite(&buffer, numbers[number_index], x+5+1, y, colour);
        temp_index += i2-10;
        number_index = 0;
        for(int i3 = temp_index; i3 <= display_num; ++i3)
        {
            number_index = i3;
        }
        if(number_index > 9)
            number_index = 9;
        buffer_draw_sprite(&buffer, numbers[number_index], x+10+2, y, colour);
        temp_index += i3-1;
        number_index = 0;
    }*/
}

int cal_grav(size_t y, Buffer& buffer)
{
    size_t height = buffer.height;
    if(y > height - height/10)
        return 0;
    if(y > height - height/8)
        return 0;
    if(y > height - height/6)
        return 1;
    if(y > height - height/4)
        return 1;
    if(y > height - height/2)
        return 1;
    else
        return 1;
}

//void init_bullet(Sprite& bullet, size_t x, size_t y, uint32_t colour, Buffer& buffer)
//{
//    //buffer_draw_sprite(buffer, bullet, x, y, colour);
//}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
    
    move_direction[0] = 0;
    move_direction[1] = 0;

    
    switch(key){
        case GLFW_KEY_ESCAPE:
            if(action == GLFW_PRESS) game_running = false;
            break;
        /*case GLFW_KEY_RIGHT_SHIFT:
            if(action == GLFW_PRESS) left_shift = true;
            else left_shift = false;
            if(action == GLFW_PRESS && diag_move) diag_move = false;
            else if(action == GLFW_PRESS) diag_move = true;
            break;*/
    
        case GLFW_KEY_W:
            if(action == GLFW_PRESS && !jump1 && air_time1 < max_jump_time)
            {
                //move_direction[1] = 1;
                //move_direction[0] = 0;
                press_w = true;
            }
            if((action == GLFW_RELEASE && jump1) || air_time1 >= max_jump_time)
            {
                //move_direction[1] = 1;
                //move_direction[0] = 0;
                press_w = false;
            }

            break;
            case GLFW_KEY_S:
            if(action == GLFW_PRESS)
            {
                //move_direction[1] = -1;
                //move_direction[0] = 0;
                //moving = true
            }
            break;
        case GLFW_KEY_A:
            if(action == GLFW_PRESS)
            {
                moving1 = true;
                moving_left1 = true;
            }
            //move_direction[0] = -1;
            //if(!diag_move) move_direction[1] = 0;
            if(action == GLFW_RELEASE)
            {
                moving1 = false;
            }

            break;
        case GLFW_KEY_D:
            if(action == GLFW_PRESS)
            {
                moving1 = true;
                moving_left1 = false;
            }
            //move_direction[0] = 1;
            //if(!diag_move) move_direction[1] = 0;
            if(action == GLFW_RELEASE)
            {
                moving1 = false;
                moving_left1 = false;
            }

            break;
            
        case GLFW_KEY_T:
            if(action == GLFW_PRESS)
            {
                
            }
            if(action == GLFW_RELEASE)
            {
                press_shoot1 = true;
                //start time;
            }
            
            break;
        case GLFW_KEY_R:
            if(action == GLFW_RELEASE)
            {
                press_reload1 = true;
            }
            break;
        case GLFW_KEY_Y:
            if(action == GLFW_PRESS)
            {
                press_slide1 = true;
            }
            else if(action == GLFW_RELEASE)
            {
                press_slide1 = false;
            }
            break;
        case GLFW_KEY_UP:
            if(action == GLFW_PRESS && !jump2 && air_time2 < max_jump_time)
            {
                //move_direction[1] = 1;
                //move_direction[0] = 0;
                press_up = true;
            }
            if((action == GLFW_RELEASE && jump2) || air_time2 >= max_jump_time)
            {
                //move_direction[1] = 1;
                //move_direction[0] = 0;
                press_up = false;
            }
            
            break;
        case GLFW_KEY_DOWN:
            if(action == GLFW_PRESS)
            {
                //move_direction[1] = -1;
                //move_direction[0] = 0;
                //moving = true
            }
            break;
        case GLFW_KEY_LEFT:
            if(action == GLFW_PRESS)
            {
                moving2 = true;
                moving_left2 = true;
            }

            if(action == GLFW_RELEASE)
            {
                moving2 = false;
                moving_left2 = true;
            }
            
            break;
        
        case GLFW_KEY_RIGHT:
            if(action == GLFW_PRESS)
            {
                moving2 = true;
                moving_left2 = false;
            }

            if(action == GLFW_RELEASE)
            {
                moving2 = false;
                moving_left2 = false;
            }
            
            break;
            
        case GLFW_KEY_N:
            if(action == GLFW_PRESS)
            {
                
            }
            if(action == GLFW_RELEASE)
            {
                press_shoot2 = true;
                //start time;
            }
            
            break;
            
        case GLFW_KEY_RIGHT_ALT:
            if(action == GLFW_RELEASE)
            {
                press_reload2 = true;
            }
            break;
        
        case GLFW_KEY_M:
            if(action == GLFW_PRESS)
            {
                press_slide2 = true;
            }
            else if(action == GLFW_RELEASE)
            {
                press_slide2 = false;
            }
            break;
        case GLFW_KEY_SPACE:
            if(action == GLFW_PRESS)
            {
                reset = true;
            }
            if(action == GLFW_RELEASE)
            {
                reset = false;
            }
            break;

        default:
            move_direction[0] = 0;
            move_direction[1] = 0;
            //moving1 = false;
            //moving2 = false;
            //press_w = false;
            //press_up = false;
            reset = false;
            break;
    }
    switch(key){
            
            
            
            
    }
    
}



uint32_t rgb_to_uint32(uint8_t r, uint8_t g, uint8_t b)
{
    return (r << 24) | (g << 16) | (b << 8) | 255;
}
int main(int argc, char* argv[])
{
    const size_t buffer_width = 608;//960;//1920;//480;//896;//224;
    const size_t buffer_height = 342;//540;//1080;//270;//512;//256;
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    /* Create a windowed mode window and its OpenGL context */
    GLFWwindow* window = glfwCreateWindow(buffer_width, buffer_height, "Deathmatch Ultimo", NULL, NULL);
    if(!window)
    {
        glfwTerminate();
        return -1;
    }
    glfwSetKeyCallback(window, key_callback);
    
    glfwMakeContextCurrent(window);
    GLenum err = glewInit();
    if(err != GLEW_OK)
    {
        fprintf(stderr, "Error initialising GLEW.\n");
        glfwTerminate();
        return -1;
    }
    
    
    int glVersion[2] = {-1, 1};
    glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]);
    glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]);
    gl_debug(__FILE__, __LINE__);
    printf("Using OpenGL: %d.%d\n", glVersion[0], glVersion[1]);
    printf("Renderer used: %s\n", glGetString(GL_RENDERER));
    printf("Shading Language: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    glfwSwapInterval(1);
    glClearColor(1.0, 0.0, 0.0, 1.0);
    // Create graphics buffer
    Buffer buffer;
    buffer.width  = buffer_width;
    buffer.height = buffer_height;
    buffer.data   = new uint32_t[buffer.width * buffer.height];
    buffer_clear(&buffer, 0);
    // Create texture for presenting buffer to OpenGL
    GLuint buffer_texture;
    glGenTextures(1, &buffer_texture);
    glBindTexture(GL_TEXTURE_2D, buffer_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, buffer.width, buffer.height, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, buffer.data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // Create vao for generating fullscreen triangle
    GLuint fullscreen_triangle_vao;
    
    glGenVertexArrays(1, &fullscreen_triangle_vao);
    // Create shader for displaying buffer
    static const char* fragment_shader =
    "\n"
    "#version 330\n"
    "\n"
    "uniform sampler2D buffer;\n"
    "noperspective in vec2 TexCoord;\n"
    "\n"
    "out vec3 outColor;\n"
    "\n"
    "void main(void){\n"
    "    outColor = texture(buffer, TexCoord).rgb;\n"
    "}\n";
    static const char* vertex_shader =
    "\n"
    "#version 330\n"
    "\n"
    "noperspective out vec2 TexCoord;\n"
    "\n"
    "void main(void){\n"
    "\n"
    "    TexCoord.x = (gl_VertexID == 2)? 2.0: 0.0;\n"
    "    TexCoord.y = (gl_VertexID == 1)? 2.0: 0.0;\n"
    "    \n"
    "    gl_Position = vec4(2.0 * TexCoord - 1.0, 0.0, 1.0);\n"
    "}\n";
    GLuint shader_id = glCreateProgram();
    {
        //Create vertex shader
        GLuint shader_vp = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(shader_vp, 1, &vertex_shader, 0);
        glCompileShader(shader_vp);
        validate_shader(shader_vp, vertex_shader);
        glAttachShader(shader_id, shader_vp);
        glDeleteShader(shader_vp);
    }
    {
        //Create fragment shader
        GLuint shader_fp = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(shader_fp, 1, &fragment_shader, 0);
        glCompileShader(shader_fp);
        validate_shader(shader_fp, fragment_shader);
        glAttachShader(shader_id, shader_fp);
        glDeleteShader(shader_fp);
    }
    glLinkProgram(shader_id);
    if(!validate_program(shader_id)){
        fprintf(stderr, "Error while validating shader.\n");
        glfwTerminate();
        glDeleteVertexArrays(1, &fullscreen_triangle_vao);
        delete[] buffer.data;
        return -1;
    }
    glUseProgram(shader_id);
    GLint location = glGetUniformLocation(shader_id, "buffer");
    glUniform1i(location, 0);
    //OpenGL setup
    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(fullscreen_triangle_vao);
    // Prepare game
    
    Sprite platform_sprite;
    
    platform_sprite.width = 150;
    platform_sprite.height = 5;
    platform_sprite.data = new uint8_t[platform_sprite.width*platform_sprite.height];
    for(int i = 0; i < platform_sprite.width * platform_sprite.height; ++i)
    {
//        if(i < 100 || i > 598)
//            platform_sprite.data[i] = 0;
//        else
            platform_sprite.data[i] = 1;
        
    }
    
    Player player_reset_dummy;
    player_reset_dummy.life = player_lives;
    player_reset_dummy.ammo = max_ammo;
    player_reset_dummy.state = PLAYER_STATE_ALIVE;
    player_reset_dummy.alive = true;
    player_reset_dummy.jump = false;
    player_reset_dummy.can_jump = false;
    player_reset_dummy.shoot = false;
    player_reset_dummy.slide = false;
    player_reset_dummy.fall = false;
    player_reset_dummy.reload = false;

    //Initialise the game itself
    Game game;
    game.width = buffer_width;
    game.height = buffer_height;
    game.num_aliens = 55;
    game.num_bullets = 0;
    game.num_platforms = NUM_PLATFORMS;
    //game.aliens = new Alien[game.num_aliens];
    
    
    game.platforms[0].x = 25;
    game.platforms[0].y = 100;
    
    game.platforms[1].x = 100;
    game.platforms[1].y = 200;
    
    game.platforms[2].x = buffer.width-platform_sprite.width-25;
    game.platforms[2].y = 100;
    
    game.platforms[3].x = buffer.width - platform_sprite.width - 100;
    game.platforms[3].y = 200;
    
    game.platforms[4].x = buffer.width/2 - platform_sprite.width/2;
    game.platforms[4].y = 165;
    
    game.platforms[5].x = buffer.width - platform_sprite.width - 90;
    game.platforms[5].y = 130;
    
    game.platforms[6].x = 90;
    game.platforms[6].y = 130;
    
    game.platforms[7].x = 85;
    game.platforms[7].y = 40;
    
    game.platforms[8].x = buffer.width - platform_sprite.width - 85;
    game.platforms[8].y = 40;
    
    game.platforms[9].x = buffer.width/2 - platform_sprite.width/2;
    game.platforms[9].y = 75;
    
    game.player[0].x = 120;
    game.player[0].y = 165;
    game.player[0].life = player_lives;
    game.player[0].ammo = max_ammo;
    game.player[0].state = PLAYER_STATE_ALIVE;
    game.player[0].alive = true;
    game.player[0].jump = false;
    game.player[0].shoot = false;
    
    game.player[1].x = buffer.width - (buffer.width * 0.33);
    game.player[1].y = 20;
    game.player[1].life = player_lives;
    game.player[1].ammo = max_ammo;
    game.player[1].state = PLAYER_STATE_ALIVE;
    game.player[1].alive = true;
    game.player[1].jump = false;
    game.player[1].shoot = false;
    
    
    
    Game game_reset_dummy;
    game_reset_dummy.width = buffer_width;
    game_reset_dummy.height = buffer_height;
    game_reset_dummy.num_aliens = 55;
    game_reset_dummy.num_bullets = 0;
    game_reset_dummy.num_platforms = NUM_PLATFORMS;
    //game.aliens = new Alien[game.num_aliens];
    
    
    game_reset_dummy.platforms[0].x = 25;
    game_reset_dummy.platforms[0].y = 100;
    
    game_reset_dummy.platforms[1].x = 100;
    game_reset_dummy.platforms[1].y = 200;
    
    game_reset_dummy.platforms[2].x = buffer.width-platform_sprite.width-25;
    game_reset_dummy.platforms[2].y = 100;
    
    game_reset_dummy.platforms[3].x = buffer.width - platform_sprite.width - 100;
    game_reset_dummy.platforms[3].y = 200;
    
    game_reset_dummy.platforms[4].x = buffer.width/2 - platform_sprite.width/2;
    game_reset_dummy.platforms[4].y = 165;
    
    game_reset_dummy.platforms[5].x = buffer.width - platform_sprite.width - 90;
    game_reset_dummy.platforms[5].y = 130;
    
    game_reset_dummy.platforms[6].x = 90;
    game_reset_dummy.platforms[6].y = 130;
    
    game_reset_dummy.platforms[7].x = 85;
    game_reset_dummy.platforms[7].y = 40;
    
    game_reset_dummy.platforms[8].x = buffer.width - platform_sprite.width - 85;
    game_reset_dummy.platforms[8].y = 40;
    
    game_reset_dummy.platforms[9].x = buffer.width/2 - platform_sprite.width/2;
    game_reset_dummy.platforms[9].y = 75;
    
    game_reset_dummy.player[0].x = 120;
    game_reset_dummy.player[0].y = 165;
    game_reset_dummy.player[0].life = player_lives;
    game_reset_dummy.player[0].ammo = max_ammo;
    game_reset_dummy.player[0].state = PLAYER_STATE_ALIVE;
    game_reset_dummy.player[0].alive = true;
    game_reset_dummy.player[0].jump = false;
    game_reset_dummy.player[0].shoot = false;
    
    game_reset_dummy.player[1].x = (buffer.width/2) - 5;
    game_reset_dummy.player[1].y = 3;
    game_reset_dummy.player[1].life = player_lives;
    game_reset_dummy.player[1].ammo = max_ammo;
    game_reset_dummy.player[1].state = PLAYER_STATE_ALIVE;
    game_reset_dummy.player[1].alive = true;
    game_reset_dummy.player[1].jump = false;
    game_reset_dummy.player[1].shoot = false;
    
    
    
//    for(size_t yi = 0; yi < 5; ++yi)
//    {
//        for(size_t xi = 0; xi < 11; ++xi)
//        {
//            game.aliens[yi * 11 + xi].x = 16 * xi + 20;
//            game.aliens[yi * 11 + xi].y = 17 * yi + 128;
//            //game.aliens[yi * 11 + xi].type = ALIEN_STATE_ALIVE;
//        }
//    }
    

    
    Sprite alien_sprite;
    alien_sprite.width = 11;
    alien_sprite.height = 8;
    alien_sprite.data = new uint8_t[88]
    {
        0,0,1,0,0,0,0,0,1,0,0, // ..@.....@..
        0,0,0,1,0,0,0,1,0,0,0, // ...@...@...
        0,0,1,1,1,1,1,1,1,0,0, // ..@@@@@@@..
        0,1,1,0,1,1,1,0,1,1,0, // .@@.@@@.@@.
        1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
        1,0,1,1,1,1,1,1,1,0,1, // @.@@@@@@@.@
        1,0,1,0,0,0,0,0,1,0,1, // @.@.....@.@
        0,0,0,1,1,0,1,1,0,0,0  // ...@@.@@...
    };
    
    Sprite alien_sprite1;
    alien_sprite1.width = 11;
    alien_sprite1.height = 8;
    alien_sprite1.data = new uint8_t[88]
    {
        0,0,1,0,0,0,0,0,1,0,0, // ..@.....@..
        1,0,0,1,0,0,0,1,0,0,1, // @..@...@..@
        1,0,1,1,1,1,1,1,1,0,1, // @.@@@@@@@.@
        1,1,1,0,1,1,1,0,1,1,1, // @@@.@@@.@@@
        1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
        0,1,1,1,1,1,1,1,1,1,0, // .@@@@@@@@@.
        0,0,1,0,0,0,0,0,1,0,0, // ..@.....@..
        0,1,0,0,0,0,0,0,0,1,0  // .@.......@.
    };
    
//    Sprite alien_sprite_death;
////    alien_sprite1.width = 11;
////    alien_sprite1.height = 8;
////    alien_sprite1.data = new uint8_t[88]
////    {
////        1,0,1,0,0,0,0,0,1,0,1, // ..@.....@..
////        0,1,0,1,0,0,0,1,0,1,0, // @..@...@..@
////        0,0,1,0,0,0,0,0,1,0,0, // @.@@@@@@@.@
////        1,1,0,0,0,1,1,0,1,0,0, // @@@.@@@.@@@
////        0,0,1,0,0,1,1,0,0,1,1, // @@@@@@@@@@@
////        0,0,1,0,0,0,0,0,1,0,0, // .@@@@@@@@@.
////        0,1,0,1,0,0,0,1,0,1,0, // ..@.....@..
////        1,0,1,0,0,0,0,0,1,0,1,  // .@.......@.
////    };
    
    Sprite custom_lol;
    custom_lol.width = 41;
    custom_lol.height = 5;
    custom_lol.data = new uint8_t[205]
    {
        1,0,0,1,1,1,0,1,0,0,0,1,0,0,0,1,1,0,0,0,1,0,1,1,0,0,0,0,1,1,1,0,1,0,1,0,1,0,1,1,1,
        1,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0,0,1,0,1,0,1,0,0,0,0,1,0,0,1,0,1,0,1,0,1,0,0,
        1,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,0,1,0,1,0,1,0,0,1,0,0,0,1,0,0,1,1,1,0,1,0,1,1,1,
        1,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0,0,1,0,1,0,1,0,0,0,0,1,0,0,1,0,1,0,1,0,0,0,1,
        1,1,0,1,1,1,0,1,1,0,0,1,0,0,0,1,1,0,0,0,1,0,1,1,0,0,0,0,0,1,0,0,1,0,1,0,1,0,1,1,1,
    };
    
    


    Sprite space_invaders_title;
    space_invaders_title.width = 50;
    space_invaders_title.height = 10;
    space_invaders_title.data = new uint8_t[500]
    {
        1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    };
    
    
    Sprite player_sprite_left1;
    player_sprite_left1.width = 22;
    player_sprite_left1.height = 14;
    player_sprite_left1.data = new uint8_t[308]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0, // ......................
        1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, // ......................
        0,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // ......................
        0,0,0,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,1, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,0, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0, // ......................
        0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0, // ......................
    };
    
    Sprite player_sprite_left2;
    player_sprite_left2.width = 22;
    player_sprite_left2.height = 14;
    player_sprite_left2.data = new uint8_t[308]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0, // ......................
        1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, // ......................
        0,0,0,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,0, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,1,1,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,1,1,1,1,0,0,0, // ......................
        0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,1,1,1,1,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,1,1,1,0,0,0, // ......................
    };
    
    
    Sprite player_sprite_left3;
    player_sprite_left3.width = 22;
    player_sprite_left3.height = 14;
    player_sprite_left3.data = new uint8_t[308]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0, // ......................
        1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // ......................
        0,0,0,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,1, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,0, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,1,1,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,1,1,1,0,0, // ......................
    };
    
    
    Sprite player_sprite_left4;
    player_sprite_left4.width = 22;
    player_sprite_left4.height = 14;
    player_sprite_left4.data = new uint8_t[308]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0, // ......................
        1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // ......................
        0,0,0,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,1, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,1,1,1,1,1,1,0, // ......................
        0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,0, // ......................
        0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
    };
    
    Sprite player_sprite_left5;
    player_sprite_left5.width = 22;
    player_sprite_left5.height = 14;
    player_sprite_left5.data = new uint8_t[308]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0, // ......................
        1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, // ......................
        0,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // ......................
        0,0,0,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,1,1,1,1,1,1,0, // ......................
        0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,0, // ......................
        0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
    };
    
    
    Sprite player_sprite_left_reload;
    player_sprite_left_reload.width = 22;
    player_sprite_left_reload.height = 14;
    player_sprite_left_reload.data = new uint8_t[308]
    {
        0,0,0,0,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,0,1,1,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,1,1,1,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,1,0,1,1,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, // ......................
        0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,1, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,0, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0, // ......................
        0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0, // ......................
    };
    
    Sprite player_sprite_left_slide;
    player_sprite_left_slide.width = 24;
    player_sprite_left_slide.height = 11;
    player_sprite_left_slide.data = new uint8_t[264]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,1,0,1,1,1,0,0,0,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // ......................
        0,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // ......................
        0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1, // ......................
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1, // ......................
    };
    

    
    Sprite player_sprite_right1;
    player_sprite_right1.width = 22;
    player_sprite_right1.height = 14;
    player_sprite_right1.data = new uint8_t[308]
    {
        0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1, // ......................
        0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // ......................
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0, // ......................
        1,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,0,0,0, // ......................
        0,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0, //- ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0, // ......................
    };
    Sprite player_sprite_right2;
    player_sprite_right2.width = 22;
    player_sprite_right2.height = 14;
    player_sprite_right2.data = new uint8_t[308]
    {
        0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // ......................
        0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0, // ......................
        0,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,0,0,0, // ......................
        0,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
        0,1,1,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0, // ......................
        0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0, // ......................
        0,0,0,1,1,1,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0, // ......................
    };
    
    Sprite player_sprite_right3;
    player_sprite_right3.width = 22;
    player_sprite_right3.height = 14;
    player_sprite_right3.data = new uint8_t[308]
    {
        0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // ......................
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0, // ......................
        1,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,0,0,0, // ......................
        0,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
        0,1,1,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0, // ......................
    };
    
    Sprite player_sprite_right4;
    player_sprite_right4.width = 22;
    player_sprite_right4.height = 14;
    player_sprite_right4.data = new uint8_t[308]
    {
        0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // ......................
        0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0, // ......................
        0,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,0,0,0, // ......................
        0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
        0,1,1,1,1,1,1,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0, // ......................
        0,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0, // ......................
    };
    
    Sprite player_sprite_right5;
    player_sprite_right5.width = 22;
    player_sprite_right5.height = 14;
    player_sprite_right5.data = new uint8_t[308]
    {
        0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // ......................
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0, // ......................
        0,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,0,0,0, // ......................
        0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
        0,1,1,1,1,1,1,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0, // ......................
        0,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
    };
    
    Sprite player_sprite_right_reload;
    player_sprite_right_reload.width = 22;
    player_sprite_right_reload.height = 14;
    player_sprite_right_reload.data = new uint8_t[308]
    {
        0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,1,1,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,1,1,1,0,0,0, // ......................
        0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,1,1,0,1,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,0,0, // ......................
        0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0, // ......................
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0, // ......................
        1,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
        0,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0, // ......................
    };
    
    Sprite player_sprite_right_slide;
    player_sprite_right_slide.width = 24;
    player_sprite_right_slide.height = 11;
    player_sprite_right_slide.data = new uint8_t[264]
    {
        0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,1,1,0,0,0,1,1,1,0,1,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0, // ......................
        0,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,0,0, // ......................
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, // ......................
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0, // ......................
        1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0, // ......................
        1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // ......................
    };
    
    Sprite player_sprite_grave;
    player_sprite_grave.width = 22;
    player_sprite_grave.height = 16;
    player_sprite_grave.data = new uint8_t[352]

    {
        0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0, // ......................
        0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
    };
    Sprite player_sprite_grave_rip;
    player_sprite_grave_rip.width = 11;
    player_sprite_grave_rip.height = 5;
    player_sprite_grave_rip.data = new uint8_t[55]
    {
        1,1,1,0,0,1,0,1,1,1,0, // ......................
        1,0,0,1,0,1,0,1,0,0,1, // ......................
        1,1,1,0,0,1,0,1,1,1,0, // ......................
        1,0,1,0,0,1,0,1,0,0,0, // ......................
        1,0,0,1,0,1,0,1,0,0,0, // ......................
    };
    
    Sprite grave_dirt;
    grave_dirt.width = 17;
    grave_dirt.height = 3;
    grave_dirt.data = new uint8_t[51]
    {
        0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,
        0,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,0,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    };
    

    
    
    
    Sprite jetpack_flame;
    jetpack_flame.width = 3;
    jetpack_flame.height = 6;
    jetpack_flame.data = new uint8_t[18]
    {
        1,1,1,
        1,1,0,
        1,1,0,
        0,1,1,
        1,1,0,
        0,1,0,
    };
    Sprite jetpack_flame_weak;
    jetpack_flame_weak.width = 3;
    jetpack_flame_weak.height = 6;
    jetpack_flame.data = new uint8_t[18]
    {
        1,1,1,
        0,1,1,
        1,1,0,
        0,1,0,
        0,0,0,
        0,0,0,

    };
    
    Sprite bullet_sprite;
    bullet_sprite.width = 4;
    bullet_sprite.height = 2;
    bullet_sprite.data = new uint8_t[8]
    {
        1,1,1,1,
        1,1,1,1,
    };
    
    Sprite UI_bullet_left;
    UI_bullet_left.width = 11;
    UI_bullet_left.height = 6;
    UI_bullet_left.data = new uint8_t[66]
    {
        1,1,1,1,1,1,1,1,0,0,0,
        1,1,1,1,1,1,1,1,1,1,0,
        1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,0,
        1,1,1,1,1,1,1,1,0,0,0,
    };
    
    Sprite UI_bullet_right;
    UI_bullet_right.width = 11;
    UI_bullet_right.height = 6;
    UI_bullet_right.data = new uint8_t[66]
    {
        0,0,0,1,1,1,1,1,1,1,1,
        0,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,
        0,1,1,1,1,1,1,1,1,1,1,
        0,0,0,1,1,1,1,1,1,1,1,
    };
    
    Sprite UI_heart;
    UI_heart.width = 13;
    UI_heart.height = 12;
    UI_heart.data = new uint8_t[156]
    {
        0,1,1,1,1,0,0,0,1,1,1,1,0,
        1,1,1,1,1,1,0,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,
        0,1,1,1,1,1,1,1,1,1,1,1,0,
        0,1,1,1,1,1,1,1,1,1,1,1,0,
        0,0,1,1,1,1,1,1,1,1,1,0,0,
        0,0,0,1,1,1,1,1,1,1,0,0,0,
        0,0,0,0,1,1,1,1,1,0,0,0,0,
        0,0,0,0,0,1,1,1,0,0,0,0,0,
        0,0,0,0,0,0,1,0,0,0,0,0,0,
    };
    
    
    Sprite ground;
    ground.width = buffer_width;
    ground.height = 4;
    ground.data = new uint8_t[ground.width*ground.height]{};
    for (int i = 0; i < ground.width*ground.height; ++i) {
        ground.data[i] = 1;
    }
    
    std::array<Sprite, 26> alphabet;
    for(int i = 0; i < alphabet.size(); ++i)
    {
        alphabet[i].width = 5;
        alphabet[i].height = 7;
    }
    alphabet[0].data = new uint8_t[5*7]
    {
        0,0,1,0,0, //..@..
        0,1,0,1,0, //.@.@.
        1,0,0,0,1, //@...@
        1,1,1,1,1, //@@@@@
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
    };
    
    alphabet[1].data = new uint8_t[5*7]
    {
        1,1,1,0,0, //@@@..
        1,0,0,1,0, //@..@.
        1,0,0,1,0, //@..@.
        1,1,1,1,0, //@@@@.
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
        1,1,1,1,0, //@@@@.
    };
    
    alphabet[2].data = new uint8_t[5*7]
    {
        0,1,1,1,0, //.@@@.
        1,0,0,0,1, //@...@
        1,0,0,0,0, //@....
        1,0,0,0,0, //@....
        1,0,0,0,0, //@....
        1,0,0,0,1, //@...@
        0,1,1,1,0, //.@@@.
    };
    
    alphabet[3].data = new uint8_t[5*7]
    {
        1,1,1,0,0, //@@@..
        1,0,0,1,0, //@..@.
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
        1,1,1,1,0, //@@@@.
    };
    alphabet[4].data = new uint8_t[5*7]
    {
        0,1,1,1,1, //.@@@@
        1,0,0,0,0, //@....
        1,0,0,0,0, //@....
        1,1,1,1,1, //@@@@@
        1,0,0,0,0, //@....
        1,0,0,0,0, //@....
        0,1,1,1,1, //.@@@@
    };
    alphabet[5].data = new uint8_t[5*7]
    {
        0,1,1,1,1, //.@@@@
        1,0,0,0,0, //@....
        1,0,0,0,0, //@....
        1,1,1,1,1, //@@@@@
        1,0,0,0,0, //@....
        1,0,0,0,0, //@....
        1,0,0,0,0, //@....
    };
    alphabet[6].data = new uint8_t[5*7]
    {
        0,1,1,1,0, //@@@@.
        1,0,0,0,1, //@...@
        1,0,0,0,0, //@....
        1,0,1,1,1, //@.@@@
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
        0,1,1,1,0, //.@@@.
    };
    alphabet[7].data = new uint8_t[5*7]
    {
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
        1,1,1,1,1, //@@@@@
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
    };
    alphabet[8].data = new uint8_t[5*7]
    {
        1,1,1,1,1, //@@@@@
        0,0,1,0,0, //..@..
        0,0,1,0,0, //..@..
        0,0,1,0,0, //..@..
        0,0,1,0,0, //..@..
        0,0,1,0,0, //..@..
        1,1,1,1,1, //@@@@@
    };
    alphabet[9].data = new uint8_t[5*7]
    {
        1,1,1,1,1, //@@@@@
        0,0,1,0,0, //..@..
        0,0,1,0,0, //..@..
        0,0,1,0,0, //..@..
        0,0,1,0,0, //..@..
        1,0,1,0,0, //@.@..
        1,1,1,1,0, //@@@..
    };
    alphabet[10].data = new uint8_t[5*7]
    {
        1,0,0,0,1, //@...@
        1,0,0,1,0, //@..@.
        1,0,1,0,0, //@.@..
        1,1,1,0,0, //@@@..
        1,0,0,1,0, //@..@..
        1,0,0,1,0, //@..@.
        1,0,0,0,1, //@...@
    };
    alphabet[11].data = new uint8_t[5*7]
    {
        1,0,0,0,0, //@....
        1,0,0,0,0, //@....
        1,0,0,0,0, //@....
        1,0,0,0,0, //@....
        1,0,0,0,0, //@....
        1,0,0,0,0, //@....
        0,1,1,1,1, //.@@@@
    };
    alphabet[12].data = new uint8_t[5*7]
    {
        1,0,0,0,1, //@...@
        1,1,0,1,1, //@@.@@
        1,0,1,0,1, //@.@.@
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
    };
    alphabet[13].data = new uint8_t[5*7]
    {
        1,0,0,0,1, //@...@
        1,1,0,0,1, //@@..@
        1,0,1,0,1, //@.@.@
        1,0,1,0,1, //@.@.@
        1,0,0,1,1, //@..@@
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
    };
    alphabet[14].data = new uint8_t[5*7]
    {
        0,1,1,1,0, //.@@@.
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
        0,1,1,1,0, //.@@@.
    };
    alphabet[15].data = new uint8_t[5*7]
    {
        1,1,1,1,0, //@@@@.P
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
        1,1,1,1,0, //@@@@.
        1,0,0,0,0, //@....
        1,0,0,0,0, //@....
        1,0,0,0,0, //@....
    };
    alphabet[16].data = new uint8_t[5*7]
    {
        0,1,1,1,0, //.@@@.Q
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
        1,0,1,0,1, //@.@.@
        1,0,0,1,0, //@..@.
        0,1,1,0,1, //.@@.@
    };
    alphabet[17].data = new uint8_t[5*7]
    {
        1,1,1,1,0, //@@@@.R
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
        1,1,1,1,0, //@@@@.
        1,0,1,0,0, //@.@..
        1,0,0,1,0, //@..@.
        1,0,0,0,1, //@...@
    };
    alphabet[18].data = new uint8_t[5*7]
    {
        0,1,1,1,1, //.@@@.S
        1,0,0,0,0, //@...@
        1,0,0,0,0, //@...@
        0,1,1,1,0, //@...@
        0,0,0,0,1, //@...@
        0,0,0,0,1, //@...@
        1,1,1,1,0, //.@@@.
    };
    alphabet[19].data = new uint8_t[5*7]
    {
        1,1,1,1,1, //.@@@.T
        0,0,1,0,0, //@...@
        0,0,1,0,0, //@...@
        0,0,1,0,0, //@...@
        0,0,1,0,0, //@...@
        0,0,1,0,0, //@...@
        0,0,1,0,0, //.@@@.
    };
    alphabet[20].data = new uint8_t[5*7]
    {
        1,0,0,0,1, //@...@U
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
        0,1,1,1,0, //.@@@.
    };
    alphabet[21].data = new uint8_t[5*7]
    {
        1,0,0,0,1, //@...@v
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
        0,1,0,1,0, //.@.@.
        0,1,0,1,0, //.@.@.
        0,0,1,0,0, //..@..
    };
    alphabet[22].data = new uint8_t[5*7]
    {
        1,0,0,0,1, //@...@W
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
        1,0,1,0,1, //@.@.@
        1,0,1,0,1, //@.@.@
        0,1,0,1,0, //.@@@.
    };
    alphabet[15].data = new uint8_t[5*7]
    {
        1,0,0,0,1, //@...@X
        1,0,0,0,1, //@...@
        0,1,0,1,0, //.@..@.
        0,0,1,0,0, //..@..
        0,1,0,1,0, //.@.@.
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
    };
    alphabet[24].data = new uint8_t[5*7]
    {
        1,0,0,0,1, //.@@@.Y
        1,0,0,0,1, //@...@
        0,1,0,1,0, //@...@
        0,0,1,0,0, //@...@
        0,0,1,0,0, //@...@
        0,0,1,0,05, //@...@
        0,1,1,1,0, //.@@@.
    };
    alphabet[25].data = new uint8_t[5*7]
    {
        1,1,1,1,1, //@@@@@Z
        0,0,0,0,1, //....@
        0,0,0,1,0, //...@.
        0,0,1,0,0, //..@..
        0,1,0,0,0, //.@...
        1,0,0,0,0, //@....
        1,1,1,1,1, //@@@@@
    };
    
    std::array<Sprite, 10> numbers;
    for(int i = 0; i < numbers.size(); ++i)
    {
        numbers[i].width = 5;
        numbers[i].height = 7;
    }
    numbers[0].data = new uint8_t[5*7]
    {
        0,1,1,1,0, //.@@@.
        1,0,0,0,1, //@...@
        1,0,0,1,1, //@..@@
        1,0,1,0,1, //@.@.@
        1,1,0,0,1, //@@..@
        1,0,0,0,1, //@...@
        0,1,1,1,0, //.@@@.
    };
    numbers[1].data = new uint8_t[5*7]
    {
        0,0,1,0,0, //..@..
        0,1,1,0,0, //.@@..
        1,0,1,0,0, //@.@..
        0,0,1,0,0, //..@..
        0,0,1,0,0, //..@..
        0,0,1,0,0, //..@..
        1,1,1,1,1, //@@@@@
    };
    numbers[2].data = new uint8_t[5*7]
    {
        0,1,1,1,0, //.@@@.
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
        0,0,0,1,0, //...@.
        0,0,1,0,0, //..@..
        0,1,0,0,0, //.@...
        1,1,1,1,1, //@@@@@
    };
    numbers[3].data = new uint8_t[5*7]
    {
        0,1,1,1,0, //.@@@.
        1,0,0,0,1, //@...@
        0,0,0,0,1, //....@
        0,0,1,1,0, //..@@.
        0,0,0,0,1, //....@
        1,0,0,0,1, //@...@
        0,1,1,1,0, //.@@@.
    };
    numbers[4].data = new uint8_t[5*7]
    {
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
        0,1,1,1,1, //.@@@@
        0,0,0,0,1, //....@
        0,0,0,0,1, //....@
        0,0,0,0,1, //....@
    };
    numbers[5].data = new uint8_t[5*7]
    {
        1,1,1,1,1, //@@@@@
        1,0,0,0,0, //@....
        0,1,1,1,0, //.@@@.
        0,0,0,0,1, //....@
        0,0,0,0,1, //....@
        0,0,0,0,1, //....@
        1,1,1,1,0, //@@@@.
    };
    numbers[6].data = new uint8_t[5*7]
    {
        0,1,1,1,0, //.@@@.
        1,0,0,0,0, //@....
        1,0,0,0,0, //@....
        1,1,1,1,0, //@@@@.
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
        0,1,1,1,0, //.@@@.
    };
    numbers[7].data = new uint8_t[5*7]
    {
        1,1,1,1,1, //@@@@@
        0,0,0,0,1, //....@
        0,0,0,0,1, //....@
        0,0,0,1,0, //...@.
        0,0,1,0,0, //..@..
        0,0,1,0,0, //..@..
        0,0,1,0,0, //..@..
    };
    numbers[8].data = new uint8_t[5*7]
    {
        0,1,1,1,0, //.@@@.
        1,0,0,0,1, //@...@
        1,0,0,0,1, //.@.@.
        0,1,1,1,0, //..@..
        1,0,0,0,1, //.@.@.
        1,0,0,0,1, //@...@
        0,1,1,1,0, //.@@@.
    };
    numbers[9].data = new uint8_t[5*7]
    {
        0,1,1,1,0, //.@@@.
        1,0,0,0,1, //@...@
        1,0,0,0,1, //@...@
        0,1,1,1,1, //.@@@@
        0,0,0,0,1, //....@
        1,0,0,0,1, //@...@
        0,1,1,1,0, //.@@@.
    };
    
    Sprite line_separator;
    line_separator.width = 1;
    line_separator.height = 11;
    line_separator.data = new uint8_t[line_separator.height];
    for(int i = 0; i < line_separator.height; ++i)
    {
        line_separator.data[i] = 1;
    }
    
    Sprite dust_sprite1;
    dust_sprite1.width = 12;
    dust_sprite1.height = 12;
    dust_sprite1.data = new uint8_t[144]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,1,0,0,
        0,0,0,0,0,1,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,1,0,0,0,
        0,0,1,0,0,0,0,0,0,0,0,0,
        0,1,0,1,0,0,1,0,0,1,0,0,
        0,0,1,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,1,1,0,0,0,
        0,0,0,1,0,0,1,0,1,1,0,0,
        0,0,0,0,0,0,1,0,0,1,0,0,
        0,0,1,0,0,0,0,1,1,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,
    };
    
    Sprite player_idle_left1;
    player_idle_left1.width = 23;
    player_idle_left1.height = 19;
    player_idle_left1.data = new uint8_t[437]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0, // ......................
        1,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0, // ......................
        1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, // ......................
        0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,1, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0, // ......................
    };
    Sprite player_idle_left2;
    player_idle_left2.width = 23;
    player_idle_left2.height = 19;
    player_idle_left2.data = new uint8_t[437]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0, // ......................
        0,0,0,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // ......................
        0,0,0,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,1, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0, // ......................
    };
    Sprite player_idle_left3;
    player_idle_left3.width = 23;
    player_idle_left3.height = 19;
    player_idle_left3.data = new uint8_t[437]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,1,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,1,1,1,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0, // ......................
        0,0,1,0,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, // ......................
        0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,1, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0, // ......................
    };
    Sprite player_idle_left4;
    player_idle_left4.width = 23;
    player_idle_left4.height = 19;
    player_idle_left4.data = new uint8_t[437]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,1,1,1,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,1,0,1,1,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,1,1,1,1,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, // ......................
        0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,1, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0, // ......................
    };
    Sprite player_idle_left5;
    player_idle_left5.width = 23;
    player_idle_left5.height = 19;
    player_idle_left5.data = new uint8_t[437]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,1,1,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, // ......................
        0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,1, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0, // ......................
    };
    Sprite player_idle_left6;
    player_idle_left6.width = 23;
    player_idle_left6.height = 19;
    player_idle_left6.data = new uint8_t[437]
    {
        0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, // ......................
        0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,1, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0, // ......................
    };
    Sprite player_idle_left7;
    player_idle_left7.width = 23;
    player_idle_left7.height = 19;
    player_idle_left7.data = new uint8_t[437]
    {
        0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,1,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, // ......................
        0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,1, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0, // ......................
    };
    Sprite player_idle_left8;
    player_idle_left8.width = 23;
    player_idle_left8.height = 19;
    player_idle_left8.data = new uint8_t[437]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, // ......................
        0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,1, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0, // ......................
    };
    Sprite player_idle_left9;
    player_idle_left9.width = 23;
    player_idle_left9.height = 19;
    player_idle_left9.data = new uint8_t[437]
    {
        0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, // ......................
        0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,1, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0, // ......................
    };
    Sprite player_idle_left10;
    player_idle_left10.width = 23;
    player_idle_left10.height = 19;
    player_idle_left10.data = new uint8_t[437]
    {
        0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, // ......................
        0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,1, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0, // ......................
    };
    Sprite player_idle_left11;
    player_idle_left11.width = 23;
    player_idle_left11.height = 19;
    player_idle_left11.data = new uint8_t[437]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, // ......................
        0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,1, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0, // ......................
    };
    Sprite player_idle_left12;
    player_idle_left12.width = 23;
    player_idle_left12.height = 19;
    player_idle_left12.data = new uint8_t[437]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,0,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, // ......................
        0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,1, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0, // ......................
    };
    Sprite player_idle_left13;
    player_idle_left13.width = 23;
    player_idle_left13.height = 19;
    player_idle_left13.data = new uint8_t[437]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,0,1,1,1,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0, // ......................
        0,1,1,0,1,1,1,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, // ......................
        0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,1, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0, // ......................
    };
    Sprite player_idle_left14;
    player_idle_left14.width = 23;
    player_idle_left14.height = 19;
    player_idle_left14.data = new uint8_t[437]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,0,1,1,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,1,1,1,0,1,1,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0, // ......................
        0,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, // ......................
        0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,1, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0, // ......................
    };
    Sprite player_idle_left15;
    player_idle_left15.width = 23;
    player_idle_left15.height = 19;
    player_idle_left15.data = new uint8_t[437]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0, // ......................
        0,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, // ......................
        0,0,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // ......................
        0,0,0,0,1,1,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,1, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0, // ......................
    };
    
    Sprite player_idle_right1;
    player_idle_right1.width = 23;
    player_idle_right1.height = 19;
    player_idle_right1.data = new uint8_t[437]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,1, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1, // ......................
        0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0, // ......................
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0, // ......................
        1,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0, //- ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
    };
    Sprite player_idle_right2;
    player_idle_right2.width = 23;
    player_idle_right2.height = 19;
    player_idle_right2.data = new uint8_t[437]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,0,0,0, // ......................
        0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,0,0,0, // ......................
        0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0, // ......................
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        1,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,0,0,0, // ......................
        0,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0, //- ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
    };
    Sprite player_idle_right3;
    player_idle_right3.width = 23;
    player_idle_right3.height = 19;
    player_idle_right3.data = new uint8_t[437]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,0,0, // ......................
        0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,1,1,1,0,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,0,1,0,0, // ......................
        0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0, // ......................
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0, // ......................
        1,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0, //- ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
    };
    Sprite player_idle_right4;
    player_idle_right4.width = 23;
    player_idle_right4.height = 19;
    player_idle_right4.data = new uint8_t[437]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0, // ......................
        0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,1,1,1,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,1,1,0,1,0,0, // ......................
        0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,1,1,1,1,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0, // ......................
        0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        1,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0, //- ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
    };
    Sprite player_idle_right5;
    player_idle_right5.width = 23;
    player_idle_right5.height = 19;
    player_idle_right5.data = new uint8_t[437]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,1,1,0, // ......................
        0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0, // ......................
        0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        1,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0, //- ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
    };
    Sprite player_idle_right6;
    player_idle_right6.width = 23;
    player_idle_right6.height = 19;
    player_idle_right6.data = new uint8_t[437]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0, // ......................
        0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        1,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0, //- ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
    };
    Sprite player_idle_right7;
    player_idle_right7.width = 23;
    player_idle_right7.height = 19;
    player_idle_right7.data = new uint8_t[437]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0, // ......................
        0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,1,1,1,0,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,1,0,0,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0, // ......................
        0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        1,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0, //- ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
    };
    Sprite player_idle_right8;
    player_idle_right8.width = 23;
    player_idle_right8.height = 19;
    player_idle_right8.data = new uint8_t[437]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0, // ......................
        0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0, // ......................
        0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        1,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0, //- ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
    };
    Sprite player_idle_right9;
    player_idle_right9.width = 23;
    player_idle_right9.height = 19;
    player_idle_right9.data = new uint8_t[437]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,1,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0, // ......................
        0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0, // ......................
        0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        1,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0, //- ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
    };
    Sprite player_idle_right10;
    player_idle_right10.width = 23;
    player_idle_right10.height = 19;
    player_idle_right10.data = new uint8_t[437]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0, // ......................
        0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        1,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0, //- ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
    };
    Sprite player_idle_right11;
    player_idle_right11.width = 23;
    player_idle_right11.height = 19;
    player_idle_right11.data = new uint8_t[437]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0, // ......................
        0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        1,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0, //- ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
    };
    Sprite player_idle_right12;
    player_idle_right12.width = 23;
    player_idle_right12.height = 19;
    player_idle_right12.data = new uint8_t[437]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0, // ......................
        0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,0,1,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,0,0,0, // ......................
        0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0, // ......................
        0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        1,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0, //- ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
    };
    Sprite player_idle_right13;
    player_idle_right13.width = 23;
    player_idle_right13.height = 19;
    player_idle_right13.data = new uint8_t[437]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,1,1,1,0,1,1,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,1,1,1,0,1,1,0, // ......................
        0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,0,0, // ......................
        0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,0,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0, // ......................
        0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        1,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0, //- ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
    };
    Sprite player_idle_right14;
    player_idle_right14.width = 23;
    player_idle_right14.height = 19;
    player_idle_right14.data = new uint8_t[437]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,0,0,0, // ......................
        0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,1,1,0,1,1,1,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,1,1,1,0, // ......................
        0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0, // ......................
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        1,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0, //- ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
    };
    Sprite player_idle_right15;
    player_idle_right15.width = 23;
    player_idle_right15.height = 19;
    player_idle_right15.data = new uint8_t[437]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,0, // ......................
        0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, // ......................
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0,0, // ......................
        1,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,1,1,0,0,0,0, // ......................
        0,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0, //- ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
    };

    
    Sprite player_idle_rightdum;
    player_idle_rightdum.width = 23;
    player_idle_rightdum.height = 19;
    player_idle_rightdum.data = new uint8_t[437]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0, // ......................
        0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0, // ......................
        1,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // ......................
        0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0, //- ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
        0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // ......................
    };
    
    Sprite dust_sprite2;
    dust_sprite2.width = 12;
    dust_sprite2.height = 12;
    dust_sprite2.data = new uint8_t[144]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,1,0,0,
        0,0,0,0,0,1,0,0,0,0,0,0,
        0,0,1,1,0,0,0,0,1,0,0,0,
        0,1,0,0,0,0,0,0,0,0,0,0,
        0,1,1,0,1,0,1,0,0,1,0,0,
        0,0,1,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,1,0,0,0,
        0,0,0,1,0,0,0,1,0,1,0,0,
        0,0,0,0,0,0,0,0,1,0,0,0,
        0,0,1,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,
    };
    
    Sprite dust_sprite3;
    dust_sprite3.width = 12;
    dust_sprite3.height = 12;
    dust_sprite3.data = new uint8_t[144]
    {
        0,0,0,0,0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,1,0,
        0,0,1,1,0,0,0,0,0,0,0,0,
        0,1,0,0,1,0,0,0,0,0,0,0,
        0,1,0,0,1,0,1,0,0,1,0,0,
        0,0,1,1,0,0,0,0,0,0,0,0,
        0,0,0,0,0,1,0,0,1,0,0,0,
        0,0,0,1,0,0,0,1,0,1,0,0,
        0,0,0,0,0,0,0,1,1,0,0,0,
        0,0,0,0,0,0,1,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,
    };
    
    Sprite dust_sprite4;
    dust_sprite4.width = 12;
    dust_sprite4.height = 12;
    dust_sprite4.data = new uint8_t[144]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,1,0,0,0,0,0,0,
        0,0,0,0,1,0,1,0,0,0,1,0,
        0,0,0,0,0,1,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,1,0,0,
        0,0,0,1,0,0,0,0,0,0,0,0,
        0,0,0,0,0,1,0,0,0,0,0,0,
        0,0,1,0,0,0,0,0,0,0,0,0,
        0,1,0,0,0,0,0,1,0,0,0,0,
        0,0,1,1,0,0,1,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,
    };
    
    
    
    SpriteAnimation* alien_animation = new SpriteAnimation;
    alien_animation -> loop = true;
    alien_animation -> num_frames = 2;
    alien_animation -> frame_duration = 10;
    alien_animation -> time = 0;
    
    alien_animation->frames = new Sprite*[2];
    alien_animation->frames[1] = &alien_sprite;
    alien_animation->frames[0] = &alien_sprite1;
    
    SpriteAnimation* dust_animation = new SpriteAnimation;
    dust_animation -> loop = true;
    dust_animation -> num_frames = 4;
    dust_animation -> frame_duration = 1;
    
    dust_animation -> frames = new Sprite*[dust_animation -> num_frames];
    dust_animation -> frames[0] = &dust_sprite1;
    dust_animation -> frames[1] = &dust_sprite2;
    dust_animation -> frames[2] = &dust_sprite3;
    dust_animation -> frames[3] = &dust_sprite4;
    
    
    SpriteAnimation* player_animation_left = new SpriteAnimation[NUM_PLAYERS];
    player_animation_left[0].loop = true;
    player_animation_left[0].num_frames = 13;
    player_animation_left[0].frame_duration = 3;
    player_animation_left[0].time = 0;
    
    player_animation_left[0].frames = new Sprite*[player_animation_left[0].num_frames];
    player_animation_left[0].frames[0] = &player_sprite_left1;
    player_animation_left[0].frames[1] = &player_sprite_left1;
    player_animation_left[0].frames[2] = &player_sprite_left2;
    player_animation_left[0].frames[3] = &player_sprite_left2;
    player_animation_left[0].frames[4] = &player_sprite_left3;
    player_animation_left[0].frames[5] = &player_sprite_left3;
    player_animation_left[0].frames[6] = &player_sprite_left4;
    player_animation_left[0].frames[7] = &player_sprite_left5;
    player_animation_left[0].frames[8] = &player_sprite_left4;
    player_animation_left[0].frames[9] = &player_sprite_left3;
    player_animation_left[0].frames[10] = &player_sprite_left3;
    player_animation_left[0].frames[11] = &player_sprite_left2;
    player_animation_left[0].frames[12] = &player_sprite_left3;
    
    player_animation_left[1].loop = true;
    player_animation_left[1].num_frames = 13;
    player_animation_left[1].frame_duration = 3;
    player_animation_left[1].time = 0;
    
    player_animation_left[1].frames = new Sprite*[player_animation_left[1].num_frames];
    player_animation_left[1].frames[0] = &player_sprite_left1;
    player_animation_left[1].frames[1] = &player_sprite_left1;
    player_animation_left[1].frames[2] = &player_sprite_left2;
    player_animation_left[1].frames[3] = &player_sprite_left2;
    player_animation_left[1].frames[4] = &player_sprite_left3;
    player_animation_left[1].frames[5] = &player_sprite_left3;
    player_animation_left[1].frames[6] = &player_sprite_left4;
    player_animation_left[1].frames[7] = &player_sprite_left5;
    player_animation_left[1].frames[8] = &player_sprite_left4;
    player_animation_left[1].frames[9] = &player_sprite_left3;
    player_animation_left[1].frames[10] = &player_sprite_left3;
    player_animation_left[1].frames[11] = &player_sprite_left2;
    player_animation_left[1].frames[12] = &player_sprite_left3;
    
    
    SpriteAnimation* player_animation_right = new SpriteAnimation[NUM_PLAYERS];
    player_animation_right[0].loop = true;
    player_animation_right[0].num_frames = 13;
    player_animation_right[0].frame_duration = 3;
    player_animation_right[0].time = 0;
    
    player_animation_right[0].frames = new Sprite*[player_animation_right[0].num_frames];
    player_animation_right[0].frames[0] = &player_sprite_right1;
    player_animation_right[0].frames[1] = &player_sprite_right1;
    player_animation_right[0].frames[2] = &player_sprite_right2;
    player_animation_right[0].frames[3] = &player_sprite_right2;
    player_animation_right[0].frames[4] = &player_sprite_right3;
    player_animation_right[0].frames[5] = &player_sprite_right3;
    player_animation_right[0].frames[6] = &player_sprite_right4;
    player_animation_right[0].frames[7] = &player_sprite_right5;
    player_animation_right[0].frames[8] = &player_sprite_right4;
    player_animation_right[0].frames[9] = &player_sprite_right3;
    player_animation_right[0].frames[10] = &player_sprite_right3;
    player_animation_right[0].frames[11] = &player_sprite_right2;
    player_animation_right[0].frames[12] = &player_sprite_right2;
    
    player_animation_right[1].loop = true;
    player_animation_right[1].num_frames = 13;
    player_animation_right[1].frame_duration = 3;
    player_animation_right[1].time = 0;
    
    player_animation_right[1].frames = new Sprite*[player_animation_right[1].num_frames];
    player_animation_right[1].frames[0] = &player_sprite_right1;
    player_animation_right[1].frames[1] = &player_sprite_right1;
    player_animation_right[1].frames[2] = &player_sprite_right2;
    player_animation_right[1].frames[3] = &player_sprite_right2;
    player_animation_right[1].frames[4] = &player_sprite_right3;
    player_animation_right[1].frames[5] = &player_sprite_right3;
    player_animation_right[1].frames[6] = &player_sprite_right4;
    player_animation_right[1].frames[7] = &player_sprite_right5;
    player_animation_right[1].frames[8] = &player_sprite_right4;
    player_animation_right[1].frames[9] = &player_sprite_right3;
    player_animation_right[1].frames[10] = &player_sprite_right3;
    player_animation_right[1].frames[11] = &player_sprite_right2;
    player_animation_right[1].frames[12] = &player_sprite_right2;
    
    
    SpriteAnimation* player_idle_anim_left = new SpriteAnimation[2];
    player_idle_anim_left[0].loop = true;
    player_idle_anim_left[0].num_frames = 15;
    player_idle_anim_left[0].frame_duration = 5;
    player_idle_anim_left[0].time = 0;
    
    player_idle_anim_left[0].frames = new Sprite*[player_idle_anim_left->num_frames];
    player_idle_anim_left[0].frames[0] = &player_idle_left1;
    //player_idle_anim_left[0].frames[1] = &player_idle_left1;
    player_idle_anim_left[0].frames[1] = &player_idle_left2;
    //player_idle_anim_left[0].frames[3] = &player_idle_left2;
    player_idle_anim_left[0].frames[2] = &player_idle_left3;
    //player_idle_anim_left[0].frames[5] = &player_idle_left3;
    player_idle_anim_left[0].frames[3] = &player_idle_left4;
    //player_idle_anim_left[0].frames[7] = &player_idle_left4;
    player_idle_anim_left[0].frames[4] = &player_idle_left5;
    //player_idle_anim_left[0].frames[9] = &player_idle_left5;
    player_idle_anim_left[0].frames[5] = &player_idle_left6;
    //player_idle_anim_left[0].frames[11] = &player_idle_left6;
    player_idle_anim_left[0].frames[6] = &player_idle_left7;
    //player_idle_anim_left[0].frames[13] = &player_idle_left7;
    player_idle_anim_left[0].frames[7] = &player_idle_left8;
    //player_idle_anim_left[0].frames[15] = &player_idle_left8;
    player_idle_anim_left[0].frames[8] = &player_idle_left9;
    //player_idle_anim_left[0].frames[17] = &player_idle_left9;
    player_idle_anim_left[0].frames[9] = &player_idle_left10;
    player_idle_anim_left[0].frames[10] = &player_idle_left10;
    player_idle_anim_left[0].frames[11] = &player_idle_left11;
    player_idle_anim_left[0].frames[12] = &player_idle_left12;
    player_idle_anim_left[0].frames[13] = &player_idle_left13;
    player_idle_anim_left[0].frames[14] = &player_idle_left14;
    player_idle_anim_left[0].frames[15] = &player_idle_left15;
    
    player_idle_anim_left[1].loop = true;
    player_idle_anim_left[1].num_frames = 15;
    player_idle_anim_left[1].frame_duration = 5;
    player_idle_anim_left[1].time = 0;
    
    player_idle_anim_left[1].frames = new Sprite*[player_idle_anim_left->num_frames];
    player_idle_anim_left[1].frames[0] = &player_idle_left1;
    //player_idle_anim_left[0].frames[1] = &player_idle_left1;
    player_idle_anim_left[1].frames[1] = &player_idle_left2;
    //player_idle_anim_left[0].frames[3] = &player_idle_left2;
    player_idle_anim_left[1].frames[2] = &player_idle_left3;
    //player_idle_anim_left[0].frames[5] = &player_idle_left3;
    player_idle_anim_left[1].frames[3] = &player_idle_left4;
    //player_idle_anim_left[0].frames[7] = &player_idle_left4;
    player_idle_anim_left[1].frames[4] = &player_idle_left5;
    //player_idle_anim_left[0].frames[9] = &player_idle_left5;
    player_idle_anim_left[1].frames[5] = &player_idle_left6;
    //player_idle_anim_left[0].frames[11] = &player_idle_left6;
    player_idle_anim_left[1].frames[6] = &player_idle_left7;
    //player_idle_anim_left[0].frames[13] = &player_idle_left7;
    player_idle_anim_left[1].frames[7] = &player_idle_left8;
    //player_idle_anim_left[0].frames[15] = &player_idle_left8;
    player_idle_anim_left[1].frames[8] = &player_idle_left9;
    //player_idle_anim_left[0].frames[17] = &player_idle_left9;
    player_idle_anim_left[1].frames[9] = &player_idle_left10;
    player_idle_anim_left[1].frames[10] = &player_idle_left10;
    player_idle_anim_left[1].frames[11] = &player_idle_left11;
    player_idle_anim_left[1].frames[12] = &player_idle_left12;
    player_idle_anim_left[1].frames[13] = &player_idle_left13;
    player_idle_anim_left[1].frames[14] = &player_idle_left14;
    player_idle_anim_left[1].frames[15] = &player_idle_left15;
    
    
    SpriteAnimation* player_idle_anim_right = new SpriteAnimation[2];
    player_idle_anim_right[0].loop = true;
    player_idle_anim_right[0].num_frames = 15;
    player_idle_anim_right[0].frame_duration = 5;
    player_idle_anim_right[0].time = 0;
    
    player_idle_anim_right[0].frames = new Sprite*[player_idle_anim_right[0].num_frames];
    player_idle_anim_right[0].frames[0] = &player_idle_right1;
    //player_idle_anim_right[0].frames[1] = &player_idle_right1;
    player_idle_anim_right[0].frames[1] = &player_idle_right2;
    //player_idle_anim_right[0].frames[3] = &player_idle_right2;
    player_idle_anim_right[0].frames[2] = &player_idle_right3;
    //player_idle_anim_right[0].frames[5] = &player_idle_right3;
    player_idle_anim_right[0].frames[3] = &player_idle_right4;
    //player_idle_anim_right[0].frames[7] = &player_idle_right4;
    player_idle_anim_right[0].frames[4] = &player_idle_right5;
    //player_idle_anim_right[0].frames[9] = &player_idle_right5;
    player_idle_anim_right[0].frames[5] = &player_idle_right6;
    //player_idle_anim_right[0].frames[11] = &player_idle_right6;
    player_idle_anim_right[0].frames[6] = &player_idle_right7;
    //player_idle_anim_right[0].frames[13] = &player_idle_right7;
    player_idle_anim_right[0].frames[7] = &player_idle_right8;
    //player_idle_anim_right[0].frames[15] = &player_idle_right8;
    player_idle_anim_right[0].frames[8] = &player_idle_right9;
    //player_idle_anim_right[0].frames[17] = &player_idle_right9;
    player_idle_anim_right[0].frames[9] = &player_idle_right10;
    player_idle_anim_right[0].frames[10] = &player_idle_right10;
    player_idle_anim_right[0].frames[11] = &player_idle_right11;
    player_idle_anim_right[0].frames[12] = &player_idle_right12;
    player_idle_anim_right[0].frames[13] = &player_idle_right13;
    player_idle_anim_right[0].frames[14] = &player_idle_right14;
    player_idle_anim_right[0].frames[15] = &player_idle_right15;
    
    player_idle_anim_right[1].loop = true;
    player_idle_anim_right[1].num_frames = 15;
    player_idle_anim_right[1].frame_duration = 5;
    player_idle_anim_right[1].time = 0;
    
    player_idle_anim_right[1].frames = new Sprite*[player_idle_anim_right[0].num_frames];
    player_idle_anim_right[1].frames[0] = &player_idle_right1;
    //player_idle_anim_right[0].frames[1] = &player_idle_right1;
    player_idle_anim_right[1].frames[1] = &player_idle_right2;
    //player_idle_anim_right[0].frames[3] = &player_idle_right2;
    player_idle_anim_right[1].frames[2] = &player_idle_right3;
    //player_idle_anim_right[0].frames[5] = &player_idle_right3;
    player_idle_anim_right[1].frames[3] = &player_idle_right4;
    //player_idle_anim_right[0].frames[7] = &player_idle_right4;
    player_idle_anim_right[1].frames[4] = &player_idle_right5;
    //player_idle_anim_right[0].frames[9] = &player_idle_right5;
    player_idle_anim_right[1].frames[5] = &player_idle_right6;
    //player_idle_anim_right[0].frames[11] = &player_idle_right6;
    player_idle_anim_right[1].frames[6] = &player_idle_right7;
    //player_idle_anim_right[0].frames[13] = &player_idle_right7;
    player_idle_anim_right[1].frames[7] = &player_idle_right8;
    //player_idle_anim_right[0].frames[15] = &player_idle_right8;
    player_idle_anim_right[1].frames[8] = &player_idle_right9;
    //player_idle_anim_right[0].frames[17] = &player_idle_right9;
    player_idle_anim_right[1].frames[9] = &player_idle_right10;
    player_idle_anim_right[1].frames[10] = &player_idle_right10;
    player_idle_anim_right[1].frames[11] = &player_idle_right11;
    player_idle_anim_right[1].frames[12] = &player_idle_right12;
    player_idle_anim_right[1].frames[13] = &player_idle_right13;
    player_idle_anim_right[1].frames[14] = &player_idle_right14;
    player_idle_anim_right[1].frames[15] = &player_idle_right15;

    uint32_t clear_colour = rgb_to_uint32(0, 128, 0);
    //glfwSwapInterval(1);
    //int i = 0;
    //int u = 0;
    
    
    
//    bool sprite_overlap_check(
//                              const Sprite& sp_a, size_t x_a, size_t y_a,
//                              const Sprite& sp_b, size_t x_b, size_t y_b
//                              );
    //bool platform_overlap = false;;
    
    int player_colours[NUM_PLAYERS][3]
    {
        {5,10,90},
        {100,1,1}
    };
    
    /*SpriteSheet graves[2][3];
    graves[0][0].fragment = player_sprite_grave;
    graves[0][0].colour = rgb_to_uint32(player_colours[0][0],player_colours[0][1],player_colours[0][2]);//(130, 103, 40);
    graves[0][0].x = 0;
    graves[0][0].y = 0;
    graves[0][1].fragment = player_sprite_grave_rip;
    graves[0][1].colour = rgb_to_uint32(0, 0, 0);
    graves[0][1].x = 7;
    graves[0][1].y = 6;
    graves[0][2].fragment = grave_dirt;
    graves[0][2].colour = rgb_to_uint32(130, 103, 40);
    graves[0][2].x = 0;
    graves[0][2].y = 0;
    
    graves[1][0].fragment = player_sprite_grave;
    graves[1][0].colour = rgb_to_uint32(player_colours[1][0],player_colours[1][1],player_colours[1][2]);//(130, 103, 40);
    graves[1][0].x = 0;
    graves[1][0].y = 0;
    graves[1][1].fragment = player_sprite_grave_rip;
    graves[1][1].colour = rgb_to_uint32(0, 0, 0);
    graves[1][1].x = 7;
    graves[1][1].y = 6;
    graves[1][2].fragment = grave_dirt;
    graves[1][2].colour = rgb_to_uint32(130, 103, 40);
    graves[1][2].x = 0;
    graves[1][2].y = 0;*/

    LayerSprite graves[2];
    graves[0].resize(3);
    graves[1].resize(3);
    graves[0][0].fragment = player_sprite_grave;
    graves[0][0].colour = rgb_to_uint32(player_colours[0][0],player_colours[0][1],player_colours[0][2]);//(130, 103, 40);
    graves[0][0].x = 0;
    graves[0][0].y = 0;
    graves[0][1].fragment = player_sprite_grave_rip;
    graves[0][1].colour = rgb_to_uint32(130, 140, 140);
    graves[0][1].x = 6;
    graves[0][1].y = 6;
    graves[0][2].fragment = grave_dirt;
    graves[0][2].colour = rgb_to_uint32(130, 103, 40);
    graves[0][2].x = 3;
    graves[0][2].y = 0;
    
    graves[1][0].fragment = player_sprite_grave;
    graves[1][0].colour = rgb_to_uint32(player_colours[1][0],player_colours[1][1],player_colours[1][2]);//(130, 103, 40);
    graves[1][0].x = 0;
    graves[1][0].y = 0;
    graves[1][1].fragment = player_sprite_grave_rip;
    graves[1][1].colour = rgb_to_uint32(130, 140, 140);
    graves[1][1].x = 6;
    graves[1][1].y = 6;
    graves[1][2].fragment = grave_dirt;
    graves[1][2].colour = rgb_to_uint32(130, 103, 40);
    graves[1][2].x = 3;
    graves[1][2].y = 0;
    
    int score_x_position[2];
    score_x_position[0] = 6*2;
    score_x_position[1] = 6*2;
    //player_scores[0] = 99;
    //player_scores[1] = 9;
    //Window open
    /*do
    {
        
        if(timesecs == 0)
        {
            timesecs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        }
        int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        int64_t timeDiff = now - timesecs;
        if(timeDiff%)
    }while(game_running);*/
    
    while (!glfwWindowShouldClose(window) && game_running){
        buffer_clear(&buffer, clear_colour);
        buffer_draw_sprite(&buffer, ground, 0, 0, rgb_to_uint32(100, 80, 30));
        //Sprite& aliensprite;
        
        jump1 = game.player[0].jump;
        air_time1 = game.player[0].air_time;
        jump2 = game.player[1].jump;
        air_time2 = game.player[1].air_time;
        game.player[0].jump = press_w;
        game.player[1].jump = press_up;
        game.player[0].shoot = press_shoot1;
        game.player[1].shoot = press_shoot2;
        game.player[0].moving = moving1;
        game.player[0].moving_left = moving_left1;
        game.player[1].moving = moving2;
        game.player[1].moving_left = moving_left2;
        game.player[0].slide = press_slide1;
        game.player[1].slide = press_slide2;
        
        game.player[0].reload = press_reload1;
        game.player[1].reload = press_reload2;
        press_reload1 = false;
        press_reload2 = false;
        
        
        
        //moving = false;
        //buffer_draw_sprite(&buffer, alien_sprite, 112, 128, rgb_to_uint32(128, 0, 0));
        //buffer_draw_sprite(&buffer, custom_lol, 97, 120, rgb_to_uint32(0, 128, 128));
        //buffer_draw_sprite(&buffer, space_invaders_title, 97, 140, rgb_to_uint32(128, 128, 128));
        
        
        for(int i = 0; i < NUM_PLATFORMS; ++i)
        {
            Platform& platform = game.platforms[i];
            //buffer_draw_sprite(&buffer, platform_sprite, game.platforms[i], 100, rgb_to_uint32(128, 128, 60));
            buffer_draw_sprite(&buffer, platform_sprite, platform.x, platform.y, rgb_to_uint32(128, 128, 60));
        }

        
        
        
        /*
         Player related
         - Basic collision check
         - y Movement
         - y Movement
         - Sprite changing
         */
//        if(game.player[0].state == PLAYER_STATE_DEAD)
//            buffer_draw_sprite(&buffer, player_sprite_grave, game.player[0].x, game.player[0].y+ground.height, rgb_to_uint32(player_colours[0][0], player_colours[0][1], player_colours[0][2]));
//        if(game.player[1].state == PLAYER_STATE_DEAD)
//            buffer_draw_sprite(&buffer, player_sprite_grave, game.player[1].x, game.player[1].y+ground.height, rgb_to_uint32(player_colours[1][0], player_colours[1][1], player_colours[1][2]));
            //Drawing the graves
        
            for(int gi = 0; gi < game.grave.size(); ++gi)
            {
                Grave &GRAVE = game.grave[gi];
                if(GRAVE.exist)
                {
                    //graves[0].colour = rgb_to_uint32(player_colours[game.grave[gi].player][0], player_colours[game.grave[gi].player][1], player_colours[game.grave[gi].player][2]);
                    draw_layer_sprite(graves[game.grave[gi].player], graves[game.grave[gi].player].size(), &buffer, game.grave[gi].x, game.grave[gi].y);
                    
                    
                    ;
                    
                }
                bool platform_overlap = false;
                for(int ui = 0; ui < NUM_PLATFORMS; ++ui)
                {
                    const Platform &PLATFORM = game.platforms[ui];
                    bool platform_overlap = sprite_overlap_check(player_sprite_grave, GRAVE.x, GRAVE.y, platform_sprite, PLATFORM.x, PLATFORM.y);
                    if(platform_overlap && GRAVE.y >= PLATFORM.y)
                    {
                        GRAVE.grounded.standing = true;
                        GRAVE.grounded.standing_on_num = ui;
                        GRAVE.y = PLATFORM.y+platform_sprite.height;
                    }
                    
                }
                if(GRAVE.y <= ground.height)
                {
                    GRAVE.grounded.standing = true;
                    
                }
                
                else if(GRAVE.y != ground.height && !GRAVE.grounded.standing)
                {
                    GRAVE.y += -1;
                }
                
                if(GRAVE.y < ground.height)
                    GRAVE.y = ground.height;
                    //draw_layer_sprite(graves[pi], graves[pi].size(), &buffer, game.grave[gi].x, game.grave[gi].y);
                    //buffer_draw_sprite(&buffer, player_sprite_grave, game.grave[gi].x, game.grave[gi].y, rgb_to_uint32(player_colours[game.grave[gi].player][0], player_colours[game.grave[gi].player][1], player_colours[game.grave[gi].player][2]));
                
            }
        
            for(int pi = 0; pi < NUM_PLAYERS; ++pi)
            {
                
                Player &PLAYER = game.player[pi];
                if(!PLAYER.alive)
                {
                    PLAYER.moving = false;
                    PLAYER.shoot = false;
                    PLAYER.jump = false;
                    PLAYER.reload = false;
                    PLAYER.slide = false;
                }

                Sprite& bulletsprite = bullet_sprite;
                Sprite& playersprite = player_sprite_left1;
                
//                if(PLAYER.moving_left)
//                    playersprite = player_sprite_left1;
//                else if(!PLAYER.moving_left)
//                    //playersprite = player_sprite_right1;
                
                //std::cout<<std::endl;
                if(!PLAYER.moving && !PLAYER.jump && !PLAYER.slide && !PLAYER.shoot)
                {
                    ++PLAYER.idle_counter;
                }
                else
                {
                    PLAYER.idle_counter = 0;
                }
                
                if(PLAYER.idle_counter >= idle_activ_time)
                {
                    std::cout<< "IDLE ANIMATION PLAYER "<< pi <<std::endl;
                }
                int player_move_direction[2] {};
                player_move_direction[0] = move_direction[0];
                player_move_direction[1] = move_direction[1];
                
                if (PLAYER.x + player_sprite_left1.width + player_move_direction[0] >= game.width - 1){
                    PLAYER.x = game.width - player_sprite_left1.width - player_move_direction[0] - 1;
                    player_move_direction[0] *= -1;
                }
                else if (static_cast<int>(PLAYER.x) + player_move_direction[0] <= 0){
                    PLAYER.x = 0;
                    player_move_direction[0] *= -1;
                }
                
                if (PLAYER.y + player_sprite_left1.height + player_move_direction[1] > game.height - 1) {
                    PLAYER.y = game.height - player_sprite_left1.height - player_move_direction[1]-1;
                    PLAYER.y = game.height - player_sprite_left1.height - player_move_direction[1]-1;
                    player_move_direction[1] *= -1;
                }
                else if (static_cast<int>(PLAYER.y) + player_move_direction[1] <= 0){
                    PLAYER.y = 0;
                    player_move_direction[1] *= -1;
                }
                
                //int player_sprite_num = 0;
                
                
                
                //x Movement
                if(PLAYER.moving && PLAYER.moving_left && PLAYER.slide && (PLAYER.sliding || PLAYER.y == 0 || PLAYER.standing.standing) && PLAYER.slide_time < max_slide_time)
                {
                    player_move_direction[0] = -slide_speed;
                    ++PLAYER.slide_time;
                    PLAYER.sliding = true;
                    
                }
                else if(PLAYER.moving && !PLAYER.moving_left  && PLAYER.slide && (PLAYER.sliding || PLAYER.y == 0 || PLAYER.standing.standing) && PLAYER.slide_time < max_slide_time)
                {
                    player_move_direction[0] = slide_speed;
                    ++PLAYER.slide_time;
                    PLAYER.sliding = true;
                }
                else if(PLAYER.moving && PLAYER.moving_left)
                {
                    player_move_direction[0] = -move_speed;
                }
                else if(PLAYER.moving && !PLAYER.moving_left)
                {
                    player_move_direction[0] = move_speed;
                }
                else
                {
                    player_move_direction[0] = 0;
                }
                
                if(!(PLAYER.moving && PLAYER.slide && (PLAYER.sliding || PLAYER.y == 0 || PLAYER.standing.standing) && PLAYER.slide_time < max_slide_time) || !PLAYER.moving)
                {
                    PLAYER.sliding = false;
                }
                    
                PLAYER.x += player_move_direction[0];
                PLAYER.y += player_move_direction[1];

                
                if(PLAYER.standing.standing || PLAYER.y == 0)
                {
                    can_jump = true;
                    PLAYER.air_time = 0;
                }
                
                if(PLAYER.air_time == max_jump_time)
                    PLAYER.fall = true;
                else
                    PLAYER.fall = false;
                
                if(PLAYER.jump && (PLAYER.can_jump || PLAYER.air_time < max_jump_time-jump_dim))
                {
                    PLAYER.y += jump_height;
                    //jump_surface_y = game.player.y;
                    PLAYER.standing.standing = false;
                    PLAYER.can_jump = false;
                    PLAYER.air_time += 1;
                    //std::cout<< air_time <<std::endl;
                }
                else if(PLAYER.jump && (PLAYER.can_jump || PLAYER.air_time <= max_jump_time))
                {
                    PLAYER.y += jump_height*0.75;
                    //jump_surface_y = game.player.y;
                    PLAYER.standing.standing = false;
                    PLAYER.can_jump = false;
                    PLAYER.air_time += 1;
                    //std::cout<< air_time <<std::endl;
                }
                
                //Gravity Implementation
                if(PLAYER.y > 0 || PLAYER.fall)
                {
                    PLAYER.y += gravity;// + cal_grav(game.player.y, buffer);
                }
                
                if(PLAYER.moving_left && PLAYER.sliding)
                {
                    PLAYER.current_sprite = player_sprite_left_slide;
                }
                else if(!PLAYER.moving_left && PLAYER.sliding)
                {
                    PLAYER.current_sprite = player_sprite_right_slide;
                }
                else if(PLAYER.moving_left && PLAYER.moving) {
                    
                    ++player_animation_left[pi].time;
                    if(player_animation_left[pi].time == player_animation_left[pi].num_frames*player_animation_left[pi].frame_duration && player_animation_left[pi].loop)
                    {
                        player_animation_left[pi].time = 0;
                        
                    }
                    
                    size_t current_frame = player_animation_left[pi].time/player_animation_left[pi].frame_duration;
                    PLAYER.current_sprite = *player_animation_left[pi].frames[current_frame];
                    
                    //std::cout<< current_frame << " : " << i <<std::endl;
                    //i += 1;
                }
                
                else if(!PLAYER.moving_left && PLAYER.moving)
                {
                    ++player_animation_right[pi].time;
                    if(player_animation_right[pi].time == player_animation_right[pi].num_frames*player_animation_right[pi].frame_duration && player_animation_right[pi].loop)
                    {
                        player_animation_right[pi].time = 0;
                        
                    }
                    
                    size_t current_frame = player_animation_right[pi].time/player_animation_right[pi].frame_duration;
                    PLAYER.current_sprite = *player_animation_right[pi].frames[current_frame];
                    
                    //std::cout<< current_frame << " : " << u <<std::endl;
                    //u += 1;
                }
                else if(PLAYER.moving_left && !PLAYER.moving)
                {
                    PLAYER.current_sprite = player_sprite_left1;
                    
                }
                else if(!PLAYER.moving_left && !PLAYER.moving)
                {
                    PLAYER.current_sprite = player_sprite_right1;
                    
                }
                if(PLAYER.jump)
                {
                    
                    if(PLAYER.moving_left)
                    {
                        PLAYER.current_sprite = player_sprite_left2;
                    }
                    else
                    {
                        PLAYER.current_sprite = player_sprite_right2;
                    }
                    
                }
                int ui = 0;
                bool platform_overlap = false;
                for(int i = 0; i < NUM_PLATFORMS; ++i)
                {
                            platform_overlap = sprite_overlap_check(PLAYER.current_sprite, PLAYER.x, PLAYER.y, platform_sprite, game.platforms[i].x, game.platforms[i].y);
                            if(platform_overlap)
                            {
                                ui = i;
                                //std::cout<< "Overlap with platform: " << i+1 <<std::endl;
                                break;
                            }
                }
                
                if(PLAYER.standing.standing)
                {
                    platform_overlap = false;
                    PLAYER.y = game.platforms[PLAYER.standing.standing_on_num].y+1;
                    
                    if(!sprite_overlap_check(playersprite, PLAYER.x, PLAYER.y, platform_sprite, game.platforms[PLAYER.standing.standing_on_num].x, game.platforms[PLAYER.standing.standing_on_num].y))
                       {
                           PLAYER.standing.standing = false;
                           //std::cout<<"Fall";
                       }
                }
                

                if(platform_overlap && PLAYER.y > game.platforms[ui].y)
                {
                    //game.player.y = game.platforms[ui].y +1;
                    PLAYER.standing.standing_on_num = ui;
                    PLAYER.standing.standing = true;
                    //std::cout<< ui << "\n";
                }
                else if(platform_overlap)
                {
                    PLAYER.y = game.platforms[ui].y-PLAYER.current_sprite.height;
                    //jump = false;
                    
                }
                
                //draw player_sprite
                //buffer_draw_sprite(&buffer, playersprite, PLAYER.x, PLAYER.y+ground.height, rgb_to_uint32(player_colours[pi][0], player_colours[pi][1], player_colours[pi][2]));
                
                //Jumping effects//
                
                /*Jetpack Flames*/
                
                if(PLAYER.jump && PLAYER.air_time <= max_jump_time)
                {
                    
                    if(PLAYER.moving_left)
                    {
                        if(PLAYER.air_time < max_jump_time)
                        {
                            buffer_draw_sprite(&buffer, jetpack_flame, PLAYER.x+7, PLAYER.y-jetpack_flame.height+4, rgb_to_uint32(200, 30, 30));
                            buffer_draw_sprite(&buffer, jetpack_flame, PLAYER.x+17, PLAYER.y-jetpack_flame.height+4, rgb_to_uint32(200, 30, 30));
                        }
                        else
                        {
                            buffer_draw_sprite(&buffer, jetpack_flame, PLAYER.x+7, PLAYER.y-jetpack_flame.height+4, rgb_to_uint32(130, 60, 60));
                            buffer_draw_sprite(&buffer, jetpack_flame, PLAYER.x+17, PLAYER.y-jetpack_flame.height+4, rgb_to_uint32(130, 60, 60));
                            
                        }
                        
                    }
                    if(!PLAYER.moving_left)
                    {
                        if(PLAYER.air_time < max_jump_time)
                        {
                            buffer_draw_sprite(&buffer, jetpack_flame, PLAYER.x+2, PLAYER.y-jetpack_flame.height+4, rgb_to_uint32(200, 30, 30));
                            buffer_draw_sprite(&buffer, jetpack_flame, PLAYER.x+12, PLAYER.y-jetpack_flame.height+4, rgb_to_uint32(200, 30, 30));
                        }
                        else
                        {
                            buffer_draw_sprite(&buffer, jetpack_flame, PLAYER.x+2, PLAYER.y-jetpack_flame.height+4, rgb_to_uint32(130, 60, 60));
                            buffer_draw_sprite(&buffer, jetpack_flame, PLAYER.x+12, PLAYER.y-jetpack_flame.height+4, rgb_to_uint32(130, 60, 60));
                            
                        }
                    }
                    
                }
                
                if(!PLAYER.slide)
                {
                    PLAYER.slide_time -= 10;
                    if(PLAYER.slide_time < 0)
                        PLAYER.slide_time = 0;
                    //PLAYER.slide_time = 0;
                }
                
                
               /*
                Bullet Management
                -Bullet creation
                -Bullet destruction
                -Bullet collision
                */
                
                if(PLAYER.reload)
                {
                    //PLAYER.ammo = max_ammo;
                    PLAYER.reload = false;
                    PLAYER.reloading = true;
                    
                    
                }
                
                if(PLAYER.reloading)
                {
                    if(PLAYER.ammo != 0 && PLAYER.reload_time == MAX_RELOAD)
                    {
                        PLAYER.reloading = false;
               
                    }
                    else if(PLAYER.reload_time != 0)
                    {
                        ++PLAYER.ammo;
                        --PLAYER.reload_time;
                        if(PLAYER.ammo > max_ammo)
                            PLAYER.ammo = max_ammo;
                        if(PLAYER.moving_left)
                            PLAYER.current_sprite = player_sprite_left_reload;
                        else
                            PLAYER.current_sprite = player_sprite_right_reload;
                    }
                    else
                        PLAYER.reloading = false;
                    
                }
                if(!PLAYER.reloading)
                    ++PLAYER.reload_time;
                    if(PLAYER.reload_time > MAX_RELOAD)
                        PLAYER.reload_time = MAX_RELOAD;
                
                
                for(size_t bi = 0; bi < game.num_bullets; ++bi)
                {
                    const Bullet& bullet = game.bullets[bi];
                    const Sprite& sprite = bullet_sprite;
                    buffer_draw_sprite(&buffer, sprite, bullet.x, bullet.y, rgb_to_uint32(128, 0, 0));
                }
                
                for(size_t bi = 0; bi < game.num_bullets; ++bi)
                {
                    game.bullets[bi].x += game.bullets[bi].dir;
                    if(game.bullets[bi].x >= game.width ||
                       game.bullets[bi].x < bullet_sprite.width)
                    {
                        game.bullets[bi] = game.bullets[game.num_bullets - 1];
                        --game.num_bullets;
                        continue;
                    }
                    
                    for(size_t pi = 0; pi < NUM_PLATFORMS; ++pi)
                    {
                        for(size_t bi = 0; bi < game.num_bullets; ++bi)
                        {
                            bool bullet_plat_overlap = sprite_overlap_check(bulletsprite, game.bullets[bi].x, game.bullets[bi].y, platform_sprite, game.platforms[pi].x, game.platforms[pi].y);
                            if(bullet_plat_overlap)
                            {
                                game.bullets[bi] = game.bullets[game.num_bullets - 1];
                                --game.num_bullets;
                            }
                        }
                        
                    }
                    bool bullet_player_overlap = sprite_overlap_check(bulletsprite, game.bullets[bi].x, game.bullets[bi].y, playersprite, PLAYER.x, PLAYER.y);
                    if(bullet_player_overlap && PLAYER.alive && !PLAYER.sliding)
                    {
                        game.bullets[bi] = game.bullets[game.num_bullets - 1];
                        --game.num_bullets;
                        game.player[pi].life = game.player[pi].life - 1;
                    }
                    
                    
                }
        //        if(shoot)t
        //            std::cout<< "shoot\n";
                
                if(PLAYER.shoot && game.num_bullets < GAME_MAX_BULLETS && PLAYER.ammo > 0 && !PLAYER.reloading && !PLAYER.sliding)
                {
                    if(PLAYER.moving_left)
                    {
                        game.bullets[game.num_bullets].x = PLAYER.x - 2;
                        game.bullets[game.num_bullets].y = PLAYER.y + 11;
                        game.bullets[game.num_bullets].dir = -bullet_speed;
                    }
                    else
                    {
                        game.bullets[game.num_bullets].x = PLAYER.x +player_sprite_left1.width + 2;
                        game.bullets[game.num_bullets].y = PLAYER.y + 11;
                        game.bullets[game.num_bullets].dir = bullet_speed;
                    }


                    ++game.num_bullets;
                    --PLAYER.ammo;
                    //std::cout<< PLAYER.ammo <<std::endl;
                }
                PLAYER.shoot = false;
                if(pi == 0)
                    press_shoot1 = false;
                else if(pi == 1)
                    press_shoot2 = false;

                if(PLAYER.idle_counter >= idle_activ_time && PLAYER.alive)
                {
                    PLAYER.idling = true;
                }
                else{
                    PLAYER.idling = false;
                }
                
                if(PLAYER.idling)
                {
                    if(PLAYER.moving_left)
                    {
                        ++player_idle_anim_left[pi].time;
                        if(player_idle_anim_left[pi].time == player_idle_anim_left[pi].num_frames*player_idle_anim_left[pi].frame_duration && player_idle_anim_left[pi].loop)
                        {
                            player_idle_anim_left[pi].time = 0;
                            PLAYER.idle_counter = idle_activ_time/1.5;
                            
                        }
                    
                        size_t current_frame = player_idle_anim_left[pi].time/player_idle_anim_left[pi].frame_duration;
                        buffer_draw_sprite(&buffer, *player_idle_anim_left[pi].frames[current_frame], PLAYER.x-1, PLAYER.y+ground.height, rgb_to_uint32(player_colours[pi][0], player_colours[pi][1], player_colours[pi][2])); //-1 x because sprite is one empty pixel larger to the left
                    }
                    else if(!PLAYER.moving_left)
                    {
                        ++player_idle_anim_right[pi].time;
                        if(player_idle_anim_right[pi].time == player_idle_anim_right[pi].num_frames*player_idle_anim_right[pi].frame_duration && player_idle_anim_right[pi].loop)
                        {
                            player_idle_anim_right[pi].time = 0;
                            PLAYER.idle_counter = idle_activ_time/1.5;
                            
                        }
                        
                        size_t current_frame = player_idle_anim_right[pi].time/player_idle_anim_right[pi].frame_duration;
                        buffer_draw_sprite(&buffer, *player_idle_anim_right[pi].frames[current_frame], PLAYER.x, PLAYER.y+ground.height, rgb_to_uint32(player_colours[pi][0], player_colours[pi][1], player_colours[pi][2]));
                    }
                }
                else if(PLAYER.alive)
                {
                    buffer_draw_sprite(&buffer, PLAYER.current_sprite, PLAYER.x, PLAYER.y+ground.height, rgb_to_uint32(player_colours[pi][0], player_colours[pi][1], player_colours[pi][2]));
                    
                }
                else if(!PLAYER.alive)
                {
                    --PLAYER.death_counter;
                    if(pi == 0)
                    {
                        int death_display_num = PLAYER.death_counter/60;
                        buffer_write_numbers(numbers, buffer, 20, buffer.height-45, death_display_num, rgb_to_uint32(player_colours[pi][0], player_colours[pi][1], player_colours[pi][2]));
                    }
                    else
                    {
                        int death_display_num = ceil(PLAYER.death_counter/60);
                        buffer_write_numbers(numbers, buffer, buffer.width - 20 - numbers[0].width, buffer.height-45, death_display_num, rgb_to_uint32(player_colours[pi][0], player_colours[pi][1], player_colours[pi][2]));
                    }
                    if(PLAYER.death_counter <= 0)
                    {
                        PLAYER = player_reset_dummy;
                        PLAYER.jump = false;
                        Player_Position player_pos = static_cast<Player_Position>(rngGen(0, POSITION_NUMS-1));
                        switch(player_pos)
                        {
                            case PLAYER_POSITION_A:
                                PLAYER.x = (buffer.width/2) - 5;
                                PLAYER.y = 20;
                                break;
                            case PLAYER_POSITION_B:
                                PLAYER.x = 120;
                                PLAYER.y = 165;
                                break;
                            case PLAYER_POSITION_C:
                                PLAYER.x = buffer.width - (buffer.width * 0.33);
                                PLAYER.y = 20;
                                break;
                            case PLAYER_POSITION_D:
                                PLAYER.x = buffer.width - (buffer.width * 0.33) + 50;
                                PLAYER.y = 220;
                                break;
                            case PLAYER_POSITION_E:
                                PLAYER.x = (buffer.width/2) - 5;
                                PLAYER.y = 165;
                                break;
                            case PLAYER_POSITION_F:
                                PLAYER.x = (buffer.width/2) - 5;
                                PLAYER.y = 200;
                                break;
                            case PLAYER_POSITION_G:
                                PLAYER.x = 120;
                                PLAYER.y = 20;
                                break;
                            case POSITION_NUMS:
                                PLAYER.x = 0;
                                PLAYER.y = 0;
                                break;
                            default:
                                PLAYER.x = 0;
                                PLAYER.y = 0;
                                break;
                                
                        }
                        PLAYER.death_counter = death_time;
                    }
                }
                /*else
                {
                    PLAYER.current_sprite = player_sprite_grave;
                    buffer_draw_sprite(&buffer, PLAYER.current_sprite, PLAYER.x, PLAYER.y+ground.height, rgb_to_uint32(player_colours[pi][0], player_colours[pi][1], player_colours[pi][2]));
                    buffer_draw_sprite(&buffer, grave_dirt, PLAYER.x+3, PLAYER.y+ground.height, rgb_to_uint32(130, 103, 40));
                }*/
                
                /*if(pi == 1)
                {
                    for(int gi = 0; gi < game.grave.size(); ++gi)
                    {
                        bool player1_grave_overlap = sprite_overlap_check(player_sprite_grave, game.grave[gi].x, game.grave[gi].y, PLAYER.current_sprite, PLAYER.x, PLAYER.y);
                        if(player1_grave_overlap)
                            buffer_draw_sprite(&buffer, game.player[0].current_sprite, game.player[0].x, game.player[0].y+ground.height, rgb_to_uint32(player_colours[0][0], player_colours[0][1], player_colours[0][2]));
                    }
                    
                }*/
                
                if(PLAYER.life < 0)
                    PLAYER.life = 0;
                
                
                
                if(PLAYER.life == 0 && PLAYER.alive)//&& PLAYER.alive)
                {
                    //PLAYER.state = PLAYER_STATE_DEAD;
                    PLAYER.alive = false;
                    PLAYER.ammo = 0;
                    game.grave.resize(game.grave.size()+1);
                    game.grave[game.grave.size()-1].player = static_cast<Player_Num>(pi);
                    game.grave[game.grave.size()-1].x = PLAYER.x;
                    game.grave[game.grave.size()-1].y = PLAYER.y+10;
                    game.grave[game.grave.size()-1].exist = true;
                    ++player_scores[!pi];
                    
                    
                    //game.grave[pi].exist = true;
                    //game.grave[pi].x = PLAYER.x;
                    //game.grave[pi].y = PLAYER.y;
                }
                
                /* Drawing The UI*/
                
                //Player 1
                if(pi == 0)
                {
                    if(PLAYER.ammo > 0)
                        buffer_draw_sprite(&buffer, UI_bullet_left, 20, buffer.height-30, rgb_to_uint32(player_colours[pi][0], player_colours[pi][1], player_colours[pi][2]));
                    if(PLAYER.ammo > 1)
                        buffer_draw_sprite(&buffer, UI_bullet_left, 25 + UI_bullet_left.width, buffer.height-30, rgb_to_uint32(player_colours[pi][0], player_colours[pi][1], player_colours[pi][2]));
                    if(PLAYER.ammo > 2)
                        buffer_draw_sprite(&buffer, UI_bullet_left, 40 + UI_bullet_left.width, buffer.height-30, rgb_to_uint32(player_colours[pi][0], player_colours[pi][1], player_colours[pi][2]));
                    
                    if(PLAYER.life > 0)
                        buffer_draw_sprite(&buffer, UI_heart, 20, buffer_height-50, rgb_to_uint32(150, 0, 0));
                    if(PLAYER.life > 1)
                        buffer_draw_sprite(&buffer, UI_heart, 22 + UI_heart.width, buffer_height-50, rgb_to_uint32(150, 0, 0));
                    if(PLAYER.life > 2)
                        buffer_draw_sprite(&buffer, UI_heart, 37 + UI_heart.width, buffer_height-50, rgb_to_uint32(150, 0, 0));
                    if(PLAYER.life > 3)
                        buffer_draw_sprite(&buffer, UI_heart, 52 + UI_heart.width, buffer_height-50, rgb_to_uint32(150, 0, 0));
                }
                    
                
                
                else if(pi == 1)
                {
                    if(PLAYER.ammo > 0)
                        buffer_draw_sprite(&buffer, UI_bullet_right, buffer.width - 20 - UI_bullet_left.width, buffer.height-30, rgb_to_uint32(player_colours[pi][0], player_colours[pi][1], player_colours[pi][2]));
                    if(PLAYER.ammo > 1)
                        buffer_draw_sprite(&buffer, UI_bullet_right, buffer.width - 25 - (UI_bullet_left.width*2), buffer.height-30, rgb_to_uint32(player_colours[pi][0], player_colours[pi][1], player_colours[pi][2]));
                    if(PLAYER.ammo > 2)
                        buffer_draw_sprite(&buffer, UI_bullet_right, buffer.width - 40 - (UI_bullet_left.width*2), buffer.height-30, rgb_to_uint32(player_colours[pi][0], player_colours[pi][1], player_colours[pi][2]));
                    
                    if(PLAYER.life > 0)
                        buffer_draw_sprite(&buffer, UI_heart, buffer_width -20 - UI_heart.width, buffer_height-50, rgb_to_uint32(150, 0, 0));
                    if(PLAYER.life > 1)
                        buffer_draw_sprite(&buffer, UI_heart, buffer_width -22 - (UI_heart.width*2), buffer_height-50, rgb_to_uint32(150, 0, 0));
                    if(PLAYER.life > 2)
                        buffer_draw_sprite(&buffer, UI_heart, buffer_width-37 - (UI_heart.width*2), buffer_height-50, rgb_to_uint32(150, 0, 0));
                    if(PLAYER.life > 3)
                        buffer_draw_sprite(&buffer, UI_heart, buffer_width-52 - (UI_heart.width*2), buffer_height-50, rgb_to_uint32(150, 0, 0));
                }

        }
            
        
        
        //buffer_draw_sprite(&buffer, UI_heart, buffer_width-40, buffer_height-80, rgb_to_uint32(150, 0, 0));
        
        if(!game.player[0].alive || !game.player[1].alive)
        {
            static int press_time = 0;
            if(reset)
            {
                if(press_time == reset_time)
                {
                    game = game_reset_dummy;
                    std::cout<< "Reset" <<std::endl;
                    press_time = 0;
                    
                }
                ++press_time;
            }
            
        }
        
        

//        if(shoot && !bullet_exist)
//        {
//
//            init_bullet = true;
//            //buffer_draw_sprite(&buffer, bullet_sprite, game.player.x-2, game.player.y+9, rgb_to_uint32(128, 30, 0));
//        }
//
//        if(init_bullet)
//        {
//            bullet_exist = true;
//            buffer_draw_sprite(&buffer, bullet_sprite, game.player.x-2, game.player.y+11, rgb_to_uint32(128, 30, 0));
//            init_bullet = false;
//
//            bullet_start_x = game.player.x-2;
//            bullet_start_y = game.player.x+11;
//
//        }
//
//        if(bullet_exist)
//        {
//            bullet_move -= 1;
//            int bullet_pos_x = bullet_start_x+bullet_move;
//            buffer_draw_sprite(&buffer, bullet_sprite, bullet_pos_x, bullet_start_y, rgb_to_uint32(128, 30, 0));
//
//            std::cout<< bullet_start_x+bullet_move << " : " << bullet_start_y <<std::endl;
//
//            if(bullet_start_x+bullet_move < 1 || bullet_start_x+bullet_move > buffer_width)
//            {
//                bullet_exist = false;
//            }
//        }

        
        /*if(moving && moving_left)
            std::cout<< "Moving left";*/
        
        //
        /*++alien_animation->time;
         if(alien_animation->time == alien_animation->num_frames * alien_animation->frame_duration)
         {
         if(alien_animation->loop)
         alien_animation->time = 0;
         else
         {
         delete alien_animation;
         alien_animation = nullptr;
         }
         }
         
         
         
         for(size_t i = 0; i < game.num_aliens; ++i)
         {
         const Alien& alien = game.aliens[i];
         for(size_t j = 0; j < game.num_bullets; ++j)
         {
         const Bullet& bullet = game.bullets[i];
         bool overlap = sprite_overlap_check(bulletsprite, bullet.x, bullet.y, alien_sprite, alien.x, alien.y);
         if(overlap)
         {
         game.aliens[i].type = ALIEN_STATE_DEAD;
         game.bullets[j] = game.bullets[game.num_bullets - 1];
         --game.num_bullets;
         //continue;
         
         }
         }
         }
         
         for(size_t ai = 0; ai < game.num_aliens; ++ai)
         {
         Sprite& aliensprite = alien_sprite;
         //if(true)//game.aliens[ai].type != ALIEN_STATE_DEAD)
         //{
         const Alien& alien = game.aliens[ai];
         size_t current_frame = alien_animation->time/alien_animation->frame_duration;
         aliensprite = *alien_animation->frames[current_frame];
         buffer_draw_sprite(&buffer, aliensprite, alien.x, alien.y, rgb_to_uint32(128, 0, 0));
         //}
         //else
         //{
         //    const Alien& alien = game.aliens[ai];
         //aliensprite = alien_sprite_death;
         //buffer_draw_sprite(&buffer, alien_sprite_death, alien.x, alien.y, rgb_to_uint32(128, 0, 0));
         //}
         
         }*/
        
        /*++dust_animation->time;
        if(dust_animation->time == dust_animation->num_frames*dust_animation->frame_duration && dust_animation->loop)
        {
            dust_animation->time = 0;
            
        }
        
        size_t current_dust_frame = dust_animation->time/dust_animation->frame_duration;
        buffer_draw_sprite(&buffer, *dust_animation->frames[current_dust_frame], game.player[0].x-3, game.player[0].y+4, rgb_to_uint32(40, 40, 20));*/
       
        buffer_write_letters(alphabet, buffer, buffer.width/2 - (score_text.size()*5)+score_text.size()+3 -2, buffer_height - 30, score_text, 3, rgb_to_uint32(0, 0, 0));
        
        for(int i = 0; i < 2; ++i)
        {
            if(player_scores[i] > 999)
                player_scores[i] = 999;
            
            if(player_scores[i] < 10)
                score_x_position[i] = 15;//12 + ((!i*6)/2);
            else if(player_scores[i] < 100)
                score_x_position[i] = 15 + (!i*3) - (i*3);
            else if(player_scores[i] < 1000)
                score_x_position[i] = 15 + (!i*6) - (i*6);
            
        }
        
        buffer_draw_sprite(&buffer, line_separator, buffer.width/2 - 1, 298, rgb_to_uint32(0, 0, 0));
        buffer_write_numbers(numbers, buffer, buffer.width/2 - score_x_position[0] -2, 300, player_scores[0], rgb_to_uint32(player_colours[0][0], player_colours[0][1], player_colours[0][2]));
        buffer_write_numbers(numbers, buffer, buffer.width/2 - 2 + score_x_position[1] -2, 300, player_scores[1], rgb_to_uint32(player_colours[1][0], player_colours[1][1], player_colours[1][2]));
        
        
        //std::cout<< "PLAYER1 SCORE " << player_scores[0] <<std::endl;
        //std::cout<< "PLAYER2 SCORE " << player_scores[1] <<std::endl;
        
        glTexSubImage2D(
                        GL_TEXTURE_2D, 0, 0, 0,
                        buffer.width, buffer.height,
                        GL_RGBA, GL_UNSIGNED_INT_8_8_8_8,
                        buffer.data
                        );
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glfwSwapBuffers(window);
        glfwPollEvents();
        //moving = false;
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    glDeleteVertexArrays(1, &fullscreen_triangle_vao);
    delete[] alien_sprite.data;
    delete[] custom_lol.data;
    delete[] space_invaders_title.data;
    delete[] player_sprite_left_reload.data;
    delete[] player_sprite_right_reload.data;
    //delete[] player_colours[0];
    //delete[] player_colours[1];
    delete[] player_sprite_left1.data;
    delete[] player_sprite_left2.data;
    //delete[] player_sprite_left3.data;
    delete[] player_sprite_right1.data;
    delete[] player_sprite_right2.data;
    delete[] platform_sprite.data;
    delete[] buffer.data;
    return 0;
}
