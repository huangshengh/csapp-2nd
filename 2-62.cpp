#include <iostream>

using std::cin;
using std::cout;
using std::endl;
 

//使用逻辑右移返回1，算数右移返回0
//x的最高位是0，就是逻辑右移，否则就是算数
//那x<0，不就是算数右移了。
int int_shifts_are_logical()
{
	int x=-1;
	return (x>>1)==1;//如果是logical右移就是1，否则就是-1；

}

int main()
{
	cout<<int_shifts_are_logical()<<endl;
	return 0;

}