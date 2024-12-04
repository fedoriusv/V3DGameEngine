#include "MyApplication.h"

#include "Utils/Logger.h"
#include "Utils/Timer.h"
#include "Stream/StreamManager.h"
#include "Memory/MemoryPool.h"
#include "Events/InputEventReceiver.h"

#include "Renderer/Formats.h"
#include "Renderer/Texture.h"
#include "Renderer/Buffer.h"
#include "Renderer/Shader.h"
#include "Renderer/PipelineState.h"
#include "Renderer/RenderTargetState.h"
#include "Renderer/ShaderProgram.h"
#include "Renderer/SamplerState.h"

#include "Resource/ResourceLoader.h"
#include "Resource/ResourceManager.h"
#include "Resource/ShaderSourceFileLoader.h"
#include "Resource/ShaderSourceStreamLoader.h"
#include "Resource/ShaderBinaryFileLoader.h"
#include "Resource/Bitmap.h"
#include "Resource/ImageFileLoader.h"

#include "Task/TaskScheduler.h"


#include "crc32c/crc32c.h"

#include <random>
#include <thread>


using namespace v3d;
using namespace v3d::platform;
using namespace v3d::utils;
using namespace v3d::event;
using namespace v3d::renderer;
using namespace v3d::resource;


MyApplication::MyApplication(int& argc, char** argv)
{

}

int MyApplication::Execute()
{
    //Test_Timer();
    Test_Thread();
    //Test_TaskContainters();
    Test_Task();

    //Test_Windows();
    //std::thread test_thread([this]() -> void
    //    {
    //        Test_Timer();
    //    });

    //test_thread.join();

    //Test_MemoryPool();
    //Test_ImageLoadStore();

    LOG_DEBUG("Tests are finished");

    delete this;
    return 0;
}

void MyApplication::Test_ImageLoadStore()
{
    LOG_DEBUG("Test_ImageLoad");
    resource::Bitmap* image = resource::ResourceManager::getInstance()->load<resource::Bitmap, resource::ImageFileLoader>("examples/test/data/textures/basetex.jpg");
    ASSERT(image, "nullptr");
    resource::ResourceManager::getInstance()->remove(image);

    //TODO test data validation
    [[maybe_unused]] u32 endTest = 0;
}

void MyApplication::Test_Timer()
{
    LOG_DEBUG("Test_Timer");

    {
        LOG_DEBUG("Test_Timer test 1");

        utils::Timer timer;
        ASSERT(timer.getTime<utils::Timer::Duration_NanoSeconds>() == 0, "Must be 0");

        timer.start();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        timer.stop();

        u64 value0 = timer.getTime<utils::Timer::Duration_MilliSeconds>();
        ASSERT(value0 == 10, "Must be 10");

        timer.reset();
        ASSERT(timer.getTime<utils::Timer::Duration_NanoSeconds>() == 0, "Must be 0");

        timer.start();
        std::this_thread::sleep_for(std::chrono::seconds(2));
        timer.stop();

        u64 value1 = timer.getTime<utils::Timer::Duration_Seconds>();
        ASSERT(value1 == 2, "Must be 2");

        timer.start();
        std::this_thread::sleep_for(std::chrono::seconds(3));
        timer.stop();

        u64 value2 = timer.getTime<utils::Timer::Duration_Seconds>();
        ASSERT(value2 == value1 + 3, "Must be 5");
    }

    {
        LOG_DEBUG("Test_Timer test 2");

        const u64 resetTime = 1000; //1 sec

        {
            auto start_time_w = std::chrono::high_resolution_clock::now();

            auto start_time = std::chrono::high_resolution_clock::now();
            auto end_time = start_time;

            u64 miliseconds = 0;
            while (true)
            {
                end_time = std::chrono::high_resolution_clock::now();
                u64 duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
                if (resetTime <= duration)
                {
                    miliseconds += duration;
                    start_time = std::chrono::high_resolution_clock::now();
                    LOG_DEBUG("Test_Timer chrono %d miliseconds are passed. Summary %d", duration, miliseconds);
                }

                if (miliseconds >= resetTime * 10)
                {
                    break;
                }
            }

            auto end_time_w = std::chrono::high_resolution_clock::now();
            u64 duration_w = std::chrono::duration_cast<std::chrono::milliseconds>(end_time_w - start_time_w).count();

            u64 error = miliseconds - (resetTime * 10);
            LOG_WARNING("Test_Timer chrono duration %d = 10 sec. Error %d ms", miliseconds, error);
        }

        {
            auto start_time = std::chrono::high_resolution_clock::now();
            auto end_time = start_time;

            utils::Timer timer;
            timer.start();

            u64 miliseconds = 0;
            [[maybe_unused]] u32 loopsCount = 0;
            while (true)
            {
                u64 duration = timer.getTime<utils::Timer::Duration_MilliSeconds>();
                if (resetTime <= duration)
                {
                    miliseconds += duration;
                    timer.reset();
                    LOG_DEBUG("Test_Timer timer %d miliseconds are passed. Summary %d", duration, miliseconds);
                }

                end_time = std::chrono::high_resolution_clock::now();
                if (miliseconds >= resetTime * 10)
                {
                    break;
                }

                ++loopsCount;
            }

            timer.stop();

            u64 error = miliseconds - (resetTime * 10);
            LOG_WARNING("Test_Timer timer duration %d = 10 sec. Error %d ms", miliseconds, error);
            ASSERT(miliseconds == resetTime * 10, "Must be 10 sec");
        }
    }

    [[maybe_unused]] u32 endTest = 0;
}

