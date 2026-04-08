#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>

const int BTN_PIN_R = 28;
const int BTN_PIN_G = 26;

const int LED_PIN_R = 4;
const int LED_PIN_G = 6;

QueueHandle_t xQueueButId_R;
QueueHandle_t xQueueButId_G;

TaskHandle_t btn1_task_handle = NULL;
TaskHandle_t btn2_task_handle = NULL;


void gpio_callback(uint gpio, uint32_t events) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (gpio == BTN_PIN_R) {
        vTaskNotifyGiveFromISR(btn1_task_handle, &xHigherPriorityTaskWoken);
    }

    if (gpio == BTN_PIN_G) {
        vTaskNotifyGiveFromISR(btn2_task_handle, &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void btn_1_task(void *p) {
    int delay = 100;

    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if (delay < 1000) delay += 100;
        else delay = 100;

        xQueueSend(xQueueButId_R, &delay, 0);
    }
}


void btn_2_task(void *p) {
    int delay = 100;

    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if (delay < 1000) delay += 100;
        else delay = 100;

        xQueueSend(xQueueButId_G, &delay, 0);
    }
}

void led_1_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    int delay = 0;
    while (true) {
        if (xQueueReceive(xQueueButId_R, &delay, 0)) {
            printf("%d\n", delay);
        }

        if (delay > 0) {
            gpio_put(LED_PIN_R, 1);
            vTaskDelay(pdMS_TO_TICKS(delay));
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(delay));
        }
    }
}

void led_2_task(void *p) {
    gpio_init(LED_PIN_G);
    gpio_set_dir(LED_PIN_G, GPIO_OUT);

    int delay = 0;
    while (true) {
        if (xQueueReceive(xQueueButId_G, &delay, 0)) {
            printf("%d\n", delay);
        }

        if (delay > 0) {
            gpio_put(LED_PIN_G, 1);
            vTaskDelay(pdMS_TO_TICKS(delay));
            gpio_put(LED_PIN_G, 0);
            vTaskDelay(pdMS_TO_TICKS(delay));
        }
    }
}


int main() {
    stdio_init_all();

    xQueueButId_R = xQueueCreate(10, sizeof(int));
    xQueueButId_G = xQueueCreate(10, sizeof(int));

    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);

    gpio_init(BTN_PIN_G);
    gpio_set_dir(BTN_PIN_G, GPIO_IN);
    gpio_pull_up(BTN_PIN_G);

    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled(BTN_PIN_G, GPIO_IRQ_EDGE_FALL, true);

    xTaskCreate(led_1_task, "LED_Task 1", 256, NULL, 1, NULL);
    xTaskCreate(btn_1_task, "BTN_Task 1", 256, NULL, 1, NULL);

    xTaskCreate(led_2_task, "LED_Task 2", 256, NULL, 1, NULL);
    xTaskCreate(btn_2_task, "BTN_Task 2", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true);
}