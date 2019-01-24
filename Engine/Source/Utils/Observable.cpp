#include "Observable.h"

namespace v3d
{
namespace utils
{

void Observable::registerNotify(Observer* obj)
{
    std::lock_guard lock(m_mutex);
    m_observers.insert(obj);
}

void Observable::unregisterNotify(Observer* obj)
{
    std::lock_guard lock(m_mutex);
    m_observers.erase(obj);
}

void Observable::unregisterAll()
{
    std::lock_guard lock(m_mutex);
    m_observers.clear();
}

void Observable::notifyObservers()
{
    std::lock_guard lock(m_mutex);
    for (auto& observer : m_observers)
    {
        observer->handleNotify(this);
    }
}

} //namespace utils
} //namespace v3d