void MyApplication::Test_Thread()
{
    LOG_DEBUG("Test_Thread");

    utils::Thread thread;

    bool run = true;
    thread.run([](void* data) -> void
        {
            bool* run = reinterpret_cast<bool*>(data);
            while (*run)
            {
            }

            int a = 0;
        }, &run);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT(thread.isRunning(), "failed");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    run = false;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT(!thread.isRunning(), "failed");

    thread.terminate();
}

void MyApplication::Test_TaskContainters()
{
    struct Val : task::ThreadSafeNode<Val>
    {
        int a;
    };

    Val a; a.a = 1;
    Val b; b.a = 2;
    Val c; c.a = 3;
    Val d; d.a = 4;

    task::ThreadSafeStack<Val> st;
    st.push(&a);
    st.push(&b);
    st.push(&c);
    st.push(&d);

    Val* sva = st.pop();
    ASSERT(sva->a == 4, "must be the same");
    Val* svb = st.pop();
    ASSERT(svb->a == 3, "must be the same");
    Val* svc = st.pop();
    ASSERT(svc->a == 2, "must be the same");
    Val* svd = st.pop();
    ASSERT(svd->a == 1, "must be the same");

    task::ThreadSafeQueue<Val> qt;



    qt.enqueue(&a);
    qt.enqueue(&b);
    qt.enqueue(&c);
    qt.enqueue(&d);

    Val* qva = qt.dequeue();
    ASSERT(qva->a == 1, "must be the same");
    Val* qvb = qt.dequeue();
    ASSERT(qvb->a == 2, "must be the same");
    Val* qvc = qt.dequeue();
    ASSERT(qvc->a == 3, "must be the same");
    Val* qvd = qt.dequeue();
    ASSERT(qvd->a == 4, "must be the same");
}

