#pragma once

#include "Common.h"
#include "Renderable.h"
#include "Resource/Resource.h"

namespace v3d
{
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Billboard class
    */
    class Billboard : public Object, public resource::Resource, public Renderable
    {
    public:

        Billboard() noexcept;
        ~Billboard();

        TypePtr getType() const final;

    public:

        std::string m_name;

    private:

        bool load(const stream::Stream* stream, u32 offset = 0) override;
        bool save(stream::Stream* stream, u32 offset = 0) const override;
    };

    inline TypePtr Billboard::getType() const
    {
        return typeOf<Billboard>();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template<>
    struct TypeOf<scene::Billboard>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d