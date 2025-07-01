/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    new_SPI.c
  * @brief   arquivo com implementações de funções SPI simplificadas para SD
  ******************************************************************************
  */
/* USER CODE END Header */

//includes
#include <sd.h>
#include "stm32f1xx_hal.h"

#include <stdint.h>

#define CMD9  0x09
#define TOKEN_START_BLOCK 0xFE


void SPI_Init(uint32_t prescaler);
uint8_t SPI_Transfer(uint8_t data);
uint8_t SD_SendCommand(uint8_t cmd, uint32_t arg, uint8_t crc);
uint8_t SD_WaitReady(void);
uint8_t SD_ReadCSD(SD_CSD_t *csdInfo);
uint8_t SD_Init(void);
uint8_t SD_ReadBlock(uint32_t block_addr, uint8_t *buffer);
uint8_t SD_WriteBlock(uint32_t block_addr, const uint8_t *buffer);

void SD_ReceiveBuffer(uint8_t *buf, uint16_t len) {
    while (len--) {
        *buf++ = SPI_Transfer(0xFF); // Envia dummy e recebe resposta
    }
}

uint8_t SD_ReadCSD(SD_CSD_t *csdInfo) {
    uint8_t response;
    uint16_t timeout = 50000;

    response = SD_SendCommand(CMD9, 0, 0xFF);
    if (response != 0x00)
        return 0; // erro ao enviar CMD9

    // Espera pelo token 0xFE
    while (--timeout && SPI_Transfer(0xFF) != TOKEN_START_BLOCK);
    if (timeout == 0)
        return 0; // Timeout

    // Lê os 16 bytes do CSD
    SD_ReceiveBuffer(csdInfo->csd, 16);

    // Lê e ignora os 2 bytes de CRC
    SPI_Transfer(0xFF);
    SPI_Transfer(0xFF);

    // Detecta versão do CSD
    if ((csdInfo->csd[0] >> 6) == 1) {
        // CSD versão 2.0 (SDHC/SDXC)
        uint32_t c_size = ((uint32_t)(csdInfo->csd[7] & 0x3F) << 16) |
                          ((uint16_t)csdInfo->csd[8] << 8) |
                          csdInfo->csd[9];
        csdInfo->capacity = (c_size + 1) * 512UL * 1024UL; // em bytes
    } else {
        // CSD versão 1.0 (SDSC)
        uint8_t read_bl_len = csdInfo->csd[5] & 0x0F;
        uint16_t c_size = ((csdInfo->csd[6] & 0x03) << 10) |
                          (csdInfo->csd[7] << 2) |
                          ((csdInfo->csd[8] & 0xC0) >> 6);
        uint8_t c_size_mult = ((csdInfo->csd[9] & 0x03) << 1) |
                              ((csdInfo->csd[10] & 0x80) >> 7);

        uint32_t block_len = 1UL << read_bl_len;
        uint32_t mult = 1UL << (c_size_mult + 2);
        uint32_t blocknr = (c_size + 1) * mult;

        csdInfo->capacity = blocknr * block_len;
    }

    return 1; // sucesso
}

// Envia e recebe 1 byte via SPI
uint8_t SPI_Transfer(uint8_t data) {

    uint8_t received = 0;
    HAL_SPI_TransmitReceive(&hspi1, &data, &received, 1, HAL_MAX_DELAY); // TRANSFORMAR EM FUNÇÃO PRÓPRIA
    return received;
}