void MyApplication::Test_Task()
{
    LOG_DEBUG("Test_Task");

    task::TaskScheduler scheduler(6);

    const u32 numJobs = 6;

    u64 jobTime[numJobs] = { 0 };
    u32 jobFinished[numJobs] = { 0 };
    task::TaskPriority jobPriority[numJobs] = { task::TaskPriority::Normal };
    std::array<std::function<void()>, numJobs> jobs;
    for (u32 i = 0; i < numJobs; ++i)
    {
        jobs[i] = [&jobTime, &jobFinished, &jobPriority, i]() -> void
            {
                u64 value = 0;
                u64 maxValue = 1'000'000'000;
                auto start = utils::Timer::getCurrentTime();
                while (value < maxValue)
                {
                    ++value;
                }
                auto end = utils::Timer::getCurrentTime();
                jobTime[i] = end - start;
                ++jobFinished[i];
            };

        scheduler.executeTask(jobPriority[i], task::TaskMask::WorkerThread, jobs[i]);
    }

    u64 jobNTime = 0;
    u32 allJobsTimeStart = utils::Timer::getCurrentTime();
    bool jobNFinished = false;

   while (true)
    {
       for (u32 i = 0; i < numJobs; ++i)
       {
           if (jobFinished[i] == 1)
           {
               LOG_DEBUG("Test_Task JOB%u is finshed time %u", i, jobTime[i]);
               ++jobFinished[i];
           }
       }

       bool finished = std::all_of(std::begin(jobFinished), std::end(jobFinished), [](u32 finish) -> bool
           {
               return (finish == 2);
           });
       if (finished)
       {
           u32 allJobsTimeEnd = utils::Timer::getCurrentTime();
           LOG_DEBUG("Test_Task JOBs time %u", allJobsTimeEnd - allJobsTimeStart);
           for (u32 i = 0; i < numJobs; ++i)
           {
               ++jobFinished[i];
           }
           std::this_thread::sleep_for(std::chrono::milliseconds(100));

           auto job = [&jobNTime, &jobNFinished]() -> void
               {
                   u64 value = 0;
                   u64 maxValue = 1'000'000'000;
                   auto start = utils::Timer::getCurrentTime();
                   while (value < maxValue)
                   {
                       ++value;
                   }
                   auto end = utils::Timer::getCurrentTime();
                   jobNTime = end - start;
                   jobNFinished = true;
               };

           scheduler.executeTask(task::TaskPriority::Normal, task::TaskMask::WorkerThread, job);
       }

       if (jobNFinished)
       {
           LOG_DEBUG("Test_Task JobN is finshed time %u", jobNTime);
           jobNFinished = false;
       }

       scheduler.mainThreadLoop();
    }


    [[maybe_unused]] u32 endTest = 0;
}

void MyApplication::Test_Windows()
{
}

void MyApplication::Test_MemoryPool()
{
    //{
    //    MemoryPool pool;

    //    char* a = (char*)pool.getMemory(253);
    //    memset(a, 'a', 253);
    //    u64 ofsa = pool.getOffsetInBlock(a);
    //    char* b = (char*)pool.getMemory(253);
    //    memset(b, 'b', 253);
    //    u64 ofsb = pool.getOffsetInBlock(b);
    //    char* c = (char*)pool.getMemory(253);
    //    memset(c, 'c', 253);
    //    u64 ofsc = pool.getOffsetInBlock(c);
    //    char* d = (char*)pool.getMemory(253);
    //    memset(d, 'd', 253);
    //    u64 ofsd = pool.getOffsetInBlock(d);
    //    char* e = (char*)pool.getMemory(253);
    //    memset(e, 'e', 253);
    //    u64 ofse = pool.getOffsetInBlock(e);
    //    char* f = (char*)pool.getMemory(253);
    //    memset(f, 'f', 10);
    //    [[maybe_unused]] u64 ofsf = pool.getOffsetInBlock(f);
    //    ////

    //    pool.freeMemory((void*)b);
    //    [[maybe_unused]] char* nb = (char*)pool.getMemory(253);
    //    memset(b, 'B', 253);
    //    [[maybe_unused]] u64 nofsb = pool.getOffsetInBlock(b);

    //    void* hugeData = pool.getMemory(1024 * 1024 * 4);
    //    [[maybe_unused]]  u64 ofhugeData = pool.getOffsetInBlock(hugeData);

    //    void* hugeData1 = pool.getMemory(1024 * 1024 * 40);
    //    [[maybe_unused]] u64 ofhugeData1 = pool.getOffsetInBlock(hugeData1);

    //    pool.freeMemory(hugeData1);

    //    void* hugeData2 = pool.getMemory(1024 * 1024 * 400);
    //    [[maybe_unused]] u64 ofhugeData2 = pool.getOffsetInBlock(hugeData2);

    //    pool.clearPools();
    //}

    //{
    //    auto randomFunc = [](u32 min, u32 max) -> u32
    //    {
    //        std::srand(u32(std::time(nullptr)));
    //        s32 rnd = std::rand();
    //        return (u32)(rnd % (max - min + 1) + min);
    //     };


    //    MemoryPool pool(2048, 32, 32, utils::MemoryPool::getDefaultMemoryPoolAllocator());
    //    std::vector<std::pair<u32, void*>> sizes;

    //    std::random_device rd;
    //    std::mt19937 g(rd());

    //    for (u32 e = 0; e < 100; ++e)
    //    {
    //        std::uniform_int_distribution<u32> uid(1, 512);
    //        for (u32 i = 0; i < 1000; ++i)
    //        {
    //            u32 rendomSize = uid(g);
    //            void* ptr = pool.getMemory(rendomSize);
    //            sizes.push_back(std::make_pair(rendomSize, ptr));
    //        }

    //        std::shuffle(sizes.begin(), sizes.end(), g);

    //        for (u32 i = 0; i < 1000; ++i)
    //        {
    //            void* ptr = sizes[i].second;
    //            pool.freeMemory(ptr);
    //        }
    //    }

    //    //pool.clearPools();
    //}
}

