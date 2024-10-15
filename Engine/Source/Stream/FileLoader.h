#pragma once

#include "FileStream.h"
#include "Utils/Logger.h"
#if defined(PLATFORM_ANDROID)
#   include "Platform/Android/AssetStream.h"
#endif

namespace v3d
{
namespace stream
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * FileLoader class
    */
    class V3D_API FileLoader final
    {
    public:

        FileLoader() = delete;
        FileLoader(const FileLoader&) = delete;

        static stream::Stream* load(const std::string& filename)
        {
#if DEBUG
            LOG_DEBUG("FileLoader::try load [%s] file", stream::FileStream::absolutePath(filename).c_str());
#endif //DEBUG

#if defined(PLATFORM_ANDROID)
            //Try to load form asset storage
            android::AssetStream* asset = ::V3D_NEW(android::AssetStream, memory::MemoryLabel::MemorySystem)(filename, AASSET_MODE_STREAMING);
            if (!asset)
            {
                LOG_ERROR("FileLoader::file: the asset [%s] can't be created", stream::FileStream::absolutePath(filename).c_str());
                return nullptr;
            }

            if (asset->isOpen())
            {
                LOG_DEBUG("FileLoader::asset [%s] has been found", filename.c_str());
                return asset;
            }

            V3D_DELETE(asset, memory::MemoryLabel::MemorySystem);
#else //NOT PLATFORM_ANDROID
            if (!stream::FileStream::isExists(filename))
            {
                return nullptr;
            }
#endif //PLATFORM_ANDROID
            stream::FileStream* file = V3D_NEW(stream::FileStream, memory::MemoryLabel::MemorySystem)(filename, stream::FileStream::e_in);
            if (!file)
            {
                LOG_ERROR("FileLoader::file: the file [%s] can't be created", stream::FileStream::absolutePath(filename).c_str());
                return nullptr;
            }

            if (!file->isOpen())
            {
                LOG_ERROR("File %s hasn't loaded", filename.c_str());
                V3D_DELETE(file, memory::MemoryLabel::MemorySystem);

                return nullptr;
            }

            LOG_DEBUG("FileLoader::file [%s] has been found", stream::FileStream::absolutePath(filename).c_str());
            return file;
        }

        static std::string getFileExtension(const std::string& fileName)
        {
            std::string extension = stream::FileStream::extension(fileName);

            std::string fileExtension = extension.substr(1, extension.size() - 1);
            return fileExtension;
        }
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace stream
} //namespace v3d
