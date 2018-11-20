#include "tool.h"
#include "configmanager.h"
#include "test.h"
int main()
{
    prt(info,"start");
    ConfigManager cm;
    prt(info,"%s",cm.get_config().str().data());
    PAUSE_HERE_FOREVER
    return 0;
}
