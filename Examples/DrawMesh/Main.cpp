// Main.cpp : Defines the entry point for the console application.
//

#include "MyApplication.h"

int main(int argc, char* argv[])
{
    app::MyApplication* application = new app::MyApplication(argc, argv);
    return application->Execute();
}
