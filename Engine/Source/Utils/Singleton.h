#pragma once

#include "Common.h"

namespace v3d
{
namespace utils
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Singleton class. Lazy Singleton
    */
    template <class T>
    class Singleton
    {
    public:

        static T*   getInstance();
        static T*   getLazyInstance();

        static T*   createInstance();
        static void freeInstance();

    protected:

        Singleton() = default;
        virtual ~Singleton() = default;

    private:

        static T*               s_instance;
        static std::once_flag   s_onceFlag;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template <class T>
    T* Singleton<T>::s_instance = nullptr;

    template <class T>
    std::once_flag Singleton<T>::s_onceFlag;

    template <class T>
    inline T *Singleton<T>::getInstance()
    {
        ASSERT(s_instance, "nullptr");
        return s_instance;
    }

    template <class T>
    inline T* Singleton<T>::getLazyInstance()
    {
        std::call_once(Singleton::s_onceFlag, []()
            {
                s_instance = V3D_NEW(T, memory::MemoryLabel::MemorySystem);
            });

        ASSERT(s_instance, "nullptr");
        return s_instance;
    }

    template<class T>
    inline T* Singleton<T>::createInstance()
    {
        ASSERT(!s_instance, "not nullptr");
        s_instance = V3D_NEW(T, memory::MemoryLabel::MemorySystem);

        ASSERT(s_instance, "nullptr");
        return s_instance;
    }

    template <class T>
    inline void Singleton<T>::freeInstance()
    {
        if (s_instance)
        {
            V3D_DELETE(s_instance,memory::MemoryLabel::MemorySystem);
        }

        s_instance = nullptr;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace utils
} //namespace v3d
