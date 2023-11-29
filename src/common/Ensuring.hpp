#ifndef  H__ENSURING__H
#define  H__ENSURING__H

#include  <stdexcept>

template <typename T>
void ThrowExceptionIfNull(const T* const ptr)
{
    if (ptr == nullptr)
    {
        throw new std::exception();
    }
}

template <typename T>
static void ThrowExceptionIfZero(T value)
{
    if (value == 0)
    {
        throw new std::exception();
    }
}

#endif  // H__ENSURING__H