void MyApplication::Test_ShaderLoader()
{
    auto checkVertexShaderReflection = [](const Shader::Resources& info)
    {
        ASSERT(info._inputAttribute.size() == 3, "wrong");
        ASSERT(info._inputAttribute[0]._location == 0, "wrong");
        ASSERT(info._inputAttribute[0]._format == Format::Format_R32G32B32_SFloat, "wrong");
        ASSERT(info._inputAttribute[1]._location == 1, "wrong");
        ASSERT(info._inputAttribute[1]._format == Format::Format_R32G32B32A32_SFloat, "wrong");
        ASSERT(info._inputAttribute[2]._location == 2, "wrong");
        ASSERT(info._inputAttribute[2]._format == Format::Format_R32G32_SFloat, "wrong");

        ASSERT(info._outputAttribute.size() == 3, "wrong");
        ASSERT(info._outputAttribute[0]._location == 0, "wrong");
        ASSERT(info._outputAttribute[0]._format == Format::Format_R32G32B32A32_SFloat, "wrong");
        ASSERT(info._outputAttribute[1]._location == 1, "wrong");
        ASSERT(info._outputAttribute[1]._format == Format::Format_R32G32_SFloat, "wrong");
        ASSERT(info._outputAttribute[2]._location == 2, "wrong");
        ASSERT(info._outputAttribute[2]._format == Format::Format_R32G32B32_SFloat, "wrong");

        ASSERT(info._uniformBuffers.size() == 3, "wrong");
        ASSERT(info._uniformBuffers[0]._id == 1, "wrong");
        ASSERT(info._uniformBuffers[0]._set == 0, "wrong");
        ASSERT(info._uniformBuffers[0]._binding == 0, "wrong");
        ASSERT(info._uniformBuffers[0]._array == 1, "wrong");
        ASSERT(info._uniformBuffers[0]._size == 64, "wrong");
        ASSERT(info._uniformBuffers[0]._name == "ubo00_size64", "wrong");
        ASSERT(info._uniformBuffers[0]._uniforms.size() == 1, "wrong");
        {
            ASSERT(info._uniformBuffers[0]._uniforms[0]._bufferID == 1, "wrong");
            ASSERT(info._uniformBuffers[0]._uniforms[0]._array == 1, "wrong");
            ASSERT(info._uniformBuffers[0]._uniforms[0]._type == DataType::Matrix4, "wrong");
            ASSERT(info._uniformBuffers[0]._uniforms[0]._size == 64, "wrong");
            ASSERT(info._uniformBuffers[0]._uniforms[0]._offset == 0, "wrong");
            //ASSERT(info._uniformBuffers[0]._uniforms[0]._name == "matrix0_00", "wrong");
        }
        ASSERT(info._uniformBuffers[1]._id == 0, "wrong");
        ASSERT(info._uniformBuffers[1]._set == 0, "wrong");
        ASSERT(info._uniformBuffers[1]._binding == 1, "wrong");
        ASSERT(info._uniformBuffers[1]._array == 1, "wrong");
        ASSERT(info._uniformBuffers[1]._size == 192, "wrong");
        ASSERT(info._uniformBuffers[1]._name == "ubo01_size192", "wrong");
        ASSERT(info._uniformBuffers[1]._uniforms.size() == 3, "wrong");
        {
            ASSERT(info._uniformBuffers[1]._uniforms[0]._bufferID == 0, "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[0]._array == 1, "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[0]._type == DataType::Matrix4, "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[0]._size == 64, "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[0]._offset == 0, "wrong");
            //ASSERT(info._uniformBuffers[1]._uniforms[0]._name == "matrix0_01", "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[1]._bufferID == 0, "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[1]._array == 1, "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[1]._type == DataType::Matrix4, "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[1]._size == 64, "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[1]._offset == 64, "wrong");
            //ASSERT(info._uniformBuffers[1]._uniforms[1]._name == "matrix1_01", "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[2]._bufferID == 0, "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[2]._array == 1, "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[2]._type == DataType::Matrix4, "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[2]._size == 64, "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[2]._offset == 128, "wrong");
            //ASSERT(info._uniformBuffers[1]._uniforms[2]._name == "matrix2_01", "wrong");
        }
        ASSERT(info._uniformBuffers[2]._id == 2, "wrong");
        ASSERT(info._uniformBuffers[2]._set == 1, "wrong");
        ASSERT(info._uniformBuffers[2]._binding == 1, "wrong");
        ASSERT(info._uniformBuffers[2]._array == 1, "wrong");
        ASSERT(info._uniformBuffers[2]._size == 256, "wrong");
        ASSERT(info._uniformBuffers[2]._name == "ubo11_size256", "wrong");
        ASSERT(info._uniformBuffers[2]._uniforms.size() == 2, "wrong");
        {
            ASSERT(info._uniformBuffers[2]._uniforms[0]._bufferID == 2, "wrong");
            ASSERT(info._uniformBuffers[2]._uniforms[0]._array == 2, "wrong");
            ASSERT(info._uniformBuffers[2]._uniforms[0]._type == DataType::Matrix4, "wrong");
            ASSERT(info._uniformBuffers[2]._uniforms[0]._size == 64 * 2, "wrong");
            ASSERT(info._uniformBuffers[2]._uniforms[0]._offset == 0, "wrong");
            //ASSERT(info._uniformBuffers[2]._uniforms[0]._name == "matrix0_11", "wrong");
            ASSERT(info._uniformBuffers[2]._uniforms[1]._bufferID == 2, "wrong");
            ASSERT(info._uniformBuffers[2]._uniforms[1]._array == 2, "wrong");
            ASSERT(info._uniformBuffers[2]._uniforms[1]._type == DataType::Matrix4, "wrong");
            ASSERT(info._uniformBuffers[2]._uniforms[1]._size == 64 * 2, "wrong");
            ASSERT(info._uniformBuffers[2]._uniforms[1]._offset == 64 * 2, "wrong");
            //ASSERT(info._uniformBuffers[2]._uniforms[1]._name == "matrix1_11", "wrong");
        }
    };

    auto checkFragmentShaderReflection = [](const Shader::Resources& info)
    {
        ASSERT(info._inputAttribute.size() == 3, "wrong");
        ASSERT(info._inputAttribute[0]._location == 0, "wrong");
        ASSERT(info._inputAttribute[0]._format == Format::Format_R32G32B32A32_SFloat, "wrong");
        ASSERT(info._inputAttribute[1]._location == 1, "wrong");
        ASSERT(info._inputAttribute[1]._format == Format::Format_R32G32_SFloat, "wrong");
        ASSERT(info._inputAttribute[2]._location == 2, "wrong");
        ASSERT(info._inputAttribute[2]._format == Format::Format_R32G32B32_SFloat, "wrong");

        ASSERT(info._outputAttribute.size() == 1, "wrong");
        ASSERT(info._outputAttribute[0]._location == 0, "wrong");
        ASSERT(info._outputAttribute[0]._format == Format::Format_R32G32B32A32_SFloat, "wrong");

        ASSERT(info._samplers.size() == 1, "wrong");
        ASSERT(info._samplers[0]._set == 0, "wrong");
        ASSERT(info._samplers[0]._binding == 0, "wrong");
        ASSERT(info._samplers[0]._name == "samplerColor00", "wrong");

        ASSERT(info._images.size() == 3, "wrong");
        ASSERT(info._images[0]._set == 0, "wrong");
        ASSERT(info._images[0]._binding == 1, "wrong");
        ASSERT(info._images[0]._array == 1, "wrong");
        ASSERT(info._images[0]._target == TextureTarget::Texture2D, "wrong");
        ASSERT(info._images[0]._ms == false, "wrong");
        ASSERT(info._images[0]._depth == false, "wrong");
        ASSERT(info._images[0]._name == "textureColor01", "wrong");
        ASSERT(info._images[1]._set == 0, "wrong");
        ASSERT(info._images[1]._binding == 2, "wrong");
        ASSERT(info._images[1]._array == 1, "wrong");
        ASSERT(info._images[1]._target == TextureTarget::Texture2D, "wrong");
        ASSERT(info._images[1]._ms == false, "wrong");
        ASSERT(info._images[1]._depth == false, "wrong");
        ASSERT(info._images[1]._name == "textureColor02", "wrong");
        ASSERT(info._images[2]._set == 1, "wrong");
        ASSERT(info._images[2]._binding == 0, "wrong");
        ASSERT(info._images[2]._array == 1, "wrong");
        ASSERT(info._images[2]._target == TextureTarget::Texture2D, "wrong");
        ASSERT(info._images[2]._ms == false, "wrong");
        ASSERT(info._images[2]._depth == false, "wrong");
        ASSERT(info._images[2]._name == "textureColor10", "wrong");
    };

    //vulkan only
    //if (m_Device->getRenderType() == Device::RenderType::Vulkan)
    //{
    //    //load source shaders from file
    //    const Shader* glslVShader = ResourceManager::getInstance()->loadShader<Shader, ShaderSourceFileLoader>(m_Device, "examples/test/data/shaders/testReflectInfoWithNames.vert");
    //    ASSERT(glslVShader != nullptr, "wrong");
    //    {
    //        const Shader::Resources& info = glslVShader->getReflectionInfo();
    //        checkVertexShaderReflection(info);
    //    }

    //    const Shader* glslFShader = ResourceLoaderManager::getInstance()->loadShader<Shader, ShaderSourceFileLoader>(m_Device, "examples/test/data/shaders/testReflectInfoWithNames.frag");
    //    ASSERT(glslFShader != nullptr, "wrong");
    //    {
    //        const Shader::Resources& info = glslFShader->getReflectionInfo();
    //        checkFragmentShaderReflection(info);
    //    }

    //    ResourceManager::getInstance()->remove(glslVShader);
    //    ResourceManager::getInstance()->remove(glslFShader);

    //    //load spirv
    //    const Shader* spirvVShader = ResourceManager::getInstance()->loadShader<Shader, ShaderBinaryFileLoader>(m_Device, "examples/test/data/shaders/testReflectInfoWithNames.vspv");
    //    ASSERT(spirvVShader != nullptr, "wrong");
    //    {
    //        const Shader::Resources& info = spirvVShader->getReflectionInfo();
    //        checkVertexShaderReflection(info);
    //    }

    //    const Shader* spirvFShader = ResourceManager::getInstance()->loadShader<Shader, ShaderBinaryFileLoader>(m_Device, "examples/test/data/shaders/testReflectInfoWithNames.fspv");
    //    ASSERT(spirvFShader != nullptr, "wrong");
    //    {
    //        const Shader::Resources& info = spirvFShader->getReflectionInfo();
    //        checkFragmentShaderReflection(info);
    //    }

    //    ResourceManager::getInstance()->remove(spirvVShader);
    //    ResourceManager::getInstance()->remove(spirvFShader);
    //}

    //{
    //    const Shader* hlslVShader = ResourceManager::getInstance()->loadShader<Shader, ShaderSourceFileLoader>(m_Device, "examples/test/data/shaders/testReflectInfoWithNames.vs");
    //    ASSERT(hlslVShader != nullptr, "wrong");
    //    {
    //        const Shader::Resources& info = hlslVShader->getReflectionInfo();
    //        checkVertexShaderReflection(info);
    //    }

    //    const Shader* hlslPShader = ResourceManager::getInstance()->loadShader<Shader, ShaderSourceFileLoader>(m_Device, "examples/test/data/shaders/testReflectInfoWithNames.ps");
    //    ASSERT(hlslPShader != nullptr, "wrong");
    //    {
    //        const Shader::Resources& info = hlslPShader->getReflectionInfo();
    //        checkFragmentShaderReflection(info);
    //    }

    //    ResourceManager::getInstance()->remove(hlslVShader);
    //    ResourceManager::getInstance()->remove(hlslPShader);
    //}

    //{
 /*       std::vector<const Shader*> hlslShaders = ResourceManager::getInstance()->loadHLSLShader<Shader, ShaderSourceFileLoader>(m_Device, "examples/test/data/shaders/testReflectInfoWithNames.hlsl", { {"mainVS", ShaderType::ShaderType_Vertex}, {"mainPS", ShaderType::ShaderType_Fragment} });
        ASSERT(!hlslShaders.empty(), "wrong");

        {
            ASSERT(hlslShaders[0] != nullptr, "wrong");
            const Shader::Resources& info = hlslShaders[0]->getReflectionInfo();
            checkVertexShaderReflection(info);
        }

        {
            ASSERT(hlslShaders[1] != nullptr, "wrong");
            const Shader::Resources& info = hlslShaders[1]->getReflectionInfo();
            checkFragmentShaderReflection(info);
        }

        ResourceManager::getInstance()->remove(hlslShaders[0]);
        ResourceManager::getInstance()->remove(hlslShaders[1]);
        hlslShaders.clear();*/
    //}

    [[maybe_unused]] u32 endTest = 0;
}

