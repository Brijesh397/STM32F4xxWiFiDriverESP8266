/****************************************FILE DESCRIPTION**************************************/
/* FILE 		: main.c
* PROJECT 		: WiFi
* PROGRAMMER 	: Brijesh Mehta
* DESCRIPTION 	: main c file
*/
/*********************************************************************************************/

/**************** Includes ************************/
#include "main.h"

/**************** Variable *************************/
char ip[15] = "0";	//Store the IP Address of the module

// API
char pckt_to_send[] = "GET /data/2.5/weather?q=London&APPID=700f83df7417b8d643ff401be2f7c7f6 HTTP/1.1\r\nHost:api.openweathermap.org\r\n\r\n";

//Web page Home
char *home = " <html><head><meta name=\"viewport\"content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\
<title>HARDWARE CONTROL SERVER </title><style>\
 html { font-family: Helvetica;display: inline-block;margin: 0px auto;text-align: center;width:100%}\
 body{ margin-top: 50px;float: left;}  h1{color: #444444;margin: 50px auto 30px;}h3{color: #444444;margin-bottom: 50px;}\
 .button{display: block;width: 80px;background-color:#1abc9c;border: none;color: white;padding: 13px 30px;\
	   text-decoration: none;\
	   font-size: 25px;\
	   margin: 30px 30px 30px 30px;\
	   cursor: pointer;\
	   border-radius: 4px;\
	   float: left;\
  }\
  .button-on{ background-color: #1abc77;}.button-on:active{background-color: #16a085;}\
  .button-off {background-color: #ff2200;}\
  .button-off:active{background-color: #ff0000;}{ font-size: 14px;color: #888;margin-bottom: 10px;}\
  .module_label{margin: 30px 30px 30px 30px;}\
 </style></head><body><h1>HARDWARE CONTROL SERVER </h1>\
<p>LIGHT  State: OFF</p>\
<a class=\"button button-on\" href=\"/light\">ON</a>\
</body></html>";

//Web page Led
char *ledON = " <html><head><meta name=\"viewport\"content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\
<title>HARDWARE CONTROL SERVER </title><style>\
 html { font-family: Helvetica;display: inline-block;margin: 0px auto;text-align: center;width:100%}\
 body{ margin-top: 50px;float: left;}  h1{color: #444444;margin: 50px auto 30px;}h3{color: #444444;margin-bottom: 50px;}\
 .button{display: block;width: 80px;background-color:#1abc9c;border: none;color: white;padding: 13px 30px;\
	   text-decoration: none;\
	   font-size: 25px;\
	   margin: 30px 30px 30px 30px;\
	   cursor: pointer;\
	   border-radius: 4px;\
	   float: left;\
  }\
  .button-on{ background-color: #1abc77;}.button-on:active{background-color: #16a085;}\
  .button-off {background-color: #ff2200;}\
  .button-off:active{background-color: #ff0000;}{ font-size: 14px;color: #888;margin-bottom: 10px;}\
  .module_label{margin: 30px 30px 30px 30px;}\
 </style></head><body><h1>HARDWARE CONTROL SERVER </h1>\
<p>LIGHT  State: ON</p>\
<a class=\"button button-off\" href=\"/light\">OFF</a>\
</body></html>";

GPIO_InitTypeDef gpioInitStruct = {0};

/*************** Function Prototype *****************/
void systemClockConfig(void);
void gpioConfig(void);

int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  systemClockConfig();

  gpioConfig();

  /* Initialize all configured peripherals */
  esp8266Init();

  //Reseting the ESP Module
  esp8266Reset();

  //Setting the Module to Station Mode
  if(esp8266SetToStationMode() != HAL_OK)
  {
	  Error_Handler();
  }

  //Connecting the Module to the Internet
  if(esp8266Connect(SSID,PASSWORD) != HAL_OK)
  {
	  Error_Handler();
  }

  //Enabling Multiple Connections
  if(esp8266EnableMultiConn() != HAL_OK)
  {
	  Error_Handler();
  }


  //Creating a TCP Server
  if(esp8266CreateTCPServer(80) != HAL_OK)
  {
	  Error_Handler();
  }

  //Getting the IP Address of the Module
  if(esp8266GetIP(ip) != HAL_OK)
  {
	  Error_Handler();
  }

  /*
   * Connecting to API and receiving data
   */
  //  //Establish TCP Connection
  //  if(esp8266EstablishTCPConnection("api.openweathermap.org") != HAL_OK)
  //  {
  //	  Error_Handler();
  //  }
  //
  //  //Send data to server and check response
  //  if(esp8266SendDataToServer(pckt_to_send) != HAL_OK)
  //  {
  //	  Error_Handler();
  //  }


  //  //Getting the IP address of a domain
  //  if(esp8266GetDomainIP("www.google.com", ip) != HAL_OK)
  //  {
  //	  Error_Handler();
  //  }
  //

  char linkId = '8';

  while (1)
  {
	  /*
	   * Waiting for a request from the TCP Client
	   * Once request received getting the Link ID
	   */
	  linkId = esp8266GetLinkId();

	  //Identify which page to display
	  if((esp8266CheckResponse("GET /light")) != ESP88266_RESPONSE_CORRECT)
	  {
		  /*
		   * Now sending the data to the TCP Client
		   */
		  esp8266SendDataToServer(linkId, home);
	  }
	  else
	  {
		  if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_2))
		  {
			  esp8266SendDataToServer(linkId, home);
			  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_2);
		  }
		  else
		  {
			  esp8266SendDataToServer(linkId, ledON);
			  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_2);
		  }
	  }
  }

}

/***************** UART2 IRQ Handler **************************/
void USART2_IRQHandler(void)
{
	esp8266IRQHandler();
}

/******************* System Clock Config *****************************/
void systemClockConfig(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/*********************** GPIO Config ****************************/
void gpioConfig(void)
{
	__HAL_RCC_GPIOC_CLK_ENABLE();

	gpioInitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	gpioInitStruct.Pin = GPIO_PIN_2;

	HAL_GPIO_Init(GPIOC, &gpioInitStruct);
}

/**************** Error Handler ************************/
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
