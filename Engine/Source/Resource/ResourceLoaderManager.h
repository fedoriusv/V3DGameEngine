#pragma once

#include "Common.h"
#include "Stream/FileStream.h"
#include "ResourceDecoder.h"
#include "Shader.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace resource
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Resource;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class FileLoader
    {
    public:
        FileLoader() {}

        stream::Stream* load(std::string filename)
        {
            /*if (!stream::FileStream::isExist(filename))
            {
                LOG_ERROR("File %s doens't exist", filename.c_str());
                return nullptr;
            }*/

            stream::FileStream* file = new stream::FileStream(filename, stream::FileStream::e_in);
            if (!file->isOpen())
            {
                LOG_ERROR("File %s didn't load", filename.c_str());
                return nullptr;
            }

            return file;
        }
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    
    /**
    * ResourceLoaderManager
    */
    class ResourceLoaderManager
    {
    public:
        
        template<class TResource, class TDecoder>
        static Resource* loadFromFile(std::string filename)
        {
            FileLoader loader;
            stream::Stream* stream = loader.load(filename);

            //rework
            ShaderHeader* header = new ShaderHeader;
            header->_version = 0;
            header->_size = stream->size();
            header->_flags = 0;
            header->_contentType = ShaderHeader::SpirVResource::SpirVResource_Source;
            header->_shaderType = ShaderHeader::ShaderType::ShaderType_Vertex;
            header->_shaderLang = ShaderHeader::ShaderLang::ShaderLang_GLSL;
            header->_apiVersion = 1.0f;
            header->_optLevel = 0;
            ResourceDecoder* decoder = new ShaderSpirVDecoder(header);
            Resource* resource = decoder->decode(stream);

            stream->close();
            delete stream;

            return resource;
        }
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
