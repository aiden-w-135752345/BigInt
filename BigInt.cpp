#include <cstring>
#include <cstdint>
#include <limits>
#include <cmath>
#include <utility>
#include "BigInt.hpp"
void BigInt::bit_and(const umax*adat,size_t alen,const umax*bdat,size_t blen,umax*out){
    size_t minlen=alen>blen?blen:alen,i=0;
    for(;i<minlen;i++){out[i]=adat[i]&bdat[i];}
    if(alen<blen){
        if(signBit(adat[alen-1])){for(;i<blen;i++){out[i]=bdat[i];}}
        else{for(;i<blen;i++){out[i]=0;}}
    }else if(blen<alen){
        if(signBit(bdat[blen-1])){for(;i<alen;i++){out[i]=adat[i];}}
        else{for(;i<alen;i++){out[i]=0;}}
    }
}
void BigInt::bit_or(const umax*adat,size_t alen,const umax*bdat,size_t blen,umax*out){
    size_t minlen=alen>blen?blen:alen,i=0;
    for(;i<minlen;i++){out[i]=adat[i]|bdat[i];}
    if(alen<blen){
        if(signBit(adat[alen-1])){for(;i<blen;i++){out[i]=(umax)-1;}}
        else{for(;i<blen;i++){out[i]=bdat[i];}}
    }else if(blen<alen){
        if(signBit(bdat[blen-1])){for(;i<alen;i++){out[i]=(umax)-1;}}
        else{for(;i<alen;i++){out[i]=adat[i];}}
    }
}
void BigInt::bit_xor(const umax*adat,size_t alen,const umax*bdat,size_t blen,umax*out){
    size_t minlen=alen>blen?blen:alen,i=0;
    for(;i<minlen;i++){out[i]=adat[i]^bdat[i];}
    if(alen<blen){
        if(signBit(adat[alen-1])){for(;i<blen;i++){out[i]=~bdat[i];}}
        else{for(;i<blen;i++){out[i]=bdat[i];}}
    }else if(blen<alen){
        if(signBit(bdat[blen-1])){for(;i<alen;i++){out[i]=~adat[i];}}
        else{for(;i<alen;i++){out[i]=adat[i];}}
    }
}

bool BigInt::inc_dec(bool carry,bool borrow,const umax*in,umax*out,size_t len){
    size_t i=0;
    if(carry&&!borrow){
        for(;i<len&&in[i]==(umax)-1;){out[i++]=0;}
        if(i<len){out[i]=in[i]+1;i++;}else{return false;}
    }
    if(borrow&&!carry){
        for(;i<len&&in[i]==0;){out[i++]=(umax)-1;}
        if(i<len){out[i]=in[i]-1;i++;}else{return true;}
    }
    for(;i<len;i++){out[i]=in[i];}
    return signBit(in[len-1]);
}
BigInt::umax BigInt::add(const umax*adat,size_t alen,const umax*bdat,size_t blen,umax*out){
    bool asign=signBit(adat[alen-1]),bsign=signBit(bdat[blen-1]);
    size_t minlen=alen>blen?blen:alen;
    bool carry=false;
    for(size_t i=0;i<minlen;i++){
        umax dig=adat[i]+bdat[i]+carry;
        carry=carry?dig<=adat[i]:dig<adat[i];
        out[i]=dig;
    }
    if(alen<blen){
        return -(umax)inc_dec(carry,asign,&bdat[minlen],&out[minlen],blen-minlen);
    }else if(blen<alen){
        return -(umax)inc_dec(carry,bsign,&adat[minlen],&out[minlen],alen-minlen);
    }else{
        return ((umax)carry)-((umax)asign)-((umax)bsign);
    }
}
void BigInt::multiply(const umax*adat,size_t alen,const umax*bdat,size_t blen,umax*out){
    size_t olen=alen+blen;
    bool asign=signBit(adat[alen-1]),bsign=signBit(bdat[blen-1]);
    constexpr static const umax HALF_BITS=bits/2;
    constexpr static const umax LOWER_HALF=((((umax)1)<<HALF_BITS)-1);
    umax carry_high=0,carry_low=0;
    for(size_t i=0;i<olen;i++){
        out[i]=carry_low;carry_low=carry_high;carry_high=0;
        for(size_t j=0;j<=i;j++){
            umax adig=j<alen?adat[j]:-(umax)asign;
            umax bdig=i-j<blen?bdat[i-j]:-(umax)bsign;
            umax a0=adig&LOWER_HALF,a1=adig>>HALF_BITS;
            umax b0=bdig&LOWER_HALF,b1=bdig>>HALF_BITS;
            umax p00=a0*b0,p01=a0*b1,p10=a1*b0,p11=a1*b1;
            
            umax middle=p10+(p00>>HALF_BITS)+(p01&LOWER_HALF);
            umax lo=(middle<<HALF_BITS)|(p00&LOWER_HALF);
            umax hi=(middle>>HALF_BITS)+(p01>>HALF_BITS)+p11;
            
            out[i]+=lo;
            if(out[i]<lo){carry_low++;if(!carry_low){carry_high++;}}
            carry_low+=hi;if(carry_low<hi){carry_high++;}
        }
    }
}
