/*
 * File:   camera.c
 * Author: liuxx
 *
 * Created on 2012年7月21日, 下午9:04
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <pthread.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>

#include "camera.h"
#include "v4l2uvc.h"
#include "gui.h"
#include "utils.h"
#include "color.h"

int piancha = 0;
int xielv = 0;
int last_piancha = 0;
int last_xielv = 0;
int stop_flag = 0;
int final_stop_flag = 0;
#define row_num  10
#define col_num  160
#define yuzhi 35                    //ÈüµÀºÚ°×²îÖµ

unsigned char image[row_num][col_num];
//////////////////////ºÚÏßÌáÈ¡////////////////////
int center_line[3][row_num]; //ºÚÏßÖÐµã×ø±ê
int fixed_center_line[3][row_num + 2]; //ÐÞÕýºóµÄºÚÏßÖÐµã×ø±ê
int stop_line[10];
unsigned char fall_edge, rise_edge; //ºÚ°×œ»œÓ±êÖŸ
unsigned char left_edge, right_edge; //ºÚÏßÁœ¶Ë×ø±ê
unsigned char exact = 0; //ÕýÈ·ÕÒµœºÚÏß


unsigned int count = 0;
unsigned int director = 0;
unsigned int tiaobian_line_0_0 = 0;
unsigned int tiaobian_line_1_0 = 0;
unsigned int tiaobian_line_0_1 = 0;
unsigned int tiaobian_line_1_1 = 0;

void Get_stopline(void) {
    unsigned char i, j, count;
    int average = 0;
    for (i = 0; i < 10; i++) {
        stop_line[i] = 0;
    }


    for (j = 0, count = 0; j < col_num; j = j + 20, count++) {
        for (i = 1; i < row_num; i++) {
            if (fall_edge == 0 && (image[i - 1][j]>(image[i][j] + yuzhi))) {
                //ŽÓ°×µœºÚÕÒºÚÏßµÄ±ßÑØ£¬²ÉÓÃj-2¶ø²»ÊÇj-1ÊÇÎªÁËÌø¹ýºÚ°×²»Ã÷ÏÔµÄµã
                rise_edge = 1; //ÕÒµœÁËºÚÏßµÄÓÒ±ßÑØ
                left_edge = i; //ŒÇÏÂÓÒ±ßÑØµÄ×ø±ê
            }
            if (rise_edge && (image[i][j])>(image[i - 1][j] + yuzhi)) {
                fall_edge = 1; //ÕÒµœÁËºÚÏßµÄ×ó±ßÑØ
                right_edge = i; //ŒÇÏÂ×ó±ßÑØµÄ×ø±ê
            }

            if (fall_edge) { //Èç¹ûÏÈÕÒµœÁËÓÒ±ßÑØ£¬ºóÓÖÕÒµœÁË×ó±ßÑØ£¬¿ÉÄÜÎªºÚÏß

                if (left_edge - right_edge < 25 && left_edge < right_edge) { //¶ÔºÚÏßµÄ¿í¶ÈŒÓÉÏÏÞÖÆ

                    stop_line[count] = (left_edge + right_edge) / 2;
                    exact = 1;
                    fall_edge = 0;
                    rise_edge = 0; //ŒÌÐøÕÒÇ°Çå³ýËùÓÐÐÅºÅ
                    right_edge = 0;
                    left_edge = 0;
                    break;
                } else { //²»Âú×ã¿í¶ÈÌõŒþ£¬ŒÌÐøÍùÏÂÕÒ
                    exact = 0;
                    fall_edge = 0;
                    rise_edge = 0; //ŒÌÐøÕÒÇ°Çå³ýËùÓÐÐÅºÅ
                    right_edge = 0;
                    left_edge = 0;
                }
            }
        }
    }
    for (i = 0; i < 7; i++) {
        if (stop_line[i] != 0 && stop_line[i + 1] != 0)/////////这里不太科学，当i==7的时候。。。
        {
            stop_line[8] = i;
            break;
        }
        if (i == 6)stop_line[9] = 8;
    }
    for (i = stop_line[8]; i < 7; i++) {
        if (stop_line[i] == 0 && stop_line[i + 1] == 0) {
            stop_line[9] = i - 1;
            break;
        }
        if (i == 6)stop_line[9] = 7;
    }

    int begin = stop_line[8];
    int end = stop_line[9];

    if (end - begin > 1) {
        for (i = begin; i <= end; i++) {
            average += stop_line[i];
        }
        average = average / (end - begin + 1);
    }

    count = 0;
    stop_flag = average;
    for (i = begin; i <= end; i++) {
        if ((stop_line[i] - average) > 1)count++;
    }

    if (end - begin > 2 && count < 2) {
        for (i = begin; i <= end; i++) {
            average += stop_line[i];
        }
        average = average / (end - begin + 1);
        stop_flag = average;
    } else stop_flag = 0;


    if (stop_flag != 0)final_stop_flag = 1;
    /*
      for(i=begin;i<=end;i++){
        if(i==begin || i==end || stop_line[i+1]==0){
          stop_line[i]=stop_line[i];
        }
        else{
          stop_line[i]=lvbo(stop_line[i-1],stop_line[i+1],stop_line[i]);
        }
       }
     */




}

