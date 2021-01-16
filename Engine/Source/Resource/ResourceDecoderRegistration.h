#pragma once

#include "Common.h"

namespace v3d
{
namespace resource
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class ResourceDecoder;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ResourceDecoderRegistration class
    */
    class ResourceDecoderRegistration
    {
    public:

        ResourceDecoderRegistration() noexcept;
        virtual ~ResourceDecoderRegistration();

        /**
        * @brief registerDecoder function
        * @param const ResourceDecoder* decoder [required]
        * @return true if decoder added successfully
        */
        bool registerDecoder(const ResourceDecoder* decoder);

        /**
        * @brief unregisterDecoder function
        * @param const ResourceDecoder* decoder [required]
        * @return true if decoder removed successfully
        */
        bool unregisterDecoder(const ResourceDecoder* decoder);

        /**
        * @brief unregisterAllDecoders function
        */
        void unregisterAllDecoders();

        /**
        * @brief getDecoders function
        * @return list of decoders
        */
        const std::vector<const ResourceDecoder*>& getDecoders() const;

    protected:

        const ResourceDecoder* findDecoder(const std::string& extension);

    private:

        std::vector<const ResourceDecoder*> m_decoders;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
