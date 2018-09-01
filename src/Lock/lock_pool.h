#ifndef LOCK_POOL_H_
#define LOCK_POOL_H_

#include <mutex>
#include <string>

using namespace std;

//负责feature mutex的统一分配
class lock_pool
{
public:
    lock_pool()
    {
        pBiasMutex = new mutex;
        pMutexArray = new mutex[lockNum];
    }
    
    inline mutex* get_feature_lock(const string& fea)
    {
        size_t index = strHash(fea) % lockNum;
        return &(pMutexArray[index]);
    }
    
    inline mutex* get_bias_lock()
    {
        return pBiasMutex;
    }

private:
    hash<string> strHash;
    mutex* pBiasMutex;
    mutex* pMutexArray;
    const int lockNum = 10009;
};



#endif /*LOCK_POOL_H_*/
