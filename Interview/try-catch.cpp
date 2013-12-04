#include<iostream>
#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>


using namespace std;

int main(int argc, char * argv[])
{
	try
	{
		throw((int)2);
		cout<<"A";		
	}
	catch(int)
	{
		cout<<"B";
	}
	catch(...)
	{
		cout<<"C";
	}

	cout<<"D";

	fork();
	fork();
	fork();

	printf("Hello World!");

	return 0;
}