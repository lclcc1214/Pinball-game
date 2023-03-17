#include <stdio.h>
#include "../common/common.h"
#include <math.h>
#define MAX_SPEED 100
const int board_len = 100;
const int board_height = SCREEN_HEIGHT-25;

typedef struct{
    int x,y,s,dx,dy,radius;
    int max_s;
}ball_info;

typedef struct{
	int start;
    int count;
    int collision;
} status_info;

typedef struct{
    int x;
} position;

void init_ball(ball_info *p){
    srand((unsigned int)time(0));
    p-> radius = 10;
    p->x = rand()%984+20;
    p->y = 20;
    p->max_s = 30;
    p->dx = rand()%29+1;
    printf("%d\n",p->dx);
    p->dy = p->max_s - p->dx;
   //p->s = 4;
    fb_draw_circle(p->x,p->y,p->radius,FB_COLOR(128,128,128));
    return;
}

void move(ball_info *p,status_info *ps,position *pb,position *p_op){
    ps->collision = 0;
    printf("x:%d,y:%d\n",p->x,p->y);
    if(p->max_s <= MAX_SPEED){
        if(ps->count / 5 > 0){
            p->max_s +=1;
            ps->count = 0;
        }
    }
    //偏移W
    p->x += p->dx;
    p->y += p->dy;
    //碰撞右边界
    if(p->x+p->radius>= SCREEN_WIDTH-15){
        ps->collision = 1;
        p->dx = -p->dx;
        p->x = 2*(SCREEN_WIDTH - 15-p->radius) -p->x;
        ps->count +=1;
    }
    //碰撞左边界
    else if (p->x - p->radius < 10){
        ps->collision = 1;
        p->dx = -p->dx;
        p->x = 2*(10+p->radius) - p->x;
        ps->count +=1;
    }
    //碰撞上边界
    else if(p->y - p->radius < 10){
        ps->collision = 1;
        p->dy = - p->dy;
        p->y = 2*(10+p->radius) - p->y;
        ps->count +=1;
    }
     else{
        //碰撞挡板
        if(p->y + p->radius >= board_height-2 && (p->x >= pb->x - board_len/2 && p->x < pb->x + board_len/2) ){
            ps->collision = 1;
            //左半边
            if(p->x < pb->x-25 ){
                p->dx -= 10;
                if(fabs(p->dx) >= p->max_s){
                     p->dx = -(p->max_s - 4);
                }
                p->dy = - fabs(p->max_s - fabs(p->dx));   
            }
            //右半边
            else if (p->x > pb->x + 25){
                 p->dx += 10;
                 if(fabs(p->dx) >= p->max_s){
                     p->dx = (p->max_s - 4);
                }
                p->dy = - fabs(p->max_s - fabs(p->dx));
            } 
            else{
                p->dy = -(p->max_s-fabs(p->dx));
            }
            p->y = 2*(board_height-2-p->radius) - p->y;
            
            ps->count += 1;
        }
        else  if(p->y + p->radius >= board_height-2 && (p->x >= p_op->x - board_len/2 && p->x < p_op->x + board_len/2) ){
            ps->collision = 1;
            //左半边
            if(p->x < p_op->x-25 ){
                p->dx -= 10;
                if(fabs(p->dx) >= p->max_s){
                     p->dx = -(p->max_s - 4);
                }
                p->dy = - fabs(p->max_s - fabs(p->dx));   
            }
            //右半边
            else if (p->x > p_op->x + 25){
                 p->dx += 10;
                 if(fabs(p->dx) >= p->max_s){
                     p->dx = (p->max_s - 4);
                }
                p->dy = - fabs(p->max_s - fabs(p->dx));
            } 
            else{
                p->dy = -(p->max_s-fabs(p->dx));
            }
            p->y = 2*(board_height-2-p->radius) - p->y;
            
            ps->count += 1;
        }

        //触底
        else if(p->y + p->radius >SCREEN_HEIGHT-10){
            ps->start = 0;
            printf("end\n");
         }

        
    
    }
}
   

void init_board(position *p){
    p->x = rand()%904 + 60;
    fb_draw_rect(p->x-board_len/2,board_height,board_len,10,FB_COLOR(30,144,255));
    return;
}

