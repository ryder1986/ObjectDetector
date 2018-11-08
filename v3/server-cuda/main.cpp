#include <iostream>
#include<configmanager.h>
#include "app.h"
#include <thread>

//#include "X11/Xlib.h"
using namespace std;

int main()
{
  //  XInitThreads();

    cout << "Hello World!" << endl;

    ConfigManager cfg;
    App app(&cfg);

    while(true){
        sleep(1000);
    }
    return 0;
}
