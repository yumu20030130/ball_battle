#include <stdio.h>
#include "../common/common.h"
#include <math.h>
#include <stdlib.h>

#define COLOR_BACKGROUND WHITE


#define RED	FB_COLOR(255,0,0)
#define LIGHT_RED FB_COLOR(252,157,154)
#define ORANGE	FB_COLOR(255,165,0)
#define YELLOW	FB_COLOR(255,255,0)
#define GREEN	FB_COLOR(0,255,0)
#define CYAN	FB_COLOR(0,127,255)
#define BLUE	FB_COLOR(0,0,255)
#define LIGHT_BLUE FB_COLOR(153,204,255)
#define PURPLE	FB_COLOR(139,0,255)
#define BLACK   FB_COLOR(0,0,0)
#define WHITE   FB_COLOR(0xff,0xff,0xff)
#define DARK_BLUE 	FB_COLOR(51,102,153)
#define LIGHT_PURPLE FB_COLOR(204, 204, 255)

#define MAX_NUM 50 //最大id号

/*星星相关*/
#define MAX_STAR_NUM 10
#define default_star_size 2
#define default_star_color BLACK

/*玩家相关*/
#define MAX_PLAYER 2
#define MAX_SPLIT 3
#define Player0_Id_Start (MAX_STAR_NUM + 1)
#define Player1_Id_Start (MAX_STAR_NUM + MAX_SPLIT + 1)
#define default_player_size 25
#define default_speed 15
#define default_player0_master_color BLUE
#define default_player0_other_color LIGHT_BLUE
#define default_player1_master_color RED
#define default_player1_other_color LIGHT_RED
#define my_player_id 0
#define other_player_id 1

/*球相关*/
typedef struct ball{
    int player;
    int x;
    int y;
    int size;
    int dx;
    int dy;
    int color;
} ball;

int is_kill(int i, int j);//判定i是否吃了j


/*消息相关*/
//client to server
#define Split 0
#define UpdateSpeed 1
#define SpeedUp 2
#define Ready 3
//server to client
#define Update 10
#define Win 11  //指示对方的输赢状态
#define Lose 12
#define Start 13
//消息统一结构
typedef struct msg{
    int type;
    int player;
    int id;
    int x;
    int y;
    int size;
    int color;
} msg;

int equal_msg(msg *a, msg *b);
void make_message(msg *message, int type, int player, int id, int x, int y, int size, int color);
void send_message(int bluetooth_fd, msg *message);


/*特殊技能相关*/
#define SPEED_UP_MAXTIME 50 //加速最长持续时间

/*显示界面相关*/
#define Split_X (SCREEN_WIDTH-80)
#define Split_Y (SCREEN_HEIGHT-80)
#define Split_Size 50
#define SpeedUp_X 80
#define SpeedUp_Y (SCREEN_HEIGHT-80)
#define SpeedUp_Size 50
#define Ready_X (SCREEN_WIDTH-80)
#define Ready_Y (SCREEN_HEIGHT-80)
#define Ready_Size 50

#define Button_Click_Color LIGHT_PURPLE
#define Button_Color DARK_BLUE
void lose_show();
void win_show();







