#include "../common/common.h"
#include"ball.h"
#include "param.h"
#include<stdlib.h>
#include<stdbool.h>

ball myself;
ball myself_split1;
ball myself_split2;


ball player0;
ball player0_split1;
ball player0_split2;

int myself_split_num=0;

int player0_split_num=0;

const int player0_player=0;

bool hasSplit=false;

bool hasSpeed=false;

static int touch_fd;
static int bluetooth_fd;

static int speed_button_color=Button_Color;
static int split_button_color=Button_Color;


static int hasStart=0;


#define default_speed 100


int speednow=default_speed;

const int default_period=100;

int max_positive(int a,int b){
	return a>b?a:b;
}

int max_negative(int a,int b){
	if(a<0&&b<0){
		return a<b?a:b;
	}
	else if(a<0&&b>=0){
		return a;
	}
	else if(a>=0&&b<0){
		return b;
	}
	return -1;
}


int min(int a,int b){
	return a<b?a:b;
}

void Ilose(){

	fb_draw_rect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,BLACK);
	fb_draw_text(SCREEN_WIDTH*2/5, SCREEN_HEIGHT/2, "LOSE", 80, WHITE); 
	fb_update();
	exit(0);
	return;
}

void Iwin(){
	fb_draw_rect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,ORANGE);
	fb_draw_text(SCREEN_WIDTH*3/7, SCREEN_HEIGHT/2, "WIN", 80, BLACK); 
	fb_update();
	exit(0);
	return;

}

static void touch_event_cb(int fd)
{
	printf("触摸事件\n");
	int type,x,y,finger;
	type = touch_read(fd, &x,&y,&finger);
        if(finger!=0){
       	     return;
   	}
	printf("触摸事件111\n");
	if(hasStart==0){
		fb_draw_circle(Start_X,Start_Y,Start_Size,RED);
		fb_draw_text(Start_X-32, Start_Y+4, "Ready", 24, COLOR_TEXT); 
		fb_update();

		printf("发送ready消息\n");
		send_msg(myself,Ready,bluetooth_fd); 
		
		return;
	}


	switch(type){
	case TOUCH_PRESS:

        if((x>=Split_X-Split_Size)&&(x<Split_X+Split_Size)&&(y>=Split_Y-Split_Size)&&(y<Split_Y+Split_Size)) {    //触碰到分裂按钮
			split_button_color=Button_Click_Color;
			send_msg(myself,Split,bluetooth_fd);    //发送分裂消息
			break;
		}
		else if((x>=SpeedUp_X-SpeedUp_Size)&&(x<SpeedUp_X+SpeedUp_Size)&&(y>=SpeedUp_Y-SpeedUp_Size)&&(y<SpeedUp_Y+SpeedUp_Size)) {    //触碰到加速按钮
			speed_button_color=Button_Click_Color;
			send_msg(myself,SpeedUp,bluetooth_fd);
			break;
		}

        else{         //正常移动
		
	    	printf("发送UpdateSpeed指令\n");
            myself.dx=(x-myself.x);
            myself.dy=(y-myself.y);

            send_msg(myself,UpdateSpeed,bluetooth_fd);
            break;
        }

	case TOUCH_MOVE:
		myself.dx=(x-myself.x);
        myself.dy=(y-myself.y);

        send_msg(myself,UpdateSpeed,bluetooth_fd);
		break;

    case TOUCH_RELEASE:
		if((x>=Split_X-Split_Size)&&(x<Split_X+Split_Size)&&(y>=Split_Y-Split_Size)&&(y<Split_Y+Split_Size)) {    //触碰到分裂按钮
			split_button_color=Button_Color;
		}
		else if((x>=SpeedUp_X-SpeedUp_Size)&&(x<SpeedUp_X+SpeedUp_Size)&&(y>=SpeedUp_Y-SpeedUp_Size)&&(y<SpeedUp_Y+SpeedUp_Size)) {    //触碰到加速按钮
			speed_button_color=Button_Color;
		}
		break;


	case TOUCH_ERROR:
		printf("close touch fd\n");
		close(fd);
		task_delete_file(fd);
		break;
	default:
		return;
	}
	fb_update();
	return;
}



