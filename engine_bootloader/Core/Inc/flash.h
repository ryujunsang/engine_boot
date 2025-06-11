/**
 * @file    flash.h
 * @author  Ferenc Nemeth
 * @date    21 Dec 2018
 * @brief   This module handles the memory related functions.
 *
 *          Copyright (c) 2018 Ferenc Nemeth - https://github.com/ferenc-nemeth
 */

#ifndef FLASH_H_
#define FLASH_H_

//#include "stm32f1xx_hal.h"
#include "stm32g4xx_hal.h"

/* Start and end addresses of the user application. */
/* STM32G474QETx 기준 (512KB 플래시) */
// STM32G4 시리즈에 맞게 메모리 정의 수정
#define FLASH_PAGE_SIZE     0x800U  // 2KB (STM32G4의 페이지 크기)
#define FLASH_SIZE          (512U * 1024U)  // G474의 플래시 크기 (모델별로 다를 수 있음)
#define FLASH_BANK_SIZE     (FLASH_SIZE / 2)
#define FLASH_BASE          0x08000000U
#define FLASH_BANK1_END     (FLASH_BASE + FLASH_BANK_SIZE)
#define FLASH_APP_START_ADDRESS  0x08010000  // 필요에 따라 조정
#define FLASH_APP_END_ADDRESS    (FLASH_BANK1_END - 0x10U)

// for config flash
#define FLASH_CONFIG_START_ADDRESS  0x0807E000  // 설정데이터 시작 주소 (440K 이후)
#define FLASH_CONFIG_END_ADDRESS    0x0807FFFF  // 설정데이터 끝 주소 (8K 영역)
#define FLASH_CONFIG_SIZE           0x2000      // 8K 크기





/* Status report for the functions. */
typedef enum {
  FLASH_OK              = 0x00u, /**< The action was successful. */
  FLASH_ERROR_SIZE      = 0x01u, /**< The binary is too big. */
  FLASH_ERROR_WRITE     = 0x02u, /**< Writing failed. */
  FLASH_ERROR_READBACK  = 0x04u, /**< Writing was successful, but the content of the memory is wrong. */
  FLASH_ERROR           = 0xFFu  /**< Generic error. */
} flash_status;

flash_status flash_erase(uint32_t address);
flash_status flash_write(uint32_t address, uint32_t *data, uint32_t length);
void flash_jump_to_app(void);

#endif /* FLASH_H_ */
