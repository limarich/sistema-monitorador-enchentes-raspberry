#include "leds.h"

// Rotina para definição da intensidade de cores do LED
uint32_t matrix_rgb(uint r, uint g, uint b, float intensity)
{
    uint8_t R = (uint8_t)(r * intensity);
    uint8_t G = (uint8_t)(g * intensity);
    uint8_t B = (uint8_t)(b * intensity);
    return (G << 24) | (R << 16) | (B << 8);
}

// Rotina para acionar a matriz de LEDs - WS2812B
void draw_pio(pixel *draw, PIO pio, uint sm)
{
    for (int16_t i = 0; i < PIXELS; i++)
    {
        uint32_t valor_led = matrix_rgb(draw[i].red, draw[i].green, draw[i].blue, draw[i].intensity);
        pio_sm_put_blocking(pio, sm, valor_led);
    }
}

// testa a matriz de leds
void test_matrix(PIO pio, uint sm)
{
    frame test_frame, black_frame;

    pixel red = {255, 0, 0, 1}, black = {0, 0, 0, 1};

    for (int16_t i = 0; i < PIXELS; i++)
    {
        test_frame[i] = red;
        black_frame[i] = black;
        draw_pio(test_frame, pio, sm);
        sleep_ms(50);
    }
    draw_pio(black_frame, pio, sm);
    sleep_ms(50);
}

// determina o padrão do pixel baseado na cor passada
pixel handle_color(color_options color, float intensity)
{
    pixel pixel_color = {0, 0, 0};
    switch (color)
    {
    case BLACK:
        pixel_color.red = 0;
        pixel_color.green = 0;
        pixel_color.blue = 0;
        pixel_color.intensity = intensity;
        break;
    case BROWN:
        pixel_color.red = 165;
        pixel_color.green = 25;
        pixel_color.blue = 0;
        pixel_color.intensity = intensity;
        break;
    case RED:
        pixel_color.red = 255;
        pixel_color.green = 0;
        pixel_color.blue = 0;
        pixel_color.intensity = intensity;
        break;
    case ORANGE:
        pixel_color.red = 205;
        pixel_color.green = 43;
        pixel_color.blue = 0;
        pixel_color.intensity = intensity;
        break;
    case YELLOW:
        pixel_color.red = 255;
        pixel_color.green = 255;
        pixel_color.blue = 0;
        pixel_color.intensity = intensity;
        break;
    case GREEN:
        pixel_color.red = 0;
        pixel_color.green = 255;
        pixel_color.blue = 0;
        pixel_color.intensity = intensity;
        break;
    case BLUE:
        pixel_color.red = 0;
        pixel_color.green = 0;
        pixel_color.blue = 255;
        pixel_color.intensity = intensity;
        break;
    case VIOLET:
        pixel_color.red = 121;
        pixel_color.green = 8;
        pixel_color.blue = 205;
        pixel_color.intensity = intensity;
        break;
    case GRAY:
        pixel_color.red = 55;
        pixel_color.green = 55;
        pixel_color.blue = 55;
        pixel_color.intensity = intensity;
        break;
    case WHITE:
        pixel_color.red = 255;
        pixel_color.green = 255;
        pixel_color.blue = 255;
        pixel_color.intensity = intensity;
        break;
    default:
        break;
    }

    return pixel_color;
}

void draw_status_level(PIO pio, uint sm, float water_level, float rain_level)
{

    pixel black = handle_color(BLACK, 1);

    frame matrix;

    // indices da matriz reservados para o nivel de agua e chuva
    uint water_index[5] = {1, 8, 11, 18, 21};
    uint rain_index[5] = {3, 6, 13, 16, 23};

    // brilho da agua e chuva
    float water_brightness = water_level * 1;
    float rain_brightness = rain_level * 1;

    // intensidade do nivel de agua e chuva
    int water_intensity = (int)(water_level * 5);
    int rain_intensity = (int)(rain_level * 5);

    pixel water_color = handle_color(BLUE, water_brightness);
    pixel rain_color = handle_color(GRAY, rain_brightness);

    // uint a matriz com a cor preta
    for (int16_t i = 0; i < PIXELS; i++)
    {
        matrix[i] = black;
    }

    // Preenche os LEDs para o nível de água
    for (int i = 0; i < 5; i++)
    {
        if (i < water_intensity)
        {
            matrix[water_index[i]] = water_color;
        }
    }

    // Preenche os LEDs para o nível de chuva
    for (int i = 0; i < 5; i++)
    {
        if (i < rain_intensity)
        {
            matrix[rain_index[i]] = rain_color;
        }
    }

    draw_pio(matrix, pio, sm);
}
