#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "shinyAllocator.h"
// @brief retargeting stdio
int _write(int file, char *ptr, int len)
{
    CDC_Transmit_FS((uint8_t *)ptr, len);
    osDelay(1);
    return len;
}
#define ERROR 0
#define SUCESS 1

const size_t KiB = 1024U;
const size_t MiB = KiB * KiB;

uint_fast8_t case1()
{
    uint_fast8_t result = SUCESS;

    const size_t KiB4 = KiB * 4;
    const size_t arenaSize = KiB4 + sizeof_shinyAllocatorInstance() + SHINYALLOCATOR_ALIGNMENT + 1U;

    __attribute__((aligned(128))) uint_fast8_t arena[arenaSize];
    shinyAllocatorInstance *pool = shinyInit(arena, arenaSize);
    result = (pool != (shinyAllocatorInstance *)NULL) ? SUCESS : ERROR;
    result = (shinyAllocate(pool, 0U) == (shinyAllocatorInstance *)NULL) ? SUCESS : ERROR;
    result = (shinyGetDiagnostics(pool).outOfMemeoryCount == 0U) ? SUCESS : ERROR;
    result = (shinyGetDiagnostics(pool).peakAllocated == 0U) ? SUCESS : ERROR;
    result = (shinyGetDiagnostics(pool).allocated == 0U) ? SUCESS : ERROR;
    result = (shinyGetDiagnostics(pool).peakRequestSize == 0U) ? SUCESS : ERROR;
    void *ptr = shinyAllocate(pool, KiB4 - SHINYALLOCATOR_ALIGNMENT);
    result = (ptr != (shinyAllocatorInstance *)NULL) ? SUCESS : ERROR;
    result = (shinyGetDiagnostics(pool).outOfMemeoryCount == 0U) ? SUCESS : ERROR;
    result = (shinyGetDiagnostics(pool).peakAllocated == KiB4) ? SUCESS : ERROR;
    result = (shinyGetDiagnostics(pool).peakRequestSize == KiB4 - SHINYALLOCATOR_ALIGNMENT) ? SUCESS : ERROR;
    result = (shinyGetDiagnostics(pool).allocated == KiB4) ? SUCESS : ERROR;
    shinyFree(pool, ptr);
    result = (shinyGetDiagnostics(pool).allocated == 0) ? SUCESS : ERROR;
    return result;
}

void case2()
{
    uint_fast8_t result = SUCESS;
    const size_t KiB4 = KiB * 4;
    const size_t arenaSize = KiB4 + sizeof_shinyAllocatorThreadSafeInstance() + sizeof_shinyAllocatorInstance() + SHINYALLOCATOR_ALIGNMENT + 1U;
    void *arena = (char *)aligned_alloc(128, arenaSize);

    auto pool = shinyInitThreadSafe(arena, arenaSize);
    result = (pool== (shinyAllocatorThreadSafeInstance *)NULL)? SUCESS:ERROR;
}
void test()
{
    printf("==================================\n");
    printf("Testing\n");
    printf("==================================\n");
    if (case1() == ERROR)
    {
        printf("\tCase 1 failed\n");
        return;
    }
    printf("==================================\n");
    printf("Tests Passesed!\n");
    printf("==================================\n");
}
