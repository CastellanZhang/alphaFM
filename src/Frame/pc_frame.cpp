#include "pc_frame.h"

bool pc_frame::init(pc_task& task, int t_num, int buf_size, int log_num)
{
    pTask = &task;
    threadNum = t_num;
    bufSize = buf_size;
    logNum = log_num;
    sem_init(&semPro, 0, 1);
    sem_init(&semCon, 0, 0);
    threadVec.clear();
    threadVec.push_back(thread(&pc_frame::pro_thread, this));
    for(int i = 0; i < threadNum; ++i)
    {
        threadVec.push_back(thread(&pc_frame::con_thread, this));
    }
    return true;
}


void pc_frame::run()
{
    for(int i = 0; i < threadVec.size(); ++i)
    {
        threadVec[i].join();
    }
}


void pc_frame::pro_thread()
{
    string line;
    size_t line_num = 0;
    int i = 0;
    bool finished_flag = false;
    while(true)
    {
        sem_wait(&semPro);
        bufMtx.lock();
        for(i = 0; i < bufSize; ++i)
        {
            if(!getline(cin, line))
            {
                finished_flag = true;
                break;
            }
            line_num++;
            buffer.push(line);
            if(line_num % logNum == 0)
            {
                cout << line_num << " lines finished" << endl;
            }
        }
        bufMtx.unlock();
        sem_post(&semCon);
        if(finished_flag)
        {
            break;
        }
    }
}


void pc_frame::con_thread()
{
    bool finished_flag = false;
    vector<string> input_vec;
    input_vec.reserve(bufSize);
    while(true)
    {
        input_vec.clear();
        sem_wait(&semCon);
        bufMtx.lock();
        for(int i = 0; i < bufSize; ++i)
        {
            if(buffer.empty())
            {
                finished_flag = true;
                break;
            }
            input_vec.push_back(buffer.front());
            buffer.pop();
        }
        bufMtx.unlock();
        sem_post(&semPro);
        pTask->run_task(input_vec);
        if(finished_flag) break;
    }
    sem_post(&semCon);
}

