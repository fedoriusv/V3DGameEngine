#pragma once

#include "FileStream.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace stream
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * FileLoader class
    */
    class FileLoader
    {
    public:
        FileLoader() = delete;

        static stream::Stream* load(std::string filename)
        {
#if DEBUG
            LOG_DEBUG("FileLoader::try load [%s] file", stream::FileStream::absolutePath(filename).c_str());
#endif //DEBUG
            if (!stream::FileStream::isExists(filename))
            {
                return nullptr;
            }

            stream::FileStream* file = new stream::FileStream(filename, stream::FileStream::e_in);
            if (!file->isOpen())
            {
                LOG_ERROR("File %s didn't load", filename.c_str());
                delete file;
                return nullptr;
            }

            return file;
        }

        static std::string getFileExtension(std::string fileName)
        {
            std::string fileExtension;

            /*const size_t pos = fileName.find_last_of('.');
            if (pos != std::string::npos)
            {
                fileExtension = std::string(fileName.begin() + pos + 1, fileName.end());
            }*/
            std::string extension = stream::FileStream::extension(fileName);
            fileExtension = extension.substr(1, extension.size() - 1);

            return fileExtension;
        }

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace stream
} //namespace v3d
