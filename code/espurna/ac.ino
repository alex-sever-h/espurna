/*

AC MODULE

Copyright (C) 2017 by Alexandru-Sever Horin <alex.sever.h@gmail.com>

*/

#if ENABLE_AC

#include <IRremoteESP8266.h>
#include <ir_Yamato.h>

const uint16_t RECV_PIN = 2;
const uint16_t SEND_PIN = 5;

IRYamato iryamato(RECV_PIN, SEND_PIN);

// -----------------------------------------------------------------------------
// AC
// -----------------------------------------------------------------------------
void acLoopDo()
{
    std::list<String> acProps = iryamato.getProps();

    for(std::list<String>::iterator it = acProps.begin(); it != acProps.end(); ++it)
    {
        String prop = *it;
        String value = iryamato.get(prop);
        std::list<String> propList = iryamato.list(prop);
        DEBUG_MSG_P(PSTR("[AC] %s Value: %s\n"), prop.c_str(), value.c_str());

        // Send MQTT messages
        mqttSend(getSetting("acTopic", MQTT_TOPIC_AC).c_str(),
                 prop.c_str(), value.c_str());
        for(std::list<String>::iterator jt = propList.begin(); jt != propList.end(); ++jt)
        {
            mqttSend(getSetting("acTopic", MQTT_TOPIC_AC).c_str(),
                     prop.c_str(), "choices", jt->c_str());
        }
    }
#if 0
    // Update websocket clients
    char buffer[100];
    sprintf_P(buffer, PSTR("{\"acVisible\": 1, \"acValue\": %s}"), powerState.c_str());
    wsSend(buffer);
#endif
}

void acMQTTCallback(unsigned int type, const char * topic, const char * payload) {
    if (type == MQTT_CONNECT_EVENT) {

        char buffer[strlen(MQTT_TOPIC_AC) + 3];
        sprintf(buffer, "%s/+", MQTT_TOPIC_AC);
        mqttSubscribe(buffer);

        DEBUG_MSG_P(PSTR("Subscribet to topic %s \n"), buffer );

        acLoopDo();
    }

    if (type == MQTT_MESSAGE_EVENT) {

        DEBUG_MSG_P(PSTR("Received topic %s payload %s\n"), topic, payload)

;

        // Match topic
        String t = mqttSubtopic((char *) topic);
        if (!t.startsWith(MQTT_TOPIC_AC)) return;

        String acFunc = t.substring(strlen(MQTT_TOPIC_AC) + 1);
        String acCommand = String(payload);

        Serial.println("Function " + acFunc + " command " + acCommand );

        iryamato.set(acFunc, acCommand);
    }

}

void acSetupMQTT() {
    mqttRegister(acMQTTCallback);
}

void acSetup() {

    apiRegister(MQTT_TOPIC_AC, MQTT_TOPIC_AC, [](char * buffer, size_t len) {
        "";
    });

    acSetupMQTT();

    // Main callbacks
    espurnaRegisterLoop(acLoop);
    #espurnaRegisterReload([]() { _ntp_configure = true; });

}

void acLoop() {
    // Check if we should read new data
    static unsigned long last_update = 0;
    if ((millis() - last_update > AC_UPDATE_INTERVAL) || (last_update == 0)) {
        last_update = millis();
        acLoopDo();
    }
}

#endif

// Have no idea where to put 'em
//    #elif defined(ESPURNA_AC)
//        setSetting("board", 25);
//        setSetting("irLedGPIO", 5, 1);
//        setSetting("irLedLogic", 1, 1);
//        setSetting("irSenseGPIO", 2, 1);
//        setSetting("irSenseLogic", 1, 1);
//
//    #else
