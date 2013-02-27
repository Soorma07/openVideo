#include <iostream>
#include <string>
#include "cameraException.h"

using namespace std;
using namespace libV4l2Capture;

int main(int argc, char **argv)
{
	string devName = "/dev/null";
	cameraException except( devName, 1 );
	cout << except.what() << endl;
	cout << "making a copy of the exception" << endl;
	cameraException newExcept = except;
	cout << newExcept.what() << endl;
	cout << "Hello testing world!" << endl;
	return 0;
}
