
#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "queue.h"
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

typedef struct
{
    float rain_level;
    float water_level;
} StatusLevel;

QueueHandle_t statusQueue;
#define QUEUE_LENGTH 10
#define QUEUE_ITEM_SIZE sizeof(StatusLevel)

#define LED_RED 13           // pino do led vermelho
#define LED_BLUE 12          // pino do led azul
#define LED_GREEN 11         // pino do led verde
#define signalDelay 1000     // tempo que o led fica aceso
#define BUTTON_A 5           // BOTÃO B para mudar o modo do semáforo
#define BUTTON_B 6           // BOTÃO A para desligar o beep
#define DEBOUNCE_MS 200      // intervalo minimo de 200ms para o debounce
#define BUZZER_A 10          // PORTA DO BUZZER A
#define BUZZER_B 21          // PORTA DO BUZZER B
#define BUZZER_FREQUENCY 200 // FREQUENCIA DO BUZZER
#define I2C_PORT i2c1        // PORTA DO i2C
#define I2C_SDA 14           // PINO DO SDA
#define I2C_SCL 15           // PINO DO SCL
#define endereco 0x3C        // ENDEREÇO
#define JOYSTICK_X 27        // pino do joystick X
#define JOYSTICK_Y 26        // pino do joystick Y

// inicializacao da PIO
void PIO_setup(PIO *pio, uint *sm);
// inicializa o display
void ssd_setup();

/* DEFINIÇÃO DAS TASKS*/

void vLedTask(void *pvParameters)
{
    StatusLevel statusLevel; // Estrutura para armazenar os níveis de água e chuva

    // Inicializa os pinos do LED RGB
    gpio_init(LED_RED);
    gpio_init(LED_GREEN);
    gpio_init(LED_BLUE);

    // Define os pinos como saída
    gpio_set_dir(LED_RED, GPIO_OUT);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_set_dir(LED_BLUE, GPIO_OUT);

    while (1)
    {
        // Tenta ler os dados da fila indefinidamente
        if (xQueueReceive(statusQueue, &statusLevel, portMAX_DELAY) == pdPASS)
        {
            // Verifica se o nível de água está acima de 70%
            if (statusLevel.water_level > 0.70)
            {
                // Verifica se o nível de chuva também está acima de 80%
                if (statusLevel.rain_level > 0.80)
                {
                    // LED Vermelho (Nível Crítico de Água e Chuva)
                    for (int i = 0; i < 2; i++)
                    {
                        gpio_put(LED_RED, 1); // Liga o LED vermelho
                        vTaskDelay(100);      // Atraso de 100ms
                        gpio_put(LED_RED, 0); // Desliga o LED vermelho
                        vTaskDelay(100);      // Atraso de 100ms
                    }
                }
                else
                {
                    // LED Azul (Nível Crítico de Água, Chuva Moderada)
                    for (int i = 0; i < 2; i++)
                    {
                        gpio_put(LED_BLUE, 1); // Liga o LED azul
                        vTaskDelay(100);       // Atraso de 100ms
                        gpio_put(LED_BLUE, 0); // Desliga o LED azul
                        vTaskDelay(100);       // Atraso de 100ms
                    }
                }
            }
            // Verifica se apenas o nível de chuva está acima de 80%
            else if (statusLevel.rain_level > 0.80)
            {
                // LED Amarelo (Nível Crítico de Chuva, Água Moderada)
                for (int i = 0; i < 2; i++)
                {
                    gpio_put(LED_RED, 1);   // Liga o LED vermelho
                    gpio_put(LED_GREEN, 1); // Liga o LED verde (amarelo)
                    vTaskDelay(100);        // Atraso de 100ms
                    gpio_put(LED_RED, 0);   // Desliga o LED vermelho
                    gpio_put(LED_GREEN, 0); // Desliga o LED verde
                    vTaskDelay(100);        // Atraso de 100ms
                }
            }
            else
            {
                // LED Verde (Condições Normais)
                gpio_put(LED_GREEN, 1); // Liga o LED verde
                vTaskDelay(300);        // Atraso de 300ms
                gpio_put(LED_GREEN, 0); // Desliga o LED verde
                vTaskDelay(300);        // Atraso de 300ms
            }
        }
    }
}

