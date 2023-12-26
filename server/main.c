#include <stdio.h>
#include "ball.h"

#define COLOR_TEXT			FB_COLOR(0x0,0x0,0x0)

extern int player_balls[MAX_PLAYER][MAX_SPLIT];
extern ball ball_list[MAX_NUM];
extern msg old_message;
extern int mp[SCREEN_WIDTH][SCREEN_HEIGHT];
extern int speed_up[MAX_PLAYER];
extern int ball_amount[MAX_PLAYER];

static int touch_fd;
static int bluetooth_fd;
static int speed_button_color = Button_Color;
static int split_button_color = Button_Color;
static int had_start = 0;
static int ready[MAX_PLAYER] = {0};
static int ready_sum = 0; //已经准备好的玩家数
const static int default_period = 100;
const static int GLOBAL_UPDATE_PERIOD = 10; 

static int min(int a, int b){
  return (a > b ? b: a);
}
static int max(int a, int b){
  return (a > b ? a: b);
}
static void make_split(int player){
	if (ball_amount[player] >= MAX_SPLIT){
		printf("ball split fail!\n");
		return;
	}
	msg message;
    printf("player%d start split!\n", player);

	int old_id = player_balls[player][ball_amount[player] - 1];
	int new_id = old_id + 1;
	fb_draw_circle(ball_list[old_id].x, ball_list[old_id].y, ball_list[old_id].size, COLOR_BACKGROUND);
	// make_message(&message, Update, ball_list[old_id].player, old_id, ball_list[old_id].x,
	// 		ball_list[old_id].y, ball_list[old_id].size, ball_list[old_id].color);
	// send_message(bluetooth_fd, &message);
	ball_list[new_id] = ball_list[old_id];
	ball_list[old_id].size = ball_list[old_id].size / 3 * 2;
	ball_list[new_id].size -= ball_list[old_id].size;
	ball_list[new_id].size = max(2, ball_list[new_id].size);

	int dx = ball_list[old_id].dx, dy = ball_list[old_id].dy;
	ball_list[old_id].dx = ball_list[old_id].dx * 3 / 2;
	ball_list[old_id].dy = ball_list[old_id].dy * 3 / 2;
	ball_list[new_id].dx = ball_list[new_id].dx * 3;
	ball_list[new_id].dy = ball_list[new_id].dy * 3;

	int dis = round(pow(dx * dx + dy * dy, 0.5)); 
	if (dis != 0){
		ball_list[new_id].x = 2 * ball_list[old_id].size * dx / dis + ball_list[old_id].x;
		ball_list[new_id].y = 2 * ball_list[old_id].size * dy / dis + ball_list[old_id].y;
	}
	else{
		ball_list[new_id].x = ball_list[old_id].x;
		ball_list[new_id].y = ball_list[old_id].y - ball_list[old_id].size - ball_list[new_id].size;	
	}
	ball_list[new_id].x = min(max(ball_list[new_id].x, ball_list[new_id].size), SCREEN_WIDTH - 1 - ball_list[new_id].size);
	ball_list[new_id].y = min(max(ball_list[new_id].y, ball_list[new_id].size), SCREEN_HEIGHT - 1 - ball_list[new_id].size);
	if (player == 0){
		ball_list[new_id].color = default_player0_other_color;
	}
	else{
		ball_list[new_id].color = default_player1_other_color;
	}
	ball_amount[player]++;
	printf("player%d has %d balls now!\n", player, ball_amount[player]);
}
static void make_speedup(int player){
	if (speed_up[player] == 0){
		speed_up[player] = 1;
		for (int i = 0; i < MAX_SPLIT; i++){
			int id = player_balls[player][i];
			ball_list[id].dx *= 2;
			ball_list[id].dy *= 2;
		}
	}
}
static void make_updatespeed(int player, int dx, int dy){
	for (int i = 0; i < ball_amount[player]; i++){
		int id = player_balls[player][i];
		if(dx>=0){
			ball_list[id].dx = max(round(1.0 * dx / sqrt(dx * dx + dy * dy) * default_speed / ball_list[id].size), 1);
		}
		else{
			ball_list[id].dx = min(round(1.0 * dx / sqrt(dx * dx + dy * dy) * default_speed / ball_list[id].size), -1);
		}
		if(dy>=0){
			ball_list[id].dy = max(round(1.0 * dy / sqrt(dx * dx + dy * dy) * default_speed / ball_list[id].size), 1);
		}
		else{
			ball_list[id].dy = min(round(1.0 * dy / sqrt(dx * dx + dy * dy) * default_speed / ball_list[id].size), -1);
		}
	}
}
static void generate_star(int id, int max_x, int max_y){
	msg message;
	int x, y, size, new_id;
	do{
		x = rand() % max_x;
		y = rand() % max_y;
		size = (rand() % 3 + 1) * default_star_size;
		x = min(max(size, x), max_x - 1 - size);
		y = min(max(size, y), max_y - 1 - size);
		new_id = id;
	}while (mp[x][y] == 1);
	mp[x][y] = 1;
	ball_list[new_id].x = x;
	ball_list[new_id].y = y;
	ball_list[new_id].size = size;
	ball_list[new_id].color = default_star_color;
	ball_list[new_id].player = -1;
	ball_list[new_id].dx = ball_list[new_id].dy = 0;
	fb_draw_circle(ball_list[new_id].x, ball_list[new_id].y, ball_list[new_id].size, ball_list[new_id].color);
	make_message(&message, Update, ball_list[new_id].player, new_id, ball_list[new_id].x,
			ball_list[new_id].y, ball_list[new_id].size, ball_list[new_id].color);
	send_message(bluetooth_fd, &message);
}
static void init_map(int max_x, int max_y){
	printf("start init map!\n");
	msg message;
	for (int i = 1; i <= MAX_STAR_NUM; i++){
		generate_star(i, max_x, max_y);
	}
	fb_update();
	printf("init map success!\n");
}

