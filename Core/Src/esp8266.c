/****************************************FILE DESCRIPTION**************************************/
/* FILE 		: esp8266.c
* PROJECT 		: WiFi
* PROGRAMMER 	: Brijesh Mehta
* DESCRIPTION 	: esp8266 config functions
*/
/*********************************************************************************************/

/************** Includes ***********************/
#include "esp8266.h"

UART_HandleTypeDef huart2;
GPIO_InitTypeDef GPIO_InitStruct = {0};

/*************** TX RX Buffers *************/
Circular_Buffer_t esp8266RxBuffer;
Circular_Buffer_t esp8266TxBuffer;

/**************** SSID PASS *********************/
char SSID[] = "Suyoga_home_2.4";
char PASSWORD[] = "con@rEy59";

/*************** COMMANDS ***********************/
char RESTART_COMMAND[] = "AT+RST\r\n";
char STATION_MODE[]	= "AT+CWMODE=1\r\n";

/********************* Function Description *********************************
 * FUNCTION		:	esp8266Init
 *
 * DESCRIPTION	:	This function is used to configure the esp8266
 *
 * PARAMETERS	:	void
 *
 * RETURN		:	void
 *
 * **************************************************************************/
void esp8266Init(void)
{
	//Enabling the GPIO and UART Clock
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = TX_PIN|RX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart2) != HAL_OK)
	{
	Error_Handler();
	}

	/************** Enabling the USART1 Interrupts ************/
	HAL_NVIC_SetPriority(USART2_IRQn, 1, 1);
	HAL_NVIC_EnableIRQ(USART2_IRQn);

	/************* Initializing the RX Buffer ***********/
	esp8266RxBuffer.front = -1;
	esp8266RxBuffer.front = -1;
}

void esp8266Reset(void)
{
	//Find the length of the Command
	uint16_t len = strlen(RESTART_COMMAND);

	//Transmit the command
	HAL_UART_Transmit(&huart2, (uint8_t*)RESTART_COMMAND, len, 1000);

	HAL_Delay(1000);
}


/********************* Function Description *********************************
 * FUNCTION		:	esp8266WriteCommand
 *
 * DESCRIPTION	:	This function is used to write command to the esp8266 Module
 *
 * PARAMETERS	:	char command
 *
 * RETURN		:	void
 *
 * **************************************************************************/
esp8266State esp8266WriteCommand(char *command)
{
	//Clear the receive buffer
	circularBufferClear(&esp8266RxBuffer);

	//Find the length of the Command
	uint16_t len = strlen(command);

	//Transmit the command
	if(HAL_UART_Transmit(&huart2, (uint8_t*)command, len, 1000) != HAL_OK)
	{
		return ESP88266_WRITE_FAILED;
	}

	//Enable the RXNE Interrupt to store the response inside the receive buffer
	__HAL_UART_ENABLE_IT(&huart2,UART_IT_RXNE);

	return ESP88266_WRITE_SUCCESSFUL;
}

/********************* Function Description *********************************
 * FUNCTION		:	esp8266CheckResponse
 *
 * DESCRIPTION	:	This function checks inside the receive buffer whether we have received the correct
 * 					response or not.
 * 					If the response is not correct it will return error.
 *
 * PARAMETERS	:	Circular_Buffer_t *rxBuffer - receive buffer
 * 					char *response - response we want to check
 *
 * RETURN		:	void
 *
 * **************************************************************************/
esp8266State esp8266CheckResponse(char *response)
{
	uint32_t currPos = 0;
	uint32_t len = strlen(response);
	char temp;

	for(uint32_t i=0; i<BUFFER_SIZE; i++)
	{
		temp = esp8266RxBuffer.data[i];
		if(temp == response[currPos])
		{
			currPos++;
			if(currPos == len)
			{
				esp8266RxBuffer.front = i+1;
				return ESP88266_RESPONSE_CORRECT;

				//Disable the Interupt
				//__HAL_UART_DISABLE_IT(&huart2,UART_IT_RXNE);
			}
		}
		else
		{
			currPos = 0;
		}
	}

	return ESP88266_RESPONSE_WRONG;
}

/********************* Function Description *********************************
 * FUNCTION		:	esp8266IRQHandler
 *
 * DESCRIPTION	:	This function handles the raised interrupts
 *
 * PARAMETERS	:	void
 *
 * RETURN		:	void
 *
 * **************************************************************************/
void esp8266IRQHandler(void)
{
	/******************* Handling the RXNE Interrupt *************************/
	if((__HAL_UART_GET_FLAG(&huart2,UART_FLAG_RXNE)) && ((__HAL_UART_GET_IT_SOURCE(&huart2, UART_IT_RXNE))))
	{
		char temp = huart2.Instance->DR;
		if(circularBufferWriteChar(&esp8266RxBuffer,temp) != BUFFER_WRITE_OK)
		{
			__HAL_UART_DISABLE_IT(&huart2,UART_IT_RXNE);
		}
	}
}

