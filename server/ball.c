#include "ball.h"


/*玩家相关*/
int ball_amount[MAX_PLAYER]; //各玩家球的总数


/*球相关*/
int id_used[MAX_NUM];
int player_balls[MAX_PLAYER][MAX_SPLIT];
ball ball_list[MAX_NUM];

int is_kill(int i, int j){ //判定i是否吃了j
    if (i == j){
        return 0;
    }
    int dx = ball_list[i].x - ball_list[j].x;
    int dy = ball_list[i].y - ball_list[j].y;
    int dis2 = dx * dx + dy * dy;
    if (ball_list[i].size > ball_list[j].size && ball_list[i].size * ball_list[i].size > dis2){
        return 1;
    }
    else{
        return 0;
    }
}
/*消息相关*/
msg old_message;

int equal_msg(msg *a, msg *b){
    if (a->type == b->type && a->player == b->player && a->id == b->id && a->x == b->x && a->y == b-> y && a->size == b->size && a->color == b->color){
        return 1;
    }
    return 0;
}
void make_message(msg *message, int type, int player, int id, int x, int y, int size, int color){
    message->type = type;
    message->player = player;
    message->id = id;
    message->x = x;
    message->y = y;
    message->size = size;
    message->color = color;
}

void send_message(int bluetooth_fd, msg *message){
    if (equal_msg(&old_message, message)){
        return;
    }
    make_message(&old_message, message->type, message->player, message->id, message->x, message->y, message->size, message->color);
    //printf("send message:%d %d %d %d %d %d %d\n", 
        //message->type, message->player, message->id, message->x, message->y, message->size,message->color);
    myWrite_nonblock(bluetooth_fd, message, sizeof(msg));
}

/*图相关*/
int mp[SCREEN_WIDTH][SCREEN_HEIGHT];

/*特殊技能相关*/
int speed_up[MAX_PLAYER]; //标记某个是否处于加速状态

/*显示界面相关*/
void lose_show(){
	fb_draw_rect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,BLACK);
	fb_draw_text(SCREEN_WIDTH*2/5, SCREEN_HEIGHT/2, "LOSE", 80, WHITE); 
	fb_update();
	exit(0);
	return;
}

void win_show(){
	fb_draw_rect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,ORANGE);
	fb_draw_text(SCREEN_WIDTH*3/7, SCREEN_HEIGHT/2, "WIN", 80, BLACK); 
	fb_update();
	exit(0);
	return;
}



