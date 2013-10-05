/* 
 * File:   main.cpp
 * Author: allen
 *
 * Created on 2012年7月21日, 下午3:26
 */

#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include<iostream>
#include<cmath>
#include"camera/camera.h"

using namespace std;

pthread_mutex_t camera_mutex;
pthread_mutex_t posture_mutex;
pthread_t camera_tid;
pthread_t posture_tid;
pthread_t ctrl_tid;
const double gyr_pi = 3.14159265;
float camera_info[2]={0.0f,0.0f};
float posture_info[3]={0.0f,0.0f,0.0f};

int camera_flag = 1;
int posture_flag = 1;
int ctrl_flag = 1;

void * cameraThread(void *pa) {
    float cameraInfo[2];
      printf("cameraThread cost us\n");
    //   int cycles = 10;
    while (camera_flag == 1) {
        struct timeval timeStart;
        gettimeofday(&timeStart, NULL);
        camera_flag = mainLoop();

        cameraInfo[0] = Get_xielv();
        cameraInfo[1] = Get_piancha();
//        cout << "cameraInfo[0] : " << cameraInfo[0] << endl;
 //       cout << "cameraInfo[1] : " << cameraInfo[1] << endl;
        //setBioJointDeg(0,20);

	cameraInfo[0]=cameraInfo[0]+1;

        if (cameraInfo[1] >= 0) //开平方是为了拟合偏差之，开平方后的数值差不多就是偏差值（厘米）
        {
            cameraInfo[1] = sqrt(abs(cameraInfo[1]));
        } else {
            cameraInfo[1] = -sqrt(abs(cameraInfo[1]));
        }

	

        // usleep(30000);
        struct timeval timeEnd;
        gettimeofday(&timeEnd, NULL);
        long timeCost = 1000000 * (timeEnd.tv_sec - timeStart.tv_sec)+(timeEnd.tv_usec - timeStart.tv_usec);
        long timeCostms = timeCost / 1000;
        printf("cameraThread cost %ld ms %ld us\n", timeCostms, timeCost);
        /*
         *以下操作camera交互数据
         *
         */
        pthread_mutex_lock(&camera_mutex);
        camera_info[0] = cameraInfo[0];
        camera_info[1] = cameraInfo[1];
        pthread_mutex_unlock(&camera_mutex);
    }
}

void * postureThread(void *pa) {

    
}

void *ctrlThread(void* pa) {

}

void initRoboard() {

     if (init_video() != 1) {
        printf("[video] init  Error !!\n");
        camera_flag = 0;
    }
    usleep(50000);
}


void runMultiplyThread(void) {
    int temp;
    memset(&camera_tid, 0, sizeof (pthread_t));
    memset(&posture_tid, 0, sizeof (pthread_t));
    memset(&ctrl_tid, 0, sizeof (pthread_t));
    /*创建线程*/
    if ((temp = pthread_create(&camera_tid,
            NULL,
            cameraThread,
            NULL)) != 0) //comment2
    {
        printf("线程camera创建失败!\n");
    } else {
        printf("线程camera被创建\n");
    }
	//陀螺仪暂时不用
 //   if ((temp = pthread_create(&posture_tid,
 //           NULL,
  //          postureThread,
 //           NULL)) != 0) //comment2
//    {
 //       printf("线程posture创建失败!\n");
  //  } else {
  //      printf("线程posture被创建\n");
 //   }
 
    if ((temp = pthread_create(&ctrl_tid,
            NULL,
            ctrlThread,
            NULL)) != 0) //comment2
    {
        printf("线程control创建失败!\n");
    } else {
        printf("线程control被创建\n");
    }

    if (camera_tid != 0) { //comment4
        pthread_join(camera_tid, NULL);
        printf("线程camera已经结束\n");
    }
 //   if (posture_tid != 0) { //comment5
  //      pthread_join(posture_tid, NULL);
  //      printf("线程posture已经结束\n");
  //  }
    if (ctrl_tid != 0) { //comment5
        pthread_join(ctrl_tid, NULL);
        printf("线程ctrl_tid已经结束\n");
    }
}

int main() {
    /*用默认属性初始化互斥锁*/
    pthread_mutex_init(&camera_mutex, NULL);
    pthread_mutex_init(&posture_mutex, NULL);
initRoboard();
    runMultiplyThread();


    return 0;
}
