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
    class QueryRequest : public Object, public utils::Observer//, public AsyncReadback<int>
    {
    public:

        QueryRequest() = delete;
        QueryRequest(QueryRequest&) = delete;
        ~QueryRequest();

    protected:

        /**
        * @brief QueryRequest constructor.
        * Private method. Use child interface for call it.
        *
        * @param std::function<void(u32)> callback [required]
        * @param cconst std::string& name [optional]
        */
        explicit QueryRequest(CommandList& cmdList, QueryType type, [[maybe_unused]] const std::string& name = "") noexcept;

        void handleNotify(const utils::Observable* query) override;
        virtual void callbackQueryResponse(QueryResult result, const void* data) = 0;

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

        QueryTimestampRequest() = delete;
        QueryTimestampRequest(QueryTimestampRequest&) = delete;
        ~QueryTimestampRequest();

        /**
        * @brief Timestamp signature in nanoseconds
        */
        using Timestamp = void(u32);

    private:

        /**
        * @brief QueryTimestampRequest constructor.
        * Private method. Use createObject interface inside CommandList class to call.
        *
        * @param std::function<void(u32)> callback [required]
        * @param cconst std::string& name [optional]
        */
        QueryTimestampRequest(CommandList& cmdList, std::function<Timestamp> callback, [[maybe_unused]] const std::string& name = "") noexcept;

        void callbackQueryResponse(QueryResult result, const void* data) override;

        friend CommandList;
        std::function<Timestamp> m_callback;
    };

    /**
    * @brief QueryOcclusionRequest class. Game side
    */
    class QueryOcclusionRequest : public QueryRequest
    {
    public:

        QueryOcclusionRequest() = delete;
        QueryOcclusionRequest(QueryOcclusionRequest&) = delete;
        ~QueryOcclusionRequest();

    private:

        /**
        * @brief QueryOcclusionRequest constructor.
        * Private method. Use createObject interface inside CommandList class to call.
        *
        * @param std::function<void(u32)> callback [required]
        * @param cconst std::string& name [optional]
        */
        QueryOcclusionRequest(CommandList& cmdList, std::function<void(u32)> callback, [[maybe_unused]] const std::string& name = "") noexcept;

        void callbackQueryResponse(QueryResult result, const void* data) override;

        friend CommandList;
        std::function<void(u32)> m_callback;
    };

    ///**
    //* @brief QueryBinaryOcclusionRequest class. Game side
    //*/
    //class QueryBinaryOcclusionRequest : public QueryRequest
    //{
    //public:

    //    QueryBinaryOcclusionRequest();
    //};

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