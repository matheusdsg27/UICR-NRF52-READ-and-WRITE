#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

// Definições de pinos
#define LED_1_PIN            17          // LED 1 - P0.17
#define LED_2_PIN            18          // LED 2 - P0.18
#define LED_3_PIN            19          // LED 3 - P0.19
#define LED_4_PIN            20          // LED 4 - P0.20
#define BUTTON_1_PIN         13          // Botão 1 - P0.13
#define UICR_CUSTOMER_0_ADDR 0x10001080  // Endereço da UICR

// Valores pré-definidos para gravação
#define OFFSET_TEMPERATURE   0x0A        // Offset da temperatura (10 em hexadecimal)
#define SETPOINT_TEMPERATURE 0x2EE       // Setpoint da temperatura (750 em hexadecimal)
#define MODE_OPERATION       0x01        // Modo de operação
#define SENSOR_STATUS        0x00        // Status do sensor

// Variável para controle de debouncing do botão
bool button_pressed = false;

// Inicialização dos LEDs e botão
void hardware_init(void) {
    // Configura LEDs como saída
    nrf_gpio_cfg_output(LED_1_PIN);
    nrf_gpio_cfg_output(LED_2_PIN);
    nrf_gpio_cfg_output(LED_3_PIN);
    nrf_gpio_cfg_output(LED_4_PIN);

    // Garante que todos os LEDs estejam apagados inicialmente (active low)
    nrf_gpio_pin_set(LED_1_PIN);
    nrf_gpio_pin_set(LED_2_PIN);
    nrf_gpio_pin_set(LED_3_PIN);
    nrf_gpio_pin_set(LED_4_PIN);

    // Configura o botão como entrada com resistor pull-up
    nrf_gpio_cfg_input(BUTTON_1_PIN, NRF_GPIO_PIN_PULLUP);
}

// Função para executar a saudação inicial com LEDs
void startup_greeting(void) {
    for (int i = 0; i < 2; i++) { // Piscar duas vezes
        nrf_gpio_pin_clear(LED_1_PIN);  // Liga LED 1
        nrf_gpio_pin_clear(LED_2_PIN);  // Liga LED 2
        nrf_gpio_pin_clear(LED_3_PIN);  // Liga LED 3
        nrf_gpio_pin_clear(LED_4_PIN);  // Liga LED 4
        nrf_delay_ms(200);              // Atraso de 200 ms
        nrf_gpio_pin_set(LED_1_PIN);    // Desliga LED 1
        nrf_gpio_pin_set(LED_2_PIN);    // Desliga LED 2
        nrf_gpio_pin_set(LED_3_PIN);    // Desliga LED 3
        nrf_gpio_pin_set(LED_4_PIN);    // Desliga LED 4
        nrf_delay_ms(200);              // Atraso de 200 ms
    }
}

// Função para processar os valores e escrever na UICR
void write_calibration_data(void) {
    // Concatena os valores pré-definidos em um único valor de 32 bits
    uint32_t data = 0;
    data |= (OFFSET_TEMPERATURE & 0xFF);           // Offset (bits 0-7)
    data |= (MODE_OPERATION & 0x03) << 10;         // Modo (bits 10-11)
    data |= (SENSOR_STATUS & 0x03) << 12;          // Status (bits 12-13)
    data |= (SETPOINT_TEMPERATURE & 0x3FF) << 14;  // Setpoint (bits 14-23)

    NRF_LOG_INFO("Valor concatenado para escrita: 0x%08X", data);

    // Habilita o apagamento na UICR
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Een;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy);
    NRF_NVMC->ERASEUICR = 1;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy);

    // Habilita escrita na UICR
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy);
    *(uint32_t *)UICR_CUSTOMER_0_ADDR = data;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy);

    // Retorna ao modo de somente leitura
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy);

    NRF_LOG_INFO("Dados gravados na UICR com sucesso!");
    NRF_LOG_FLUSH();

    // Reinicia o sistema
    NVIC_SystemReset();
}

// Função principal
int main(void) {
    // Inicializa sistema de logs
    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();
    NRF_LOG_INFO("Iniciando firmware de gravação de dados na UICR...");

    // Inicializa hardware (LEDs e botão)
    hardware_init();

    // Executa saudação inicial com LEDs
    startup_greeting();

    while (true) {
        // Verifica se o botão foi pressionado
        if (!button_pressed && nrf_gpio_pin_read(BUTTON_1_PIN) == 0) {
            button_pressed = true;  // Marca o botão como pressionado
            NRF_LOG_INFO("Botão 1 pressionado! Iniciando gravação...");
            write_calibration_data();  // Escreve os dados na UICR e reinicia
        } else if (nrf_gpio_pin_read(BUTTON_1_PIN) == 1) {
            button_pressed = false;  // Libera o selo do botão
        }

        NRF_LOG_FLUSH();  // Descarrega logs pendentes
    }
}
