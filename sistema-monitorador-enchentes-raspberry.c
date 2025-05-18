
#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/i2c.h"

#include "pio_matrix.pio.h"
#include "lib/buzzer.h"
#include "lib/leds.h"
#include "lib/ssd1306.h"
#include "lib/font.h"

#define LED_RED 13            // pino do led vermelho
#define LED_BLUE 12           // pino do led azul
#define LED_GREEN 11          // pino do led verde
#define signalDelay 1000      // tempo que o led fica aceso
#define BUTTON_A 5            // BOTÃO B para mudar o modo do semáforo
#define BUTTON_B 6            // BOTÃO A para desligar o beep
#define DEBOUNCE_MS 200       // intervalo minimo de 200ms para o debounce
#define BUZZER_A 10           // PORTA DO BUZZER A
#define BUZZER_B 21           // PORTA DO BUZZER B
#define BUZZER_FREQUENCY 1500 // FREQUENCIA DO BUZZER
#define I2C_PORT i2c1         // PORTA DO i2C
#define I2C_SDA 14            // PINO DO SDA
#define I2C_SCL 15            // PINO DO SCL
#define endereco 0x3C         // ENDEREÇO

// variaveis relacionadas a matriz de led
PIO pio;
uint sm;
// variavel do display
ssd1306_t ssd; // Inicializa a estrutura do display

// inicializacao da PIO
void PIO_setup(PIO *pio, uint *sm);
// inicializa o display
void ssd_setup();

/* DEFINIÇÃO DAS TASKS*/

void vLedTask(void *pvParameters)
{
    // INICIALIZA OS PINOS DO LED RGB
    gpio_init(LED_RED);
    gpio_init(LED_GREEN);
    gpio_init(LED_BLUE);

    // DEFINE OS PINOS COMO SAÍDA
    gpio_set_dir(LED_RED, GPIO_OUT);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_set_dir(LED_BLUE, GPIO_OUT);

    while (1)
    {

        // LIGA O LED VERMELHO
        gpio_put(LED_RED, 1);
        vTaskDelay(signalDelay / portTICK_PERIOD_MS);
        // DESLIGA O LED VERMELHO
        gpio_put(LED_RED, 0);
        vTaskDelay(signalDelay / portTICK_PERIOD_MS);

        // LIGA O LED VERDE
        gpio_put(LED_GREEN, 1);
        vTaskDelay(signalDelay / portTICK_PERIOD_MS);
        // DESLIGA O LED VERDE
        gpio_put(LED_GREEN, 0);
        vTaskDelay(signalDelay / portTICK_PERIOD_MS);

        // LIGA O LED AZUL
        gpio_put(LED_BLUE, 1);
        vTaskDelay(signalDelay / portTICK_PERIOD_MS);
        // DESLIGA O LED AZUL
        gpio_put(LED_BLUE, 0);
        vTaskDelay(signalDelay / portTICK_PERIOD_MS);
    }
}

void vBuzzerTask(void *pvParameters)
{
    // INCIALIZA OS BUZZERS
    initialization_buzzers(BUZZER_A, BUZZER_B);

    while (1)
    {

        buzzer_pwm(BUZZER_A, BUZZER_FREQUENCY, 1000);     // 1000ms
        vTaskDelay(pdMS_TO_TICKS(1000));                  // 1 segundo
        buzzer_pwm(BUZZER_B, BUZZER_FREQUENCY * 3, 1000); // 1000ms
        vTaskDelay(pdMS_TO_TICKS(1000));                  // 1 segundo
        buzzer_pwm(BUZZER_A, BUZZER_FREQUENCY * 6, 1000); // 1000ms
        vTaskDelay(pdMS_TO_TICKS(1000));                  // 1 segundo
    }
}

int main()
{
    stdio_init_all();

    while (true)
    {
        // REGISTRO DAS TASKS
        xTaskCreate(vLedTask, "Task de gerenciamento do LED", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
        // xTaskCreate(vBuzzerTask, "Task de gerenciamento do Buzzer", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

        vTaskStartScheduler();
        panic_unsupported();
    }
}
