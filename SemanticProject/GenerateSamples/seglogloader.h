#ifndef SEGLOGLOADER_H
#define SEGLOGLOADER_H
#include "types.h"

extern std::string imgname;
extern std::string videoName;
extern std::string videoNameSeg;

extern int HEIGHT;
extern int WIDTH;
extern int VIS_HEIGHT;
extern int VIS_WIDTH;
extern double FAC_HEIGHT;

int seed_compare_fno(const void *t1, const void *t2);

class SegLogLoader
{
public:
    ~SegLogLoader();
    SegLogLoader();
    void loadSegLog(char *szFile);
    void loadColorTabel(char *filename);

    unsigned char colorTable[MAXLABNUM][4];

    std::map<int, int> map_prid_label;

    ONESEED		*seeds;
    int			seednum ;
    ONEPRID		*prids;
    int			pridnum;
    int			pridbufnum;
    int			seedidx;
};

#endif // SEGLOGLOADER_H
