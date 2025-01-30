#include <stdio.h>
#include "pico/stdlib.h"
#include "ws2812.pio.h"
#include "hardware/pio.h"
#include "hardware/timer.h"
#include "digits.h"

#define IS_RGBW false
#define WS2812_PIN 7

#define RED_LED_PIN 13
#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 6

static int count = 0;

bool led_buffer[NUM_PIXELS]; // Buffer de LEDs
bool button_pressed = false; // Flag para indicar que um botão foi pressionado

void gpio_irq_handler(uint gpio, uint32_t events); // Função de tratamento de interrupção
void copy_array(bool *dest, const bool *src); // Função para copiar um array para outro

static inline void put_pixel(uint32_t pixel_grb); // Função para enviar um pixel para o buffer
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b); // Função para converter um pixel para um inteiro
void set_one_led(uint8_t r, uint8_t g, uint8_t b); // Função para definir a cor de todos os LEDs

int main()
{
    // Inicialização do PIO
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    stdio_init_all();

    // Configuração do pino do LED vermelho
    gpio_init(RED_LED_PIN);
    gpio_set_dir(RED_LED_PIN, GPIO_OUT);

    // Configuração do pinos do botão A
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);

    // Configuração do pinos do botão B
    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN);

    // Inicialização do programa do WS2812
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    // Configuração das interrupções dos botões
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // Inicialização do buffer de LEDs com o número 0
    copy_array(led_buffer, zero);
    set_one_led(255, 0, 0);

    while (true) {
        if (button_pressed) { // Verifica se um botão foi pressionado
            set_one_led(255, 0, 0);
        } else {
            button_pressed = false; // Atualiza a flag para indicar que nenhum botão foi pressionado
        }

        // Pisca o LED vermelho 5 vezes por 5 segundos
        for (int i = 0; i < 5; i++) {
            gpio_put(RED_LED_PIN, true);
            sleep_ms(50);
            gpio_put(RED_LED_PIN, false);
            sleep_ms(150);
        }
    }
}

void gpio_irq_handler(uint gpio, uint32_t events)
{
    volatile static uint32_t last_time = 0; // Último tempo que um botão foi pressionado
    volatile uint32_t current_time = to_ms_since_boot(get_absolute_time()); // Tempo atual

    // Verifica se o botão foi pressionado muito rapidamente
    if (current_time - last_time < 400) {
        return;
    }

    last_time = current_time; // Atualiza o tempo do último botão pressionado


    if (gpio == BUTTON_A_PIN) { // Verifica se o botão A foi pressionado
        count++; // Incrementa o contador

        if (count > 9) count = 0; // Verifica se o contador ultrapassou o limite
    } else if (gpio == BUTTON_B_PIN) { // Verifica se o botão B foi pressionado
        count--; // Decrementa o contador

        if (count < 0) count = 9; // Verifica se o contador ultrapassou o limite
    }

    // Define o buffer de LEDs com base no contador
    switch (count) {
        case 0:
            copy_array(led_buffer, zero);
            break;
        case 1:
            copy_array(led_buffer, one);
            break;
        case 2:
            copy_array(led_buffer, two);
            break;
        case 3:
            copy_array(led_buffer, three);
            break;
        case 4:
            copy_array(led_buffer, four);
            break;
        case 5:
            copy_array(led_buffer, five);
            break;
        case 6:
            copy_array(led_buffer, six);
            break;
        case 7:
            copy_array(led_buffer, seven);
            break;
        case 8:
            copy_array(led_buffer, eight);
            break;
        case 9:
            copy_array(led_buffer, nine);
            break;
    }

    button_pressed = true; // Atualiza a flag para indicar que um botão foi pressionado
}

void copy_array(bool *dest, const bool *src)
{
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        dest[i] = src[i];
    }
}

static inline void put_pixel(uint32_t pixel_grb)
{
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

void set_one_led(uint8_t r, uint8_t g, uint8_t b)
{
    // Define a cor com base nos parâmetros fornecidos
    uint32_t color = urgb_u32(r, g, b);

    // Define todos os LEDs com a cor especificada
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        if (led_buffer[i])
        {
            put_pixel(color); // Liga o LED com um no buffer
        }
        else
        {
            put_pixel(0);  // Desliga os LEDs com zero no buffer
        }
    }
}