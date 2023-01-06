#include "QueryRequest.h"

#include "Core/Context.h"
#include "Core/Query.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace renderer
{

QueryRequest::QueryRequest(CommandList& cmdList, QueryType type, const std::string& name) noexcept
    : m_cmdList(cmdList)
    , m_query(nullptr)
    , m_type(type)
{
    m_query = m_cmdList.getContext()->createQuery(type, std::bind(&QueryRequest::callbackQueryResponse, this, std::placeholders::_1, std::placeholders::_2), name);
    ASSERT(m_query, "m_image is nullptr");
    m_query->registerNotify(this);
}

QueryRequest::~QueryRequest()
{
    LOG_DEBUG("QueryRequest::~QueryRequest destructor %llx", this);
    ASSERT(m_query, "query nullptr");
    m_query->unregisterNotify(this);

    if (m_cmdList.isImmediate())
    {
        m_cmdList.getContext()->removeQuery(m_query);
    }
    else
    {
        /*CommandDestroyQuery*/
        class CommandDestroyQuery final : public Command
        {
        public:

            explicit CommandDestroyQuery(Query* query) noexcept
                : m_query(query)
            {
                LOG_DEBUG("CommandDestroyQuery constructor");
            };

            CommandDestroyQuery() = delete;
            CommandDestroyQuery(CommandDestroyQuery&) = delete;

            ~CommandDestroyQuery()
            {
                LOG_DEBUG("CommandDestroyQuery destructor");
            };

            void execute(const CommandList& cmdList)
            {
                LOG_DEBUG("CommandDestroyQuery execute");
                cmdList.getContext()->removeQuery(m_query);
            }

        private:

            Query* const m_query;
        };

        m_cmdList.pushCommand(new CommandDestroyQuery(m_query));
    }
}

void QueryRequest::handleNotify(const utils::Observable* query)
{
    LOG_DEBUG("QueryRequest::handleNotify to delete the query %xll", this);
    ASSERT(m_query == query, "not same");

    m_query = nullptr;
}


QueryTimestampRequest::QueryTimestampRequest(CommandList& cmdList, std::function<void(u32)> callback, const std::string& name) noexcept
    : QueryRequest(cmdList, QueryType::TimeStamp, name)
    , m_callback(callback)
{
}

void QueryTimestampRequest::callbackQueryResponse(QueryResult result, const void* data)
{
    if (m_callback && result == QueryResult::Success && data)
    {
        u32 timestamp = *reinterpret_cast<const u32*>(data);
        m_callback(timestamp);
    }
}

QueryTimestampRequest::~QueryTimestampRequest()
{
}


QueryOcclusionRequest::QueryOcclusionRequest(CommandList& cmdList, std::function<void(u32)> callback, const std::string& name) noexcept
    : QueryRequest(cmdList, QueryType::TimeStamp, name)
    , m_callback(callback)
{
}

void QueryOcclusionRequest::callbackQueryResponse(QueryResult result, const void* data)
{
    if (m_callback && result == QueryResult::Success && data)
    {
        u32 samples = *reinterpret_cast<const u32*>(data);
        m_callback(samples);
    }
}

QueryOcclusionRequest::~QueryOcclusionRequest()
{
}

} //namespace renderer
} //namespace v3d