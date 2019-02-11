#include "Model.h"

namespace v3d
{
namespace scene
{

Model::Model(ModleHeader* header) noexcept
    : Resource(header)
{
}

Model::~Model()
{
}


void Model::init(stream::Stream * stream)
{
    ASSERT(stream, "nullptr");
    m_stream = stream;
}

bool Model::load()
{
    if (m_loaded)
    {
        return true;
    }
    ASSERT(m_stream, "nullptr");

    //TODO:
    ASSERT(false, "not implemented");

    m_loaded = true;
    return false;
}

} //namespace scene
} //namespace v3d