void Get_blackline(void) { //Œò»¯µÄÕÒºÚÏß³ÌÐò£šÃ¿Ò»ÐÐ¶Œ°ŽÕÕ±ßÑØŒì²â·šÌáÈ¡ºÚÏß£¬Ã»ÓÐ¿ŒÂÇºÚÏß
    unsigned char i, j; //µÄ±ä»¯Ç÷ÊÆ£¬Ò²Ã»ÓÐ¶ÔÔÓµãŽŠÀí£¬Ö»ËµÃ÷ÉÏÏÂÑØ·šÌáÈ¡ºÚÏßµÄËŒÏë£©
    unsigned char count_line;
    fall_edge = 0;
    rise_edge = 0;

    for (i = 0; i < 3; i++) {
        for (j = 0; j < row_num; j++) {
            center_line[i][j] = 0;
            fixed_center_line[i][j] = 0;
        }
    }



    for (i = 0; i < row_num; i++) {
        count_line = 0;
        for (j = col_num - 1; j > 2; j--) {
            if (fall_edge == 0 && (image[i][j]>(image[i][j - 2] + yuzhi))) {
                //ŽÓ°×µœºÚÕÒºÚÏßµÄ±ßÑØ£¬²ÉÓÃj-2¶ø²»ÊÇj-1ÊÇÎªÁËÌø¹ýºÚ°×²»Ã÷ÏÔµÄµã
                rise_edge = 1; //ÕÒµœÁËºÚÏßµÄÓÒ±ßÑØ
                left_edge = j; //ŒÇÏÂÓÒ±ßÑØµÄ×ø±ê
            }
            if (rise_edge && ((image[i][j] + yuzhi) < image[i][j - 2])) {
                fall_edge = 1; //ÕÒµœÁËºÚÏßµÄ×ó±ßÑØ
                right_edge = j - 2; //ŒÇÏÂ×ó±ßÑØµÄ×ø±ê
            }

            if (fall_edge) { //Èç¹ûÏÈÕÒµœÁËÓÒ±ßÑØ£¬ºóÓÖÕÒµœÁË×ó±ßÑØ£¬¿ÉÄÜÎªºÚÏß

                if (left_edge - right_edge < 25 && left_edge > right_edge) { //¶ÔºÚÏßµÄ¿í¶ÈŒÓÉÏÏÞÖÆ


                    center_line[count_line][i] = (right_edge >> 1)+(left_edge >> 1);
                    //×ó±ßÑØºÍÓÒ±ßÑØ×ø±êÈ¡ÆœŸùÖµŸÍÕÒµœÁËºÚÏßµÄÖÐµã×ø±ê


                    if (j > 30)
                        j = j - 30;
                    else j = 2;

                    exact = 1;
                    if (count_line == 2)break;
                    count_line++;
                    fall_edge = 0;
                    rise_edge = 0; //ŒÌÐøÕÒÇ°Çå³ýËùÓÐÐÅºÅ
                    right_edge = 0;
                    left_edge = 0;

                } else { //²»Âú×ã¿í¶ÈÌõŒþ£¬ŒÌÐøÍùÏÂÕÒ
                    exact = 0;
                    fall_edge = 0;
                    rise_edge = 0; //ŒÌÐøÕÒÇ°Çå³ýËùÓÐÐÅºÅ
                    right_edge = 0;
                    left_edge = 0;
                }
            }
        }

        if (exact == 0) center_line[count_line][i] = 0; //Ã»ÓÐÕÒµœºÚÏßµÄÐÐœøÐÐÇåÁãŽŠÀí

        exact = 0; //ÕÒÏÂÒ»ÐÐºÚÏßÇ°Çå³ýËùÓÐÐÅºÅ
        rise_edge = 0;
        fall_edge = 0;
        left_edge = 0;
        right_edge = 0;
    }
}

