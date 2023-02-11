#include "QueryRequest.h"

#include "Core/Context.h"
#include "Core/Query.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace renderer
{

    /*CommandBeginQuery*/
    class CommandBeginQuery final : public Command
    {
    public:
        CommandBeginQuery(CommandBeginQuery&) = delete;
        CommandBeginQuery(Query* query, u32 id, const std::string& tag) noexcept
            : m_query(query)
            , m_id(id)
            , m_tag(tag)
        {
#if DEBUG_COMMAND_LIST
            LOG_DEBUG("CommandBeginQuery constructor");
#endif //DEBUG_COMMAND_LIST
        }

        ~CommandBeginQuery()
        {
#if DEBUG_COMMAND_LIST
            LOG_DEBUG("CommandBeginQuery constructor");
#endif //DEBUG_COMMAND_LIST
        }

        void execute(const CommandList& cmdList)
        {
#if DEBUG_COMMAND_LIST
            LOG_DEBUG("CommandBeginQuery execute");
#endif //DEBUG_COMMAND_LIST
            cmdList.getContext()->beginQuery(m_query, m_id, m_tag);
        }

        Query* m_query;
        u32 m_id;
        std::string m_tag;
    };

    /*CommandEndQuery*/
    class CommandEndQuery final : public Command
    {
    public:
        CommandEndQuery(CommandEndQuery&) = delete;
        CommandEndQuery(Query* query, u32 id) noexcept
            : m_query(query)
            , m_id(id)
        {
#if DEBUG_COMMAND_LIST
            LOG_DEBUG("CommandEndQuery constructor");
#endif //DEBUG_COMMAND_LIST
        }

        ~CommandEndQuery()
        {
#if DEBUG_COMMAND_LIST
            LOG_DEBUG("CommandEndQuery constructor");
#endif //DEBUG_COMMAND_LIST
        }

        void execute(const CommandList& cmdList)
        {
#if DEBUG_COMMAND_LIST
            LOG_DEBUG("CommandEndQuery execute");
#endif //DEBUG_COMMAND_LIST
            cmdList.getContext()->endQuery(m_query, m_id);
        }

        Query* m_query;
        u32 m_id;
        std::string m_tag;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

QueryRequest::QueryRequest(CommandList& cmdList, QueryType type, const std::string& name) noexcept
    : m_cmdList(cmdList)
    , m_query(nullptr)
    , m_type(type)
{
    LOG_DEBUG("QueryRequest::QueryRequest constructor %llx", this);
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

void QueryRequest::handleNotify(const utils::Observable* query, void* msg)
{
    LOG_DEBUG("QueryRequest::handleNotify to delete the query %xll", this);
    ASSERT(m_query == query, "not same");

    m_query = nullptr;
}


QueryTimestampRequest::QueryTimestampRequest(CommandList& cmdList, std::function<Timestamp> callback, u32 size, const std::string& name) noexcept
    : QueryRequest(cmdList, QueryType::TimeStamp, name)
{
    m_query = m_cmdList.getContext()->createQuery(m_type, size, [this, &cmdList, callback](QueryResult result, u32 size, const void* data) -> void
        {
            if (result == QueryResult::Success && data)
            {
                //TODO Async readback
                memcpy(m_result.data(), data, size);
                callback(m_result);
                //cmdList.pushReadbackCall(nullptr, [callback, timestamp, id, tag](Object* caller)
                //    {
                //        callback(timestamp, id, tag);
                //    });
            }
        }, name);
    ASSERT(m_query, "m_query is nullptr");
    m_query->registerNotify(this);

    m_result.resize(size);
}

QueryTimestampRequest::QueryTimestampRequest(CommandList& cmdList, std::function<TimestampTaged> callback, u32 size, const std::string& name) noexcept
    : QueryRequest(cmdList, QueryType::TimeStamp, name)
{
    m_query = m_cmdList.getContext()->createQuery(m_type, size, [this, &cmdList, callback](QueryResult result, u32 size, const void* data) -> void
        {
            if (result == QueryResult::Success && data)
            {
                //TODO Async readback
                memcpy(m_result.data(), data, size);
                callback(m_result, m_tags);
                //cmdList.pushReadbackCall(nullptr, [callback, timestamp, id, tag](Object* caller)
                //    {
                //        callback(timestamp, id, tag);
                //    });
            }
        }, name);
    ASSERT(m_query, "m_query is nullptr");
    m_query->registerNotify(this);

    m_result.resize(size);
    m_tags.resize(size);
}

void QueryTimestampRequest::timestampQuery(u32 id, const std::string& tag)
{
    ASSERT(m_query, "Must be valid");

    ASSERT(id < m_tags.size(), "range out");
    m_tags[id] = tag;

    if (m_cmdList.isImmediate())
    {
        m_cmdList.getContext()->timestampQuery(m_query, id);
    }
    else
    {
        /*CommandTimestampQuery*/
        class CommandTimestampQuery final : public Command, utils::Observer
        {
        public:
            CommandTimestampQuery(CommandTimestampQuery&) = delete;
            CommandTimestampQuery(Query* query, u32 id, const std::string& tag) noexcept
                : m_query(query)
                , m_id(id)
                , m_tag(tag)
            {
#if DEBUG_COMMAND_LIST
                LOG_DEBUG("CommandTimestampQuery constructor");
#endif //DEBUG_COMMAND_LIST
            }

            ~CommandTimestampQuery()
            {
#if DEBUG_COMMAND_LIST
                LOG_DEBUG("CommandTimestampQuery constructor");
#endif //DEBUG_COMMAND_LIST
            }

            void execute(const CommandList& cmdList)
            {
#if DEBUG_COMMAND_LIST
                LOG_DEBUG("CommandTimestampQuery execute");
#endif //DEBUG_COMMAND_LIST
                cmdList.getContext()->timestampQuery(m_query, m_id, m_tag);
            }

            void handleNotify(const utils::Observable* object, void* msg) override
            {
                LOG_ERROR("CommandTimestampQuery query %llx was deleted", m_query);
                m_query = nullptr;
            }

            Query* m_query;
            u32 m_id;
            std::string m_tag;
        };

        m_cmdList.pushCommand(new CommandTimestampQuery(m_query, id, tag));
    }
}


QueryOcclusionRequest::QueryOcclusionRequest(CommandList& cmdList, std::function<QuerySamples> callback, u32 size, const std::string& name) noexcept
    : QueryRequest(cmdList, QueryType::Occlusion, name)
{
    m_query = m_cmdList.getContext()->createQuery(m_type, size, [this, &cmdList, callback](QueryResult result, u32 size, const void* data) -> void
        {
            if (result == QueryResult::Success && data)
            {
                //TODO
                memcpy(m_result.data(), data, size);
                callback(m_result);
                //cmdList.pushReadbackCall(nullptr, [callback, timestamp, id, tag](Object* caller)
                //    {
                //        callback(timestamp, id, tag);
                //    });
            }
        }, name);
    ASSERT(m_query, "m_query is nullptr");
    m_query->registerNotify(this);

    m_result.resize(size);
}

QueryOcclusionRequest::QueryOcclusionRequest(CommandList& cmdList, std::function<QuerySamplesTaged> callback, u32 size, const std::string& name) noexcept
    : QueryRequest(cmdList, QueryType::Occlusion, name)
{
    m_query = m_cmdList.getContext()->createQuery(m_type, size, [this, &cmdList, callback](QueryResult result, u32 size, const void* data) -> void
        {
            if (result == QueryResult::Success && data)
            {
                //TODO
                memcpy(m_result.data(), data, size);
                callback(m_result, m_tags);
                //cmdList.pushReadbackCall(nullptr, [callback, timestamp, id, tag](Object* caller)
                //    {
                //        callback(timestamp, id, tag);
                //    });
            }
        }, name);
    ASSERT(m_query, "m_query is nullptr");
    m_query->registerNotify(this);

    m_result.resize(size);
    m_tags.resize(size);
}

void QueryOcclusionRequest::beginQuery(u32 id, const std::string& tag)
{
    ASSERT(m_query, "Must be valid");

    ASSERT(id < m_tags.size(), "range out");
    m_tags[id] = tag;

    if (m_cmdList.isImmediate())
    {
        m_cmdList.getContext()->beginQuery(m_query, id);
    }
    else
    {
        m_cmdList.pushCommand(new CommandBeginQuery(m_query, id, tag));
    }
}

void QueryOcclusionRequest::endQuery(u32 id)
{
    ASSERT(m_query, "Must be valid");
    if (m_cmdList.isImmediate())
    {
        m_cmdList.getContext()->endQuery(m_query, id);
    }
    else
    {
        m_cmdList.pushCommand(new CommandEndQuery(m_query, id));
    }
}

QueryBinaryOcclusionRequest::QueryBinaryOcclusionRequest(CommandList& cmdList, std::function<QueryBinarySample> callback, const std::string& name) noexcept
    : QueryRequest(cmdList, QueryType::BinaryOcclusion, name)
{
    //m_query = m_cmdList.getContext()->createQuery(m_type, 0, [&cmdList, callback](QueryResult result, u32 size, const void* data, u32 id, const std::string& tag) -> void
    //    {
    //        if (result == QueryResult::Success && data)
    //        {
    //            bool sampled = *reinterpret_cast<const bool*>(data);
    //            cmdList.pushReadbackCall(nullptr, [callback, sampled, id, tag](Object* caller)
    //                {
    //                    callback(sampled, id, tag);
    //                });
    //        }
    //    }, name);
    //ASSERT(m_query, "m_query is nullptr");
    //m_query->registerNotify(this);
}

} //namespace renderer
} //namespace v3d