static void init_player(){
	printf("start init player!\n");
	for (int i = 0; i < MAX_PLAYER; i++){
		for (int j = 0; j < MAX_SPLIT; j++){
			player_balls[i][j] = Player0_Id_Start + i * MAX_SPLIT + j;
		}
	}
	msg message;
	printf("start init player0!\n");
	ball_amount[0] = 1;
	int new_id = Player0_Id_Start;
	player_balls[0][0] = new_id;
	ball_list[new_id].x = SCREEN_WIDTH / 4;
	ball_list[new_id].y = SCREEN_HEIGHT / 2;
	ball_list[new_id].size = default_player_size;
	ball_list[new_id].color = default_player0_master_color;
	ball_list[new_id].player = 0;
	ball_list[new_id].dx = ball_list[new_id].dy = 0;
	printf("start init player1!\n");
	ball_amount[1] = 1;
	new_id = Player1_Id_Start;
	player_balls[1][0] = new_id;
	ball_list[new_id].x = SCREEN_WIDTH / 4 * 3;
	ball_list[new_id].y = SCREEN_HEIGHT / 2;
	ball_list[new_id].size = default_player_size;
	ball_list[new_id].color = default_player1_master_color;
	ball_list[new_id].player = 1;
	ball_list[new_id].dx = ball_list[new_id].dy = 0;
	printf("success init player!\n");
}

static void update_pos(){//根据各球速度更新位置信息
	msg message;
	for (int k = 0; k < MAX_PLAYER; k++){
		for (int i = 0; i < ball_amount[k]; i++){
			int id = player_balls[k][i];
			// if (ball_list[id].dx == 0 && ball_list[id].dy == 0){ //静止不需要更新
			// 	continue;
			// }
			fb_draw_circle(ball_list[id].x, ball_list[id].y, ball_list[id].size, COLOR_BACKGROUND); //该步无需传消息，让客户端自己处理即可
			ball_list[id].x += ball_list[id].dx;
			ball_list[id].y += ball_list[id].dy;
			//防止越界
			ball_list[id].x = min(max(ball_list[id].x, ball_list[id].size), SCREEN_WIDTH - 1 - ball_list[id].size);
			ball_list[id].y = min(max(ball_list[id].y, ball_list[id].size), SCREEN_HEIGHT - 1 - ball_list[id].size);
			fb_draw_circle(ball_list[id].x, ball_list[id].y, ball_list[id].size, ball_list[id].color);
			make_message(&message, Update, ball_list[id].player, id, ball_list[id].x,
					ball_list[id].y, ball_list[id].size, ball_list[id].color);
			send_message(bluetooth_fd, &message);
		}
	}
	fb_update();
}

