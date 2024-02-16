#include <cstdint>
#include <string>
#include <limits>
#include <memory>
class BigInt;
BigInt operator""_BigInt(const char*);
class BigInt{
    typedef uintmax_t umax;
    typedef std::make_signed<size_t>::type signed_size_t;
    static std::shared_ptr<umax> alloc(size_t len){return{new umax[len],std::default_delete<umax[]>()};}
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
        if(!value){return -1;}
        size_t c=0;
        value&=-value;
        for(size_t i=bits/2;i;i>>=1){if(value&~((~(umax)0)/((((umax)1)<<i)+1))){c+=i;}}
        return c;
    }
    static size_t clz(umax value){
        size_t r=bits;
        for(size_t s=bits/2;s;s>>=1){if(value&((~(umax)0)<<(s-1))){value>>=s;r-=s;}}
        return r;
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
    BigInt(std::shared_ptr<umax>&&data,size_t length):dat(data),len(length){}
public:
    BigInt(intmax_t value=0):dat(alloc(1)),len(1){dat.get()[0]=value;}
    short sign()const{return signBit(dat.get()[len-1])?-1:(len>1||dat.get()[0])?1:0;}
    size_t popcnt(){
        size_t total=0;
        for(size_t i=0;i<len;i++){total+=popcnt(dat.get()[i]);}
        if(signBit(dat.get()[len-1])){return len*bits-total;}
        return total;
    }
    size_t ctz(){
        size_t i=0;
        while(i<len&&!dat.get()[i]){i++;}
        return i<len?i*bits+ctz(dat.get()[i]):-1;
    }
    size_t log2(){
        return len*bits-clz(dat.get()[len-1])-1;
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
        if(len<=word_shift){return BigInt(-(signed_size_t)signBit(dat.get()[len-1]));}
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
    friend BigInt operator/(BigInt a,const BigInt&b){divmod(a,b);return a;}
    friend BigInt operator%(BigInt a,const BigInt&b){return divmod(a,b);}
    static BigInt divmod(BigInt&a,const BigInt&b){
        if(b.sign()<0){BigInt r=divmod(a,-b);a.negate();return r;}
        if(a.sign()<0){
            a.negate();
            BigInt r=divmod(a,b);
            if(r){++a;r=b-r;}
            a.negate();
            return r;
        }
        BigInt r=0;
        for(BigInt bit=1_BigInt<<(a.len*bits);bit;bit>>=1){
            r<<=1;
            if(a&bit){r|=1_BigInt;};
            if(r>=b){r-=b;a|=bit;}else{a&=~bit;}
        }
        return r;
    }
    static BigInt pow(BigInt base,BigInt exponent){
        BigInt ret=1;
        while(exponent){
            if(exponent&1_BigInt){ret*=base;}
            exponent>>=1;base*=base;
        }
        return ret;
    }
    const umax*getData(){return dat.get();}
    template<class T>
    static BigInt parse(const T*str,size_t len){
        BigInt num=0;
        if(str[0]=='0'){
            if(len==1){return num;}
            if(str[1]=='x'||str[1]=='X'){
                for(size_t i=2;i<len;i++){
                    num<<=4;
                    if('0'<=str[i]&&str[i]<='9'){
                        num|=BigInt(str[i]-'0');
                    }else if('a'<=str[i]&&str[i]<='f'){
                        num|=BigInt(10+str[i]-'a');
                    }else if('A'<=str[i]&&str[i]<='F'){
                        num|=BigInt(10+str[i]-'A');
                    }else{throw std::invalid_argument("Bad hexadecimal BigInt");}
                }
                return num;
            }else if(str[1]=='b'||str[1]=='B'){
                for(size_t i=2;i<len;i++){
                    num<<=1;
                    if(str[i]=='1'){num|=BigInt(1);}else if(str[i]!='0'){throw std::invalid_argument("Bad binary BigInt");}
                }
                return num;
            }else if(str[1]=='o'||str[1]=='O'){
                for(size_t i=2;i<len;i++){
                    num<<=3;
                    if('0'<=str[i]&&str[i]<'8'){num|=BigInt(str[i]-'0');}else{throw std::invalid_argument("Bad octal BigInt");}
                }
                return num;
            }
        }
        for(size_t i=0;i<len;i++){
            num*=BigInt(10);
            if('0'<=str[i]&&str[i]<='9'){num+=BigInt(str[i]-'0');}else{throw std::invalid_argument("Bad decimal BigInt");}
        }
        return num;
    }
    std::string toString(short radix)const{
        BigInt num=*this,bigRadix=BigInt(radix);
        std::string str;
        if(sign()<0){num.negate();}
        do{str.push_back("0123456789abcdefghijklmnopqrstuvwxyz"[divmod(num,bigRadix).dat.get()[0]]);}while(num);
        if(sign()<0){str.push_back('-');}
        for(size_t i=0,len=str.size();2*i<len;i++){char tmp=str[i];str[i]=str[len-i];str[len-i]=tmp;}
        return str;
    }
};
#include <cstring>
inline BigInt operator""_BigInt(const char*str){return BigInt::parse(str,std::strlen(str));}