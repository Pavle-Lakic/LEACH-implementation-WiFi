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

pinMode(LED_BUILTIN, OUTPUT);
// by default LED will be OFF
digitalWrite(LED_BUILTIN, HIGH);

#if DEBUG
    Serial.begin(9600);
    delay(10);
    Serial.println();
#endif

init_node_name(&Node);

#if DEBUG
    Serial.printf("MAC address = %02X:%02X:%02X:%02X:%02X:%02X\n", Node.nodeName[0], Node.nodeName[1], Node.nodeName[2], Node.nodeName[3], Node.nodeName[4], Node.nodeName[5]);
#endif

}

void loop() {
  // put your main code here, to run repeatedly:
}