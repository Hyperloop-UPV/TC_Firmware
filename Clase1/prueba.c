#include <stdio.h>
#include "stdint.h"
#include "stdbool.h"
#include "string.h"

#define MAX_VAL 1000
int maximo_comun_divisor(int a, int b){
    if(b == 0) return a;
    return maximo_comun_divisor(b,a%b);
}
int suma(int *a, int *b){
    return *a + *b;
}
int divisor = 4;
int main(){
    int numero_elegido = 16;
    int resultado = suma(&numero_elegido,&divisor);
    printf("resultado : %d",resultado);
    // printf("Pon el numero que quieras comprobar el maximo comun divisor con el %d:",divisor);
    // scanf("%d",&numero_elegido);
    if(MAX_VAL < numero_elegido){
        printf("El numero es mayor de lo esperado");
    }else{
        int resultado = maximo_comun_divisor(divisor,numero_elegido);
        printf("El resultado es: %d\n",resultado);
    }
    return 0;
}

