#include "pico/stdlib.h"// Biblioteca padrão do Raspberry Pi Pico
#include <stdio.h>// Biblioteca para entrada e saída padrão
#include <math.h> // Biblioteca matemática
#include "hardware/pio.h"// Biblioteca para controle da interface PIO
#include "hardware/clocks.h"// Biblioteca para configuração dos clocks
#include "hardware/adc.h"// Biblioteca para controle do ADC
#include "pico/bootrom.h"// Biblioteca para funções do bootrom
#include "pio_matrix.pio.h"// Programa PIO para controle da matriz LED

#define ledV 13  // Definição do pino do LED verde
#define botA 5   // Definição do pino do botão A
#define botB 6   // Definição do pino do botão B

uint16_t contador = 0;  // Contador para rastrear estado do programa
static volatile uint32_t last_time = 0; // Variável para debounce de interrupção

#define NUM_LEDS 25 // Número de LEDs na matriz
#define OUT_PIN 7   // Pino de dados conectado à matriz

// Matrizes para exibição dos números de 0 a 9, representadas como intensidades dos LEDs

double numero0[25] = {0.0, 0.3, 0.3, 0.3, 0.0,
                      0.0, 0.3, 0.0, 0.3, 0.0,
                      0.0, 0.3, 0.0, 0.3, 0.0,
                      0.0, 0.3, 0.0, 0.3, 0.0,
                      0.0, 0.3, 0.3, 0.3, 0.0};

double numero1[25] = {0.0, 0.0, 0.3, 0.0, 0.0,
                      0., 0.0, 0.3, 0.3, 0.0,
                      0.0, 0.0, 0.3, 0.0, 0.0,
                      0.0, 0.0, 0.3, 0.0, 0.0,
                      0.0, 0.3, 0.3, 0.3, 0.0};

double numero2[25] = {0., 0.3, 0.3, 0.3, 0.0,
                      0.0, 0.3, 0.0, 0.0, 0.0,
                      0.0, 0.3, 0.3, 0.3, 0.0,
                      0.0, 0.0, 0.0, 0.3, 0.0,
                      0.0, 0.3, 0.3, 0.3, 0.0};

double numero3[25] = {0.0, 0.3, 0.3, 0.3, 0.0,
                      0.0, 0.3, 0.0, 0., 0.0,
                      0.0, 0.3, 0.3, 0.3, 0.0,
                      0.0, 0.3, 0.0, 0.0, 0.0,
                      0.0, 0.3, 0.3, 0.3, 0.0};

double numero4[25] = {0.0, 0.3, 0.0, 0.3, 0.0,
                      0.0, 0.3, 0.0, 0.3, 0.0,
                      0.0, 0.3, 0.3, 0.3, 0.0,
                      0.0, 0.3, 0.0, 0.0, 0.0,
                      0.0, 0.0, 0.0, 0.3, 0.0};

double numero5[25] = {0.0, 0.3, 0.3, 0.3, 0.0,
                      0.0, 0.0, 0.0, 0.3, 0.0,
                      0.0, 0.3, 0.3, 0.3, 0.0,
                      0.0, 0.3, 0.0, 0.0, 0.0,
                      0.0, 0.3, 0.3, 0.3, 0.0};

double numero6[25] = {0.0, 0.3, 0.3, 0.3, 0.0,
                      0.0, 0.0, 0.0, 0.3, 0.0,
                      0.0, 0.3, 0.3, 0.3, 0.0,
                      0.0, 0.3, 0.0, 0.3, 0.0,
                      0.0, 0.3, 0.3, 0.3, 0.0};

double numero7[25] = {0.3, 0.3, 0.3, 0.0, 0.0,
                      0.0, 0.0, 0.3, 0.0, 0.0,
                      0.0, 0.3, 0.3, 0.3, 0.0,
                      0.0, 0.0, 0.3, 0.0, 0.0,
                      0., 0.0, 0.3, 0.0, 0.0};

double numero8[25] = {0.0, 0.3, 0.3, 0.3, 0.0,
                      0.0, 0.3, 0.0, 0.3, 0.0,
                      0.0, 0.3, 0.3, 0.3, 0.0,
                      0.0, 0.3, 0.0, 0.3, 0.0,
                      0.0, 0.3, 0.3, 0.3, 0.0};