int lvbo(int a, int b, int c) {//ÖÐÖµÂË²šº¯Êý(¿ÉÒÔÓÐ¶àÖÖÐÎÊœ£©
    int max, min;
    if (a == 0 || b == 0) return c; //Èç¹ûÇ°ºó³öÏÖÁË0£¬ÈÏÎª
    max = a;
    min = b;
    if (a <= b) { //¶ÔÕâÁœžöÊý×öÅÅÐò
        min = a;
        max = b;
    }
    if (c >= min && c <= max) return c; //cÔÚa£¬bÖÐŒä£¬·µ»Øc

    else
        printf("llllllllllllllllllllllllllllllllllllllllllllll\n");
    return (max + min) >> 1; //·ñÔò·µ»ØabµÄÆœŸùÖµ
}

void Pic_lvbo() {
    unsigned char i, j;
    for (i = 1; i < row_num - 1; i++)
        for (j = 1; j < col_num - 1; j++) {
            if (image[i][j] > image[i][j - 1] + 30 && image[i][j] > image[i][j + 1] + 30)
                image[i][j] = (image[i][j - 1] + image[i][j + 1]) >> 1;
            if (image[i][j] < image[i][j - 1] - 30 && image[i][j] < image[i][j + 1] - 30)
                image[i][j] = (image[i][j - 1] + image[i][j + 1]) >> 1;
            if (image[i][j] > image[i - 1][j] + 30 && image[i][j] > image[i + 1][j] + 30)
                image[i][j] = (image[i][j - 1] + image[i][j + 1]) >> 1;
            if (image[i][j] < image[i - 1][j] - 30 && image[i][j] < image[i + 1][j] - 30)
                image[i][j] = (image[i][j - 1] + image[i][j + 1]) >> 1;
        }

}

