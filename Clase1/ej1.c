#include "stdio.h"
#include "stdint.h"
#define LEDS 8
#ifdef UNION
union{
    struct{
        unsigned char led1 : 1;
        unsigned char led2 : 1;
        unsigned char led3 : 1;
        unsigned char led4 : 1;
        unsigned char led5 : 1;
        unsigned char led6 : 1;
        unsigned char led7 : 1;
        unsigned char led8 : 1;
    }u;
    unsigned char status;
}Leds;

#else
uint8_t leds;
#endif
void encender(){
    printf("Encendiendo todos los leds\n");
    #ifdef UNION
    #else
    for(int i = 0; i < 8; i++){
        if(leds & (1 << i)){
            printf("El led %d ya estaba encendido\n",i + 1);
        }else{
            leds |= (1 << i);
            printf("Encendiendo led %d\n",i + 1);
        }
    }
    #endif
}
void apagar(){
    printf("Apagando todos los leds\n");
    for(int i = 7; i >= 0; i--){
        if((leds & (1 << i)) == 0){
            printf("El led %d ya estaba apagado\n",i + 1);
        }else{
            printf("Apagando led %d\n",i + 1);
            uint8_t temp = leds;
            leds ^= (1 << i);
        }
    }
}

void mode_automatico(){
    encender();
    apagar();
}
void modificar(uint8_t led){
    uint8_t estado;
    printf("Elige el estado del led: %d, Pulse 1 o 0: \n",led);
    scanf("%hhu",&estado);
    if(estado > 1){
        printf("Ese estado es imposible o se enciende o se apaga, no hay más\n");
        return;
    }
    uint8_t mask = 1 << (led - 1);
    leds &= ~mask; // limpiar bit
    leds |=(estado << (led-1));
}
void togglear(uint8_t led){
    leds ^=(1 << (led - 1));
}
void checkear(uint8_t led){
    if(leds & (1 << (led - 1))) printf("El led %d esta encendido",led);
    else printf("El led %d esta apagado",led);
}
int main(){
    char mode;
    do{
        printf("Para automatico pulsa A, para manual pulsa M:\n");
        scanf(" %c",&mode);
    }while(mode != 'A' && mode != 'M');

    if(mode == 'A'){
        mode_automatico();
    }else if(mode == 'M'){
        do{
            printf("\n\nElige entre las siguientes acciones\n 1.Modificar el estado de un led, pulse M\n 2.Togglear un led, pulse T\n 3.Checkear el estado de un Led,pulse C\n 4.Pasar al modo automatico, pulse A\n");
            scanf(" %c",&mode);
            switch(mode){
                uint8_t led;
                case 'M':
                    
                    printf("Elige del 1 al 8 el led a modificar: ");
                    scanf("%hhu",&led);
                    if(led > 8){
                        printf("Solo hay 8 leds que estas intentando hacer");
                    }else{
                        modificar(led);
                    }
                    
                    break;
                case 'T':
                    printf("Elige del 1 al 8 el led a togglear: ");
                    scanf("%hhu",&led);
                     if(led > 8){
                        printf("Solo hay 8 leds que estas intentando hacer");
                    }else{
                        togglear(led);
                    }
                    
                    break;
                case 'C':
                    printf("Elige del 1 al 8 el led que quieres checkear el estado: ");
                    scanf("%hhu",&led);
                     if(led > 8){
                        printf("Solo hay 8 leds que estas intentando hacer");
                    }else{
                        checkear(led);
                    }
                    
                    break;
                case 'A':
                    mode_automatico();
                    break;
            }
        }while(mode != 'A');
        
    }
    return 0;
}
