/** @file node.ino
 *  @brief
 *  
 *  This file represents baremetal implementation 
 *  of node for LEACH protocol
 *   
 *  @author Pavle Lakic
 *  @bug No known bugs
 */
#include <Arduino.h>

void setup() {

pinMode(LED_BUILTIN, OUTPUT);

#if DEBUG
  Serial.begin(9600);
  delay(10);
  Serial.println();
  Serial.println(WiFi.macAddress());
#endif
}

void loop() {
  // put your main code here, to run repeatedly:
}