void MyApplication::Test_CreateShaderProgram()
{
 /*   {
        const Shader* vertShader = nullptr;
        {
            const std::string vertexSource("\
        struct VS_INPUT\n\
        {\n\
            float3 Position : POSITION;\n\
            float3 Color : COLOR;\n\
        };\n\
        \n\
        struct VS_OUTPUT\n\
        {\n\
            float4 Pos : SV_POSITION;\n\
            float4 Col : COLOR;\n\
        };\n\
        \n\
        cbuffer ConstantBuffer\n\
        {\n\
            matrix projectionMatrix;\n\
            matrix modelMatrix;\n\
            matrix viewMatrix;\n\
        };\n\
        \n\
        VS_OUTPUT main(VS_INPUT Input)\n\
        {\n\
            VS_OUTPUT Out;\n\
            Out.Pos = mul(modelMatrix, float4(Input.Position, 1.0));\n\
            Out.Pos = mul(viewMatrix, Out.Pos);\n\
            Out.Pos = mul(projectionMatrix, Out.Pos);\n\
            Out.Col = float4(Input.Color, 1.0);\n\
            return Out;\n\
        }");
            const stream::Stream* vertexStream = stream::StreamManager::createMemoryStream(vertexSource);

            ShaderHeader vertexHeader(ShaderType::ShaderType_Vertex);
            vertexHeader._contentType = ShaderHeader::ShaderResource::ShaderResource_Source;
            vertexHeader._shaderModel = renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_5_0;

            vertShader = resource::ResourceLoaderManager::getInstance()->composeShader<Shader, resource::ShaderSourceStreamLoader>(m_Context, "vertex", &vertexHeader, vertexStream);
            ASSERT(vertShader, "nullptr");
        }

        const Shader* fragShader = nullptr;
        {
            const std::string fragmentSource("\
        struct PS_INPUT\n\
        {\n\
            float4 Pos : SV_POSITION;\n\
            float4 Col : COLOR;\n\
        };\n\
        \n\
        float4 main(PS_INPUT Input) : SV_TARGET0\n\
        {\n\
            return Input.Col;\n\
        }");
            const stream::Stream* fragmentStream = stream::StreamManager::createMemoryStream(fragmentSource);

            ShaderHeader fragmentHeader(ShaderType::ShaderType_Fragment);
            fragmentHeader._contentType = ShaderHeader::ShaderResource::ShaderResource_Source;
            fragmentHeader._shaderModel = renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_5_0;

            fragShader = resource::ResourceLoaderManager::getInstance()->composeShader<Shader, resource::ShaderSourceStreamLoader>(m_Context, "fragment", &fragmentHeader, fragmentStream);
            ASSERT(fragShader, "nullptr");
        }


        CommandList* commandList = new renderer::CommandList(m_Context, CommandList::CommandListType::ImmediateCommandList);
        ASSERT(commandList, "nullptr");

        ShaderProgram* singleProgram = commandList->createObject<renderer::ShaderProgram>(std::vector<const Shader*>({ vertShader }));
        ASSERT(singleProgram, "wrong");
        renderer::ShaderProgram* twoShaderProgram = commandList->createObject<ShaderProgram, std::vector<const Shader*>>({ {vertShader, fragShader} });
        ASSERT(twoShaderProgram, "wrong");

        delete singleProgram;
        delete twoShaderProgram;

        delete commandList;

        ResourceManager::getInstance()->remove(vertShader);
        ResourceManager::getInstance()->remove(fragShader);
    }*/
}

