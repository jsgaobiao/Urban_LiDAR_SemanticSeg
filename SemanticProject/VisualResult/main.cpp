#include <iostream>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <QString>
#include <QStringList>
#include "dsvlprocessor.h"

using namespace std;
void loadClassifyLog(std::string filename, std::map<int, std::vector<CLASSIFYINFO> >& mapper)
{
    std::ifstream logfile;
    logfile.open(filename.c_str());
    if (!logfile.is_open())
        return;

    mapper.clear();
    while(!logfile.eof()){
        CLASSIFYINFO data;
        int time;
        std::string header, file_info;
        int tt;

        logfile>>header>>file_info>>data.name>>data.truth>>data.label;
        for (int i=0; i<classes_num; i++){
            logfile>>data.scores[i];
        }
        if (header.empty())
            return;

        QString regioninfo(data.name.c_str());
        QStringList list = regioninfo.split("_");

        data.regionid = list[1].toInt();
        time = list[0].toInt();

        std::map<int, std::vector<CLASSIFYINFO> >::iterator iter;
        iter = mapper.find(time);

        if (iter == mapper.end()){
            std::vector<CLASSIFYINFO> vec;
            vec.push_back(data);
            mapper[time] = vec;
        }
        else{
            iter->second.push_back(data);
        }
    }
}

void loadNegSampleLog(std::string filename, std::map<int, std::vector<CLASSIFYINFO> >& mapper)
{
    std::ifstream logfile;
    logfile.open(filename.c_str());
    if (!logfile.is_open())
        return;

    mapper.clear();
    while(!logfile.eof()){
        CLASSIFYINFO data;
        int time;
        std::string header;
        int tt;

        logfile>>header>>time>>data.name>>tt>>data.label;
        if (header.empty())
            return;

        QString regioninfo(data.name.c_str());
        QStringList list = regioninfo.split("_");

        data.regionid = list[1].toInt();

        std::map<int, std::vector<CLASSIFYINFO> >::iterator iter;
        iter = mapper.find(time);

        if (iter == mapper.end()){
            std::vector<CLASSIFYINFO> vec;
            vec.push_back(data);
            mapper[time] = vec;
        }
        else{
            iter->second.push_back(data);
        }
    }
}

int main()
{
    std::map<int, std::vector<CLASSIFYINFO> > mapper, pair_mapper;
    std::string logfilename = "/home/pku-m/SemanticMap/OnlineTest/cnn_test_0329/simple_version/testlog.txt";
    std::string pairlogfilename = "/home/pku-m/SemanticMap/OnlineTest/cnn_test_0329/pairwise_version/result_test_log.txt";
    std::string dsvlfilename = "/media/pku-m/TOSHIBA/Data/20170410_campus/origin/campus1/3D/1.dsvl";
    loadClassifyLog(logfilename, mapper);
    loadClassifyLog(pairlogfilename, pair_mapper);

    DsvlProcessor dsvl(dsvlfilename);
    //dsvl.VisualClassifyResult(mapper);
    dsvl.VisualClassifyResult(mapper, pair_mapper);

//    std::string logfilename = "/home/pku-m/SemanticMap/NegtiveSample/negsample_log.txt";
//    std::string dsvlfilename = "/media/pku-m/TOSHIBA/Data/20170410_campus/origin/campus2/3D/2.dsvl";

//    loadNegSampleLog(logfilename, mapper);
//    DsvlProcessor dsvl(dsvlfilename);

//    dsvl.VisualNegSampleResult(mapper);

    cout << "Hello World!" << endl;
    return 0;
}
