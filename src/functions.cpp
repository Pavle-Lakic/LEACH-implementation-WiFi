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



void send_packet_to_ap(Node_s* node)
{
    WiFiUDP Udp;
    IPAddress dnsAddress;
    dnsAddress = WiFi.dnsIP();
    char node_name[40] = {0};
    char upper_nibla;
    char lower_nibla;
    char upper_nibla_string[2];
    char lower_nibla_string[2];

    for (int i = 0; i < 6; i++) {
        lower_nibla = node->nodeName[i] & 0x0F;
        upper_nibla = (node->nodeName[i] & 0xF0) >> 4;
        sprintf(upper_nibla_string, "%X", upper_nibla);
        sprintf(lower_nibla_string, "%X", lower_nibla);
        strcat(node_name, upper_nibla_string);
        strcat(node_name, lower_nibla_string);
    }
    //strcat(node_name, "\r\n");

Serial.print("NODE NAME = ");
Serial.println(node_name);
Serial.println(strlen(node_name));



/*
    Udp.beginPacket(dnsAddress, UDP_BROADCAST_PORT);
    Udp.write(";");
    Udp.write(node->nodeName);
    Udp.write()
*/
}

void get_adc_value(Node_s* node)
{
    node->adc_value = analogRead(ADC_PIN);
}

int connect_to_strongest_ssid(Node_s* node)
{
    int ret = FAILED_TO_CONNECT;
    unsigned long start;

    WiFi.mode(WIFI_STA);
    WiFi.begin(String(node->strongest_ssid), NODE_PASS);

#if DEBUG
    Serial.print("Connecting to ");
    Serial.print(node->strongest_ssid);
    Serial.println("...");
#endif
    start = millis();

    while (WiFi.status() != WL_CONNECTED && ((millis() - start) < CONNECTION_TIMEOUT)) {
        delay(20);
    }

    if ((millis() - start) > CONNECTION_TIMEOUT) {
        ret = CONNECTION_TIMEOUT;
    } 

    if (WiFi.status() == WL_CONNECTED) {
        ret = CONNECTED;
    }

    return ret;
}

bool ssid_is_valid(const char* txt)
{
    bool ret = false;

    if (strlen(txt) == 12) {
        if ((isxdigit(txt[0]) && isxdigit(txt[1]) && isxdigit(txt[2]) &&
            isxdigit(txt[3]) && isxdigit(txt[4]) && isxdigit(txt[5]) &&
            isxdigit(txt[6]) && isxdigit(txt[7]) && isxdigit(txt[8]) &&
            isxdigit(txt[9]) && isxdigit(txt[10]) && isxdigit(txt[11]))
            || (strcmp(txt, BASE_SSID) == 0)) {
                ret = true;
            }
    }

    return ret;
}

int find_strongest_connection(Node_s* node)
{
    int ret;
    int n = 0;
    int power = -100;
    char cmp_ssid[20] = "Not valid";

    strncpy(node->strongest_ssid, cmp_ssid, strlen(cmp_ssid));
    WiFi.forceSleepWake();
    n = WiFi.scanNetworks();

    if (n == 0) {

#if DEBUG
    Serial.println("No networks found!");
#endif

    ret = NO_NETWORKS_FOUND;
    }
    else {
        for (int i = 0; i < n; i++) {
            if (ssid_is_valid(WiFi.SSID(i).c_str())) {
                if (WiFi.RSSI(i) > power) {
                    power = WiFi.RSSI(i);
                    strncpy(node->strongest_ssid, WiFi.SSID(i).c_str(), strlen(WiFi.SSID(i).c_str()));
                    ret = VALID_SSID_FOUND;
                }
            }
            delay(20);
        }
    }

    if (strcmp(node->strongest_ssid, cmp_ssid) == 0) {
        ret = NOT_VALID_SSID;
    }

#if DEBUG
    Serial.print("SSID = ");
    Serial.println(node->strongest_ssid);
    Serial.print("RSSI = ");
    Serial.println(power);
#endif

    return ret;
}

void handle_node(Node_s* node)
{
    int ssid_status;
    int connection_status;

    if (node->cluster_head == true) {
        //TODO case of cluster head
    }
    else {
        ssid_status = find_strongest_connection(node);

        if (ssid_status == VALID_SSID_FOUND) {

            connection_status = connect_to_strongest_ssid(node);

            if (connection_status == CONNECTED) {

#if DEBUG
                Serial.println("Connection successful");
#endif
                get_adc_value(node);
                send_packet_to_ap(node);
            }
            else {

#if DEBUG
                Serial.println("Could not connect!");
#endif                
            }

        }
        else {

#if DEBUG
            Serial.println("No valid connections found! Deep sleep after\n");
#endif

        }

    }
}

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