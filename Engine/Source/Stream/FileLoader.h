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
    class FileLoader final
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
            android::AssetStream* asset = new android::AssetStream(filename, AASSET_MODE_STREAMING);
            if (asset->isOpen())
            {
                LOG_DEBUG("FileLoader::asset [%s] has been found", filename.c_str());
                return asset;
            }

            delete asset;
#else //NOT PLATFORM_ANDROID
            if (!stream::FileStream::isExists(filename))
            {
                return nullptr;
            }
#endif //PLATFORM_ANDROID
            stream::FileStream* file = new stream::FileStream(filename, stream::FileStream::e_in);
            if (!file->isOpen())
            {
                LOG_ERROR("File %s hasn't loaded", filename.c_str());
                delete file;
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
