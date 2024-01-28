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
    BigInt p=1;
    for(int i=1;i<100;i++){p*=BigInt(i);}
    for(int i=1;i<100;i++){p/=BigInt(i);}
    printBig(p);
}