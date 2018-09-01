#ifndef PC_FRAME_H
#define PC_FRAME_H

#include <string>
#include <vector>
#include <utility>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <semaphore.h>
#include <iostream>
#include "pc_task.h"

using namespace std;

class pc_frame
{
public:
    pc_frame(){}
    bool init(pc_task& task, int t_num, int buf_size = 5000, int log_num = 200000);
    void run();

private:
    int threadNum;
    pc_task* pTask;
    mutex bufMtx;
    sem_t semPro, semCon;
    queue<string> buffer;
    vector<thread> threadVec;
    int bufSize;
    int logNum;
    void pro_thread();
    void con_thread();
};


#endif //PC_FRAME_H
