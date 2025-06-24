///* USER CODE BEGIN Header */
///**
//  ******************************************************************************
//  * @file    new_SPI.c
//  * @brief   arquivo com implementações de funções SPI simplificadas para SD
//  ******************************************************************************
//  */
///* USER CODE END Header */
//
////includes
//#include "new_SPI.h"
//#include "stm32f1xx_hal.h"
//
//// Envia e recebe 1 byte via SPI
//uint8_t SPI_Transfer(uint8_t data) {
//
//    uint8_t received = 0;
//    HAL_SPI_TransmitReceive(&hspi2, &data, &received, 1, HAL_MAX_DELAY); // TRANSFORMAR EM FUNÇÃO PRÓPRIA
//    return received;
//}
//
//// Envia comando SD (CMD0, CMD17 etc.)
//uint8_t SD_SendCommand(uint8_t cmd, uint32_t arg, uint8_t crc) {
//
//    uint8_t response;
//    SD_CS_LOW(); //Ativação do SD slave
//    SPI_Transfer(0xFF); // 8 ciclos de clock na linha SPI
//    	//tempo entre CS e comando com resposta neutra - "Acordar" SD
//
//    SPI_Transfer(0x40 | cmd);       // Comando
//    SPI_Transfer(arg >> 24);        // Argumento byte 1
//    SPI_Transfer(arg >> 16);        // Argumento byte 2
//    SPI_Transfer(arg >> 8);         // Argumento byte 3
//    SPI_Transfer(arg);              // Argumento byte 4
//    SPI_Transfer(crc);              // CRC (necessário para CMD0 e CMD8)
//
//    // Espera resposta (máx 8 bytes)
//    for (uint8_t i = 0; i < 10; i++) {
//
//        response = SPI_Transfer(0xFF); //envio de comando nulo para ler resposta
//        if (!(response & 0x80)) break; //VERIFICAR OQ FAZ
//    }
//    return response;
//}
//
//// Lê um bloco de 512 bytes
//uint8_t SD_ReadBlock(uint32_t block_addr, uint8_t *buffer) { //Define bloco desejado e local do buffer para leitura
//
//    if (SD_SendCommand(CMD17, block_addr * 512, 0xFF) != 0x00) { //Se resposta de falha, desativa SD e termina função
//        SD_CS_HIGH(); return 0;
//    }
//
//    // Espera token de início (0xFE)
//    for (uint32_t i = 0; i < 100000; ++i) { //Espera token de início
//        if (SPI_Transfer(0xFF) == 0xFE) break;
//    }
//
//    for (uint16_t i = 0; i < SD_BLOCK_LEN; ++i) { //Aloca bytes sequenciamente no buffer
//        buffer[i] = SPI_Transfer(0xFF);
//    }
//
//    SPI_Transfer(0xFF); // ignorar CRC
//    SPI_Transfer(0xFF); // ignorar CRC
//    SD_CS_HIGH();       // desabilitar slave
//    SPI_Transfer(0xFF); // processamento interno slave
//    return 1;
//}
//
//// Inicializa cartão SD (modo SPI)
//uint8_t SD_Init(void) {
//
//    SD_CS_HIGH(); //Ativa SD slave
//    for (int i = 0; i < 10; i++) SPI_Transfer(0xFF); // clock idle por 10 ciclos do SD - Inicialização
//
//    if (SD_SendCommand(CMD0, 0, 0x95) != 0x01) return 0; // CMD0 → idle SPI state
//    if (SD_SendCommand(CMD1, 0, 0xFF) != 0x00) return 0; // CMD1 → initialization command
//
//    SD_CS_HIGH(); //Desativa SD
//    SPI_Transfer(0xFF); //Espera instruções internas SD
//    return 1;
//}
//
//// Espera o cartão SD estar pronto (resposta != 0xFF)
//uint8_t SD_WaitReady(void) {
//    uint32_t timeout = 50000;
//    while (--timeout && (SPI_Transfer(0xFF) != 0xFF)); //Espera resposta neutra do SD (0xFF)
//    return (timeout != 0) ? 1 : 0; //Se der timeout, restorna 0 (Não inicializou)
//}
//
///* Definição de Handler de processos SPI */
//
//SPI_HandleTypeDef hspi2;
//
///* Inicializador de SPI HAL */ //TRANSFORMAR EM FUNÇÃO PRÓPRIA?
//
///* SPI2 init function */
//void MX_SPI2_Init(void)
//{
//
//  /* USER CODE BEGIN SPI2_Init 0 */
//
//  /* USER CODE END SPI2_Init 0 */
//
//  /* USER CODE BEGIN SPI2_Init 1 */
//
//  /* USER CODE END SPI2_Init 1 */
//  hspi2.Instance = SPI2;
//  hspi2.Init.Mode = SPI_MODE_MASTER;
//  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
//  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
//  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
//  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
//  hspi2.Init.NSS = SPI_NSS_SOFT;
//  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
//  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
//  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
//  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
//  hspi2.Init.CRCPolynomial = 10;
//  if (HAL_SPI_Init(&hspi2) != HAL_OK)
//  {
//    Error_Handler();
//  }
////  /* USER CODE BEGIN SPI2_Init 2 */
////  hspi1.Instance = SPI1;
////  hspi1.Init.Mode = SPI_MODE_MASTER;
////  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
////  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
////  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
////  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
////  hspi1.Init.NSS = SPI_NSS_SOFT;
////  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
////  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
////  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
////  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
////  hspi1.Init.CRCPolynomial = 10;
////  if (HAL_SPI_Init(&hspi1) != HAL_OK)
////  {
////    Error_Handler();
////  }
//  /* USER CODE END SPI2_Init 2 */
//
//}
//
//void HAL_SPI_MspInit(SPI_HandleTypeDef* spiHandle)
//{
//
//  GPIO_InitTypeDef GPIO_InitStruct = {0};
//  if(spiHandle->Instance==SPI2)
//  {
//  /* USER CODE BEGIN SPI2_MspInit 0 */
//
//  /* USER CODE END SPI2_MspInit 0 */
//    /* SPI2 clock enable */
//    __HAL_RCC_SPI2_CLK_ENABLE();
//
//    __HAL_RCC_GPIOB_CLK_ENABLE();
//    /**SPI2 GPIO Configuration
//    PB13     ------> SPI2_SCK
//    PB14     ------> SPI2_MISO
//    PB15     ------> SPI2_MOSI
//    */
//    GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_15;
//    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
//    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
//
//    GPIO_InitStruct.Pin = GPIO_PIN_14;
//    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
//
//  /* USER CODE BEGIN SPI2_MspInit 1 */
//
//  /* USER CODE END SPI2_MspInit 1 */
//  }
//}
//
//void HAL_SPI_MspDeInit(SPI_HandleTypeDef* spiHandle)
//{
//
//  if(spiHandle->Instance==SPI2)
//  {
//  /* USER CODE BEGIN SPI2_MspDeInit 0 */
//
//  /* USER CODE END SPI2_MspDeInit 0 */
//    /* Peripheral clock disable */
//    __HAL_RCC_SPI2_CLK_DISABLE();
//
//    /**SPI2 GPIO Configuration
//    PB13     ------> SPI2_SCK
//    PB14     ------> SPI2_MISO
//    PB15     ------> SPI2_MOSI
//    */
//    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15);
//
//  /* USER CODE BEGIN SPI2_MspDeInit 1 */
//
//  /* USER CODE END SPI2_MspDeInit 1 */
//  }
//}