// Envia comando SD (CMD0, CMD17 etc.)
uint8_t SD_SendCommand(uint8_t cmd, uint32_t arg, uint8_t crc) {

    uint8_t response;
    SD_CS_LOW(); //Ativação do SD slave
    SPI_Transfer(0xFF); // 8 ciclos de clock na linha SPI
    	//tempo entre CS e comando com resposta neutra - "Acordar" SD

    SPI_Transfer(0x40 | cmd);       // Comando
    SPI_Transfer(arg >> 24);        // Argumento byte 1
    SPI_Transfer(arg >> 16);        // Argumento byte 2
    SPI_Transfer(arg >> 8);         // Argumento byte 3
    SPI_Transfer(arg);              // Argumento byte 4
    SPI_Transfer(crc);              // CRC (necessário para CMD0 e CMD8)

    // Espera resposta (máx 8 bytes)
    for (uint8_t i = 0; i < 10; i++) {

        response = SPI_Transfer(0xFF); //envio de comando nulo para ler resposta
        if (!(response & 0x80)) break; //VERIFICAR OQ FAZ
    }
    return response;
}

// Lê um bloco de 512 bytes
uint8_t SD_ReadBlock(uint32_t block_addr, uint8_t *buffer) { //Define bloco desejado e local do buffer para leitura

    if (SD_SendCommand(CMD17, block_addr * 512, 0xFF) != 0x00) { //Se resposta de falha, desativa SD e termina função
        SD_CS_HIGH(); return 0;
    }

    // Espera token de início (0xFE)
    for (uint32_t i = 0; i < 100000; ++i) { //Espera token de início
        if (SPI_Transfer(0xFF) == 0xFE) break;
    }

    for (uint16_t i = 0; i < SD_BLOCK_LEN; ++i) { //Aloca bytes sequenciamente no buffer
        buffer[i] = SPI_Transfer(0xFF);
    }

    SPI_Transfer(0xFF); // ignorar CRC
    SPI_Transfer(0xFF); // ignorar CRC
    SD_CS_HIGH();       // desabilitar slave
    SPI_Transfer(0xFF); // processamento interno slave
    return 1;
}

uint8_t SD_WriteBlock(uint32_t block_addr, const uint8_t *buffer) {

    // Envia comando CMD24 para escrever 1 bloco
    if (SD_SendCommand(CMD24, block_addr * 512, 0xFF) != 0x00) {
        SD_CS_HIGH();
        return 0;
    }

    SPI_Transfer(0xFF); // delay mínimo
    SPI_Transfer(TOKEN_WRITE); // envia token de início

    // Envia 512 bytes do buffer
    for (uint16_t i = 0; i < SD_BLOCK_LEN; ++i) {
        SPI_Transfer(buffer[i]);
    }

    // Envia CRC (pode ser falso, pois CRC é desativado em SPI)
    SPI_Transfer(0xFF);
    SPI_Transfer(0xFF);

    // Lê token de resposta do cartão (esperado: 0x05)
    uint8_t response = SPI_Transfer(0xFF);
    if ((response & 0x1F) != 0x05) {
        SD_CS_HIGH();
        SPI_Transfer(0xFF);
        return 0; // erro na aceitação do dado
    }

    // Espera até o cartão sair do busy (responder 0xFF)
    while (SPI_Transfer(0xFF) != 0xFF);

    SD_CS_HIGH();
    SPI_Transfer(0xFF); // pós-processamento
    return 1; // sucesso
}


// Inicializa cartão SD (modo SPI)
#define ACMD41 0x29

