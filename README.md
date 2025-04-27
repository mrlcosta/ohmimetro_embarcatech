# Ohmímetro Digital com Raspberry Pi Pico

## Descrição do Projeto

Este projeto implementa um ohmímetro digital utilizando o microcontrolador RP2040 (Raspberry Pi Pico). O dispositivo mede a resistência de componentes usando um divisor de tensão, identifica o resistor comercial mais próximo da série E24 e exibe o código de cores correspondente em um display OLED.

## Funcionalidades

- Medição de resistências com precisão razoável
- Identificação do valor comercial mais próximo da série E24
- Exibição do código de cores do resistor (3 faixas)
- Interface visual através de display OLED SSD1306

## Componentes Necessários

- Raspberry Pi Pico (RP2040)
- Display OLED SSD1306 (I2C)
- Resistor de 10k ohm (referência)
- Resistor a ser medido
- Jumpers para conexão com o resistor a ser medido

## Pinagem

- **GPIO 28**: Entrada analógica para o divisor de tensão (ADC)
- **GPIO 14**: SDA para display OLED
- **GPIO 15**: SCL para display OLED

## Princípio de Funcionamento

O sistema utiliza o princípio do divisor de tensão:

1. Um resistor conhecido (10k ohm) é conectado em série com o resistor desconhecido
2. A tensão no ponto médio é medida pelo ADC do RP2040
3. Com base na leitura ADC, a resistência é calculada pela fórmula:

R_x = R_conhecido * ADC_leitura / (ADC_RESOLUTION - ADC_leitura)

4. O valor comercial mais próximo na série E24 é identificado
5. As cores correspondentes ao resistor são determinadas e exibidas

## Código de Cores

O sistema exibe as três faixas de cores para o resistor:
- **Primeira faixa**: Primeiro dígito do valor
- **Segunda faixa**: Segundo dígito do valor
- **Terceira faixa**: Multiplicador (número de zeros)


## Série E24

A série E24 inclui os seguintes valores por década:
1.0, 1.1, 1.2, 1.3, 1.5, 1.6, 1.8, 2.0, 2.2, 2.4, 2.7, 3.0,
3.3, 3.6, 3.9, 4.3, 4.7, 5.1, 5.6, 6.2, 6.8, 7.5, 8.2, 9.1

Multiplicados por potências de 10: (1Ω, 10Ω, 100Ω, 1kΩ, etc.)

## Como Usar

1. Conecte o Raspberry Pi Pico ao computador
2. Carregue o código no microcontrolador
3. Monte o circuito do divisor de tensão com o resistor de referência
4. Conecte o resistor desconhecido ao circuito
5. Observe no display:
- As três cores do resistor (nas três primeiras linhas)
- O valor medido da resistência
- O valor comercial mais próximo

## Limitações

- A precisão das medições depende da qualidade do ADC
- Resistores com valores muito altos ou muito baixos podem apresentar erros maiores
- O sistema é projetado para resistores de 3 faixas de cores

## Desenvolvimento

Projeto desenvolvido por Wilton Lacerda Silva e adaptado por Muriel Costa.