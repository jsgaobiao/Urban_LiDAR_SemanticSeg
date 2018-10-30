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
#include "seglogloader.h"

using namespace std;
using namespace cv;



class DsvlProcessor
{
public:
    DsvlProcessor(std::string filename);
    void ProcessOneFrame ();
    bool ReadOneDsvlFrame ();
    void Processing(DsvlProcessor &dsvl_bg);
    void CheckStreamByPrid(cv::Mat& img, int prid);
    void InitRmap(RMAP *rm);
    void ReleaseRmap(RMAP *rm);

public:

    static void MouseCallback(int event, int x, int y, int, void* userdata);

    void setSeglog(SegLogLoader *value);

public:
    int dsvlbytesiz;
    int dsvbytesiz;
    int labbytesiz;
    int dFrmNum;
    ONEDSVFRAME	*onefrm;
    SegLogLoader *seglog;
    std::ifstream dfp;
    RMAP rm;
    bool isRunning;
};

#endif // DSVLPROCESSOR_H
