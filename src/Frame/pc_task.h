#ifndef PC_TASK_H
#define PC_TASK_H

#include <string>
#include <vector>

using std::string;
using std::vector;

class pc_task
{
public:
    pc_task(){}
    virtual void run_task(vector<string>& dataBuffer) = 0;
};


#endif //PC_TASK_H
