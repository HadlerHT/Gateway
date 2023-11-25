#include "wifimanager.h"

extern "C" void app_main(void) {

    WiFiManager wifiManager;
    wifiManager.connect();

}

