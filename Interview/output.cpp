#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

using namespace std;

int main(int argc, char* argv[])
{
	unsigned int a = 0x12345678;
	unsigned char *p = (unsigned char *)&a;

	for (int i = 0; i < sizeof(a); i++)
	{
		//cout<<"p["<<i<<"] = "<<p[i]<<endl;
		printf("%x", p[i]);
	}

	char str[] = "Hello\0world";
	char str2[15] = {0};
	strcpy(str2, str);
	cout<<"str size is = "<<sizeof(str)<<endl;
	cout<<"str2 size is = "<<sizeof(str2)<<endl;

	char t[100] = {0};
	cout<<"t size is = "<<sizeof(t)<<endl;

	return 0;
}