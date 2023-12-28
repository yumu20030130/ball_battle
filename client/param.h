#define TIME_X	(SCREEN_WIDTH-160)
#define TIME_Y	0
#define TIME_W	100
#define TIME_H	30

#define SEND_X	(SCREEN_WIDTH-80)
#define SEND_Y	20
#define SEND_W	60
#define SEND_H	60


#define Split_X (SCREEN_WIDTH-80)
#define Split_Y (SCREEN_HEIGHT-80)
#define Split_Size 50


#define SpeedUp_X 80
#define SpeedUp_Y (SCREEN_HEIGHT-80)
#define SpeedUp_Size 50

#define Start_X (SCREEN_WIDTH-80)
#define Start_Y (SCREEN_HEIGHT-80)
#define Start_Size 50



#define BasicPoint_X 100
#define BasicPoint_Y (SCREEN_HEIGHT-100)
#define Basic_BigR 90
#define Basic_SmallR 20


#define COLOR_BACKGROUND FB_COLOR(0xff,0xff,0xff)
#define COLOR_TEXT FB_COLOR(0x0,0x0,0x0)


#define RED	FB_COLOR(255,0,0)
#define ORANGE	FB_COLOR(255,165,0)
#define YELLOW	FB_COLOR(255,255,0)
#define GREEN	FB_COLOR(0,255,0)
#define CYAN	FB_COLOR(0,127,255)
#define BLUE	FB_COLOR(0,0,255)
#define PURPLE	FB_COLOR(139,0,255)
#define WHITE   FB_COLOR(255,255,255)
#define BLACK   FB_COLOR(0,0,0)


#define MAX_NUM 50
#define MAX_STAR_NUM 10
#define MAX_PLAYER 2
#define MAX_SPLIT 3

#define Player0_Id_Start (MAX_STAR_NUM + 1)
#define Player1_Id_Start (MAX_STAR_NUM + MAX_SPLIT + 1)

#define default_star_size 2
#define default_player_size 20
#define default_star_color BLACK
#define default_player_color BLUE


#define Button_Click_Color ORANGE

#define Button_Color CYAN