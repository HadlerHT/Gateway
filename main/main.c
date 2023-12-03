#include <stdio.h>

#include "wifimanager.h"
#include "mqttclient.h"

void app_main(void) {

    wifi_espSetup();
    wifi_initializeStation();

    mqtt_clientStart();        

};