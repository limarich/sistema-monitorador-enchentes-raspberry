
#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"

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
#define JOYSTICK_X 27         // pino do joystick X
#define JOYSTICK_Y 26         // pino do joystick Y

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

void vDisplayTask(void *pvParameters)
{

    ssd1306_t ssd; // Inicializa a estrutura do display
    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA);                                        // Pull up the data line
    gpio_pull_up(I2C_SCL);                                        // Pull up the clock line
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd);                                         // Configura o display
    ssd1306_send_data(&ssd);                                      // Envia os dados para o display

    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    while (1)
    {
        ssd1306_fill(&ssd, false); // LIMPA O DISPLAY
        ssd1306_draw_string(&ssd, "Sistema de monitoramento", 0, 0);
        ssd1306_draw_string(&ssd, "de enchentes", 0, 8);

        ssd1306_send_data(&ssd);         // ATUALIZA A TELA
        vTaskDelay(pdMS_TO_TICKS(1000)); // 1 segundo
    }
}

void vMatrixLedsTask(void *pvParameters)
{
    PIO pio;
    uint sm;
    // configurações da PIO
    pio = pio0;
    uint offset = pio_add_program(pio, &pio_matrix_program);
    sm = pio_claim_unused_sm(pio, true);
    pio_matrix_program_init(pio, sm, offset, LED_PIN);

    pixel draw[PIXELS];
    test_matrix(pio, sm);
    while (1)
    {
        // TESTE DA MATRIZ
        draw_traffic_light(pio, sm, GREEN, false);
        vTaskDelay(pdMS_TO_TICKS(1000));
        draw_traffic_light(pio, sm, YELLOW, false);
        vTaskDelay(pdMS_TO_TICKS(1000));
        draw_traffic_light(pio, sm, RED, false);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void vJoystickTask(void *pvParameters)
{
    adc_init(); // INICIALIZA O ADC

    // INICIALIZA O JOYSTICK
    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);

    while (1)
    {
        // LÊ O VALOR DO JOYSTICK
        adc_select_input(0);
        uint16_t x = adc_read();
        adc_select_input(1);
        uint16_t y = adc_read();

        printf("X: %d Y: %d\n", x, y); // IMPRIME O VALOR DO JOYSTICK

        vTaskDelay(pdMS_TO_TICKS(100)); // 100ms
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
        xTaskCreate(vDisplayTask, "Task de gerenciamento do display", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
        xTaskCreate(vMatrixLedsTask, "Task de gerenciamento da matriz", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
        xTaskCreate(vJoystickTask, "Task de gerenciamento do Joystick", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

        vTaskStartScheduler();
        panic_unsupported();
    }
}
