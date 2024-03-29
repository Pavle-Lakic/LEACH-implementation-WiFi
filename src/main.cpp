/** @file main.cpp
 *  @brief
 *  
 *  This file represents baremetal implementation 
 *  of node for LEACH protocol
 *   
 *  @author Pavle Lakic
 *  @bug No known bugs
 */
#include <Arduino.h>
#include "includes.h"

Node_s Node;

void setup() {

    timer1_enable(TIM_DIV256, TIM_EDGE, TIM_SINGLE);
    timer1_write(8388607);

    WiFi.disconnect();
    WiFi.forceSleepBegin(); // turn off WiFi by default.
    WiFi.persistent(false);
    uint16_t round;
    uint8_t ch_enable;

    pinMode(LED_BUILTIN, OUTPUT);
    // by default LED will be ON
    digitalWrite(LED_BUILTIN, LOW);

#if DEBUG
    Serial.begin(115200);
    delay(10);
    Serial.println();
#endif

    if (mount_fs()) {

#if ROUNDS_RESET
        write_fs(0, 1);
#endif

    read_fs(&round, &ch_enable);
    }

    Node.P = 1.0/NUMBER_OF_ROUNDS;
    Node.round = round;
    Node.ch_enable = ch_enable;
    init_node_name(&Node);

#if DEBUG
    Serial.printf("Beggining of new round!\r\nround = %hu\r\nch_enable = %d\r\n", Node.round, Node.ch_enable);
    Serial.printf("Node MAC = %02X:%02X:%02X:%02X:%02X:%02X\r\n", Node.nodeName[0], Node.nodeName[1], Node.nodeName[2], Node.nodeName[3], Node.nodeName[4], Node.nodeName[5]);
#endif

    mode_decision(&Node);

    if (Node.cluster_head == true) {
        digitalWrite(LED_BUILTIN, LOW);
    }
    else {
        digitalWrite(LED_BUILTIN, HIGH);
    }

#if DEBUG
    Serial.println("Mode of work decided!");
    if (Node.cluster_head == true) {
        Serial.println("I`m cluster head for current round!");
    }
    else {
        Serial.println("I`m station for current round!");
    }
#endif

    handle_node(&Node);
    prepare_next_round(&Node);
    delay(200); // wait for UDP to be sent.
    sleeping_time(&Node);
}

void loop() {

}