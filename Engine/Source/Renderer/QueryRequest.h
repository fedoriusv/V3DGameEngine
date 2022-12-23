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

        void handleNotify(const utils::Observable* query) override;
        virtual void callbackQueryResponse(QueryResult result, const void* data) = 0;

    protected:

        explicit QueryRequest(CommandList& cmdList, QueryType type) noexcept;

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

        void callbackQueryResponse(QueryResult result, const void* data) override;

    private:

        QueryTimestampRequest(CommandList& cmdList, std::function<void(u32)>) noexcept;

        friend CommandList;
        std::function<void(u32)> m_callback;
    };

    ///**
    //* @brief QueryOcclusionRequest class. Game side
    //*/
    //class QueryOcclusionRequest : public QueryRequest
    //{
    //public:

    //    QueryOcclusionRequest();
    //};

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