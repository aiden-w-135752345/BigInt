#include <cstring>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <new>
#include <cmath>
#include <utility>
class BigInt{
    struct Data{size_t refcount;uintmax_t data[1];};
    Data* dat;
    size_t len;
    typedef signed char schar;
    typedef unsigned char uchar;
    typedef uintmax_t umax;
    static Data*allocData(size_t len){
        Data*dat=(Data*)malloc(sizeof(Data)+(len-1)*sizeof(umax));
        if(!dat){throw std::bad_alloc();}
        dat->refcount=1;
        return dat;
    }
    static Data*allocData(Data*orig,size_t len){
        Data*dat=(Data*)realloc(orig,sizeof(Data)+(len-1)*sizeof(umax));
        if(!dat){throw std::bad_alloc();}
        return dat;
    }
    template<class T>constexpr static auto digits = std::numeric_limits<T>::digits;
/*    template<class T>typename std::make_signed_t<T> to_signed(T value) {
        typedef std::make_signed_t<T> signed_T;
        typedef std::numeric_limits<signed_T> limits;
        return value<=limits::max()?(signed_T)value:signed_T(value-limits::min())+limits::min();
    }*/
    template<class T>static T signBit(T value){return value>>(digits<T>-1);}
    template<class T>static umax signExtend(T value){return value|((-(umax)signBit(value))<<digits<T>);}
    static umax signExtend(umax value){return value;}
    
