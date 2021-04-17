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
#include <Ticker.h>
#include <FS.h>

/** Flag which will print debug messages over serial terminal.*/
#define DEBUG                   1

/**
 * Structure which defines node. This structure consists of 
 * Node name, ADC value, current round, and flag which indicates
 * if node can be cluster head for current round.
*/
typedef struct
{
    uint8_t nodeName[6];    /**< Node name (its mac address)*/
    uint16_t adc_value;     /**< ADC value of node*/
    uint8_t round;          /**< Current round*/
    uint8_t ch_enable;      /**< Flag which indicats if node can be CH in current round*/
} Node_s;

/**
 * @brief Fills nodeName from structure with mac address.
 * @param Pointer to Node_s structure.
 * @return none.
 */
void init_node_name (Node_s * node);

/**
 * @brief Generates random number between 0 and 1.
 * @param none.
 * @return random number between 0 and 1.
 */
float random_number(void);

#endif // INCLUDES_H_