/** @file functions.cpp
 *  @brief
 *  
 *  This file contains relevant defines, structures and
 *  functions that main.cpp will call.
 *  
 *  @author Pavle Lakic
 *  @bug No known bugs
 */

#include "includes.h"

void init_node_name (Node_s * node)
{
    wifi_get_macaddr(STATION_IF, node->nodeName);
}

float random_number(void)
{
  float a;

  a = ESP8266TrueRandom.random(10000);
  a = a/10000;

#if DEBUG
  Serial.print("random_number = ");
  Serial.println(a, 3);
#endif

  return a;
}