    template<class T>static void pack(T lo,T hi,umax&low,umax&high){
        if(digits<T>*2 < digits<umax>){
            low=lo|(((umax)hi)<<digits<T>)|(((umax)high)<<(2*digits<T>));
            high=-(umax)signBit(hi);
        }else{
            low=lo|(((umax)hi)<<digits<T>);
            high=(hi>>(digits<umax>-digits<T>))|((-(umax)signBit(hi))<<(2*digits<T>-digits<umax>));
        }
    }
    static void pack(umax lo,umax hi,umax&low,umax&high){low=lo;high=hi;}
    BigInt&shrink(){
        while(len>1&&!(dat->data[len-1]+signBit(dat->data[len-2]))){len--;}
        if(len==1&&dat->data[0]>>(digits<size_t>-1)==(-signBit(dat->data[0]))>>(digits<size_t>-1)){
            len=(size_t)(dat->data[0]);free(dat);dat=nullptr;
        }else{
            Data*resized=(Data*)realloc(dat,sizeof(Data)+(len-1)*sizeof(umax));
            if(resized){dat=resized;}
        }
        return *this;
    }
    struct Signed{};
    BigInt(size_t val,Signed){dat=nullptr;len=val;}
    BigInt(Data*data,size_t length):dat(data),len(length){}
    BigInt(umax lo,umax hi){
        if(-signBit(lo)!=hi){
            dat=allocData(2);
            dat->data[0]=lo;
            dat->data[1]=hi;
            len=2;
        }else if(lo>>(digits<size_t>-1)!=(-signBit(lo))>>(digits<size_t>-1)){
            dat=allocData(1);
            dat->data[0]=lo;
            len=1;
        }else{dat=nullptr;len=lo;}
    }
public:
    BigInt():dat(nullptr),len(0){}
    BigInt(std::make_signed_t<size_t> val){dat=nullptr;len=val;}
    BigInt(const BigInt&that):dat(that.dat),len(that.len){if(dat){dat->refcount++;}}
    BigInt(BigInt&&that):BigInt(){swap(*this,that);}
    BigInt&operator=(BigInt that){swap(*this,that);return *this;}
    ~BigInt(){if(dat){if(0==--(dat->refcount))free(dat);}}
    friend void swap(BigInt&a,BigInt&b){using std::swap;swap(a.dat,b.dat);swap(a.len,b.len);}
    int sign()const{
        if(!dat){
            if(signBit(len)){return -1;}
            return len?1:0;
        }
        if(signBit(dat->data[len-1])){return -1;}
        for(size_t i=0;i<len;i++){if(dat->data[i]){return 1;}}
        return 0;
    }
    explicit operator bool() const{return sign();};
    friend bool operator< (const BigInt&a,const BigInt&b){return (a-b).sign()<0;}
    friend bool operator<=(const BigInt&a,const BigInt&b){return (a-b).sign()<=0;}
    friend bool operator==(const BigInt&a,const BigInt&b){return (a-b).sign()==0;}
    friend bool operator>=(const BigInt&a,const BigInt&b){return (a-b).sign()>=0;}
    friend bool operator> (const BigInt&a,const BigInt&b){return (a-b).sign()>0;}
    friend bool operator!=(const BigInt&a,const BigInt&b){return (a-b).sign()!=0;}
    BigInt operator>>(size_t shift)const{
        if(!shift){return (*this);}
        if(!dat){
            if(shift<digits<size_t>){return BigInt((len>>shift)|((-signBit(len))<<(digits<size_t>-shift)));}
            return BigInt(0);
        }
        size_t bit_shift=shift%digits<umax>;
        size_t word_shift=shift/digits<umax>;
        if(!bit_shift){
            Data*out=allocData(len-word_shift);
            for(size_t i=word_shift;i<len;i++){out->data[i-word_shift]=dat->data[i];}
            return BigInt(out,len-word_shift);
        }
        Data*out=allocData(len-word_shift);
        for(size_t i=word_shift;i<len-1;i++){
            out->data[i-word_shift]=(dat->data[i]>>bit_shift)|(dat->data[i+1]<<(digits<umax>-bit_shift));
        }
        out->data[len-1-word_shift]=(dat->data[len-1]>>bit_shift)|((-signBit(dat->data[len-1]))<<(digits<umax>-bit_shift));
        return BigInt(out,len-word_shift).shrink();
    }
    BigInt&operator>>=(size_t shift){(*this)=(*this)>>shift;return *this;}
    BigInt&operator<<=(size_t shift){(*this)=(*this)<<shift;return *this;}
    BigInt operator<<(size_t shift)const{
        if(!shift){return (*this);}
        if(!dat){
            size_t bit_shift=shift%digits<size_t>;
            if(shift<digits<size_t>&&(-signBit(len))>>(digits<size_t>-bit_shift-1)==len>>(digits<size_t>-bit_shift-1)){
                return BigInt(len<<bit_shift);
            }
            bit_shift=shift%digits<umax>;
            size_t word_shift=shift/digits<umax>;
            if(!bit_shift){
                Data*out=allocData(1+word_shift);
                for(size_t i=0;i<word_shift;i++){out->data[i]=0;}
                out->data[word_shift]=signExtend(len);
                return BigInt(out,1+word_shift);
            }
            umax lo=signExtend(len)<<bit_shift;
            umax hi=((-(umax)signBit(len))<<bit_shift)|(len>>(digits<umax>-bit_shift));
            bool needs_bigger=-signBit(lo)!=hi;
            Data*out=allocData(1+needs_bigger+word_shift);
            for(size_t i=0;i<word_shift;i++){out->data[i]=0;}
            out->data[word_shift]=lo;
            if(needs_bigger){out->data[1+word_shift]=hi;}
            return BigInt(out,1+needs_bigger+word_shift);
        }
        size_t bit_shift=shift%digits<umax>;
        size_t word_shift=shift/digits<umax>;
        if(!bit_shift){
            Data*out=allocData(len+word_shift);
            for(size_t i=0;i<word_shift;i++){out->data[i]=0;}
            for(size_t i=0;i<len;i++){out->data[i+word_shift]=dat->data[i];}
            return BigInt(out,len+word_shift);
        }
        umax ext=((-signBit(dat->data[len-1]))<<bit_shift)|(dat->data[len-1]>>(digits<umax>-bit_shift));
        bool needs_bigger=-signBit(dat->data[len-1]<<bit_shift)!=ext;
        Data*out=allocData(len+needs_bigger+word_shift);
        for(size_t i=0;i<word_shift;i++){out->data[i]=0;}
        out->data[word_shift]=dat->data[0]<<bit_shift;
        for(size_t i=1;i<len;i++){out->data[i+word_shift]=(dat->data[i]<<bit_shift)|(dat->data[i-1]>>(digits<umax>-bit_shift));}
        if(needs_bigger){out->data[len+word_shift]=ext;}
        return BigInt(out,len+needs_bigger+word_shift);
    }
private:
    static void bit_and(const umax*adat,size_t alen,const umax*bdat,size_t blen,umax*out);
public:
    friend BigInt operator&(const BigInt&a,const BigInt&b){
        if(!(a.dat||b.dat)){return BigInt(a.len&b.len,Signed{});}
        umax aext=signExtend(a.len),bext=signExtend(b.len);
        size_t alen=a.dat?a.len:1,blen=b.dat?b.len:1;
        umax*adat=a.dat?a.dat->data:&aext;
        umax*bdat=b.dat?b.dat->data:&bext;
        
        size_t olen=alen>blen?alen:blen;
        Data*out=allocData(olen);
        bit_and(adat,alen,bdat,blen,out->data);
        return BigInt(out,olen).shrink();
    }
    BigInt&operator&=(const BigInt&that){
        if(!dat){return (*this)=(*this)&that;}
        if(!that.dat){
            umax ext=signExtend(that.len);
            bit_and(dat->data,len,&ext,1,dat->data);
        }else{
            size_t olen=len>that.len?len:that.len;
            dat=allocData(dat,olen);
            bit_and(dat->data,len,that.dat->data,that.len,dat->data);
            len=olen;
        }
        return *this;
    }
private:
    static void bit_or(const umax*adat,size_t alen,const umax*bdat,size_t blen,umax*out);
public:
    friend BigInt operator|(const BigInt&a,const BigInt&b){
        if(!(a.dat||b.dat)){return BigInt(a.len|b.len,Signed{});}
        umax aext=signExtend(a.len),bext=signExtend(b.len);
        size_t alen=a.dat?a.len:1,blen=b.dat?b.len:1;
        umax*adat=a.dat?a.dat->data:&aext;
        umax*bdat=b.dat?b.dat->data:&bext;
        
        size_t olen=alen>blen?alen:blen;
        Data*out=allocData(olen);
        bit_or(adat,alen,bdat,blen,out->data);
        return BigInt(out,olen).shrink();
    }
    BigInt&operator|=(const BigInt&that){
        if(!dat){return (*this)=(*this)|that;}
        if(!that.dat){
            umax ext=signExtend(that.len);
            bit_or(dat->data,len,&ext,1,dat->data);
        }else{
            size_t olen=len>that.len?len:that.len;
            dat=allocData(dat,olen);
            bit_or(dat->data,len,that.dat->data,that.len,dat->data);
            len=olen;
        }
        return *this;
    }
private:
    static void bit_xor(const umax*adat,size_t alen,const umax*bdat,size_t blen,umax*out);
public:
    friend BigInt operator^(const BigInt&a,const BigInt&b){
        if(!(a.dat||b.dat)){return BigInt(a.len^b.len,Signed{});}
        umax aext=signExtend(a.len),bext=signExtend(b.len);
        size_t alen=a.dat?a.len:1,blen=b.dat?b.len:1;
        umax*adat=a.dat?a.dat->data:&aext;
        umax*bdat=b.dat?b.dat->data:&bext;
        
        size_t olen=alen>blen?alen:blen;
        Data*out=allocData(olen);
        bit_xor(adat,alen,bdat,blen,out->data);
        return BigInt(out,olen).shrink();
    }
    BigInt&operator^=(const BigInt&that){
        if(!dat){return (*this)=(*this)^that;}
        if(!that.dat){
            umax ext=signExtend(that.len);
            bit_xor(dat->data,len,&ext,1,dat->data);
        }else{
            size_t olen=len>that.len?len:that.len;
            dat=allocData(dat,olen);
            bit_xor(dat->data,len,that.dat->data,that.len,dat->data);
            len=olen;
        }
        return *this;
    }
    BigInt operator~()const{
        if(!dat){return BigInt(~len,Signed{});}
        Data*out=allocData(len);
        for(size_t i=0;i<len;i++){out->data[i]=~dat->data[i];}
        return BigInt(out,len);
    }
    BigInt& invert(){
        if(dat){
            for(size_t i=0;i<len;i++){dat->data[i]=~dat->data[i];}
        }else{len=~len;}
        return *this;
    }
private:
    static bool inc_dec(bool carry,bool borrow,const umax*in,umax*out,size_t len);
public:
    BigInt&operator++(){
        if(!dat){
            umax total=signExtend(len)+1;
            (*this)=BigInt(total,(umax)(!total)-signBit(len));
        }else{
            dat=allocData(dat,len+1);
            dat->data[len]=-(umax)inc_dec(true,false,dat->data,dat->data,len);
            shrink();
        }
        return *this;
    }
    BigInt operator++(int){BigInt old=*this;operator++();return old;}
    BigInt&operator--(){
        if(!dat){
            (*this)=BigInt(signExtend(len)-1,-(umax)(!len)-signBit(len));
        }else{
            dat=allocData(dat,len+1);
            dat->data[len]=-(umax)inc_dec(false,true,dat->data,dat->data,len);
            shrink();
        }
        return *this;
    }
    BigInt operator--(int){BigInt old=*this;operator--();return old;}
    BigInt inc_dec(bool carry,bool borrow)const{
        if(carry==borrow){return *this;}
        if(!dat){
            umax total=signExtend(len)+((umax)carry)-((umax)borrow);
            return BigInt(total,(umax)(total<signExtend(len))-signBit(len)-(umax)(borrow));
        }
        Data*out=allocData(len+1);
        out->data[len]=-(umax)inc_dec(carry,borrow,dat->data,out->data,len);
        return BigInt(out,len+1).shrink();
    }
private:
    static umax add(const umax*adat,size_t alen,const umax*bdat,size_t blen,umax*out);
public:
    friend BigInt operator+(const BigInt&a,const BigInt&b){
        if(!(a.dat||b.dat)){
            size_t lo=a.len+b.len,hi=(size_t)(lo<a.len)-signBit(a.len)-signBit(b.len);
            umax low,high;
            pack(lo,hi,low,high);
            return BigInt(low,high);
        }
        umax aext=signExtend(a.len),bext=signExtend(b.len);
        size_t alen=a.dat?a.len:1,blen=b.dat?b.len:1;
        umax*adat=a.dat?a.dat->data:&aext;
        umax*bdat=b.dat?b.dat->data:&bext;
        
        size_t olen=(alen>blen?alen:blen)+1;
        Data*out=allocData(olen);
        out->data[olen-1]=add(adat,alen,bdat,blen,out->data);
        return BigInt(out,olen).shrink();
    }
    BigInt&operator+=(const BigInt&that){
        if(!dat){return *this=*this+that;}
        if(!that.dat){
            umax ext=signExtend(that.len);
            dat=allocData(dat,len+1);
            dat->data[len]=add(dat->data,len,&ext,1,dat->data);
            len++;
        }else{
            size_t olen=(len>that.len?len:that.len)+1;
            dat=allocData(dat,olen);
            dat->data[olen-1]=add(dat->data,len,that.dat->data,that.len,dat->data);
            len=olen;
        }
        return shrink();
    }
    BigInt operator-()const{return (~*this).inc_dec(true,false);}
    BigInt& negate(){return ++invert();}
    friend BigInt operator-(const BigInt&a,const BigInt&b){return a+-b;}
    BigInt&operator-=(const BigInt&that){(*this)+=-that;return *this;}
private:
    static void multiply(const umax*adat,size_t alen,const umax*bdat,size_t blen,umax*out);
    static void multiply(size_t&x,size_t&y);
public:
    friend BigInt operator*(const BigInt&a,const BigInt&b){
        if(!(a.dat||b.dat)){
            size_t lo=a.len,hi=b.len;
            multiply(lo,hi);
            umax low,high;
            pack(lo,hi,low,high);
            return BigInt(low,high);
        }
        umax aext=signExtend(a.len),bext=signExtend(b.len);
        size_t alen=a.dat?a.len:1,blen=b.dat?b.len:1,olen=alen+blen;
        umax*adat=a.dat?a.dat->data:&aext;
        umax*bdat=b.dat?b.dat->data:&bext;
        Data*out=allocData(olen);
        multiply(adat,alen,bdat,blen,out->data);
        return BigInt(out,olen);
    }
    BigInt operator*=(const BigInt&that){(*this)=(*this)*that;return (*this);}
    BigInt operator/=(const BigInt&that){(*this)=(*this)/that;return (*this);}
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
    static void divmod(const BigInt&a,const BigInt&b,BigInt&q,BigInt&r){
        if(b<0){divmod(a,-b,q,r);q.negate();return;}
        if(a<0){
            divmod(-a,b,q,r);
            if(r.sign()){q=-q-1;r=b-r;}else{q.negate();}
            return;
        }
        q=r=0;
        for(size_t i=(a.dat?a.len:1)*digits<umax>;i--;){
            r=r<<1|((a>>i)&1);
            q<<=1;
            if(r>=b){r-=b;q|=BigInt(1);}
        }
    }
    friend BigInt operator/(const BigInt&a,const BigInt&b){BigInt q,r;divmod(a,b,q,r);return q;}
    friend BigInt operator%(const BigInt&a,const BigInt&b){BigInt q,r;divmod(a,b,q,r);return r;}
};