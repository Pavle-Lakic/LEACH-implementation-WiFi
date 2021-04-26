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

char accumulateBuffer[255] = {0};

void sleeping_time(unsigned long start)
{
    unsigned long stop =  micros();
    unsigned long diff = stop - start;
    unsigned long sleepTime = SLEEP_PERIOD - diff;

#if DEBUG
    Serial.print("Time to sleep in ms = ");
    Serial.println(sleepTime/1000);
#endif

    ESP.deepSleep(sleepTime);
}

void prepare_next_round(Node_s* node)
{
    uint16_t next_round = node->round + 1;
    uint8_t ch_enable;

    if (node->cluster_head == true) {
        ch_enable = 0;
    }
    else {
        ch_enable = 1;
    }

    if (next_round >= NUMBER_OF_ROUNDS) {
        next_round = 0;
        ch_enable = 1;
    }
    write_fs(next_round, ch_enable);
}

void send_to_base(Node_s* node)
{
    WiFiUDP Udp;
    IPAddress broadcast, dnsAddress;
    int connected = FAILED_TO_CONNECT;

    connected = connect_to_strongest_ssid(node);

    if (connected == CONNECTED) {

    dnsAddress = WiFi.dnsIP();
#if DEBUG
        Serial.println("Sending udp to base..");
#endif

        broadcast = create_broadcast_address(dnsAddress);
        Udp.beginPacket(broadcast, UDP_BROADCAST_PORT);
        Udp.write(accumulateBuffer);
        Udp.endPacket();
    }
}

bool check_if_message_is_valid(char *txt, unsigned char l)
{
    bool correct = false;

    if (l <= MAX_MESSAGE_SIZE) {
        if (    
            (txt[0] == ';') && isxdigit(txt[1]) && isxdigit(txt[2]) &&
            isxdigit(txt[3]) && isxdigit(txt[4]) && isxdigit(txt[5]) &&
            isxdigit(txt[6]) && isxdigit(txt[7]) && isxdigit(txt[8]) &&
            isxdigit(txt[9]) && isxdigit(txt[10]) && isxdigit(txt[11]) &&
            isxdigit(txt[12]) && (txt[13] == ':') 
            ) 
        {
            correct = true;
        }
    }


    return correct;
}

void parse_packets(Node_s* node)
{
    WiFiUDP Udp;
    uint32_t timeout_start = millis();
    char packetBuffer[255] = {0};

    accumulateBuffer[254] = '\0';

    Udp.begin(UDP_BROADCAST_PORT);

    while ((millis() - timeout_start) < WAIT_FOR_PACKETS) {
        yield();

        int packetSize = Udp.parsePacket();

        if (packetSize) {
#if DEBUG
                Serial.printf("Received packet of size %d from %s:%d\n    (to %s:%d, free heap = %d B)\n",
                    packetSize,
                    Udp.remoteIP().toString().c_str(), Udp.remotePort(),
                    Udp.destinationIP().toString().c_str(), Udp.localPort(),
                    ESP.getFreeHeap());
#endif

                int n = Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
                packetBuffer[n] = '\0';

#if DEBUG
                Serial.println("Contents of packet buffer:");
                Serial.println(packetBuffer);
#endif
                bool valid_message = false;
                valid_message = check_if_message_is_valid(packetBuffer, n);

                if (valid_message == true) {
                    strcat(accumulateBuffer, packetBuffer);
                }
                else {
#if DEBUG
                    Serial.println("Message invalid!");
#endif
                }
        }

    }

#if DEBUG
    Serial.println("Done waiting for stations! Accumulated buffer = ");
    Serial.println(accumulateBuffer);
#endif
    
}

