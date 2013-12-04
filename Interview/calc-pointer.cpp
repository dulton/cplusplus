#include<iostream>

using namespace std;

int main(int argc, char*argv[])
{
	int arr[4][3] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
	int (*ptr)[3] = arr;
	int *p = arr[0];

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			cout<<"arr["<<i<<"]["<<j<<"] = "<<arr[i][j]<<endl;
		}
	}


	for (int i = 0; i < 12; i++)
	{
		cout<<"p["<<i<<"] = " << *(p + i) <<endl;
	}

	/*for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			cout<<"p["<<i<<"]["<<j<<"] = " << (*p + i)[j] <<endl;
		}
	}*/

	//cout<<"Value = "<<*(ptr + 1)[1]<<endl;
	cout<<"Value = "<<*(*(arr + 1) +2 )<<endl;

	return 0;
}