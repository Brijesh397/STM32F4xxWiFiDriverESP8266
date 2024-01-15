/****************************************FILE DESCRIPTION**************************************/
/* FILE 		: esp8266.h
* PROJECT 		: WiFi
* PROGRAMMER 	: Brijesh Mehta
* DESCRIPTION 	: esp8266 config
*/
/*********************************************************************************************/

#ifndef ESP8266_H_
#define ESP8266_H_

/************** Includes ***********************/
#include "main.h"

extern UART_HandleTypeDef huart2;

extern GPIO_InitTypeDef GPIO_InitStruct;


typedef enum
{
	ESP88266_WRITE_SUCCESSFUL  =	0x00,
	ESP88266_WRITE_FAILED	   =	0x01,
	ESP88266_RESPONSE_CORRECT  =	0x02,
	ESP88266_RESPONSE_WRONG	   = 	0x03,
	ESP88266_WAIT			   =	0x04
}esp8266State;

/*************** PIN Definition ***************/
#define TX_PIN			GPIO_PIN_2
#define TX_PORT			GPIOA

#define RX_PIN			GPIO_PIN_3
#define RX_PORT			GPIOA

#define ESP8266_MAX_WAIT			20000U

/**************** SSID PASS *********************/
extern char SSID[];
extern char PASSWORD[];

/*************** COMMANDS ***********************/
extern char RESTART_COMMAND[];
extern char STATION_MODE[];

/*************** Function Prototype ***************/
void esp8266Init(void);

esp8266State esp8266WriteCommand(char *command);
esp8266State esp8266CheckResponse(char *response);

void esp8266IRQHandler(void);

HAL_StatusTypeDef esp8266WaitUntilTimeout(uint32_t time);

/************ Command Functions *****************/
void esp8266Reset(void);
HAL_StatusTypeDef esp8266SetToStationMode(void);
HAL_StatusTypeDef esp8266Connect(char *ssid, char *password);
HAL_StatusTypeDef esp8266GetIP(char *ip);
HAL_StatusTypeDef esp8266EnableMultiConn(void);
HAL_StatusTypeDef esp8266CreateTCPServer(uint8_t portNumber);
char esp8266GetLinkId(void);
HAL_StatusTypeDef esp8266SendDataToServer(volatile char linkId, char *data);
HAL_StatusTypeDef esp8266GetDomainIP(char *url, char *ip);
HAL_StatusTypeDef esp8266EstablishTCPConnection(char *ip);

#endif /* ESP8266_H_ */
