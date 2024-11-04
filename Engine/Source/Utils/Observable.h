#pragma once

#include "Common.h"

namespace v3d
{
namespace utils
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Observable;

    /**
    * @brief Observer interface.
    */
    class V3D_API Observer
    {
    public:

        Observer() = default;
        virtual ~Observer() = default;

        virtual void handleNotify(const Observable* object, void* msg = nullptr) = 0;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Observable class.
    */
    class V3D_API Observable
    {
    public:

        Observable() = default;
        virtual ~Observable() = default;

        void registerNotify(Observer* object);

        void unregisterNotify(Observer* object);
        void unregisterAll();

        void notifyObservers(void* msg = nullptr) const;

    private:

        mutable std::recursive_mutex m_mutex;
        std::set<Observer*>          m_observers;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace utils
} //namespace v3d
