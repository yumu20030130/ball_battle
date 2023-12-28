#include "../common/common.h"
#include "param.h"
#define MOD_NUM 10
#define DISTANCE 10

#define Split 0
#define UpdateSpeed 1
#define SpeedUp 2
#define Ready 3
#define Update 10

#define Win 11
#define Lose 12

#define Start 13




typedef struct msg{
    int type;
    int player;
    int id;
    int x;
    int y;
    int size;
    int color;
} msg;


typedef struct ball{
    int player;
    int id;
    int x;
    int y;

    int dx;         //运动方向在水平和竖直方向上的分量
    int dy;

    int size;

    int color;
} ball;

void send_msg(ball myself,int type,int fd);
void receive_msg(msg *m,int fd);
void ball_clear(ball *m);
void ball_divide(int fd);

void ball_change(ball *m1,ball *m2);
void init_from_msg(ball *theball,msg *m);