#include <cstring>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <new>
#include <cmath>
#include <utility>
#include <memory>
class BigInt{
    typedef uintmax_t umax;
    static auto alloc(size_t len){
        return std::shared_ptr<umax>(new umax[len-1],std::default_delete<umax[]>());
    }
    std::shared_ptr<umax> dat;
    size_t len;
    

    constexpr static auto bits = std::numeric_limits<umax>::digits;
    static umax signBit(umax value){return value>>(bits-1);}
    static size_t popcnt(umax value){
        for(size_t i=1;i<bits;i<<=1){
            umax mask=((umax)-1)/((((umax)1)<<i)+1);
            value=(value&mask)+((value>>i)&mask);
        }
        return value;
    }
    static size_t ctz(umax value){
        size_t c=bits;
        value&=-value;
        if(value){c--;}
        for(size_t i=bits/2;i;i>>=1){
            if(value&(((umax)-1)/((((umax)1)<<i)+1))){c-=i;}
        }
        return 0;
    }
    static size_t log2(umax value){
        size_t r = 0;
        for(size_t s=bits/2;s;s>>=1){if(value&(((((umax)1)<<s)-1)<<s)){value>>=s;r|=s;}}
        return r;// floor(log2(value))
    }
    BigInt&shrink(){
        while(len>1&&!(dat.get()[len-1]+signBit(dat.get()[len-2]))){len--;}
        try{
            auto resized=alloc(len);
            for(umax i=0;i<len;i++){resized.get()[i]=dat.get()[i];}
            dat=std::move(resized);
        }catch(std::bad_alloc){}
        return *this;
    }
    struct Signed{};
    BigInt(size_t value,Signed):dat(alloc(1)),len(1){dat.get()[0]=std::make_signed_t<size_t>(value);}
    BigInt(std::shared_ptr<umax>&&data,size_t length):dat(data),len(length){}
public:
    BigInt(std::make_signed_t<size_t> value=0):dat(alloc(1)),len(1){dat.get()[0]=value;}
    int sign()const{return signBit(dat.get()[len-1])?-1:1;}
    size_t popcnt(){
        size_t total=0;
        for(size_t i=0;i<len;i++){total+=popcnt(dat.get()[i]);}
        if(signBit(dat.get()[len-1])){return len*bits-total;}
        return total;
    }
    size_t ctz(){
        size_t i=0;
        while(dat.get()[i]){i++;}
        return i*bits+ctz(dat.get()[i]);
    }
    explicit operator bool() const{return sign();};
    friend bool operator< (const BigInt&a,const BigInt&b){return (a-b).sign()<0;}
    friend bool operator<=(const BigInt&a,const BigInt&b){return (a-b).sign()<=0;}
    friend bool operator==(const BigInt&a,const BigInt&b){return (a-b).sign()==0;}
    friend bool operator>=(const BigInt&a,const BigInt&b){return (a-b).sign()>=0;}
    friend bool operator> (const BigInt&a,const BigInt&b){return (a-b).sign()>0;}
    friend bool operator!=(const BigInt&a,const BigInt&b){return (a-b).sign()!=0;}
    
    BigInt operator-()const{return (~*this).inc_dec(true,false);}
    friend BigInt operator-(const BigInt&a,const BigInt&b){return a+-b;}
    friend BigInt operator/(const BigInt&a,const BigInt&b){BigInt q=a,r;divmod(q,b,r);return q;}
    friend BigInt operator%(const BigInt&a,const BigInt&b){BigInt q=a,r;divmod(q,b,r);return r;}
    
    BigInt&invert(){return*this=~*this;}
    BigInt&operator++(){return *this=inc_dec(true,false);}
    BigInt operator++(int){BigInt old=*this;operator++();return old;}
    BigInt&operator--(){return *this=inc_dec(false,true);}
    BigInt operator--(int){BigInt old=*this;operator--();return old;}
    BigInt&negate(){return ++invert();}
    
    BigInt&operator>>=(size_t shift){return *this=(*this)>>shift;}
    BigInt&operator<<=(size_t shift){return *this=(*this)<<shift;}
    BigInt&operator&=(const BigInt&that){return *this=(*this)&that;}
    BigInt&operator|=(const BigInt&that){return *this=(*this)|that;}
    BigInt&operator^=(const BigInt&that){return *this=(*this)^that;}
    BigInt&operator+=(const BigInt&that){return *this=(*this)+that;}
    BigInt&operator-=(const BigInt&that){return *this=(*this)-that;}
    BigInt&operator*=(const BigInt&that){return *this=(*this)*that;}
    BigInt&operator/=(const BigInt&that){return *this=(*this)/that;}
    BigInt&operator%=(const BigInt&that){return *this=(*this)%that;}
    
