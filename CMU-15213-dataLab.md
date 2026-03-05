## 项目操作

~~~shell
# 进入目录编辑bits.c
cd cmu-15213;
cd datalab-handout;
nano bits.c;

# CTRL + O 保存后enter
# CTRL + X 退出

./dlc bits.c;
make;
./driver.pl;
~~~

## 位运算操作符

~ : **按位取反**，对操作数的每个位取反。

&：**按位与**，两个操作数对应位都为1时，结果位才为1。

|：**按位或**，两个操作数对应位有一个为1时，结果位就为1。

^：**按位异或**，两个操作数对应位不同时，结果位为1；相同时为0。

<<：**左移**，将二进制位向左移动指定位数，右边空位补0。

\>\> ：**右移**，将二进制位向右移动指定位数。

# 整型部分

## Function 1 : bitXor (Rating: 1)

**题意** ：返回x和y按位异或的值。

**德摩根定律** ：x ^ y = ~(x & y) & ~(~x & ~y).

~~~c
int bitXor(int x, int y) {
    return (~(x & y)) & (~(~x & ~y));
}
~~~

## Function 2 : tmin (Rating : 1)

**题意** ：返回int型的最小值。

**原理** ：1 << 31

~~~c
int tmin(void) {
    return 1 << 31;
}
~~~

## Function 3 : isTmax (Rating : 1)

**题意** ：判断输入值 `x` 是否为二进制补码（two's complement）表示中的最大值。如果是，则返回 1；否则，返回 0。

~~~c
int isTmax(int x) {
    // 一行版本：检查 ~(x+1) == x 且 x+1 != 0
    return !(~(x + 1) ^ x) & !!(x + 1);
}
~~~

## Function 4 : allOddBits (Rating : 2)

**题意** ：检查一个32位整数中所有奇数编号的位是否都被设置为1。位编号从0开始，其中0是最低有效位（LSB），31是最高有效位（MSB）。

~~~c
int allOddBits(int x) {
    int mask = 0xAA | (0xAA << 8);
    mask = mask | (mask << 16);
    return !((x & mask) ^ mask);
}
~~~

## Function 5 : negate (Rating : 2)

**题意** ：返回x的相反数。

~~~c
int negate(int x) {
    return (~x) + 1;
}
~~~

## Function 6 : isAsciiDigit (Rating : 3)

**题意** ：判断整数x是否在0x30到0x39之间（闭区间）。如果是，返回1；否则返回0。

~~~c
int isAsciiDigit(int x){
    return !((x >> 4) ^ 0x3) & !!(((x & 0xF) + (~0xA + 1)) >> 31);
    // !((x >> 4) ^ 0x3) 判断高位是否为3
    // ((x & 0xF) + (~0xA + 1)) >> 31 对低四位减10，取符号位，符号位为1则符合
}
~~~

## Function 7 : conditional (Rating : 3)

**题意** ：模拟 C 语言中的三元条件运算符 x ? y : z。

~~~c
int conditional(int x, int y, int z){
    int mask = ~(!!x) + 1;
    return (y & mask) | (z & ~mask);
}
~~~

## Function 8 : isLessOrEqual (Rating : 3)

**题意** ：判断x是否小于等于y。

~~~c
int isLessOrEqual(int x, int y) {
    int sign_x = (x >> 31) & 1;
    int sign_y = (y >> 31) & 1;
    int same = !(sign_x ^ sign_y); // 判断是否同号
    int dif = x + ~y + 1;
    // if dif < 0, then x < y, signal = 1.
    // if dif = 0, then x = y, signal = 0.
    
    // if dif > 0, then x > y, signal = 0.
    int zero = !(dif ^ 0);
    int signal = !!(dif >> 31);
    return (same & (zero | signal)) | (~same & sign_x); //同号则看符号位和零标志位，异号则需y为正。
}
~~~

## Function 9 : logicalNeg (Rating : 4)

**题意** ：模拟C语言中的 ! 运算符。

~~~c
int logicalNeg(int x) {
    int negX = ~x + 1; // x非0时， x | (-x) = -1 或 intMin(x = intMin 时)
    int sign = (negX | x) >> 31; 
    return sign + 1;
}
~~~

## Function 10 : howManyBits (Rating : 4)(未完成)

**题意** ：计算表示一个数所需的最小位数（包括符号位）。

~~~c
int howManyBits(int x) {
    int b16, b8, b4, b2, b1, b0;
    int sign = x >> 31;
    x = (sign & ~x) | (~sign & x);
    b16 = !!(x >> 16) << 4;
    x = x >> b16;
    b8 = !!(x >> 8) << 3;
    x = x >> b8;
    b4 = !!(x >> 4) << 2;
    x = x >> b4;
    b2 = !!(x >> 2) << 1;
    x = x >> b2;
    b1 = !!(x >> 1);
    x = x >> b1;
    b0 = x;
    return b16 + b8 + b4 + b2 + b1 + b0 + 1;
}
~~~

# 浮点数部分

## 浮点数结构

符号位 s ：1bit。

指数部分 exp : 8bit。

尾数部分 frac : 23bit。

### ① E = 0（全 0）

- M = 0 → **±0**
- M ≠ 0 → **非规格化数**（很小的数）

### ② 1 ≤ E ≤ 254

→ **正常规格化浮点数**

value=(−1)^S×(1.M)×2^(E−127)

### ③ E = 255（全 1）

- M = 0 → **± 无穷大（±∞）**
- M ≠ 0 → **NaN（非数值）**

## Function 11 : floatScale2 (Rating : 4)

**题意** ：给定浮点数 f，返回2 * f。

~~~c
unsigned floatScale2(unsigned uf) {
    unsigned s = uf >> 31;
    unsigned exp = (uf >> 23) & 0xFF;
    unsigned frac = uf & 0x7FFFFF;
    if(exp == 0 && frac == 0) return uf; // uf = 0
    if(exp == 0xFF) return uf; // uf= (∞)
    if(exp == 0){
		frac <<= 1;
        return (s << 31) | frac;
    }
    exp++;
    return (((s << 8) | exp) << 23) | frac;
}
~~~

## Function 12 : floatFloat2Int (Rating : 4)

**题意** ：将浮点数 f 转换为整数。

~~~c
int floatFloat2Int(unsigned uf) {
    unsigned s = uf >> 31;
    unsigned exp = ((uf >> 23) & 0xFF);
    unsigned frac = uf & 0x7FFFFF;
    if(exp < 127) return 0;
    if(exp > 157) return 0x80000000;
    frac = frac | (1 << 23);
    if(exp >= 150){
        frac <<= (exp - 150);
    }else{
        frac >>= (150 - exp);
    }
    if(s) return ~frac + 1;
    return frac;
}
~~~

## Function 13 : floatPower2 (Rating : 4)

**题意** ：返回2.0 ^ x，其中 x 是整数。

~~~c
unsigned floatPower2(int x) {
    if(x < -23) return 0;
    if(x > 127) return +INF;
    if(x >= -23 && x <= -1){
        return 1 << (x + 23);
    }
    x += 127;
    return x << 23;
}
~~~



