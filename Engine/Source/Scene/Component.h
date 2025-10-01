#pragma once

#include "Common.h"
#include "Object.h"

namespace v3d
{
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Component interface
    */
    class Component
    {
    public:

        virtual TypePtr getType() const = 0;
        virtual bool isBaseOfType(TypePtr type) const = 0;

        template<typename T>
        bool isBaseOfType() const 
        {
            return isBaseOfType(typeOf<T>());
        }

    protected:

        Component() noexcept = default;
        virtual ~Component() = default;
    };

    inline bool Component::isBaseOfType(TypePtr type) const
    {
        return type == typeOf<Component>();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template<typename T, typename Base = Component>
    class ComponentBase : public Base
    {
    public:

        using BaseClass = Base;

        template<typename... Args>
        explicit ComponentBase(Args&&... args) noexcept
            : Base(std::forward<Args>(args)...)
        {
        }

        inline TypePtr getType() const override
        {
            return typeOf<T>();
        }

        bool isBaseOfType(TypePtr type) const override
        {
            if (type == typeOf<T>())
            {
                return true;
            }

            return Base::isBaseOfType(type);
        }

    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template<>
    class ComponentBase<Component, void> : public Component
    {
    public:

        template<typename... Args>
        explicit ComponentBase(Args&&... args) noexcept
        {
        }

        virtual TypePtr getType() const override
        {
            return typeOf<Component>();
        }

        bool isBaseOfType(TypePtr type) const override
        {
            return type == getType();
        }
    };

} //namespace scene

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template<>
    struct TypeOf<scene::Component>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d