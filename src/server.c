//服务端代码

#include "func.h"

#define COLOR_BACKGROUND	FB_COLOR(255,255,255)

static int touch_fd;
static ball_info ball,*p_ball;
static status_info status,*p_status;
static position board,*p_board;
static int t[128];
//蓝牙连接队友的板子
static position op_board, * op_p_board;
//蓝牙fd
static int bluetooth_fd1;
static int bluetooth_fd2;

static void touch_event_cb(int fd)
{
	int type,x,y,finger;
	
	//printf("enter touch_event\n");
	type = touch_read(fd, &x,&y,&finger);
	switch(type){
	case TOUCH_PRESS:
		printf("TOUCH_PRESS：x=%d,y=%d,finger=%d\n",x,y,finger);
		if(!status.start && x>= 437 && x<587 && y>=270 && y <=330){
				status.start = 1;
				fb_draw_rect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,COLOR_BACKGROUND);
    			//画出边框
				fb_draw_rect(0,0,SCREEN_WIDTH,10,FB_COLOR(255,215,0));
				fb_draw_rect(0,0,10,SCREEN_HEIGHT,FB_COLOR(255,215,0));
				fb_draw_rect(1014,0,10,SCREEN_HEIGHT,FB_COLOR(255,215,0));
				//fb_draw_rect(437,270,150,60,COLOR_BACKGROUND);
				init_ball(p_ball);
				init_board(p_board);

				t[0] = status.start;
				t[1] = p_board->x;
				t[2] = p_ball->x;
				t[3] = p_ball->dx;
				t[4] = p_ball->dy;
				myWrite_nonblock(bluetooth_fd2, t, 5* sizeof(int));

		}
		else{
			fb_draw_rect(p_board->x-board_len/2,board_height,board_len,10,FB_COLOR(255,255,255)); //清除上次的
			if(x<=62) x=62;
			else if (x + 60> SCREEN_WIDTH) x = 962;
			fb_draw_rect(x-board_len/2,board_height,board_len,10,FB_COLOR(30,144,255));
			p_board->x = x;
			t[0] = 2;
			t[1] = p_board->x;
			myWrite_nonblock(bluetooth_fd2, t, 2 * sizeof(int));
		}
		break;
	case TOUCH_MOVE:
		printf("TOUCH_MOVE：x=%d,y=%d,finger=%d\n",x,y,finger);
		fb_draw_rect(p_board->x-board_len/2,board_height,board_len,10,FB_COLOR(255,255,255)); //清除上次的
		if(x<=62) x=62;
		else if (x + 60> SCREEN_WIDTH) x = 962;
		fb_draw_rect(x-board_len/2,board_height,board_len,10,FB_COLOR(30,144,255));
		p_board->x = x;

		t[0] = 2;
		t[1] = p_board->x;
		myWrite_nonblock(bluetooth_fd2, t, 2 * sizeof(int));
		break;
	case TOUCH_RELEASE:
		printf("TOUCH_RELEASE：x=%d,y=%d,finger=%d\n",x,y,finger);
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

static void move_event_cb(int fd){
		if(status.start){ 
			fb_draw_circle(p_ball->x,p_ball->y,p_ball->radius,FB_COLOR(255,255,255));
			printf("moving\n");
			move(p_ball,p_status,p_board,op_p_board);
			fb_draw_circle(p_ball->x,p_ball->y,p_ball->radius,FB_COLOR(128,128,128));

			if(p_status->collision == 1){
				t[0] = 3;
				t[1] = p_board->x;
				t[2] = p_ball->x;
				t[3] = p_ball->y;
				t[4] = p_ball->dx;
				t[5] = p_ball->dy;
				t[6]= p_ball->max_s;
				myWrite_nonblock(bluetooth_fd2, t, 7* sizeof(int));
			}	



			if(!status.start){
				fb_draw_border(437,270,150,60,FB_COLOR(0,0,0));
				//fb_draw_rect(462,270,100,60,FB_COLOR(255,255,255));
				fb_draw_text(445,305,"重新开始",32,FB_COLOR(0,0,0));
				t[0] = 0;
				myWrite_nonblock(bluetooth_fd2, t,  sizeof(int));
			}
			fb_update();
		}
}

//蓝牙读取
static void bluetooth_tty_event_cb(int fd)
{
	int t[128];
	int n;

	n = myRead_nonblock(fd, t, sizeof(int));
	if (n <= 0) {
		printf("close bluetooth tty fd\n");
		//task_delete_file(fd);
		//close(fd);
		exit(0);
		return;
	}
	else {
		fb_draw_rect(op_p_board->x-board_len/2, board_height, board_len, 10, FB_COLOR(255, 255, 255)); //清除上次的
		fb_draw_rect(t[0]-board_len/2, board_height, board_len, 10, FB_COLOR(254, 67, 101)); //画出客户端玩家的木板
		op_p_board->x = t[0];
	}
	
	return;
}

//打开蓝牙文件
static int bluetooth_tty_init(const char* dev)
{
	int fd = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK); /*非阻塞模式*/
	if (fd < 0) {
		printf("bluetooth_tty_init open %s error(%d): %s\n", dev, errno, strerror(errno));
		return -1;
	}
	return fd;
}

int main(int argc, char *argv[])
{
	fb_init("/dev/fb0");
	font_init("./font.ttc");
	status.start = 0;
	status.count = 0;
	status.collision =0;
	p_ball = &ball;
	p_status = &status;
	p_board =&board;
	op_p_board = &op_board;

	fb_draw_rect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,COLOR_BACKGROUND);
    //画出边框
	//fb_draw_rect(0,0,SCREEN_WIDTH,10,FB_COLOR(255,255,255));
	//fb_draw_rect(0,0,10,SCREEN_HEIGHT,FB_COLOR(255,255,255));
	//fb_draw_rect(1014,0,10,SCREEN_HEIGHT,FB_COLOR(255,255,255));

	//开始游戏界面
	fb_draw_border(437,270,150,60,FB_COLOR(0,0,0));
	fb_draw_text(445,305,"开始游戏",32,FB_COLOR(0,0,0));
	fb_update();
	//打开多点触摸设备文件, 返回文件fd
	touch_fd = touch_init("/dev/input/event2");
	//添加任务, 当touch_fd文件可读时, 会自动调用touch_event_cb函数
	printf("%d\n",touch_fd);
	task_add_file(touch_fd, touch_event_cb);

	//蓝牙连接
	//读，读取客户端写入的信息
	bluetooth_fd1 = bluetooth_tty_init("/dev/rfcomm0");
	if (bluetooth_fd1 == -1) return 0;
	task_add_file(bluetooth_fd1, bluetooth_tty_event_cb);
	//写，向客户端写入信息
	bluetooth_fd2 = bluetooth_tty_init("/dev/rfcomm1");
	if (bluetooth_fd2 == -1) return 0;


	 task_add_timer(100,move_event_cb);

	task_loop(); //进入任务循环
	return 0;
}
