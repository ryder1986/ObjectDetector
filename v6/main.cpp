#include <iostream>
#include <thread>
#include "app.h"
#include "configmanager.h"
#include "jsonpacket.h"
#ifdef IS_UNIX
//#include "X11/Xlib.h"
#endif
using namespace std;

int main()
{
#ifdef IS_UNIX
  //  XInitThreads();
#endif
    cout << "Hello World!" << endl;

    ConfigManager cfg;
    App app;
   // cfg.get_config()
    PAUSE_HERE_FOREVER
    return 0;
}
