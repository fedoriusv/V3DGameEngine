#pragma once

class MyApplication
{
public:

    MyApplication(int& argc, char** argv);
    ~MyApplication();

    int Execute();

private:

    void Test_Timer();
    void Test_MemoryPool();
    void Test_Thread();
    void Test_TaskContainters();
    void Test_Task();
    void Test_Windows();

    void Test_ImageLoadStore();

    void Test_ShaderLoader();
    void Test_CreateShaderProgram();
    void Test_ShaderParam();
    void Test_CreatePipeline();
};
