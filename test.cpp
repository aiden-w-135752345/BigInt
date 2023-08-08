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
    BigInt i=BigInt(1)<<64;
    printf("\n");
    printBig(i);
}