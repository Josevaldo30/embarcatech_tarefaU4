#include "pico/stdlib.h"
#include <stdio.h>
#include <math.h>
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "pico/bootrom.h"
#include "pio_matrix.pio.h"

#define ledV 13
#define botA 5
#define botB 6

uint16_t contador = 0;
static volatile uint32_t last_time = 0;

#define NUM_LEDS 25 // Número de LEDs na matriz
#define OUT_PIN 7   // Pino de dados conectado à matriz

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

uint32_t matrix_rgb(float r, float g, float b)
{
    unsigned char R, G, B;
    R = r * 255;
    G = g * 255;
    B = b * 255;

    return (G << 24) | (R << 16) | (B << 8);
}

void padrao( double *desenho, uint32_t valor_led, PIO pio, uint sm, double r, double g, double b){

    for( int i =0; i< NUM_LEDS; i++){
        valor_led=matrix_rgb(desenho[24-i], g, b);
        pio_sm_put_blocking(pio,sm,valor_led);
    }
}

void gpio_irq_handler(uint gpio, uint32_t events);



int main(){
    PIO pio = pio0;
    bool ok;
    uint16_t i;
    uint32_t valor_led;
    double r=0,g=0,b=0;
    
    stdio_init_all();

    ok = set_sys_clock_hz(128000,false);
    if(ok){
        printf("Configuração de clock ok.");
    }
   

    uint sm = pio_claim_unused_sm(pio,true);
    uint offset = pio_add_program(pio,&pio_matrix_program);
    pio_matrix_program_init(pio,sm,offset,OUT_PIN);

    gpio_init(ledV);
    gpio_set_dir(ledV,GPIO_OUT);

    gpio_init(botA);
    gpio_set_dir(botA,GPIO_IN);
    gpio_pull_up(botA);

    gpio_init(botB);
    gpio_set_dir(botB,GPIO_IN);
    gpio_pull_up(botB);

    gpio_set_irq_enabled_with_callback(botA, GPIO_IRQ_EDGE_FALL,true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(botB, GPIO_IRQ_EDGE_FALL,true, &gpio_irq_handler);
  
   
    while(true){

        gpio_put(ledV, 1);
        sleep_ms(100);
        gpio_put(ledV, 0);
        sleep_ms(100);


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


       
void gpio_irq_handler(uint gpio, uint32_t events)
{
    
    uint32_t current_time = to_us_since_boot(get_absolute_time());
  
    if (current_time - last_time > 200000) 
    {
        last_time = current_time; 
        if(gpio==5){
        contador++;
        printf("A\n");
        }
        if(gpio==6){
        contador--;
        printf("B\n");                      
    }
}
  

}