    BigInt operator>>(size_t shift)const{
        if(!shift){return *this;}
        size_t bit_shift=shift%bits,word_shift=shift/bits;
        if(len<=word_shift){return BigInt(-signBit(dat.get()[len-1]),Signed{});}
        if(!bit_shift){
            auto out=alloc(len-word_shift);
            for(size_t i=word_shift;i<len;i++){out.get()[i-word_shift]=dat.get()[i];}
            return BigInt(std::move(out),len-word_shift);
        }else{
            umax ext=(dat.get()[len-1]>>bit_shift)|((-signBit(dat.get()[len-1]))<<(bits-bit_shift));
            bool can_shrink=-signBit(dat.get()[len-1]<<(bits-bit_shift))==ext;
            auto out=alloc(len-word_shift-can_shrink);
            for(size_t i=word_shift;i<len-1;i++){
                out.get()[i-word_shift]=(dat.get()[i]>>bit_shift)|(dat.get()[i+1]<<(bits-bit_shift));
            }
            if(!can_shrink){out.get()[len-1-word_shift]=ext;}
            return BigInt(std::move(out),len-word_shift-can_shrink);
        }
    }
    BigInt operator<<(size_t shift)const{
        if(!shift){return *this;}
        size_t bit_shift=shift%bits,word_shift=shift/bits;
        if(!bit_shift){
            auto out=alloc(len+word_shift);
            for(size_t i=0;i<word_shift;i++){out.get()[i]=0;}
            for(size_t i=0;i<len;i++){out.get()[i+word_shift]=dat.get()[i];}
            return BigInt(std::move(out),len+word_shift);
        }
        umax ext=((-signBit(dat.get()[len-1]))<<bit_shift)|(dat.get()[len-1]>>(bits-bit_shift));
        bool needs_bigger=-signBit(dat.get()[len-1]<<bit_shift)!=ext;
        auto out=alloc(len+needs_bigger+word_shift);
        for(size_t i=0;i<word_shift;i++){out.get()[i]=0;}
        out.get()[word_shift]=dat.get()[0]<<bit_shift;
        for(size_t i=1;i<len;i++){out.get()[i+word_shift]=(dat.get()[i]<<bit_shift)|(dat.get()[i-1]>>(bits-bit_shift));}
        if(needs_bigger){out.get()[len+word_shift]=ext;}
        return BigInt(std::move(out),len+needs_bigger+word_shift);
    }
private:
    static void bit_and(const umax*adat,size_t alen,const umax*bdat,size_t blen,umax*out);
public:
    friend BigInt operator&(const BigInt&a,const BigInt&b){
        size_t olen=a.len>b.len?a.len:b.len;
        auto out=alloc(olen);
        bit_and(a.dat.get(),a.len,b.dat.get(),b.len,out.get());
        return BigInt(std::move(out),olen).shrink();
    }
private:
    static void bit_or(const umax*adat,size_t alen,const umax*bdat,size_t blen,umax*out);
public:
    friend BigInt operator|(const BigInt&a,const BigInt&b){
        size_t olen=a.len>b.len?a.len:b.len;
        auto out=alloc(olen);
        bit_or(a.dat.get(),a.len,b.dat.get(),b.len,out.get());
        return BigInt(std::move(out),olen).shrink();
    }
private:
    static void bit_xor(const umax*adat,size_t alen,const umax*bdat,size_t blen,umax*out);
public:
    friend BigInt operator^(const BigInt&a,const BigInt&b){
        size_t olen=a.len>b.len?a.len:b.len;
        auto out=alloc(olen);
        bit_xor(a.dat.get(),a.len,b.dat.get(),b.len,out.get());
        return BigInt(std::move(out),olen).shrink();
    }
    BigInt operator~()const{
        auto out=alloc(len);
        for(size_t i=0;i<len;i++){out.get()[i]=~dat.get()[i];}
        return BigInt(std::move(out),len);
    }
private:
    static bool inc_dec(bool carry,bool borrow,const umax*in,umax*out,size_t len);
public:
    BigInt inc_dec(bool carry,bool borrow)const{
        if(carry==borrow){return *this;}
        auto out=alloc(len+1);
        out.get()[len]=-(umax)inc_dec(carry,borrow,dat.get(),out.get(),len);
        return BigInt(std::move(out),len+1).shrink();
    }
private:
    static umax add(const umax*adat,size_t alen,const umax*bdat,size_t blen,umax*out);
public:
    friend BigInt operator+(const BigInt&a,const BigInt&b){
        size_t olen=(a.len>b.len?a.len:b.len)+1;
        auto out=alloc(olen);
        out.get()[olen-1]=add(a.dat.get(),a.len,b.dat.get(),b.len,out.get());
        return BigInt(std::move(out),olen).shrink();
    }
private:
    static void multiply(const umax*adat,size_t alen,const umax*bdat,size_t blen,umax*out);
public:
    friend BigInt operator*(const BigInt&a,const BigInt&b){
        size_t olen=a.len+b.len;
        auto out=alloc(olen);
        multiply(a.dat.get(),a.len,b.dat.get(),b.len,out.get());
        return BigInt(std::move(out),olen).shrink();
    }
    friend BigInt operator""_BigInt(const char*raw){
        BigInt cooked=0;
        if(raw[0]=='0'){
            if(raw[1]=='x'||raw[1]=='X'){
                for(int i=2;raw[i];i++){
                    cooked<<=4;
                    cooked|=BigInt(raw[i]-
                                   ('a'<=raw[i]&&raw[i]<='f')?'a':
                                   ('A'<=raw[i]&&raw[i]<='F')?'A':'0');
                }
            }else if(raw[1]=='b'||raw[1]=='B'){
                for(int i=2;raw[i];i++){
                    cooked<<=1;
                    cooked|=BigInt(raw[i]-'0');
                }
            }else{
                for(int i=1;raw[i];i++){
                    cooked<<=3;
                    cooked|=BigInt(raw[i]-'0');
                }
            }
        }else{
            for(int i=0;raw[i];i++){
                cooked*=BigInt(10);
                cooked+=BigInt(raw[i]-'0');
            }
        }
        return cooked;
    }
    static void divmod(BigInt&a,const BigInt&b,BigInt&r){
        if(b<0){divmod(a,-b,r);a.negate();return;}
        if(a<0){
            a.negate();
            divmod(a,b,r);
            if(r.sign()){++a;r=b-r;}a.negate();
            return;
        }
        r=0;
        for(BigInt bit=BigInt(1)<<(a.len*bits);bit;bit>>=1){
            r<<=1;
            if(a&bit){r|=BigInt(1);};
            if(r>=b){r-=b;a|=bit;}else{a&=~bit;}
        }
    }
    //static BigInt pow(const BigInt&base,const BigInt&exp){}
};