#include <iostream>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <QString>
#include <QStringList>
#include "dsvlprocessor.h"
#include "seglogloader.h"

using namespace std;

std::string imgname;
std::string videoName;
std::string videoNameSeg;

int HEIGHT;
int WIDTH;
int VIS_HEIGHT;
int VIS_WIDTH;
double FAC_HEIGHT;

int main()
{
    std::string dsvlfilename = "/home/gaobiao/Documents/SemanticSeg_IV2019/data/2-2-bg.dsvl";
    std::string seglogfilename = "/home/gaobiao/Documents/SemanticSeg_IV2019/data/2-2-bgalign.log";
    // 如果修改类别数量，需要修改Ground对应的类别编号（samplegenerator.cpp 359行）
    std::string colortablefilename = "/home/gaobiao/Documents/SemanticSeg_IV2019/data/colortable_7.txt";
    videoName = "/home/gaobiao/Documents/SemanticSeg_IV2019/data/gt.avi";
    videoNameSeg = "/home/gaobiao/Documents/SemanticSeg_IV2019/data/seg.avi";
    imgname = "/home/gaobiao/Documents/SemanticSeg_IV2019/data/images/";

    HEIGHT = 32;
    WIDTH  = 1080;
    VIS_HEIGHT = 144;
    VIS_WIDTH  = 1080;
    FAC_HEIGHT = (double)VIS_HEIGHT / (double)HEIGHT;

    SegLogLoader segloader;
    segloader.loadSegLog(const_cast<char*>(seglogfilename.c_str()));
    segloader.loadColorTabel(const_cast<char*>(colortablefilename.c_str()));

    DsvlProcessor dsvl(dsvlfilename);
    dsvl.setSeglog(&segloader);
    dsvl.Processing();

    cout << "Over!" << endl;
    return 0;
}
