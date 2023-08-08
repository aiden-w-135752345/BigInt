#include "BigInt.hpp"
#include <cstdio>
void printBig(BigInt big){
    BigInt mask=1;
    while(big>mask){mask<<=1;}
    while(mask){
        printf("%d",(bool)(big&mask));
        mask>>=1;
    }
    printf("\n");
}
int main(){
    BigInt product=1;
    for(int i=0;i<100;i++){
        product*=BigInt(i+1);
        printBig(product);
    }
    for(int i=0;i<3;i++){
        product/=BigInt(i+1);
        printBig(product);
    }
}