void vBuzzerTask(void *pvParameters)
{
    StatusLevel statusLevel; // Estrutura para armazenar os níveis de água e chuva

    // Inicializa os buzzers
    initialization_buzzers(BUZZER_A, BUZZER_B);

    while (1)
    {
        // Tenta ler os dados da fila indefinidamente
        if (xQueueReceive(statusQueue, &statusLevel, portMAX_DELAY) == pdPASS)
        {
            // Verifica se o nível de água está acima de 70%
            if (statusLevel.water_level > 0.70)
            {
                // Verifica se o nível de chuva também está acima de 80%
                if (statusLevel.rain_level > 0.80)
                {
                    // Ativa os buzzers com uma frequência mais alta para indicar estado crítico
                    buzzer_pwm(BUZZER_A, BUZZER_FREQUENCY * 8, 100); // Ativa o Buzzer A por 100ms
                    vTaskDelay(pdMS_TO_TICKS(50));                   // Aguarda 50ms
                    buzzer_pwm(BUZZER_B, BUZZER_FREQUENCY * 8, 100); // Ativa o Buzzer B por 100ms
                    vTaskDelay(pdMS_TO_TICKS(50));                   // Aguarda 50ms
                    buzzer_pwm(BUZZER_A, BUZZER_FREQUENCY * 8, 100); // Ativa o Buzzer A por 100ms
                    vTaskDelay(pdMS_TO_TICKS(50));                   // Aguarda 50ms
                }
                else
                {
                    // Ativa os buzzers com frequência moderada para indicar nível de água alto
                    buzzer_pwm(BUZZER_A, BUZZER_FREQUENCY * 3, 30); // Ativa o Buzzer A por 30ms
                    vTaskDelay(pdMS_TO_TICKS(50));                  // Aguarda 50ms
                    buzzer_pwm(BUZZER_B, BUZZER_FREQUENCY * 3, 30); // Ativa o Buzzer B por 30ms
                    vTaskDelay(pdMS_TO_TICKS(50));                  // Aguarda 50ms
                    buzzer_pwm(BUZZER_A, BUZZER_FREQUENCY * 3, 30); // Ativa o Buzzer A por 30ms
                    vTaskDelay(pdMS_TO_TICKS(50));                  // Aguarda 50ms
                }
            }
            // Verifica se apenas o nível de chuva está acima de 80%
            else if (statusLevel.rain_level > 0.80)
            {
                // Ativa os buzzers com frequência muito alta para indicar nível crítico de chuva
                buzzer_pwm(BUZZER_B, BUZZER_FREQUENCY * 15, 30); // Ativa o Buzzer B por 30ms
                vTaskDelay(pdMS_TO_TICKS(50));                   // Aguarda 50ms
                buzzer_pwm(BUZZER_A, BUZZER_FREQUENCY * 15, 30); // Ativa o Buzzer A por 30ms
                vTaskDelay(pdMS_TO_TICKS(50));                   // Aguarda 50ms
                buzzer_pwm(BUZZER_B, BUZZER_FREQUENCY * 15, 30); // Ativa o Buzzer B por 30ms
                vTaskDelay(pdMS_TO_TICKS(50));                   // Aguarda 50ms
            }
        }
    }
}

