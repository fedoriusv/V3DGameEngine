#pragma once

#include "Common.h"

namespace v3d
{
namespace utils
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Observable;

    class Observer
    {
    public:

        Observer() = default;
        virtual ~Observer() = default;
        virtual void handleNotify(Observable* obj) = 0;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Observable
    {
    public:

        Observable() = default;
        virtual ~Observable() = default;

        void registerNotify(Observer* obj);

        void unregisterNotify(Observer* obj);
        void unregisterAll();

        void notifyObservers();


    private:

        std::recursive_mutex m_mutex;
        std::set<Observer*> m_observers;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace utils
} //namespace v3d