static void judge_kill(){ //判定是否有kill事件
	msg message;
	for (int player = 0; player < MAX_PLAYER; player++){
		for (int i = 0; i < ball_amount[player]; i++){
			int id = player_balls[player][i];
			for (int j = 1; j <= MAX_STAR_NUM; j++){ //遍历所有星星
				if (is_kill(id, j)){
					ball_list[id].size += ball_list[j].size;
					fb_draw_circle(ball_list[j].x, ball_list[j].y, ball_list[j].size, COLOR_BACKGROUND);
					fb_draw_circle(ball_list[id].x, ball_list[id].y, ball_list[id].size, ball_list[id].color);
					make_message(&message, Update, ball_list[j].player, j, ball_list[j].x,
							ball_list[j].y, ball_list[j].size, COLOR_BACKGROUND);
					send_message(bluetooth_fd, &message);
					make_message(&message, Update, ball_list[id].player, id, ball_list[id].x,
							ball_list[id].y, ball_list[id].size, ball_list[id].color);
					send_message(bluetooth_fd, &message);
					printf("%d kill %d\n", id, j);
					//回收被吃的j节点
					mp[ball_list[j].x][ball_list[j].y] = 0;
					generate_star(j, SCREEN_WIDTH, SCREEN_HEIGHT); //重新随机生成一个星星

				}
			}
			for (int other_player = 0; other_player < MAX_PLAYER; other_player++){ //遍历其他选手
				if (player == other_player){
					continue;
				}
				for (int k = 0; k < ball_amount[other_player];){//遍历该选手的所有球
					int j = player_balls[other_player][k];
					if (is_kill(id, j)){
						ball_list[id].size += ball_list[j].size;
						fb_draw_circle(ball_list[j].x, ball_list[j].y, ball_list[j].size, COLOR_BACKGROUND);
						fb_draw_circle(ball_list[id].x, ball_list[id].y, ball_list[id].size, ball_list[id].color);
						make_message(&message, Update, ball_list[j].player, j, ball_list[j].x,
								ball_list[j].y, 0, COLOR_BACKGROUND); //半径为0表示该球死亡
						send_message(bluetooth_fd, &message);
						make_message(&message, Update, ball_list[id].player, id, ball_list[id].x,
								ball_list[id].y, ball_list[id].size, ball_list[id].color);
						send_message(bluetooth_fd, &message);
						printf("%d kill %d\n", id, j);
						for (int cur = j + 1; cur <= MAX_STAR_NUM + other_player * MAX_SPLIT + ball_amount[other_player]; cur++){
							ball_list[cur - 1] = ball_list[cur];
						}
						ball_amount[other_player]--;
					}
					else{
						k++;
					}
				}
			}
		}
	}
	ball_list[Player0_Id_Start].color = default_player0_master_color;
	ball_list[Player1_Id_Start].color = default_player1_master_color;
	fb_update();
}
void judge_win(){
	msg message;
	if (ball_amount[0] == 0){
		make_message(&message, Win, 1, 0, 0, 0, 0, 0);
		send_message(bluetooth_fd, &message);
		lose_show();
	}
	else if(ball_amount[1] == 0){
		make_message(&message, Lose, 1, 0, 0, 0, 0, 0);
		send_message(bluetooth_fd, &message);
		win_show();
	}
}
static void touch_event_cb(int fd) //读取触摸屏输入的回调函数，需要更改
{
	int type,x,y,finger;
	type = touch_read(fd, &x,&y,&finger);
	if (finger != 0){
		return;
	}
	if (had_start == 0){
		if (ready[0] == 0){
			printf("check!\n");
			ready_sum++;
			fb_draw_circle(Ready_X,Ready_Y,Ready_Size, Button_Color);
			fb_draw_text(Ready_X-32, Ready_Y+4, "Ready", 24, COLOR_TEXT); 
			fb_update();
		}
		ready[0] = 1;
		return;
	}
	switch(type){
	case TOUCH_PRESS:
		printf("type=%d,x=%d,y=%d,finger=%d\n",type,x,y,finger);
        if((x>=Split_X-Split_Size)&&(x<Split_X+Split_Size)&&(y>=Split_Y-Split_Size)&&(y<Split_Y+Split_Size)) {    //触碰到分裂按钮
			split_button_color = Button_Click_Color;
			make_split(my_player_id);
		}
		else if((x>=SpeedUp_X-SpeedUp_Size)&&(x<SpeedUp_X+SpeedUp_Size)&&(y>=SpeedUp_Y-SpeedUp_Size)&&(y<SpeedUp_Y+SpeedUp_Size)) {    //触碰到加速按钮
			speed_button_color = Button_Click_Color;
			make_speedup(my_player_id);
		}
		else{
			int id = player_balls[my_player_id][0];
			int dx = x - ball_list[id].x; 
			int dy = y - ball_list[id].y;
			make_updatespeed(my_player_id, dx, dy);
		}
		break;
	case TOUCH_RELEASE:
		if((x>=Split_X-Split_Size)&&(x<Split_X+Split_Size)&&(y>=Split_Y-Split_Size)&&(y<Split_Y+Split_Size)) {    //触碰到分裂按钮
			split_button_color = Button_Color;
		}
		else if((x>=SpeedUp_X-SpeedUp_Size)&&(x<SpeedUp_X+SpeedUp_Size)&&(y>=SpeedUp_Y-SpeedUp_Size)&&(y<SpeedUp_Y+SpeedUp_Size)) {    //触碰到加速按钮
			speed_button_color = Button_Color;
		}
		break;
	case TOUCH_ERROR:
		printf("close touch fd\n");
		task_delete_file(fd);
		close(fd);
		break;
	default:
		return;
	}
	fb_update();
	return;
}