void Modify_centerline(void) {
    unsigned char i, j;
    tiaobian_line_0_1 = 0;
    tiaobian_line_1_1 = 0;
    tiaobian_line_0_1 = 10;
    tiaobian_line_1_1 = 10;





    for (i = 1; i < row_num; i++) {
        if (center_line[0][i - 1] - center_line[0][i] > 50 &&
                center_line[0][i] < 100 &&
                center_line[0][i] > 10) {
            tiaobian_line_0_1 = i;
            break;
        } else if (i == row_num - 1)tiaobian_line_0_1 = row_num;
    }

    if (tiaobian_line_0_1 < row_num) {
        for (i = tiaobian_line_0_1; i < row_num; i++) {


            center_line[2][i] = center_line[1][i];
            center_line[1][i] = center_line[0][i];
            center_line[0][i] = 0;
        }
    }

    for (i = 1; i < row_num; i++) {
        if (center_line[1][i - 1] - center_line[1][i] > 40 &&
                center_line[1][i] < 100 &&
                center_line[1][i] > 10) {
            tiaobian_line_1_1 = i;
            break;
        } else tiaobian_line_1_1 = row_num;
    }
    if (tiaobian_line_1_1 < row_num) {
        for (i = tiaobian_line_1_1; i < row_num; i++) {

            center_line[2][i] = center_line[1][i];

            center_line[1][i] = 0;
        }
    }

    for (i = 1; i < row_num; i++) {
        if ((center_line[0][i] - center_line[0][i - 1]) > 50 &&
                center_line[0][i] > 125) {
            tiaobian_line_0_0 = i;
            break;
        } else tiaobian_line_0_0 = 0;
    }
    if (tiaobian_line_0_0 > 0) {
        for (i = 0; i < tiaobian_line_0_0; i++) {

            center_line[2][i] = center_line[1][i];
            center_line[1][i] = center_line[0][i];
            center_line[0][i] = 0;
        }
    }



    for (i = 0; i < 3; i++) {
        for (j = 0; j < row_num; j++) {

            fixed_center_line[i][j] = center_line[i][j];

        }
    }


    printf("tiaobian_line_0_0:%d\n", tiaobian_line_0_0);
    printf("tiaobian_line_0_1:%d\n", tiaobian_line_0_1);
    printf("tiaobian_line_1_0:%d\n", tiaobian_line_1_0);
    printf("tiaobian_line_1_1:%d\n", tiaobian_line_1_1);

}

void Modify_blackline(void) { //ÖÐÖµÂË²šÐÞÕýºÚÏßÖÐÐÄ×ø±ê
    unsigned char i, t;
    for (t = 0; t < 3; t++) {
        int begin = fixed_center_line[t][row_num];
        int end = fixed_center_line[t][row_num + 1];
        for (i = begin; i <= end; i++) {
            if (i == 0 || i == row_num - 1 || fixed_center_line[t][i + 1] == 0) {
                fixed_center_line[t][i] = fixed_center_line[t][i];
            } else {
                fixed_center_line[t][i] =
                        lvbo(center_line[t][i - 1], center_line[t][i + 1], center_line[t][i]);
            }
        }
    }
}

void Get_Info() {
    int t, i;
    int delta;
    for (t = 0; t < 3; t++) {
        for (i = 0; i < row_num; i++) {
            if (fixed_center_line[t][i] != 0) {
                fixed_center_line[t][row_num] = i;
                break;
            } else if (i == row_num - 1)fixed_center_line[t][row_num] = row_num - 1;
        }

        for (i = fixed_center_line[t][row_num]; i < row_num; i++) {
            if (fixed_center_line[t][i] == 0) {
                fixed_center_line[t][row_num + 1] = i - 1;
                break;
            } else if (i == row_num - 1)fixed_center_line[t][row_num + 1] = row_num - 1;
        }
    }

    int begin_0 = fixed_center_line[0][row_num];
    int end_0 = fixed_center_line[0][row_num + 1];
    int begin_1 = fixed_center_line[1][row_num];
    int end_1 = fixed_center_line[1][row_num + 1];
    int average_0 = 0;
    int average_1 = 0;
    /*
            for(i=begin_0;i<end_0;i++)
            {
            average_0+=fixed_center_line[0][i];
            }
            average_0=average_0/(end_0-begin_0+1)

            for(i=begin_1;i<end_1;i++)
            {
            average_1+=fixed_center_line[1][i];
            }
            average_1=average_1/(end_1-begin_1+1)
     */
    if (end_0 - begin_0 >= 2 && end_1 - begin_1 >= 2) {
        xielv = (fixed_center_line[0][end_0] - fixed_center_line[0][begin_0]) / (end_0 - begin_0)+(fixed_center_line[1][end_1] - fixed_center_line[1][begin_1]) / (end_1 - begin_1);
       // xielv = xielv / 2;
       // piancha = fixed_center_line[0][begin_0] / 2 + fixed_center_line[1][begin_1] / 2 - 80;
	 xielv = (fixed_center_line[0][end_0] - fixed_center_line[0][begin_0]) / (end_0 - begin_0);
        piancha = (fixed_center_line[0][begin_0] + fixed_center_line[0][begin_0 + 1]) / 2 - 120;

        last_xielv = xielv;
        last_piancha = piancha;
    }
    else if (end_0 - begin_0 >= 2) {
        xielv = (fixed_center_line[0][end_0] - fixed_center_line[0][begin_0]) / (end_0 - begin_0);
        piancha = (fixed_center_line[0][begin_0] + fixed_center_line[0][begin_0 + 1]) / 2 - 120;
        last_xielv = xielv;
        last_piancha = piancha;
    }
    else if (end_1 - begin_1 >= 2) {
        xielv = (fixed_center_line[1][end_1] - fixed_center_line[1][begin_1]) / (end_1 - begin_1);
        piancha = (fixed_center_line[1][begin_0] + fixed_center_line[1][begin_0 + 1]) / 2 - 40;
        last_xielv = xielv;
        last_piancha = piancha;
    } else {
        xielv = last_xielv;
        piancha = last_piancha;
    }
}