void MyApplication::Test_ShaderParam()
{
 /*   {
        const Shader* hlslVShader = ResourceManager::getInstance()->loadShader<Shader, ShaderSourceFileLoader>(m_Context, "examples/test/data/shaders/testReflectInfoWithNames.vs");
        ASSERT(hlslVShader != nullptr, "wrong");

        const Shader* hlslPShader = ResourceManager::getInstance()->loadShader<Shader, ShaderSourceFileLoader>(m_Context, "examples/test/data/shaders/testReflectInfoWithNames.ps");
        ASSERT(hlslPShader != nullptr, "wrong");

        CommandList* commandList = new CommandList(m_Context, CommandList::CommandListType::ImmediateCommandList);
        ASSERT(commandList, "nullptr");

        ShaderProgram* shaderProgram = commandList->createObject<ShaderProgram, std::vector<const Shader*>>({ {hlslVShader, hlslPShader} });
        ASSERT(shaderProgram, "shaderProgram");

        {
            struct UBO11_size256
            {
                core::Matrix4D matrix0_11[2];
                core::Matrix4D matrix1_11[2];
            } ubo11_size256;
            ASSERT(sizeof(ubo11_size256) == 256, "wrong");

            ShaderParam param256("ubo11_size256");
            ASSERT(param256._id == crc32c::Crc32c(std::string("ubo11_size256")), "wrong");
            shaderProgram->bindUniformsBuffer<ShaderType::ShaderType_Vertex>(param256, 0, sizeof(UBO11_size256), & ubo11_size256);

            struct UBO01_size192
            {
                core::Matrix4D matrix0_01;
                core::Matrix4D matrix1_01;
                core::Matrix4D matrix2_01;
            } ubo01_size192;
            ASSERT(sizeof(ubo01_size192) == 192, "wrong");

            ShaderParam param192("ubo01_size192");
            ASSERT(param192._id == crc32c::Crc32c(std::string("ubo01_size192")), "wrong");
            shaderProgram->bindUniformsBuffer<ShaderType::ShaderType_Vertex>(param192, 0, sizeof(UBO01_size192), &ubo01_size192);
        }

        {
            Texture2D* texture = commandList->createObject<Texture2D>(TextureUsage::TextureUsage_Sampled, Format::Format_R32G32B32A32_SFloat, core::Dimension2D(1,1));
            ASSERT(texture, "nullptr");

            SamplerState* sampler = commandList->createObject<SamplerState>();

            ShaderParam textureColor01("textureColor01");
            ASSERT(textureColor01._id == crc32c::Crc32c(std::string("textureColor01")), "wrong");
            ShaderParam textureColor10("textureColor10");
            ASSERT(textureColor10._id == crc32c::Crc32c(std::string("textureColor10")), "wrong");
            ShaderParam samplerColor00("samplerColor00");
            ASSERT(samplerColor00._id == crc32c::Crc32c(std::string("samplerColor00")), "wrong");

            shaderProgram->bindTexture<ShaderType::ShaderType_Fragment>(textureColor01, texture);
            shaderProgram->bindTexture<ShaderType::ShaderType_Fragment>(textureColor10, texture);
            shaderProgram->bindSampler<ShaderType::ShaderType_Fragment>(samplerColor00, sampler);

            delete sampler;
            delete texture;
        }
        delete shaderProgram;

        delete commandList;

        ResourceManager::getInstance()->remove(hlslVShader);
        ResourceManager::getInstance()->remove(hlslPShader);
    }*/
}

void MyApplication::Test_CreatePipeline()
{
    //TODO
}

MyApplication::~MyApplication()
{
}