static void bluetooth_tty_event_cb(int fd) //蓝牙信息读取的回调函数，需要修改内容
{
	msg data;
	int n;
	n = myRead_nonblock(fd, &data, sizeof(data)); //非阻塞式读
	printf("receive message:%d %d %d %d %d %d %d\n", 
		data.type, data.player, data.id, data.x, data.y, data.size, data.color);
	if(n <= 0) {
		printf("close bluetooth tty fd\n");
		exit(0);
		return;
	}
	if (data.type == Ready){
		if (ready[data.player] == 0){
			ready_sum++;
		}
		ready[data.player] = 1;
	}
	else if (data.type == Split){
		make_split(data.player);
	}
	else if (data.type == SpeedUp){
		make_speedup(data.player);
	}
	else if (data.type == UpdateSpeed){
		int player = data.player;
		int dx = data.x;
		int dy = data.y;
		make_updatespeed(player, dx, dy);
	}
	fb_update();
}

static int bluetooth_tty_init(const char *dev)
{
	int fd = open(dev, O_RDWR|O_NOCTTY|O_NONBLOCK); /*非阻塞模式*/
	if(fd < 0){
		printf("bluetooth_tty_init open %s error(%d): %s\n", dev, errno, strerror(errno));
		return -1;
	}
	return fd;
}

static int cnt = 0;
static void timer_cb(int period)
{
	if (had_start == 0){
		if (ready_sum == MAX_PLAYER){
			msg message;
			fb_draw_rect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,COLOR_BACKGROUND);
			make_message(&message, Start, 1, 0, 0, 0, 0, 0);
			send_message(bluetooth_fd, &message);
			init_map(SCREEN_WIDTH, SCREEN_HEIGHT);
			init_player();
			had_start = 1;
		}
		return;
	}
	update_pos();
	judge_kill();
	judge_win();

	/*全局更新计数器*/
	cnt++;
	if (cnt == GLOBAL_UPDATE_PERIOD){
		msg message;
		fb_draw_rect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,COLOR_BACKGROUND);
		for (int i = 1; i <= MAX_STAR_NUM; i++){
			fb_draw_circle(ball_list[i].x, ball_list[i].y, ball_list[i].size, ball_list[i].color);
		}
		for (int i = 0; i < MAX_PLAYER; i++){
			for (int j = 0; j < ball_amount[i]; j++){
				int id = player_balls[i][j];
				fb_draw_circle(ball_list[id].x, ball_list[id].y, ball_list[id].size, ball_list[id].color);
			}
		}
		cnt = 0;
	}

	/*加速计数器*/
	for (int i = 0; i < MAX_PLAYER; i++){
		if (speed_up[i]){
			speed_up[i]++;
			if (speed_up[i] == SPEED_UP_MAXTIME){
				speed_up[i] = 0;
				for (int j = 0; j < MAX_SPLIT; j++){
					int id = player_balls[i][j];
					ball_list[id].dx /= 2;
					ball_list[id].dy /= 2;
				}
			}
		}
	}

	/*刷新交互界面*/
	//分裂按钮
	fb_draw_circle(Split_X,Split_Y,Split_Size,split_button_color);
	fb_draw_text(Split_X-32, Split_Y+4, "Divide", 24, COLOR_TEXT);   

	//加速按钮
	fb_draw_circle(SpeedUp_X,SpeedUp_Y,SpeedUp_Size,speed_button_color);  
	fb_draw_text(SpeedUp_X-25, SpeedUp_Y+4, "Speed", 24, COLOR_TEXT);
	fb_update();
	return;
}

int main(int argc, char *argv[])
{
	fb_init("/dev/fb0");
	font_init("./font.ttc");
	fb_image *img;
	img = fb_read_jpeg_image("/root/lab-2022-st/out/start.jpg");
	fb_draw_image(0,0,img,0);
	fb_update();
	
	//对应的文件描述符绑定给对应的回调函数
	touch_fd = touch_init("/dev/input/event2");
	task_add_file(touch_fd, touch_event_cb);
	bluetooth_fd = bluetooth_tty_init("/dev/rfcomm0");
	if(bluetooth_fd == -1) return 0;

	task_add_file(bluetooth_fd, bluetooth_tty_event_cb); //bluetooth_tty_event_cb指定蓝牙信息读取位置

	task_add_timer(default_period, timer_cb); /*增加0.5秒的定时器及对应的回调函数*/

	task_loop();
	return 0;
}
