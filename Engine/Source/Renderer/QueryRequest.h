#pragma once

#include "Common.h"
#include "Object.h"
#include "CommandList.h"
#include "Core/Query.h"
#include "Renderer/AsyncReadback.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief QueryRequest base class. Game side
    */
    class QueryRequest : public Object, public utils::Observer
    {
    public:

        /**
        * @brief QueryRequest destructor.
        */
        virtual ~QueryRequest();

    protected:

        /**
        * @brief QueryRequest constructor.
        * Private method. Use child interface for call it.
        *
        * @param QueryType type [required]
        * @param const std::string& name [optional]
        */
        explicit QueryRequest(CommandList& cmdList, QueryType type, [[maybe_unused]] const std::string& name = "") noexcept;

        QueryRequest() = delete;
        QueryRequest(QueryRequest&) = delete;

        void handleNotify(const utils::Observable* query, void* msg) override;

        friend CommandList;
        CommandList& m_cmdList;

        Query* m_query;
        QueryType m_type;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief QueryTimestampRequest class. Game side
    */
    class QueryTimestampRequest : public QueryRequest
    {
    public:

        /**
        * @brief Timestamp signature in nanoseconds
        */
        using Timestamp = void(const std::vector<u32>& timestamp);

        /**
        * @brief Timestamp with string tags signature in nanoseconds
        */
        using TimestampTaged = void(const std::vector<u32>& timestamp, const std::vector<std::string>& tags);

        /**
        * @brief QueryTimestampRequest destructor.
        */
        ~QueryTimestampRequest() = default;

        /**
        * @brief timestampQuery function.
        * request query timestamp by id.
        *
        * @param u32 id [required]
        * @param const std::string& tag [optional]
        */
        void timestampQuery(u32 id, const std::string& tag = "");

    private:

        /**
        * @brief QueryTimestampRequest constructor.
        * Private method. Use createObject interface inside CommandList class to call.
        *
        * @param std::function<Timestamp> callback [required]
        * @param const std::string& name [optional]
        */
        QueryTimestampRequest(CommandList& cmdList, std::function<Timestamp> callback, u32 size, [[maybe_unused]] const std::string& name = "") noexcept;

        /**
        * @brief QueryTimestampRequest constructor.
        * Private method. Use createObject interface inside CommandList class to call.
        *
        * @param std::function<TimestampTaged> callback [required]
        * @param const std::string& name [optional]
        */
        QueryTimestampRequest(CommandList& cmdList, std::function<TimestampTaged> callback, u32 size, [[maybe_unused]] const std::string& name = "") noexcept;

        QueryTimestampRequest() = delete;
        QueryTimestampRequest(QueryTimestampRequest&) = delete;

        friend CommandList;

        std::vector<u32> m_result;
        std::vector<std::string> m_tags;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief QueryOcclusionRequest class. Game side
    */
    class QueryOcclusionRequest : public QueryRequest
    {
    public:

        /**
        * @brief OcclusionQuery signature in count of samples
        */
        using QuerySamples = void(const std::vector<u32>& samples);

        /**
        * @brief OcclusionQuery with tags signature in count of samples
        */
        using QuerySamplesTaged = void(const std::vector<u32>& samples, const std::vector<std::string>& tags);

        QueryOcclusionRequest() = delete;
        QueryOcclusionRequest(QueryOcclusionRequest&) = delete;
        ~QueryOcclusionRequest() = default;

        /**
        * @brief beginQuery/endQuery function.
        * request query timestamp by id.
        * @param u32 id [required]
        * @param const std::string& tag[optional]
        */
        void beginQuery(u32 id, const std::string& tag = "");
        void endQuery(u32 id);

    private:

        /**
        * @brief QueryOcclusionRequest constructor.
        * Private method. Use createObject interface inside CommandList class to call.
        *
        * @param std::function<QuerySamples> callback [required]
        * @param const std::string& name [optional]
        */
        QueryOcclusionRequest(CommandList& cmdList, std::function<QuerySamples> callback, u32 size, [[maybe_unused]] const std::string& name = "") noexcept;

        /**
        * @brief QueryOcclusionRequest constructor.
        * Private method. Use createObject interface inside CommandList class to call.
        *
        * @param std::function<QuerySamplesTaged> callback [required]
        * @param const std::string& name [optional]
        */
        QueryOcclusionRequest(CommandList& cmdList, std::function<QuerySamplesTaged> callback, u32 size, [[maybe_unused]] const std::string& name = "") noexcept;

        friend CommandList;

        std::vector<u32> m_result;
        std::vector<std::string> m_tags;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief QueryBinaryOcclusionRequest class. Game side
    */
    class QueryBinaryOcclusionRequest : public QueryRequest
    {
    public:

        /**
        * @brief QueryBinarySample signature in passsed of sample
        */
        using QueryBinarySample = void(bool sampled, u32 id, const std::string& tag);

        QueryBinaryOcclusionRequest() = delete;
        QueryBinaryOcclusionRequest(QueryBinaryOcclusionRequest&) = delete;
        ~QueryBinaryOcclusionRequest() = default;

    private:

        /**
        * @brief QueryBinaryOcclusionRequest constructor.
        * Private method. Use createObject interface inside CommandList class to call.
        *
        * @param std::function<QueryBinarySample> callback [required]
        * @param const std::string& name [optional]
        */
        QueryBinaryOcclusionRequest(CommandList& cmdList, std::function<QueryBinarySample> callback, [[maybe_unused]] const std::string& name = "") noexcept;

        friend CommandList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    ///**
    //* @brief QueryPipelineStatisticRequest class. Game side
    //*/
    //class QueryPipelineStatisticRequest : public QueryRequest
    //{
    //public:

    //    QueryPipelineStatisticRequest();
    //};

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d