/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
/* 
 * bitAnd - x&y using only ~ and | 
 *   Example: bitAnd(6, 5) = 4
 *   Legal ops: ~ |
 *   Max ops: 8
 *   Rating: 1
 */
//~非，|或
int bitAnd(int x, int y) {
  return ~((~x)|(~y));//非非或非就是与
}
/* 
 * getByte - Extract byte n from word x
 *   Bytes numbered from 0 (LSB) to 3 (MSB)
 *   Examples: getByte(0x12345678,1) = 0x56
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */

//在word中取出第n位，0是最低位有效数字，3是最高位有效数字
int getByte(int x, int n) {
    return (x>>(n<<3))&0xff;//不可以*8，左移就可以；

}
/* 
 * logicalShift - shift x to the right by n, using a logical shift
 *   Can assume that 0 <= n <= 31
 *   Examples: logicalShift(0x87654321,4) = 0x08765432
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3 
 */
//逻辑右移，因为int是采用算数右移的。
//用掩码，n位之前的都是0，后面都是1，先x执行算数右移再&一下。
int logicalShift(int x, int n) {
    int m = ~((1<<31)>>n<<1);
    return (x>>n) & m ;
}
/*
 * bitCount - returns count of number of 1's in word
 *   Examples: bitCount(5) = 2, bitCount(7) = 3
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 40
 *   Rating: 4
 *   采用分治法，从2到4到8到16.
 *   0x55，是01010101
 */

int bitCount(int x)
{
    int c;
    c = (x&0x55555555)+((x>>1)&0x55555555);//+优先级要比&高
    c = (c&0x33333333)+((c>>2)&0x33333333);
    c = (c&0x0f0f0f0f)+((c>>4)&0x0f0f0f0f);
    c = (c&0x00ff00ff)+((c>>8)&0x00ff00ff);
    c = (c&0x0000ffff)+((c>>16)&0x0000ffff);
    return c;
}
/* 
 * bang - Compute !x without using !
 *   Examples: bang(3) = 0, bang(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 *   !非0返回0，0返回1.
 */
int bang(int x) {
    int temp = ~x+1;//-x
    temp = x|temp;//非0的话最高位就是1，因为正数和负数
    temp = temp>>31;
    return temp+1;
}
/* 
 * tmin - return minimum two's complement integer 
 *   补码最小数
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
  return 1<<31;
}
/* 
 * fitsBits - return 1 if x can be represented as an 
 *  n-bit, two's complement integer.
 *  就是能不能用n位补码的形式表示x。
 *   1 <= n <= 32
 *   Examples: fitsBits(5,3) = 0, fitsBits(-4,3) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int fitsBits(int x, int n) {
    int shiftNumber = 32 + (~n+1);// 32 - n
    return !(x^((x<<shiftNumber)>>shiftNumber));
    /*先左移32-n位，在右移32-n位，即保留最后n位数。在与x异或
     若两者相同表示x可被表示为一个n位整数，！0为1
     */
}
/* 
 * divpwr2 - Compute x/(2^n), for 0 <= n <= 30
 *  Round toward zero 向0舍入
 *   Examples: divpwr2(15,1) = 7, divpwr2(-33,4) = -2
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 *   对于正数直接右移，负数要加偏移量
 */
int divpwr2(int x, int n) {
    int pwr=(1<<n)+(~0);//偏移量，2^n-1
    int mask = x>>31;
    int bais = mask&pwr;//如果是正数，bais就是0，负数就是偏移量。
    return (x+bais)>>n;
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   相反数
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  return (~x)+1;
}
/* 
 * isPositive - return 1 if x > 0, return 0 otherwise 
 *   Example: isPositive(-1) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 3
 */
int isPositive(int x) {
    int m = x>>31;//最高位右移，0000。。。，111.。
    return (!x)^(!(m&x));//
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 同号的时候只要检查x-y的值是否大于0；
 没有用到溢出来检查，只要异号，就直接看x的符号就好了。
 如果x异号的话，只需要考虑x就好了，x是1那么就<=,返回1，如果x是0就返回0，不小于。
 
 */
int isLessOrEqual(int x, int y) {
    int sigx = (x>>31)&1;//get the symbol
    int sigy = (y>>31)&1;
    int sig = (sigx^sigy)&sigx;//同号是0，异号是x的号
    int temp = x+((~y)+1);//x-y
    temp = ((temp>>31)&1)&(!(sigx^sigy));//x,y同号的话temp最高位不变，异号的话是0
    return (sig|temp|((!(x^y))));
    
}
/*
 * ilog2 - return floor(log base 2 of x), where x > 0
 *   Example: ilog2(16) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 90
 *   Rating: 4
 不是返回指数，返回最高位1的位置,若是大于0，就返回16
    二分法，左移右移看是否不是0.
 */
int ilog2(int x) {
    int word = (!!(x>>16))<<4;//!!是为了将x>>16变成一个1
    word = word+((!!(x>>(word+8)))<<3);//如果word不是0，那么就要在16位基础上再往前看
                                     //如果word是0，就右移8位
    word = word +((!!(x>>(word+4)))<<2);
    word = word +((!!(x>>(word+2)))<<1);
    word = word +(!!(x>>(word+1)));
    
    return word;
}
/* 
 * float_neg - Return bit-level equivalent of expression -f for
 *   floating point argument f.
 返回-f的位级表示
 *   Both the argument and result are passed as unsigned int's, but
 
 参数和返回值都是unsigned但是都被解释为单精度浮点数。当小数域全为0时，是nan
 
 浮点数是用IEEE浮点表示，v=(-1)^S*M*2^E;
 规则：E = e - bias,m=1+f; e 是exp
 不规则： E = 1-bias,m = f;
 *   they are to be interpreted as the bit-level representations of
 *   single-precision floating point values.
 *   When argument is NaN, return argument.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 10
 *   Rating: 2
 */
unsigned float_neg(unsigned uf) {
    unsigned result;
    unsigned tmp;
    result=uf ^ 0x80000000; //将符号位改反
    tmp=uf & (0x7fffffff);
    if(tmp > 0x7f800000)//,阶码全是1，小数域非0。此时是NaN
        result = uf;
    return result;
}
/* 
 * float_i2f - Return bit-level equivalent of expression (float) x
 返回单精度浮点数x的位级表示
 *   Result is returned as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point values.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_i2f(int x) {
    unsigned sign = 0;
    unsigned exp =0;//E = e-127;这个用来表示偏置量；
    
    
    unsigned M = 0;
    
    sign = ((x>>31)&0x1)<<31;
    unsigned res = 0;
    
  return 2;
}
/* 
 * float_twice - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_twice(unsigned uf) {
    unsigned f = uf;
    if ((f & 0x7F800000) == 0) //
    {
        //左移一位
        f = ((f & 0x007FFFFF) << 1) | (0x80000000 & f);
    }
    else if ((f & 0x7F800000) != 0x7F800000)
    {
        f =f + 0x00800000;
    }
    return f;
}
