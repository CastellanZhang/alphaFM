#include "pc_frame.h"
#include "test_task.h"

int main()
{
    test_task task;
    pc_frame frame;
    frame.init(task, 2, 5, 5);
    frame.run();
    return 0;
}