static void bluetooth_tty_event_cb(int fd)
{
    msg m;
    receive_msg(&m,fd);

	if(m.type==Start){
		hasStart=1;
		fb_draw_rect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,COLOR_BACKGROUND);
		fb_update();
		return;
	}

	else if(m.type==Win){

		printf("收到了win消息\n");
		Iwin();
		return;
	}
	else if(m.type==Lose){
		printf("收到了lose消息\n");
		Ilose();
		return;
	}

    else if(m.type==Update){
		// printf("收到Update消息\n");
        if(m.player==myself.player){
			if(m.id==myself.id){           //本体球
				if(m.size==0){         //本体被吃了，交换到分裂的球
					if(myself_split_num==1){         //剩下的一定是split1
						ball_change(&myself,&myself_split1);
						ball_clear(&myself_split1);
						fb_update;
						myself_split_num-=1;

					}
					else if(myself_split_num==2){
						ball_change(&myself,&myself_split1);
						ball_change(&myself_split1,&myself_split2);
						ball_clear(&myself_split2);
						fb_update;
						myself_split_num-=1;
					}
					
				}
				else{    //收到更新己方位置的消息
					if(myself.size>0){
						printf("update myself\n");
						fb_draw_circle(myself.x, myself.y, myself.size,COLOR_BACKGROUND);
					}
					
					myself.x=m.x;
					myself.y=m.y;
					myself.color=m.color;
					myself.size=m.size;
					fb_draw_circle(m.x, m.y, m.size, m.color);
				}
			}
			
			else{          //分裂球
				if(m.size==0){        //分裂的球被吃了
					if(myself_split_num>0){
						myself_split_num-=1;
						if(myself_split_num==1){
							if(m.id==myself_split1.id){       //split1被吃掉，2变成1
								fb_draw_circle(myself_split1.x, myself_split1.y, myself_split1.size, COLOR_BACKGROUND);
								ball_change(&myself_split1,&myself_split2);
								ball_clear(&myself_split2);
								fb_update();
							}
							else{
								fb_draw_circle(myself_split2.x, myself_split2.y, myself_split2.size, COLOR_BACKGROUND);
								ball_clear(&myself_split2);
								fb_update();
							}
						}
						else if(myself_split_num==0){
							fb_draw_circle(myself_split1.x, myself_split1.y, myself_split1.size, COLOR_BACKGROUND);
							ball_clear(&myself_split1);
							fb_update();
						}
					}
				}
				else{
					if(myself_split_num==0&&myself_split1.size==0){    //分裂第一个
						init_from_msg(&myself_split1,&m);
						fb_draw_circle(m.x, m.y, m.size, m.color);
						myself_split_num+=1;
					}
					else if(myself_split_num==1&&myself_split2.size==0&&m.id!=myself_split1.id){  //分裂第二个
						init_from_msg(&myself_split2,&m);
						fb_draw_circle(m.x, m.y, m.size, m.color);
						myself_split_num+=1;
					}

					else if(myself_split_num>0&&m.id==myself_split1.id){
						fb_draw_circle(myself_split1.x, myself_split1.y, myself_split1.size, COLOR_BACKGROUND);
						myself_split1.x=m.x;
						myself_split1.y=m.y;
						myself_split1.size=m.size;
						fb_draw_circle(myself_split1.x, myself_split1.y, myself_split1.size, myself_split1.color);
					}
					else if(myself_split_num==2&&m.id==myself_split2.id){
						fb_draw_circle(myself_split2.x, myself_split2.y, myself_split2.size, COLOR_BACKGROUND);
						myself_split2.x=m.x;
						myself_split2.y=m.y;
						myself_split2.size=m.size;
						fb_draw_circle(myself_split2.x, myself_split2.y, myself_split2.size, myself_split2.color);
					}
				}
					
				
			}
			
        }

		else if(m.player==player0_player){
			if(m.id==player0.id){
				if(m.size==0){
					if(player0_split_num==1){         //剩下的一定是split1
						ball_change(&player0,&player0_split2);
						ball_clear(&player0_split2);
						fb_update;
						player0_split_num-=1;
						
					}
					else if(player0_split_num==2){
						ball_change(&player0,&player0_split1);
						ball_change(&player0_split1,&player0_split2);
						ball_clear(&player0_split2);
						fb_update;
						player0_split_num-=1;
					}
				}
				else{    //更新位置信息
					if(player0.size>0){
						fb_draw_circle(player0.x, player0.y, player0.size,COLOR_BACKGROUND);
					}
					player0.x=m.x;
					player0.y=m.y;
					player0.color=m.color;
					player0.size=m.size;
					fb_draw_circle(m.x, m.y, m.size, m.color);
				}
			}

			else{
				if(m.size==0){        //分裂的球被吃了
					if(player0_split_num>0){
						player0_split_num-=1;
						if(player0_split_num==1){
							if(m.id==player0_split1.id){       //split1被吃掉，2变成1
								fb_draw_circle(player0_split1.x, player0_split1.y, player0_split1.size, COLOR_BACKGROUND);
								ball_change(&player0_split1,&player0_split2);
								ball_clear(&player0_split2);
								fb_update();
							}
							else{
								fb_draw_circle(player0_split2.x, player0_split2.y, player0_split2.size, COLOR_BACKGROUND);
								ball_clear(&player0_split2);
								fb_update();
							}
						}
						else if(player0_split_num==0){
							fb_draw_circle(player0_split1.x, player0_split1.y, player0_split1.size, COLOR_BACKGROUND);
							ball_clear(&player0_split1);
							fb_update();
						}
					}
				}
				else if(player0_split_num==0&&player0_split1.size==0){    //分裂第一个
					init_from_msg(&player0_split1,&m);
					fb_draw_circle(m.x, m.y, m.size, m.color);
					player0_split_num+=1;
				}
				else if(player0_split_num==1&&player0_split2.size==0&&m.id!=player0_split1.id){  //分裂第二个
					init_from_msg(&player0_split2,&m);
					fb_draw_circle(m.x, m.y, m.size, m.color);
					player0_split_num+=1;
				}

				else if(player0_split_num>0&&m.id==player0_split1.id){
					fb_draw_circle(player0_split1.x, player0_split1.y, player0_split1.size, COLOR_BACKGROUND);
					player0_split1.x=m.x;
					player0_split1.y=m.y;
					player0_split1.size=m.size;
					fb_draw_circle(player0_split1.x, player0_split1.y, player0_split1.size, player0_split1.color);
				}
				else if(player0_split_num==2&&m.id==player0_split2.id){
					fb_draw_circle(player0_split2.x, player0_split2.y, player0_split2.size, COLOR_BACKGROUND);
					player0_split2.x=m.x;
					player0_split2.y=m.y;
					player0_split2.size=m.size;
					fb_draw_circle(player0_split2.x, player0_split2.y, player0_split2.size, player0_split2.color);
				}
			}


		}
        else{    //更新位置信息
            fb_draw_circle(m.x, m.y, m.size, m.color);
        }
    }
    fb_update();
    return;
}