int Get_xielv() {
    return xielv;
}

int Get_piancha() {
    return piancha;
}

struct vdIn *videoIn;
int width = 320;
int height = 240;

int init_video() {
    const char *videodevice = NULL;
    //  int format = V4L2_PIX_FMT_MJPEG;
    int format = V4L2_PIX_FMT_YUYV;
    int i;
    int grabmethod = 1;

    int fps = 15;

    char *avifilename = NULL;
    if (videodevice == NULL || *videodevice == 0) {
        videodevice = "/dev/video0";
    }
    if (avifilename == NULL || *avifilename == 0) {
        avifilename = "video.avi";
    }
    videoIn = (struct vdIn *) calloc(1, sizeof (struct vdIn));

    if (init_videoIn
            (videoIn, (char *) videodevice, width, height, fps, format,
            grabmethod, avifilename) < 0) {
        return 0;
    }
    return 1;
}

int mainLoop() {
    if (!(videoIn->signalquit)) {
        return 0;
    }
    int ii, jj;
    int i = 0;
    int j = 0;

    for (ii = 0; ii < height; ii = ii + 24) {
        for (jj = 0; jj < width * 2; jj = jj + 4) {
            image[i][j] = videoIn->framebuffer[ii * width * 2 + jj];
            j++;
        }
        //printf("\n");
        //i++;
    }
    printf("end end end end end end end end end end\n");
    Pic_lvbo();
    Get_blackline();
    Get_stopline();
    Modify_centerline();
    Modify_blackline();
    Get_Info();

    for (i = 0; i < 3; i++) {
        printf("line%d\n", i);
        printf("%d\t", tiaobian_line_0_1);
        printf("%d\n", tiaobian_line_1_1);
        for (j = 0; j < row_num + 2; j++) {
            printf("%d\t", fixed_center_line[i][j]);
        }
        printf("\n");
    }
    printf("\n");
    printf("xielv:%d\n", xielv);
    printf("piancha:%d\n", piancha);
    printf("\n");
    printf("stop_line\n");
    for (i = 0; i < 10; i++)printf("%d\t", stop_line[i]);
    printf("\n");
    printf("final_stop_flag:%d\n", final_stop_flag);

    /*
            currtime = SDL_GetTicks();
            if (currtime - lasttime > 0) {
                frmrate = 1000 / (currtime - lasttime);
            }
            lasttime = currtime;
     */

    if (uvcGrab(videoIn) < 0) {
        printf("Error grabbing \n");
        return 0;
    }
    return 1;
    //  usleep(10000);
}

/*

int main(int argc, char** argv) {
    if (init_video() == 0) {
        printf("init error");
        return;
    }
    int i = 0;
    while (videoIn->signalquit) {
        mainLoop();
        i++;
        if (i > 2) {

        }
    }
    return (EXIT_SUCCESS);
}
 *
 */
