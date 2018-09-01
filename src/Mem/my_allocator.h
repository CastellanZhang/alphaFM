#ifndef MY_ALLOCATOR_H_
#define MY_ALLOCATOR_H_

#include <unordered_map>
#include <string>
#include <cstring>
#include <iostream>
//#include <ext/pool_allocator.h>

using namespace std;

//U is double or float
template<typename T, typename U, template<typename> class MODEL_UNIT>
class my_allocator : public allocator<T>
{
public:
    typedef allocator<T> base_type;
    typedef typename allocator<T>::size_type size_type;
    typedef typename allocator<T>::pointer pointer;

    template<typename Other>
    struct rebind
    {
        typedef my_allocator<Other, U, MODEL_UNIT> other;
    };

    my_allocator() {}

    my_allocator(my_allocator<T, U, MODEL_UNIT> const&) {}

    my_allocator<T, U, MODEL_UNIT>& operator=(my_allocator<T, U, MODEL_UNIT> const&) { return (*this); }

    template<typename Other>
    my_allocator(my_allocator<Other, U, MODEL_UNIT> const&) {}

    template<typename Other>
    my_allocator<T, U, MODEL_UNIT>& operator=(my_allocator<Other, U, MODEL_UNIT> const&) { return (*this); }


    pointer allocate(size_type count)
    {
        if(typeid(T) == typeid(__detail::_Hash_node_base*))
        {
            //return (pointer)(poolAlloc.allocate(count));
            return allocator<T>::allocate(count);
        }
        // here, count is 1, T is std::__detail::_Hash_node<std::pair<const char* const, MODEL_UNIT<U> >, false>
        if(1 != count)
        {
            cerr << "my_allocator::allocate: count exception" << endl;
            exit(1);
        }
        if(typeid(T) != typeid(std::__detail::_Hash_node<std::pair<const char* const, MODEL_UNIT<U> >, false>))
        {
            cerr << "my_allocator::allocate: _Hash_node type exception" << endl;
            exit(1);
        }
        int mem_size = (int)sizeof(T) + MODEL_UNIT<U>::get_ext_mem_size();
        return (pointer)mem_pool::get_mem((size_t)mem_size);
    }

    void deallocate(pointer ptr, size_type count)
    {
        if(typeid(T) == typeid(__detail::_Hash_node_base*))
        {
            //poolAlloc.deallocate(ptr, count);
            allocator<T>::deallocate(ptr, count);
        }
    }

private:
    //__gnu_cxx::__pool_alloc<T> poolAlloc;
};


class my_hash
{
public:
    size_t operator()(const char* const& key) const noexcept
    {
        //return hash<string>()(key);
        return _Hash_impl::hash(key, strlen(key));
    }
};

/*
namespace std
{
    //template<>
    //struct __is_fast_hash<my_hash> : public std::true_type
    //{};
    //template<>
    //struct is_default_constructible<my_hash> : public std::true_type
    //{};
    //template<>
    //struct is_copy_assignable<my_hash> : public std::true_type
    //{};
namespace __detail
{
    template<>
    struct __is_noexcept_hash<const char*, my_hash> : public std::true_type
    {};
}
}
*/

struct my_equal
{
    bool operator()(const char* const& x, const char* const& y) const
    {
        return 0 == strcmp(x, y);
    }
};


template<typename T>
struct has_member_fn_M_v
{
private:
    template<typename U>
        static auto check(int) -> decltype(std::declval<U>()._M_v(), std::true_type());
    template<typename U>
        static auto check(...) -> decltype(std::false_type());
public:
    static const bool value = std::is_same<decltype(check<T>(0)), std::true_type>::value;
};


template <class T> typename enable_if<has_member_fn_M_v<T>::value, size_t>::type get_value_offset_in_Hash_node(T*)
{
    return (size_t)&(((T*)(1))->_M_v().second) - 1;
}


template <class T> typename enable_if<!has_member_fn_M_v<T>::value, size_t>::type get_value_offset_in_Hash_node(T*)
{
    return (size_t)&(((T*)(1))->_M_v.second) - 1;
}


#endif /*MY_ALLOCATOR_H_*/