double numero9[25] = {0.0, 0.3, 0.3, 0.3, 0.0,
                      0.0, 0.3, 0.0, 0.3, 0.0,
                      0.0, 0.3, 0.3, 0.3, 0.0,
                      0.0, 0.3, 0.0, 0.0, 0.0,
                      0.0, 0.3, 0.3, 0.3, 0.0};

 
// Função para converter valores RGB para formato da matriz LED 
 uint32_t matrix_rgb(float r, float g, float b)
{
    unsigned char R, G, B;
    R = r * 255;
    G = g * 255;
    B = b * 255;

    return (G << 24) | (R << 16) | (B << 8);// Formato de cor RGB
}


// Função para exibir um padrão na matriz LED
void padrao( double *desenho, uint32_t valor_led, PIO pio, uint sm, double r, double g, double b){

    for( int i =0; i< NUM_LEDS; i++){
        valor_led=matrix_rgb(desenho[24-i], g, b);// Define a cor para cada LED
        pio_sm_put_blocking(pio,sm,valor_led); // Envia os valores para o PIO
    }
}

// Protótipo da função de interrupção do GPIO
void gpio_irq_handler(uint gpio, uint32_t events);



int main(){
    PIO pio = pio0;// Seleciona o bloco PIO 0
    bool ok;
    uint16_t i;
    uint32_t valor_led;
    double r=0,g=0,b=0; // Cores iniciais
    
    stdio_init_all();// Inicializa entrada/saída padrão


    ok = set_sys_clock_hz(128000,false);// Configuração do clock do sistema
    if(ok){
        printf("Configuração de clock ok.");
    }
   

    uint sm = pio_claim_unused_sm(pio,true);// Obtém um estado da máquina PIO
    uint offset = pio_add_program(pio,&pio_matrix_program);// Adiciona programa PIO
    pio_matrix_program_init(pio,sm,offset,OUT_PIN);// Inicializa PIO

    // Configuração do LED verde
    gpio_init(ledV);
    gpio_set_dir(ledV,GPIO_OUT);

    // Configuração dos botões de entrada
    gpio_init(botA);
    gpio_set_dir(botA,GPIO_IN);
    gpio_pull_up(botA);

    gpio_init(botB);
    gpio_set_dir(botB,GPIO_IN);
    gpio_pull_up(botB);

     // Configuração das interrupções dos botões
    gpio_set_irq_enabled_with_callback(botA, GPIO_IRQ_EDGE_FALL,true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(botB, GPIO_IRQ_EDGE_FALL,true, &gpio_irq_handler);
  
   
    while(true){

        gpio_put(ledV, 1); // Liga o LED verde
        sleep_ms(100);
        gpio_put(ledV, 0);// desliga o LED verde
        sleep_ms(100);

        // Exibição do número correspondente ao contador na matriz
        if(contador==0){padrao(numero0,valor_led,pio,sm, r, g, b);
        };
        if(contador==1){
            padrao(numero1,valor_led,pio,sm, r, g, b);
        };
        if(contador==2){
            padrao(numero2,valor_led,pio,sm, r, g, b);
        };
        if(contador==3){
            padrao(numero3,valor_led,pio,sm, r, g, b);
        };
        if(contador==4){
            padrao(numero4,valor_led,pio,sm, r, g, b);
        };
        if(contador==5){
            padrao(numero5,valor_led,pio,sm, r, g, b);
        };
        if(contador==6){
            padrao(numero6,valor_led,pio,sm, r, g, b);
        };
        if(contador==7){
            padrao(numero7,valor_led,pio,sm, r, g, b);
        };
        if(contador==8){
            padrao(numero8,valor_led,pio,sm, r, g, b);
        };
        if(contador==9){
            padrao(numero9,valor_led,pio,sm, r, g, b);
        }
        
    }
    return 0;
}


// Função de interrupção para os botões       
void gpio_irq_handler(uint gpio, uint32_t events)
{
    
    uint32_t current_time = to_us_since_boot(get_absolute_time());// Tempo atual
  
    if (current_time - last_time > 200000) // Debounce de 200ms
    {
        last_time = current_time; 
        if(gpio==5){ // Se for o botão A
        contador++;// Incrementa contador
        printf("A\n");
        }
        if(gpio==6){// Se for o botão B
        contador--;// Decrementa contador
        printf("B\n");                      
    }
}
  

}