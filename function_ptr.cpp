#include <iostream>
#include <mutex>
using namespace std;

class test {

};

class dtest: public test {
	virtual void add() {
	}
};

int main(int argc, char *argv[])
{
	double i = 100;
	double *j = &i;
	double *k;

	char ch[32];
	cout<<"Size is: "<<sizeof(ch)<<endl;


	return 0;
}