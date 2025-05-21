/**
 * @file    flash.c
 * @author  Ferenc Nemeth
 * @date    21 Dec 2018
 * @brief   This module handles the memory related functions.
 *
 *          Copyright (c) 2018 Ferenc Nemeth - https://github.com/ferenc-nemeth
 */

#include "flash.h"

/* Function pointer for jumping to user application. */
typedef void (*fnc_ptr)(void);


/**
 * @brief 주소에 해당하는 플래시 페이지 번호 가져오기
 * @param Addr: 플래시 메모리 주소
 * @return 페이지 번호
 */
static uint32_t GetPage(uint32_t Addr)
{
    return (Addr - FLASH_BASE) / FLASH_PAGE_SIZE;
}

/**
 * @brief 주소에 해당하는 플래시 뱅크 번호 가져오기
 * @param Addr: 플래시 메모리 주소
 * @return 뱅크 번호 (FLASH_BANK_1 또는 FLASH_BANK_2)
 */
static uint32_t GetBank(uint32_t Addr)
{
    uint32_t bank = 0;

    if (READ_BIT(SYSCFG->MEMRMP, SYSCFG_MEMRMP_FB_MODE) == 0)
    {
        /* 뱅크 스왑 없음 */
        if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
        {
            bank = FLASH_BANK_1;
        }
        else
        {
            bank = FLASH_BANK_2;
        }
    }
    else
    {
        /* 뱅크 스왑 */
        if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
        {
            bank = FLASH_BANK_2;
        }
        else
        {
            bank = FLASH_BANK_1;
        }
    }

    return bank;
}

/**
 * @brief   This function erases the memory.
 * @param   address: First address to be erased (the last is the end of the flash).
 * @return  status: Report about the success of the erasing.
 */
flash_status flash_erase(uint32_t address)
{
    HAL_FLASH_Unlock();
    flash_status status = FLASH_ERROR;
    FLASH_EraseInitTypeDef erase_init;
    uint32_t error = 0u;

    erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_init.Page = GetPage(address);  // 주소를 페이지 번호로 변환하는 함수 필요
    erase_init.Banks = GetBank(address);  // 주소에 해당하는 뱅크 계산

    /* G4는 페이지 수 계산 방식이 다름 */
    uint32_t end_page = GetPage(FLASH_APP_END_ADDRESS);
    erase_init.NbPages = end_page - erase_init.Page + 1;

    if (HAL_OK == HAL_FLASHEx_Erase(&erase_init, &error))
    {
        status = FLASH_OK;
    }

    HAL_FLASH_Lock();
    return status;
}


/**
 * @brief   This function flashes the memory.
 * @param   address: First address to be written to.
 * @param   *data:   Array of the data that we want to write.
 * @param   *length: Size of the array.
 * @return  status: Report about the success of the writing.
 */
flash_status flash_write(uint32_t address, uint32_t *data, uint32_t length)
{
    flash_status status = FLASH_OK;
    HAL_FLASH_Unlock();

    /* G4는 64비트(더블워드) 단위로 프로그래밍해야 함 */
    uint64_t temp_data;

    /* 8바이트 정렬을 확인 */
    if (address % 8 != 0)
    {
        status |= FLASH_ERROR_WRITE;
        HAL_FLASH_Lock();
        return status;
    }

    /* 32비트 데이터를 64비트로 변환하여 쓰기 */
    for (uint32_t i = 0; i < length; i += 2)
    {
        /* 끝에 도달했는지 확인 */
        if (address >= FLASH_APP_END_ADDRESS)
        {
            status |= FLASH_ERROR_SIZE;
            break;
        }

        /* 64비트 데이터 구성 */
        if (i + 1 < length) {
            temp_data = ((uint64_t)data[i+1] << 32) | data[i];
        } else {
            temp_data = (uint64_t)data[i]; // 홀수 개수 처리
        }

        /* 실제 플래시 쓰기 */
        if (HAL_OK != HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, temp_data))
        {
            status |= FLASH_ERROR_WRITE;
            break;
        }

        /* 읽기 확인 */
        if (temp_data != (*(volatile uint64_t*)address))
        {
            status |= FLASH_ERROR_READBACK;
            break;
        }

        /* 다음 더블워드 위치로 이동 */
        address += 8;
    }

    HAL_FLASH_Lock();
    return status;
}


/**
 * @brief   Actually jumps to the user application.
 * @param   void
 * @return  void
 */
#if 1
void flash_jump_to_app(void)
{
  /* Function pointer to the address of the user application. */
  fnc_ptr jump_to_app;
  jump_to_app = (fnc_ptr)(*(volatile uint32_t*) (FLASH_APP_START_ADDRESS+4u));
  HAL_DeInit();
  /* Change the main stack pointer. */
  __set_MSP(*(volatile uint32_t*)FLASH_APP_START_ADDRESS);
  jump_to_app();
}
#else
void flash_jump_to_app(void) {
    if((*(volatile uint32_t*)APP_ADDRESS & 0x2FFE0000) == 0x20000000) {
        fnc_ptr jump_app = (fnc_ptr)(*(volatile uint32_t*)(APP_ADDRESS + 4));
        __disable_irq();
        SysTick->CTRL = 0;
        SCB->VTOR = APP_ADDRESS;
        __set_MSP(*(volatile uint32_t*)APP_ADDRESS);
        jump_app();
    }
}
#endif
