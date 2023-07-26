#include "iic_common.h"

#include <stdio.h>

SemaphoreHandle_t print_mux = NULL;

void take_semaphore() 
{
    if(print_mux == NULL)
        print_mux = xSemaphoreCreateMutex();

    xSemaphoreTake(print_mux, portMAX_DELAY);
}

void give_semaphore()
{
    xSemaphoreGive(print_mux);
}

/**
 * @brief test function to show buffer
 */
void display_buffer_contents(uint8_t *buf, int len)
{
    for (int i = 0; i < len; i++) {
        printf("%c", buf[i]);
        if ((i + 1) % 128 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}