/********************* Function Description *********************************
 * FUNCTION		:	esp8266WaitUntilTimeout
 *
 * DESCRIPTION	:	This function prevents getting stuck inside a while loop.
 * 					If the time is greater than timeout then return HAL_TIMEOUT
 *
 * PARAMETERS	:	uint32_t timeout
 *
 * RETURN		:	void
 *
 * **************************************************************************/
HAL_StatusTypeDef esp8266WaitUntilResponse(char *response, uint32_t timeout)
{
	uint32_t tickstart = HAL_GetTick();

	while((esp8266CheckResponse(response) != ESP88266_RESPONSE_CORRECT))
	{
		if((HAL_GetTick() - tickstart) > timeout)
		{
			return HAL_TIMEOUT;
		}
	}

	return HAL_OK;
}

/********************* Function Description *********************************
 * FUNCTION		:	esp8266SetMode
 *
 * DESCRIPTION	:	This function sets the module to station mode
 *
 * PARAMETERS	:	void
 *
 * RETURN		:	HAL_StatusTypeDef
 *
 * **************************************************************************/
HAL_StatusTypeDef esp8266SetToStationMode(void)
{
	esp8266WriteCommand(STATION_MODE);

	if(esp8266WaitUntilResponse("OK\r\n", ESP8266_MAX_WAIT) != HAL_OK)
	{
		return HAL_TIMEOUT;
	}

	return HAL_OK;
}

/********************* Function Description *********************************
 * FUNCTION		:	esp8266Connect
 *
 * DESCRIPTION	:	This function is used to connect the module to wifi
 *
 * PARAMETERS	:	char *ssid - SSID
 * 					char *password - Password
 *
 * RETURN		:	HAL_StatusTypeDef
 *
 * **************************************************************************/
HAL_StatusTypeDef esp8266Connect(char *ssid, char *password)
{
	char command[BUFFER_SIZE] = "0";

	sprintf(command,"AT+CWJAP=\"%s\",\"%s\"\r\n",ssid,password);

	//Writing to the ESP Module
	esp8266WriteCommand(command);

	if(esp8266WaitUntilResponse("OK\r\n", ESP8266_MAX_WAIT) != HAL_OK)
	{
		return HAL_TIMEOUT;
	}
	return HAL_OK;
}

/********************* Function Description *********************************
 * FUNCTION		:	esp8266GetIP
 *
 * DESCRIPTION	:	This function is used to get the IP address of the module after connecting to an AP
 *
 * PARAMETERS	:	char *ssid - IP to store the IP
 *
 * RETURN		:	HAL_StatusTypeDef
 *
 * **************************************************************************/
HAL_StatusTypeDef esp8266GetIP(char *ip)
{
	char temp = '0';
	uint8_t i = 0;

	//Writing to the ESP Module
	esp8266WriteCommand("AT+CIFSR\r\n");

	if(esp8266WaitUntilResponse("CIFSR:STAIP,\"", ESP8266_MAX_WAIT) != HAL_OK)
	{
		return HAL_TIMEOUT;
	}

	HAL_Delay(100);	//Waiting for rest of the data to come

	while(1)
	{
		temp = circularBufferRead(&esp8266RxBuffer);
		if(temp == '\"')
		{
			break;
		}
		ip[i] = temp;
		i++;
	}
	return HAL_OK;
}

/********************* Function Description *********************************
 * FUNCTION		:	esp8266EnableMultiConn
 *
 * DESCRIPTION	:	This function is used to enable multiple connections so mulitple clients
 * 					can connect to the TCP Server
 *
 * PARAMETERS	:	void
 *
 * RETURN		:	HAL_StatusTypeDef
 *
 * **************************************************************************/
HAL_StatusTypeDef esp8266EnableMultiConn(void)
{
	//Writing to the ESP Module
	esp8266WriteCommand("AT+CIPMUX=1\r\n");

	if(esp8266WaitUntilResponse("OK\r\n", ESP8266_MAX_WAIT) != HAL_OK)
	{
		return HAL_TIMEOUT;
	}

	return HAL_OK;
}

/********************* Function Description *********************************
 * FUNCTION		:	esp8266CreateTCPServer
 *
 * DESCRIPTION	:	This function is used to create a TCP Server
 *
 * PARAMETERS	:	void
 *
 * RETURN		:	HAL_StatusTypeDef
 *
 * **************************************************************************/
HAL_StatusTypeDef esp8266CreateTCPServer(uint8_t portNumber)
{
	char command[BUFFER_SIZE] = "0";

	sprintf(command,"AT+CIPSERVER=1,%d\r\n",portNumber);

	//Writing to the ESP Module
	esp8266WriteCommand(command);

	if(esp8266WaitUntilResponse("OK\r\n", ESP8266_MAX_WAIT) != HAL_OK)
	{
		return HAL_TIMEOUT;
	}

	return HAL_OK;
}

