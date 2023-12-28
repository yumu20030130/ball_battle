#include"ball.h"


extern ball myself;
extern ball myself_split1;
extern ball myself_split2;


void send_msg(ball myself,int type,int fd){
    printf("发送消息:");
    //发送消息
    msg m;
    m.type=type;
    m.player=myself.player;
    m.id=myself.id;
    m.x=myself.dx;
    m.y=myself.dy;
    m.size=myself.size;
    m.color=myself.color;
    
    printf("%d %d %d %d %d %d %d\n", m.type, m.player, m.id, m.x, m.y, m.size,m.color);
    // char buf[128];
    // sprintf(buf, "%d%d%d%d%d%d%d", m.type, m.player, m.id, m.x, m.y, m.size,m.color);
    // printf("%s\n",buf);
    myWrite_nonblock(fd,&m,sizeof(msg));
    return;
}




void receive_msg(msg *m,int fd){
    // char buf[128];     //存放接收到的消息
	int n;

	// n = myRead_nonblock(fd, buf, sizeof(buf)-1);

    n=myRead_nonblock(fd,m,sizeof(msg));
	if(n <= 0) {
		exit(0);
		return;
	}
	// buf[n] = '\0';
	// printf("bluetooth tty receive \"%s\"\n", buf);

    printf("收到更新消息:%d %d %d %d %d %d %d\n", m->type, m->player, m->id, m->x, m->y, m->size,m->color);
	return;

}


void ball_clear(ball *m){
    m->x=0;
    m->y=0;
    m->dx=0;
    m->dy=0;
    m->size=0;
    m->id=0;
    m->color=COLOR_BACKGROUND;
}


void ball_change(ball *m1,ball *m2){
    fb_draw_circle(m1->x,m1->y,m1->size,COLOR_BACKGROUND);
    m1->x=m2->x;
    m1->y=m2->y;
    m1->dx=m2->dx;
    m1->dy=m2->dy;
    m1->size=m2->size;
    m1->color=m2->color;

    fb_draw_circle(m1->x,m1->y,m1->size,m1->color);
    return;
}

void init_from_msg(ball *theball,msg *m){
    theball->player=m->player;
    theball->id=m->id;
    theball->x=m->x;
    theball->y=m->y;
    theball->size=m->size;
    theball->color=m->color;
}


void ball_divide(int fd){
    fb_draw_circle(myself.x,myself.y,myself.size,COLOR_BACKGROUND);
    send_msg(myself,Split,fd);
    return;
}
