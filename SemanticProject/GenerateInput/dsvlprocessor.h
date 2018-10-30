#ifndef DSVLPROCESSOR_H
#define DSVLPROCESSOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <math.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>

#include "types.h"

using namespace std;
using namespace cv;



class DsvlProcessor
{
public:
    DsvlProcessor(std::string filename);
    void ProcessOneFrame ();
    bool ReadOneDsvlFrame ();
    void Processing();
    void InitRmap(RMAP *rm);
    void ReleaseRmap(RMAP *rm);
private:
    int dsbytesiz;
    int labbytesiz;
    int dFrmNum;
    ONEDSVFRAME	*onefrm;
    std::ifstream dfp;
    RMAP rm;
    bool isRunning;
};

#endif // DSVLPROCESSOR_H
