/** @file includes.h
 *  @brief Functions which node of LEACH protocol will use.
 *
 *  This file contains other necessary includes,
 *	structures, and defines needed for complete
 *	node functionality.
 *
 *  @author Pavle Lakic
 *  @bug No known bugs.
 */
#ifndef INCLUDES_H_
#define INCLUDES_H_

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "ESP8266TrueRandom.h"
#include <stdlib.h>
#include <string.h>
#include <Ticker.h>
#include <FS.h>
#include "LittleFS.h"

/** Name of file where round and ch_enable flag are written.*/
#define FILENAME                "/setup.txt"

/** Flag which will print debug messages over serial terminal.*/
#define DEBUG                   1

/** This flag will create file in FS where round and ch_enable will be saved.
 *  Also it will reset round to 0, and ch_enable to 1.
*/
#define ROUNDS_RESET            1

/** Number of rounds determined apriori. Currently it will be the same as
 *  number of nodes.
*/
#define NUMBER_OF_ROUNDS        7

/** Base station SSID.*/
#define BASE_SSID               "BASE_STATION"

/** This will be password for all nodes.*/
#define NODE_PASS               "teorijazavere"

/** Timeout for connection in ms.*/
#define CONNECTION_TIMEOUT      15000

/** Local UDP port where data from stations will be sent.*/
#define UDP_BROADCAST_PORT      50000

/** Analog input pin.*/
#define ADC_PIN                   A0

/**
 * Structure which defines node.
*/
typedef struct
{
    uint8_t     nodeName[6];            /**< Node name (its mac address).*/
    uint16_t    adc_value;              /**< ADC value of node.*/
    uint8_t     round;                  /**< Current round.*/
    uint8_t     ch_enable;              /**< Flag which indicats if node can be CH in current round.*/
    bool        cluster_head;           /**< True if node is cluster head for current round.*/
    float       P;                      /**< Probability that node will become cluster head in round 0.*/
    char      strongest_ssid[20];     /**< Strongest SSID node can connect to, will depend on case.*/
} Node_s;

/**
 * Return codes of node.  
*/
typedef enum
{
    CONNECTED,
    FAILED_TO_CONNECT,
    NOT_VALID_SSID,
    NO_NETWORKS_FOUND,
    VALID_SSID_FOUND
} node_return_codes_e;

/**
 * @brief Sends UDP packet to cluster head (access point).
 * @param node Pointer to Node_s structure.
 * @return none
 */
void send_packet_to_ap(Node_s* node);

/**
 * @brief Gets and stores value from ADC.
 * @param node Pointer to Node_s structure.
 * @return none
 */
void get_adc_value(Node_s* node);

/**
 * @brief Tries to connect to strongest valid SSID.
 * @param node Pointer to Node_s structure.
 * @return connection status defined in node_return_codes_e.
 */
int connect_to_strongest_ssid(Node_s* node);

/**
 * @brief Checks if SSID has valid patern.
 * @param txt SSID of found network.
 * @return true if valid, else false.
 */
bool ssid_is_valid(const char* txt);

/**
 * @brief Tries to find strongest valid connection. Connection might be
 * valid if SSID has predefined pattern (MAC address of node).
 * @param node Pointer to Node_s structure.
 * @return connection status defined in node_return_codes_e.
 */
int find_strongest_connection(Node_s* node);

/**
 * @brief Handles node regarding if node is cluster head or station
 * @param node Pointer to Node_s structure.
 * @return none.
 */
void handle_node(Node_s* node);

/**
 * @brief Decides if node will be cluster head or not for current round.
 * @param node Pointer to Node_s structure.
 * @return none.
 */
void mode_decision(Node_s* Node);

/**
 * @brief Fills nodeName from structure with mac address.
 * @param node Pointer to Node_s structure.
 * @return none.
 */
void init_node_name (Node_s * node);

/**
 * @brief Generates threshold for current round.
 * @param node Pointer to Node_s structure.
 * @return Threshold.
 */
float calculate_threshold(Node_s* node);

/**
 * @brief Generates random number between 0 and 1.
 * @param none.
 * @return random number between 0 and 1.
 */
float random_number(void);

/**
 * @brief Mounts LittleFs.
 * @param none.
 * @return True if successful.
 */
bool mount_fs(void);

/**
 * @brief Writes to file in Fs.
 * @param round Current round.
 * @param ch_enable Flag which indicates if node can be CH for current round.
 * @return none.
 */
void write_fs(uint16_t round, uint8_t ch_enable);

/**
 * @brief Reads current round and ch_enable flag from FS.
 * @param round Current round.
 * @param ch_enable Flag which indicates if node can be CH for current round.
 * @return none.
 */
void read_fs(uint16_t* round, uint8_t* ch_enable);
#endif // INCLUDES_H_