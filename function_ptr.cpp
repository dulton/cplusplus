#include <iostream>

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

	cout<<"Class pointer size: "<<sizeof(void (test::*)())<<endl;

	cout<<"Application's name is: "<<argv[0]<<endl;
	cout<<"Empty class test size is: "<<sizeof(test)<<endl;
	cout<<"Empty class dtest size is: "<<sizeof(dtest)<<endl;
	cout<<"Empty class pointer size is: "<<sizeof(new test())<<endl;

	cout<<"Variable address size is: "<<sizeof(&i)<<endl;
	cout<<"Variable pointer(*k) size is: "<<sizeof(*k)<<endl;
	cout<<"Variable pointer(*j) size is: "<<sizeof(*j)<<endl;

	return 0;
}