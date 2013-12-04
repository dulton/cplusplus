#include<iostream>
#include<unistd.h>
#include<sys/types.h>

using namespace std;

int main(int argc, char* argv[])
{
	int pid = fork();

	if (pid == -1)
		cout<<"ERROR"<<endl;
	else if (pid == 0)
		cout<<"This is parent process"<<endl;
	else
		cout<<"This is child process and its id is: "<<pid<<endl;

	int pid2 = fork();

	cout<<"pid2 = "<<pid2<<endl;

		int pid3 = fork();

	cout<<"pid3 = "<<pid3<<endl;

	return 0;
}