bool set_access_point(Node_s* node)
{    
    char node_name[40] = {0};
    char adc_value_string[20] = {0};
    char upper_nibla;
    char lower_nibla;
    char upper_nibla_string[2];
    char lower_nibla_string[2];
    bool success =  false;
    char buffer_string [50] = {0};

    node_name[39] = '\0';
    adc_value_string [19] = '\0';
    buffer_string[49] = '\0';

    for (int i = 0; i < 6; i++) {
        lower_nibla = node->nodeName[i] & 0x0F;
        upper_nibla = (node->nodeName[i] & 0xF0) >> 4;
        sprintf(upper_nibla_string, "%X", upper_nibla);
        sprintf(lower_nibla_string, "%X", lower_nibla);
        strcat(node_name, upper_nibla_string);
        strcat(node_name, lower_nibla_string);
    }

    strcat(buffer_string, ";");
    strcat(buffer_string, node_name);
    strcat(buffer_string, ":");
    sprintf(adc_value_string, "%u", node->adc_value);
    strcat(buffer_string, adc_value_string);
    strcat(accumulateBuffer, buffer_string);

    WiFi.mode(WIFI_AP);

    if(WiFi.softAP(node_name, NODE_PASS, WIFI_CHANNEL, false, MAX_CONNECTED) == true) {

        success = true;
   }

    return success;
}

IPAddress create_broadcast_address(IPAddress dns)
{
    IPAddress broadcast(dns[0], dns[1], dns[2], 255);

    return broadcast;
}

void send_packet_to_ap(Node_s* node)
{
    WiFiUDP Udp;
    IPAddress dnsAddress;
    IPAddress broadcastAddress;
    dnsAddress = WiFi.dnsIP();

#if DEBUG
    Serial.print("Local IP address = ");
    Serial.println(WiFi.localIP());
    Serial.print("DNS address = ");
    Serial.println(dnsAddress);
#endif

    char node_name[40] = {0};
    char adc_value_string[20] = {0};
    char upper_nibla;
    char lower_nibla;
    char upper_nibla_string[2];
    char lower_nibla_string[2];

    node_name[39] = '\0';
    adc_value_string [19] = '\0';

    for (int i = 0; i < 6; i++) {
        lower_nibla = node->nodeName[i] & 0x0F;
        upper_nibla = (node->nodeName[i] & 0xF0) >> 4;
        sprintf(upper_nibla_string, "%X", upper_nibla);
        sprintf(lower_nibla_string, "%X", lower_nibla);
        strcat(node_name, upper_nibla_string);
        strcat(node_name, lower_nibla_string);
    }

    sprintf(adc_value_string, "%u", node->adc_value);

#if DEBUG
    Serial.print("NODE NAME = ");
    Serial.println(node_name);
    Serial.println(strlen(node_name));
#endif

    broadcastAddress = create_broadcast_address(dnsAddress);

#if DEBUG
    Serial.print("Broadcast address = ");
    Serial.println(broadcastAddress);
#endif

    if (Udp.beginPacket(broadcastAddress, UDP_BROADCAST_PORT) == 1) {

#if DEBUG
    Serial.println("Sending packet!");
#endif

    Udp.write(";");
    Udp.write(node_name);
    Udp.write(":");
    Udp.write(adc_value_string);

    }

    if (Udp.endPacket() == 1) {

#if DEBUG
    Serial.println("Packet sent!");
#endif

    }
    
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

    if (node->cluster_head == false) {
        WiFi.begin(String(node->strongest_ssid), NODE_PASS);
    }
    else {
        WiFi.begin(BASE_SSID, NODE_PASS);
    }

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
    bool success = false;

    if (node->cluster_head == true) {
       success =  set_access_point(node);

        if (success == true) {

#if DEBUG
        Serial.println("Successfully created Access Point!");
#endif
        parse_packets(node);

        send_to_base(node);

        }
        else {

#if DEBUG
        Serial.println("Did not create Access point! Deep sleep?");
#endif

        }
    }
    else {
        WiFi.forceSleepBegin();
        delay(1000);
        WiFi.forceSleepWake();

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