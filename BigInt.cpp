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
void BigInt::multiply(size_t&x,size_t&y){
    constexpr static const size_t HALF_BITS=digits<size_t>/2;
    constexpr static const size_t LOWER_HALF=((((size_t)1)<<HALF_BITS)-1);
    const size_t x0=x&LOWER_HALF,x1=x>>HALF_BITS,x2=signBit(x)*LOWER_HALF,x3=x2;
    const size_t y0=y&LOWER_HALF,y1=y>>HALF_BITS,y2=signBit(y)*LOWER_HALF,y3=y2;
    const size_t p00=x0*y0,p01=x0*y1,p02=x0*y2,p03=x0*y3;
    const size_t p10=x1*y0,p11=x1*y1,p12=x1*y2;
    const size_t p20=x2*y0,p21=x2*y1;
    const size_t p30=x3*y0;
    size_t r0=(p00&LOWER_HALF);
    size_t r1=(p00>>HALF_BITS)+(p01&LOWER_HALF)+(p10&LOWER_HALF);
    size_t r2=(p01>>HALF_BITS)+(p10>>HALF_BITS)+((p02+p11+p20)&LOWER_HALF);
    size_t r3=(p02>>HALF_BITS)+(p11>>HALF_BITS)+(p20>>HALF_BITS)+((p03+p12+p21+p30)&LOWER_HALF);
    r1+=r0>>HALF_BITS;r0&=LOWER_HALF;
    r2+=r1>>HALF_BITS;r1&=LOWER_HALF;
    r3+=r2>>HALF_BITS;r2&=LOWER_HALF;r3&=LOWER_HALF;
    x=r0+(r1<<HALF_BITS);
    y=r2+(r3<<HALF_BITS);
}

/*BigInt::umax BigInt::addScaled(const umax*idat,size_t ilen,umax scale,umax*odat,size_t olen){
    constexpr static const umax HALF_BITS=digits<umax>/2;
    constexpr static const umax LOWER_HALF=((((umax)1)<<HALF_BITS)-1);
    const umax s0=scale&LOWER_HALF,s1=scale>>HALF_BITS;
    size_t minlen=ilen>olen?olen:ilen,i=0;
    umax carry=0;
    for(;i<minlen;i++){
        umax i0=idat[i]&LOWER_HALF,i1=idat[i]>>HALF_BITS;
        umax p11=i1*s1,p01=i0*s1,p10=i1*s0,p00=i0*s0;
        umax middle = p10 + (p00 >> HALF_BITS) + (p01&LOWER_HALF);
        umax low=(middle << HALF_BITS) | (p00&LOWER_HALF);
        umax high=p11 + (middle >> HALF_BITS) + (p01 >> HALF_BITS);
        umax odig=low+carry;
        odat[i]+=odig;
        carry=(odig<carry) + (odat[i]<odig) + high;
    }
    {
        umax i0=idat[i]&LOWER_HALF,i1=idat[i]>>HALF_BITS;
        umax p11=i1*s1,p01=i0*s1,p10=i1*s0,p00=i0*s0;
        umax middle = p10 + (p00 >> HALF_BITS) + (p01&LOWER_HALF);
        umax low=(middle << HALF_BITS) | (p00&LOWER_HALF);
        umax high=p11 + (middle >> HALF_BITS) + (p01 >> HALF_BITS);
    }
    for(;i<olen;i++){
        odat[i]+=carry;
        
    }
    
}*/
/*BigInt::umax BigInt::scale(const umax*in,size_t len,umax scale,umax*out){
    bool isign=signBit(in[len-1]),ssign=signBit(scale);
    constexpr static const umax HALF_BITS=digits<umax>/2;
    constexpr static const umax LOWER_HALF=((((umax)1)<<HALF_BITS)-1);
    umax carry_low=0,carry_high=0;
    umax total_low=0,total_high=0;
    if(ssign){
        for(size_t i=0;i<len;i++){
            out[i]=carry_low;carry_low=carry_high;carry_high=0;
            umax lo=-total_low;
            umax mid=total_low-total_high-(total_low?1:0);
            umax hi=total_high-(total_low?(l<=h):(l<h));
            out[i]+=lo;
            bool c=out[i]<lo;
            carry_low+=mid+c;
            carry_high+=hi+(c?carry_low<=mid:carry_low<mid);
            umax idig=in[i];
            total_low+=idig;
            if(total_low<idig){total_high++;}
        }
        total_low-=carry_low;
    }else{for(size_t i=0;i<=len;i++){out[i]=0;}}
    
    umax s0=scale&LOWER_HALF,s1=scale>>HALF_BITS;
    umax carry_high=0,carry_low=0;
    for(size_t i=0;i<len;i++){
        out[i]+=carry_low;carry_low=carry_high+(out[i]<carry_low);carry_high=0;
        
        umax i0=in[i]&LOWER_HALF,i1=in[i]>>HALF_BITS;
        umax p00=i0*s0,p01=i0*s1,p10=i1*s0,p11=i1*s1;
        umax middle=p10+(p00>>HALF_BITS)+(p01&LOWER_HALF);
        umax lo=(middle<<HALF_BITS)|(p00&LOWER_HALF);
        umax hi=(middle>>HALF_BITS)+(p01>>HALF_BITS)+p11;
        out[i]+=lo;
        if(out[i]<lo){carry_low++;if(!carry_low){carry_high++;}}
        carry_low+=hi;if(carry_low<hi){carry_high++;}
    }
    return carry_low-isign*scale-total_low;
}*/
void BigInt::multiply(const umax*adat,size_t alen,const umax*bdat,size_t blen,umax*out){
    size_t olen=alen+blen;
    bool asign=signBit(adat[alen-1]),bsign=signBit(bdat[blen-1]);
    constexpr static const umax HALF_BITS=digits<umax>/2;
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