uint8_t SD_Init(void) {
    uint8_t r, ocr[4];
    uint32_t timeout;

    // Inicia SPI em baixa velocidade (~281 kHz se clk = 72 MHz)
    SPI_Init(SPI_BAUDRATEPRESCALER_256);

    SD_CS_HIGH();
    for (int i = 0; i < 10; i++) SPI_Transfer(0xFF); // 80 clocks com CS alto

    SD_CS_LOW();
    if (SD_SendCommand(CMD0, 0, 0x95) != 0x01) {
        SD_CS_HIGH();
        return 0;
    }

    r = SD_SendCommand(CMD8, 0x1AA, 0x87);
    if (r == 0x01) {
        // SD v2.0
        timeout = 100000;
        do {
            SD_SendCommand(CMD55, 0, 0xFF);
            r = SD_SendCommand(ACMD41, 0x40000000, 0xFF); // HCS = 1
        } while (r != 0x00 && --timeout);
        if (timeout == 0) {
            SD_CS_HIGH();
            return 0;
        }

        // Verifica se é SDHC com CMD58
        if (SD_SendCommand(CMD58, 0, 0xFF) == 0x00) {
            for (int i = 0; i < 4; i++) {
                ocr[i] = SPI_Transfer(0xFF);
            }
            // Se o bit 30 do OCR estiver setado, é SDHC
            if (ocr[0] & 0x40) {
                // Cartão é SDHC (endereçamento por bloco)
            }
        }

    } else {
        // SD v1.x (sem resposta ao CMD8)
        timeout = 100000;
        do {
            r = SD_SendCommand(CMD55, 0, 0xFF);
            r = SD_SendCommand(ACMD41, 0, 0xFF);
        } while (r != 0x00 && --timeout);
        if (timeout == 0) {
            // Tenta CMD1 (SD antigo/MMC)
            timeout = 100000;
            do {
                r = SD_SendCommand(CMD1, 0, 0xFF);
            } while (r != 0x00 && --timeout);
            if (timeout == 0) {
                SD_CS_HIGH();
                return 0;
            }
        }
    }

    SD_CS_HIGH();
    SPI_Transfer(0xFF); // Pós-processamento SD

    // Aumenta clock do SPI após inicialização (ex: ~9 MHz com prescaler 8)
    SPI_Init(SPI_BAUDRATEPRESCALER_8);

    return 1; // sucesso
}

// Espera o cartão SD estar pronto (resposta != 0xFF)
uint8_t SD_WaitReady(void) {
    uint32_t timeout = 50000;
    while (--timeout && (SPI_Transfer(0xFF) != 0xFF)); //Espera resposta neutra do SD (0xFF)
    return (timeout != 0) ? 1 : 0; //Se der timeout, restorna 0 (Não inicializou)
}

/* Definição de Handler de processos SPI */

SPI_HandleTypeDef hspi1;

/* Inicializador de SPI HAL */ //TRANSFORMAR EM FUNÇÃO PRÓPRIA?

/* SPI2 init function */
void SPI_Init(uint32_t prescaler) {
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = prescaler;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 10;
    HAL_SPI_Init(&hspi1);
}

void HAL_SPI_MspInit(SPI_HandleTypeDef* spiHandle)
{

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(spiHandle->Instance==SPI1)
    {
    /* USER CODE BEGIN SPI1_MspInit 0 */

    /* USER CODE END SPI1_MspInit 0 */
      /* SPI1 clock enable */
      __HAL_RCC_SPI1_CLK_ENABLE();

      __HAL_RCC_GPIOA_CLK_ENABLE();
      /**SPI1 GPIO Configuration
      PA5     ------> SPI1_SCK
      PA6     ------> SPI1_MISO
      PA7     ------> SPI1_MOSI
      */
      GPIO_InitStruct.Pin = SD_SCK_Pin|SD_MOSI_Pin;
      GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
      HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

      GPIO_InitStruct.Pin = SD_MISO_Pin;
      GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      HAL_GPIO_Init(SD_MISO_GPIO_Port, &GPIO_InitStruct);
  }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* spiHandle)
{

	if(spiHandle->Instance==SPI1)
	{
	/* USER CODE BEGIN SPI1_MspDeInit 0 */

	/* USER CODE END SPI1_MspDeInit 0 */
	/* Peripheral clock disable */
	__HAL_RCC_SPI1_CLK_DISABLE();

	/**SPI1 GPIO Configuration
	PA5     ------> SPI1_SCK
	PA6     ------> SPI1_MISO
	PA7     ------> SPI1_MOSI
	*/
	HAL_GPIO_DeInit(GPIOA, SD_SCK_Pin|SD_MISO_Pin|SD_MOSI_Pin);

  }
}
