// CreateWindow.cpp : Defines the entry point for the console application.
//

#include "MyApplication.h"

int main(int argc, char* argv[])
{
    MyApplication* application = new MyApplication(argc, argv);
    return application->Execute();
}