/********************* Function Description *********************************
 * FUNCTION		:	esp8266SendDataToServer
 *
 * DESCRIPTION	:	This function is used to send data to server
 *
 * PARAMETERS	:	char *data - data to be sent
 * 					uint32_t linkId - link Id
 *
 * RETURN		:	HAL_StatusTypeDef
 *
 * **************************************************************************/
HAL_StatusTypeDef esp8266SendDataToServer(volatile char linkId, char *data)
{
	char command[BUFFER_SIZE] = "0";

	uint32_t len = strlen(data);

	sprintf(command,"AT+CIPSEND=%c,%ld\r\n",linkId,len);

	//Writing to the ESP Module
	esp8266WriteCommand(command);

	// > Indicates that now you can send data
	if(esp8266WaitUntilResponse(">", ESP8266_MAX_WAIT) != HAL_OK)
	{
		return HAL_TIMEOUT;
	}

	//Send Data
	esp8266WriteCommand(data);

	//Wait for send Okay
	if(esp8266WaitUntilResponse("SEND OK", ESP8266_MAX_WAIT) != HAL_OK)
	{
		return HAL_TIMEOUT;
	}

	//Close the Connection with the client
	memset(command,'\0',BUFFER_SIZE);

	sprintf(command,"AT+CIPCLOSE=%c\r\n",linkId);

	esp8266WriteCommand(command);

	if(esp8266WaitUntilResponse("OK\r\n", ESP8266_MAX_WAIT) != HAL_OK)
	{
		return HAL_TIMEOUT;
	}

	return HAL_OK;

}

/********************* Function Description *********************************
 * FUNCTION		:	esp8266GetDomainIP
 *
 * DESCRIPTION	:	This function is used to get the IP Address of a domain
 *
 * PARAMETERS	:	char *url
 *
 * RETURN		:	HAL_StatusTypeDef
 *
 * **************************************************************************/
HAL_StatusTypeDef esp8266GetDomainIP(char *url, char*ip)
{
	char temp = '0';
	uint8_t i = 0;

	char command[BUFFER_SIZE] = "0";

	sprintf(command,"AT+CIPDOMAIN=\"%s\"\r\n",url);

	//Writing to the ESP Module
	esp8266WriteCommand(command);

	if(esp8266WaitUntilResponse("CIPDOMAIN:", ESP8266_MAX_WAIT) != HAL_OK)
	{
		return HAL_TIMEOUT;
	}

	HAL_Delay(100);	//Waiting for rest of the data to come

	while(1)
	{
		temp = circularBufferRead(&esp8266RxBuffer);
		if(temp == '\r')
		{
			break;
		}
		ip[i] = temp;
		i++;
	}
	return HAL_OK;
}

/********************* Function Description *********************************
 * FUNCTION		:	esp8266EstablishTCPConnection
 *
 * DESCRIPTION	:	This function is used to establish a tcp connection
 *
 * PARAMETERS	:	char *ip
 *
 * RETURN		:	HAL_StatusTypeDef
 *
 * **************************************************************************/
HAL_StatusTypeDef esp8266EstablishTCPConnection(char *ip)
{
	char command[BUFFER_SIZE] = "0";

	sprintf(command,"AT+CIPSTART=\"TCP\",\"%s\",80\r\n",ip);

	//Writing to the ESP Module
	esp8266WriteCommand(command);

	if(esp8266WaitUntilResponse("OK\r\n", ESP8266_MAX_WAIT) != HAL_OK)
	{
		if(esp8266WaitUntilResponse("ALREADY CONNECT\r\n", ESP8266_MAX_WAIT) != HAL_OK)
		{
			return HAL_TIMEOUT;
		}
	}

	return HAL_OK;
}

/********************* Function Description *********************************
 * FUNCTION		:	esp8266GetLinkId
 *
 * DESCRIPTION	:	This function is used to find the LinkId
 *
 * PARAMETERS	:	char *linkId variable to store the linkId
 *
 * RETURN		:	HAL_StatusTypeDef
 *
 * **************************************************************************/
char esp8266GetLinkId(void)
{
	//Clearing the circular buffer
	circularBufferClear(&esp8266RxBuffer);

	//Wait for +IPD. this will be received in the RX buffer when a TCP Client tries to connect to server
	while((esp8266CheckResponse("+IPD,")) != ESP88266_RESPONSE_CORRECT);

	//Wait for the rest of the data to be received
	HAL_Delay(100);

	//Extract the Link Id
	char linkId = circularBufferRead(&esp8266RxBuffer);

	//Return the Link Id
	return linkId;
}