void vDisplayTask(void *pvParameters)
{

    StatusLevel statusLevel; // Estrutura para armazenar os dados dos sensores

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

    char chuva[16], agua[16]; // Variáveis para armazenar os dados do joystick

    while (1)
    {
        ssd1306_fill(&ssd, false); // LIMPA O DISPLAY
        ssd1306_draw_string(&ssd, "Sistema de monitoramento", 0, 0);
        ssd1306_draw_string(&ssd, "de enchentes", 0, 8);
        ssd1306_draw_string(&ssd, "", 0, 16);
        ssd1306_draw_string(&ssd, "agua:", 0, 32);
        if (xQueueReceive(statusQueue, &statusLevel, portMAX_DELAY) == pdPASS)
        {
            snprintf(chuva, sizeof(chuva), "chuva: %.2f%%", statusLevel.rain_level * 100); // formata a leitura do joystick
            snprintf(agua, sizeof(agua), "agua: %.2f%%", statusLevel.water_level * 100);   // formata a leitura do joystick

            ssd1306_draw_string(&ssd, chuva, 0, 16);
            ssd1306_draw_string(&ssd, agua, 0, 32);
        }

        ssd1306_send_data(&ssd); // ATUALIZA A TELA
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void vMatrixLedsTask(void *pvParameters)
{
    StatusLevel statusLevel; // Estrutura para armazenar os dados dos sensores

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

        if (xQueueReceive(statusQueue, &statusLevel, portMAX_DELAY) == pdPASS)
        {
            draw_status_level(pio, sm, statusLevel.water_level, statusLevel.rain_level); // desenha o grafico
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void vJoystickTask(void *pvParameters)
{
    adc_init(); // INICIALIZA O ADC

    // INICIALIZA O JOYSTICK
    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);

    // Lê os valores de idle (posição neutra)
    adc_select_input(0);
    uint16_t y_idle = adc_read();
    adc_select_input(1);
    uint16_t x_idle = adc_read();

    while (1)
    {
        // LÊ O VALOR DO JOYSTICK
        adc_select_input(0);
        int16_t y_raw = adc_read() - y_idle;
        adc_select_input(1);
        int16_t x_raw = adc_read() - x_idle;

        // NORMALIZA PARA A FAIXA 0.0 a 1.0
        float y_normalized = (float)(y_raw + 2048) / 4095.0f;
        float x_normalized = (float)(x_raw + 2048) / 4095.0f;

        // GARANTE QUE OS VALORES FIQUEM ENTRE 0 e 1
        if (y_normalized < 0.0f)
            y_normalized = 0.0f;
        if (y_normalized > 1.0f)
            y_normalized = 1.0f;
        if (x_normalized < 0.0f)
            x_normalized = 0.0f;
        if (x_normalized > 1.0f)
            x_normalized = 1.0f;

        // CRIA O OBJETO DE DADOS
        StatusLevel statusLevel = {
            .rain_level = x_normalized,
            .water_level = y_normalized};

        // ENVIA PARA A FILA
        if (xQueueSend(statusQueue, &statusLevel, pdMS_TO_TICKS(125)) == pdPASS)
        {
            printf("Enviado para a fila: chuva=%.2f, agua=%.2f\n", statusLevel.rain_level, statusLevel.water_level);
        }
        else
        {
            printf("Fila cheia, dados perdidos!\n");
        }

        vTaskDelay(pdMS_TO_TICKS(250)); // 250ms
    }
}

int main()
{
    stdio_init_all();

    // Criação da fila
    statusQueue = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);
    if (statusQueue == NULL)
    {
        printf("Erro ao criar a fila dos sensores.\n");
        return 1;
    }

    while (true)
    {
        // REGISTRO DAS TASKS
        xTaskCreate(vLedTask, "Task de gerenciamento do LED", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
        xTaskCreate(vBuzzerTask, "Task de gerenciamento do Buzzer", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
        xTaskCreate(vDisplayTask, "Task de gerenciamento do display", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
        xTaskCreate(vMatrixLedsTask, "Task de gerenciamento da matriz", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
        xTaskCreate(vJoystickTask, "Task de gerenciamento do Joystick", configMINIMAL_STACK_SIZE, NULL, 2, NULL);

        vTaskStartScheduler();
        panic_unsupported();
    }
}