void init_myself(){
	myself.dx=0;
	myself.dy=0;
	myself.player=1;
	myself.id=Player1_Id_Start;
	myself.size=0;
	myself.color=RED;

	myself_split1.size=0;
	myself_split2.size=0;

	return;
}


void init_player0(){
	player0.dx=0;
	player0.dy=0;
	player0.player=0;
	player0.id=Player0_Id_Start;
	player0.size=0;
	player0.color=BLUE;

	player0_split1.size=0;
	player0_split2.size=0;
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

static void timer_cb(int period) /*该函数每period毫秒执行一次*/
{
	if(hasStart==1){
		fb_draw_circle(myself.x,myself.y,myself.size,myself.color);

		fb_draw_circle(Split_X,Split_Y,Split_Size,split_button_color);
		fb_draw_text(Split_X-32, Split_Y+4, "divide", 24, COLOR_TEXT);        //分裂按钮

		//摇杆(貌似叫这个)
		// fb_draw_ring(BasicPoint_X,BasicPoint_Y,Basic_BigR,Basic_SmallR,YELLOW,BLACK);

		//加速功能，以后需要可以添加
		fb_draw_circle(SpeedUp_X,SpeedUp_Y,SpeedUp_Size,speed_button_color);   //加速按钮
		fb_draw_text(SpeedUp_X-25, SpeedUp_Y+4, "Speed", 24, COLOR_TEXT);

		fb_update();
	}
	return;
}


int main(){

	fb_init("/dev/fb0");
	font_init("/root/lab-2022-st/out/font.ttc");

	fb_image *img;
	img = fb_read_jpeg_image("/root/lab-2022-st/out/333.jpg");

	fb_draw_image(0,0,img,0);
	fb_update();	

	// fb_draw_circle(Split_X,Split_Y,Split_Size,split_button_color);
    // fb_draw_text(Split_X-32, Split_Y+4, "divide", 24, COLOR_TEXT);        //分裂按钮
	// fb_update();

	//摇杆(貌似叫这个)
	// fb_draw_ring(BasicPoint_X,BasicPoint_Y,Basic_BigR,Basic_SmallR,YELLOW,BLACK);
	// fb_update();

	//加速功能，以后需要可以添加
	// fb_draw_circle(SpeedUp_X,SpeedUp_Y,SpeedUp_Size,speed_button_color);   //加速按钮
	// fb_draw_text(SpeedUp_X-25, SpeedUp_Y+4, "Speed", 24, COLOR_TEXT);
	// fb_update();

    init_myself();
	init_player0();
	
	bluetooth_fd = bluetooth_tty_init("/dev/rfcomm0");
	if(bluetooth_fd==-1){
		return 0;
	}

	//打开多点触摸设备文件, 返回文件fd
	touch_fd = touch_init("/dev/input/event1");
	
	//添加任务, 当touch_fd文件可读时, 会自动调用touch_event_cb函数
	task_add_file(touch_fd, touch_event_cb);
	
    task_add_file(bluetooth_fd, bluetooth_tty_event_cb);

	task_add_timer(default_period,timer_cb);

	printf("111\n");
    
	task_loop(); //进入任务循环


    return 0;
}


