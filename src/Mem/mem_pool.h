#ifndef MEM_POOL_H_
#define MEM_POOL_H_


class mem_pool
{
public:
    static void* get_mem(size_t size)
    {
        if(size > blockSize) return NULL;
        if(NULL == pBegin || (char*)pBegin + size > pEnd)
        {
            pBegin = malloc(blockSize);
            pEnd = (char*)pBegin + blockSize;
        }
        void* res = pBegin;
        pBegin = (char*)pBegin + size;
        return res;
    }
private:
    mem_pool() {}
    static void* pBegin;
    static void* pEnd;
    static const size_t blockSize = 64 * 1024 * 1024;
};

void* mem_pool::pBegin = NULL;
void* mem_pool::pEnd = NULL;


#endif /*MEM_POOL_H_*/
