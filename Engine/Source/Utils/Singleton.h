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
        static void freeInstance();

    protected:

                    Singleton();
        virtual     ~Singleton();

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
    Singleton<T>::Singleton()
    {
    }

    template <class T>
    Singleton<T>::~Singleton()
    {
    }

    template <class T>
    T *Singleton<T>::getInstance()
    {
        std::call_once(Singleton::s_onceFlag, []()
        {
            s_instance = new T;
        });

        ASSERT(s_instance, "nullptr");
        return s_instance;
    }

    template <class T>
    void Singleton<T>::freeInstance()
    {
        if (s_instance)
        {
            delete s_instance;
        }

        s_instance = nullptr;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace utils
} //namespace v3d
