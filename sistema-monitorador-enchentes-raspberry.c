
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

int main()
{
    stdio_init_all();

    while (true)
    {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
