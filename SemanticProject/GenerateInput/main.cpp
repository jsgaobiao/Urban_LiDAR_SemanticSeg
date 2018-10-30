#include <iostream>
#include <QDir>
#include "dsvlprocessor.h"
using namespace std;

int main()
{
    std::string dsvlfilename = "/home/gaobiao/Documents/2-1/2-1.dsvl";
    DsvlProcessor dsvl(dsvlfilename);
    dsvl.Processing();

    cout << "Hello World!" << endl;
    return 0;
}
