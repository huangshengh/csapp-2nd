#include <iostream>


using std::cin;
using std::cout;
using std::endl;

/*unsigned replace_byte(unsigned x,unsigned char b,int i)
{
	unsigned char c = 0xFF;
	int shift_val = (sizeof(unsigned))<<i;
	unsigned char d = c<<shift_val;
	unsigned  e =  x&(~d);
	unsigned  f = b<<shift_val;
	unsigned  g = e+f;
	return g;
}
*/
unsigned replace_byte(unsigned x, unsigned char b,int i)
{

	char *p=(char *)&x;

    p[i]=b;

    return x;
}


int main(int argc, char const *argv[])
{
	unsigned x=0x12345678;
	unsigned char b=0xAB;
	int i = 2;
	
	cout<<h<<endl;
	return 0;
}