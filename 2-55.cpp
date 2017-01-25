#include <stdio.h>
#include <iostream>


//本机使用小端法，连续存储。
//跨越多个字节的程序对象，如int4个字节，在存储器中是按照从最低有效位开始存储。

using std::cin;
using std::cout;
using std::endl;

typedef unsigned char* byte_pointer;
//将其他类型转化成unsigned char* ，把他们的指针值输出来，因为位模式使用16进制表示


void show_bytes(byte_pointer start,int len)
{
	int i;
	for(i=0;i<len;++i)
		printf("%.2x ",start[i]);
	//start是，每个数据的指针，指针是他在虚拟地址空间中的位置，但是start[i]是他们指针上的值
	
	printf("\n");
}

void show_int(int x)
{
	show_bytes((byte_pointer)& x,sizeof(int));
	int * a = &x;
	cout<<a<<endl;
}

void show_float(float x)
{
	show_bytes((byte_pointer)& x,sizeof(float));
}

void show_pointer(void* x)
{
	show_bytes((byte_pointer)&x,sizeof(void *));
}

void show_short(short x)
{
	show_bytes((byte_pointer) &x,sizeof(short));
}


void test_show_bytes(int val)
{
	int ival = val;
	float fval = val;
	int * pval = &val;
	short sval = val;
	show_int(ival);
	show_short(sval);
	show_float(fval);
	show_pointer(pval);
}

int main()
{
	int val = 12345;
	test_show_bytes(12345);
	return 0;
}
