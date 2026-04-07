#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"

const int BTN_PIN_R = 28;
const int BTN_PIN_Y = 21;

const int LED_PIN_R = 5;
const int LED_PIN_Y = 10;

QueueHandle_t xQueueBtn;
SemaphoreHandle_t xSemaphoreLedR;
SemaphoreHandle_t xSemaphoreLedY;

void btn_callback(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_FALL) {
        xQueueSendFromISR(xQueueBtn, &gpio, NULL);
    }
}

void btn_task(void *p) {
    uint gpio;

    while (true) {
        if (xQueueReceive(xQueueBtn, &gpio, portMAX_DELAY)) {
            if (gpio == BTN_PIN_R) {
                xSemaphoreGive(xSemaphoreLedR);
            }
            if (gpio == BTN_PIN_Y) {
                xSemaphoreGive(xSemaphoreLedY);
            }
        }
    }
}

void led_r_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    int ativo = 0;

    while (true) {
        if (xSemaphoreTake(xSemaphoreLedR, 0)) {
            ativo = !ativo;
        }

        if (ativo) {
            gpio_put(LED_PIN_R, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        } else {
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

void led_y_task(void *p) {
    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);

    int ativo = 0;

    while (true) {
        if (xSemaphoreTake(xSemaphoreLedY, 0)) {
            ativo = !ativo;
        }

        if (ativo) {
            gpio_put(LED_PIN_Y, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(LED_PIN_Y, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        } else {
            gpio_put(LED_PIN_Y, 0);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

int main() {
    stdio_init_all();

    xQueueBtn = xQueueCreate(32, sizeof(uint));
    xSemaphoreLedR = xSemaphoreCreateBinary();
    xSemaphoreLedY = xSemaphoreCreateBinary();

    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);

    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, GPIO_IN);
    gpio_pull_up(BTN_PIN_Y);

    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
    gpio_set_irq_enabled(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true);

    xTaskCreate(btn_task, "BTN", 256, NULL, 1, NULL);
    xTaskCreate(led_r_task, "LED_R", 256, NULL, 1, NULL);
    xTaskCreate(led_y_task, "LED_Y", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (1);
}