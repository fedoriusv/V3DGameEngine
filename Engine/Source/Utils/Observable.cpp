#include "Observable.h"

namespace v3d
{
namespace utils
{

void Observable::registerNotify(Observer* object)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_observers.insert(object);
}

void Observable::unregisterNotify(Observer* object)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_observers.erase(object);
}

void Observable::unregisterAll()
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_observers.clear();
}

void Observable::notifyObservers(void* msg) const
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    for (auto& observer : m_observers)
    {
        observer->handleNotify(this, msg);
    }
}

} //namespace utils
} //namespace v3d
