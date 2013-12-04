#include<iostream>

using namespace std;

int main(int argc, char*argv[])
{
	typedef enum {V1, V2} Value;
	typedef struct header
	{
		unsigned short size;
		unsigned short id;
		unsigned char type;

		Value value;

		struct
		{
			unsigned first : 3;
			unsigned pad : 2;
		} status;

		#define first status.first;
		#define pad status.pad;

		unsigned int s;
		unsigned int ns;
	} header_t;


	cout<<"Size of header_t = "<<sizeof(header_t)<<endl;
	cout<<"unsigned short size is = "<<sizeof(unsigned short)<<endl;
	cout<<"unsigned size is = "<<sizeof(unsigned)<<endl;
	cout<<"unsigned int size is = "<<sizeof(unsigned int)<<endl;
	cout<<"unsigned char size is = "<<sizeof(unsigned char)<<endl;
	cout<<"value size is = "<<sizeof(Value)<<endl;
	//cout<<"status size is = "<<sizeof(stat)<<endl;

	return 0;
}