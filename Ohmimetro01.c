/*
 * Por: Wilton Lacerda Silva
 *    Ohmímetro utilizando o ADC da BitDogLab
 *    editado por muriel costa
 *
 * 
 * Neste exemplo, utilizamos o ADC do RP2040 para medir a resistência de um resistor
 * desconhecido, utilizando um divisor de tensão com dois resistores.
 * O resistor conhecido é de 10k ohm e o desconhecido é o que queremos medir.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>  // Adicionado para fabs()
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define ADC_PIN 28 // GPIO para o voltímetro
#define Botao_A 5  // GPIO para botão A

int R_conhecido = 10000;   // Resistor de 10k ohm
float R_x = 0.0;           // Resistor desconhecido
int ADC_RESOLUTION = 4095; // Resolução do ADC (12 bits)

// Função para encontrar o resistor comercial mais próximo da série E24
float encontrar_resistor_comercial(float resistencia) {
    // Valores da série E24
    const float serie_e24[] = {
        1.0, 1.1, 1.2, 1.3, 1.5, 1.6, 1.8, 2.0, 2.2, 2.4, 2.7, 3.0,
        3.3, 3.6, 3.9, 4.3, 4.7, 5.1, 5.6, 6.2, 6.8, 7.5, 8.2, 9.1
    };
    const int num_valores = 24;
    
    // Caso a resistência seja zero ou negativa
    if (resistencia <= 0) return 0;
    
    // Encontrar a década (1, 10, 100, 1k, 10k, etc.)
    float multiplicador = 1.0;
    float valor_normalizado = resistencia;
    
    // Normalizar para um valor entre 1.0 e 10.0
    while (valor_normalizado >= 10.0) {
        valor_normalizado /= 10.0;
        multiplicador *= 10.0;
    }
    
    while (valor_normalizado < 1.0 && valor_normalizado > 0) {
        valor_normalizado *= 10.0;
        multiplicador /= 10.0;
    }
    
    // Encontrar o valor mais próximo na série E24
    float diferenca_minima = 1000.0;
    float valor_mais_proximo = 1.0;
    
    for (int i = 0; i < num_valores; i++) {
        float diferenca = fabs(valor_normalizado - serie_e24[i]);
        if (diferenca < diferenca_minima) {
            diferenca_minima = diferenca;
            valor_mais_proximo = serie_e24[i];
        }
    }
    
    // Retornar o valor comercial mais próximo
    return valor_mais_proximo * multiplicador;
}

// Função para obter as cores do resistor (simplificada para faixa 500Ω-100kΩ)
void obter_cores_resistor(float valor, char *cor1, char *cor2, char *cor3) {
    // Tabela de cores
    const char *cores[] = {"Preto", "Marrom", "Vermelho", "Laranja", "Amarelo", 
                          "Verde", "Azul", "Violeta", "Cinza", "Branco"};
    
    // Normaliza o valor para obter dígitos
    float valor_norm = valor;
    int multiplicador = 0;
    
    // Normalizar para um valor entre 10.0 e 99.9
    while (valor_norm >= 100.0) {
        valor_norm /= 10.0;
        multiplicador++;
    }
    
    while (valor_norm < 10.0) {
        valor_norm *= 10.0;
        multiplicador--;
    }
    
    // Obter os dois primeiros dígitos
    int primeiro_digito = (int)(valor_norm / 10.0);
    int segundo_digito = (int)(valor_norm) % 10;
    
    // Atribui as cores
    strcpy(cor1, cores[primeiro_digito]);
    strcpy(cor2, cores[segundo_digito]);
    strcpy(cor3, cores[multiplicador]);
}

// Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define botaoB 6
void gpio_irq_handler(uint gpio, uint32_t events)
{
  reset_usb_boot(0, 0);
}

int main()
{
  // Para ser utilizado o modo BOOTSEL com botão B
  gpio_init(botaoB);
  gpio_set_dir(botaoB, GPIO_IN);
  gpio_pull_up(botaoB);
  gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
  // Aqui termina o trecho para modo BOOTSEL com botão B

  gpio_init(Botao_A);
  gpio_set_dir(Botao_A, GPIO_IN);
  gpio_pull_up(Botao_A);

  // I2C Initialisation. Using it at 400Khz.
  i2c_init(I2C_PORT, 400 * 1000);

  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
  gpio_pull_up(I2C_SDA);                                        // Pull up the data line
  gpio_pull_up(I2C_SCL);                                        // Pull up the clock line
  ssd1306_t ssd;                                                // Inicializa a estrutura do display
  ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
  ssd1306_config(&ssd);                                         // Configura o display
  ssd1306_send_data(&ssd);                                      // Envia os dados para o display

  // Limpa o display. O display inicia com todos os pixels apagados.
  ssd1306_fill(&ssd, false);
  ssd1306_send_data(&ssd);

  adc_init();
  adc_gpio_init(ADC_PIN); // GPIO 28 como entrada analógica

  float tensao;
  char str_x[5]; // Buffer para armazenar a string
  char str_y[5]; // Buffer para armazenar a string
  char str_comercial[16]; // Buffer para armazenar o valor comercial
  char cor1[10];  // Primeira cor
  char cor2[10];  // Segunda cor 
  char cor3[10];  // Terceira cor

  bool cor = true;
  while (true)
  {
    adc_select_input(2); // Seleciona o ADC para eixo X. O pino 28 como entrada analógica

    float soma = 0.0f;
    for (int i = 0; i < 500; i++)
    {
      soma += adc_read();
      sleep_ms(1);
    }
    float media = soma / 500.0f;

    // Fórmula simplificada: R_x = R_conhecido * ADC_encontrado /(ADC_RESOLUTION - adc_encontrado)
    R_x = (R_conhecido * media) / (ADC_RESOLUTION - media);
    sprintf(str_x, "%1.0f", media); // Converte o inteiro em string
    sprintf(str_y, "%1.0f", R_x);   // Converte o float em string
    // Encontrar o resistor comercial mais próximo
    float r_comercial = encontrar_resistor_comercial(R_x);
    
    // Verificar se está dentro da faixa de medição
    if (r_comercial < 500 || r_comercial > 100000) {
        sprintf(str_comercial, "-");
        sprintf(str_y, "-");   // Converte o float em string
        strcpy(cor1, "-");
        strcpy(cor2, "-");
        strcpy(cor3, "-");
    } else {
        // Obter as cores do resistor
        obter_cores_resistor(r_comercial, cor1, cor2, cor3);
        
        // Preparar a string do valor comercial com unidade (apenas Ohm ou kOhm)
        if (r_comercial < 1000) {
            sprintf(str_comercial, "%.1fOhm", r_comercial);
        } else {
            sprintf(str_comercial, "%.1fkOhm", r_comercial / 1000.0f);
        }
    }

    

    // Atualiza o conteúdo do display com animações
    ssd1306_fill(&ssd, !cor);                          // Limpa o display
    ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);      // Desenha um retângulo
    ssd1306_line(&ssd, 3, 37, 123, 37, cor);           // Desenha uma linha
    ssd1306_draw_string(&ssd, cor1, 8, 6);             // Primeira cor
    ssd1306_draw_string(&ssd, cor2, 8, 16);            // Segunda cor
    ssd1306_draw_string(&ssd, cor3, 8, 28);            // Terceira cor
    ssd1306_draw_string(&ssd, "OHM", 13, 41);          // Desenha uma string
    ssd1306_draw_string(&ssd, "Resisten.", 50, 41);    // Desenha uma string
    ssd1306_line(&ssd, 49, 37, 49, 60, cor);           // Desenha uma linha vertical
    ssd1306_draw_string(&ssd, str_y, 5, 52);           // Desenha uma string
    ssd1306_draw_string(&ssd, str_comercial, 55, 52);  // Desenha uma string
    ssd1306_send_data(&ssd);                           // Atualiza o display
    sleep_ms(700);
  }
}