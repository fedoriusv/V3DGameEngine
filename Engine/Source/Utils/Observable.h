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

        virtual ~Observer() {};
        virtual void handleNotify(Observable* ob) = 0;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Observable
    {
    public:

        virtual ~Observable() {};

        void registerNotify(Observer* obj);

        void unregisterNotify(Observer* obj);
        void unregisterAll();

        void notifyObservers();


    private:

        std::set<Observer*> m_observers;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace utils
} //namespace v3d
