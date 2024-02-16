#include "include/BigInt.hpp"
#include <iostream>
int main(){
    BigInt p=1;
    for(int i=1;i<100;i++){p*=BigInt(i);}
    std::cout<<p.toString()<<'\n';
    for(int i=1;i<100;i++){p/=BigInt(i);}
    std::cout<<p.toString()<<'\n';
}