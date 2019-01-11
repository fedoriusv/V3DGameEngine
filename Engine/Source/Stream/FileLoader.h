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
        FileLoader() = default;

        static stream::Stream* load(std::string filename)
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
                delete file;
                return nullptr;
            }

            return file;
        }

        static std::string getFileExtension(std::string fileName)
        {
            std::string fileExtension;

            const size_t pos = fileName.find_last_of('.');
            if (pos != std::string::npos)
            {
                fileExtension = std::string(fileName.begin() + pos + 1, fileName.end());
            }

            return fileExtension;
        }

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace stream
} //namespace v3d
