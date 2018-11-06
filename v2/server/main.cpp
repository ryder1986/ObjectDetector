#include <iostream>
#include<configmanager.h>
#include "app.h"
#include <thread>
using namespace std;

int main()
{
    cout << "Hello World!" << endl;

    ConfigManager cfg;
    App app(&cfg);

    while(true){
        sleep(1000);
    }
    return 0;
}
