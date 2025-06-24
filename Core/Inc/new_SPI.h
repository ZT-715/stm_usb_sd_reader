/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    new_SPI.h
  * @brief   arquivo com declarações de funções SPI simplificadas para SD
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __NEW_SPI_H__
#define __NEW_SPI_H__
// Includes
#include "main.h"


// SPI Interruption handler
extern SPI_HandleTypeDef hspi2;

// Defines
#define SD_CS_LOW()       HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET) //Finalizar comunicação
#define SD_CS_HIGH()      HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET)   //Iniciar comunicação

#define SD_BLOCK_LEN 512 //Tamanho padrão de blocos no SD

#define CMD0   0x00  // Reseta o cartão e entra no modo SPI
#define CMD1   0x01  // Inicializa o cartão (SD v1.x e MMC)
#define CMD8   0x08  // Verifica se o cartão é SD v2.0+ e aceita 2.7–3.6V
#define CMD9   0x09  // Lê o CSD (Card Specific Data)
#define CMD10  0x0A  // Lê o CID (Card Identification)
#define CMD12  0x0C  // Para a leitura contínua
#define CMD16  0x010 // Define o tamanho do bloco (em bytes) – geralmente 512
#define CMD17  0x11  // Lê um bloco único
#define CMD18  0x12  // Lê múltiplos blocos
#define CMD24  0x18  // Escreve um bloco único
#define CMD25  0x19  // Escreve múltiplos blocos
#define CMD55  0x37  // Prefixo para comandos da aplicação (ACMD)
#define ACMD41 0x29  // Inicializa SDHC/SDXC (seguido de CMD55)
#define CMD58  0x3A  // Lê o OCR (Operation Conditions Register) – útil para detectar SDHC

// Declarações de Funções
void MX_SPI2_Init(void);

#endif /* __NEW_SPI_H__ */
