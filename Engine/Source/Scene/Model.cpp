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

ModleHeader::ModleHeader() noexcept
    : _mode(renderer::PolygonMode::PolygonMode_Triangle)
    , _frontFace(renderer::FrontFace::FrontFace_Clockwise)
    , _content(0)
    , _localTransform(false)
    , _indexBuffer(false)
{
    _vertex._count = 0;
    _vertex._globalSize = 0;

    _index._count = 0;
    _index._globalSize = 0;
}

ModleHeader::~ModleHeader()
{
}

} //namespace scene
} //namespace v3d
