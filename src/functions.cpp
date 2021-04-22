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


void mode_decision(Node_s* node)
{
    float rnd_numb;
    float T;

    rnd_numb = random_number();
    T = calculate_threshold(node);

#if DEBUG
    Serial.print("Generated random number = ");
    Serial.println(rnd_numb);
#endif

    if ((rnd_numb < T) && (node->ch_enable == 1)) {
        node->cluster_head = true;
    }
    else {
        node->cluster_head = false;
    }

}

bool mount_fs(void)
{ 
  bool success = LittleFS.begin();
  if (!success){
#if DEBUG
    Serial.println("Could not mount SPIFFS!");
#endif
  }
  return success;
}

void write_fs(uint16_t round, uint8_t ch_enable)
{
    uint8_t round_str[sizeof(uint16_t)*8 + 1];
    uint8_t ch_enable_str[sizeof(uint8_t)*8 +1];

    File fp = LittleFS.open(FILENAME, "w");

    if (!fp) {
#if DEBUG
        Serial.printf("Could not open %s to write!\n", FILENAME);
#endif        
    }
    else {
        utoa(round, (char*)round_str, 10);
        utoa(ch_enable, (char*)ch_enable_str, 10);
        fp.write(round_str, sizeof(round_str));
        fp.write(ch_enable_str, sizeof(ch_enable_str));
        fp.close();
    }
}

void read_fs(uint16_t* round, uint8_t* ch_enable)
{
    uint8_t round_str[sizeof(uint16_t)*8 + 1];
    uint8_t ch_enable_str[sizeof(uint8_t)*8 +1];
    File fp = LittleFS.open(FILENAME, "r");
    if (!fp) {
#if DEBUG
        Serial.printf("Could not open %s to read!\n", FILENAME);
#endif        
    }
    else {
        fp.read(round_str, sizeof(round_str));
        fp.read(ch_enable_str, sizeof(ch_enable_str));
        fp.close();
        *round = atoi((char*)round_str);
        *ch_enable = atoi((char*)ch_enable_str);
    }
}

void init_node_name (Node_s* node)
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

float calculate_threshold(Node_s* node)
{
    float T;

    T = node->P/(1 - node->P * (node->round % ((unsigned char)round(1/node->P))));

#if DEBUG
    Serial.print("Current round in calculate_threshold = ");
    Serial.println(node->round);
    Serial.print("T = ");
    Serial.println(T, 3);
#endif
  
    return T;
}