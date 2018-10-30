#include <iostream>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <QString>
#include <QStringList>
#include "dsvlprocessor.h"

using namespace std;

int main()
{
    std::string dsvlfilename = "/home/gaobiao/Documents/2-1/2-2-new.dsvl";

    DsvlProcessor dsvl(dsvlfilename);
    dsvl.Processing();

    cout << "Hello World!" << endl;
    return 0;
}
