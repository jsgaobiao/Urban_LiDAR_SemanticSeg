#include "seglogloader.h"
#include <stdio.h>
#include <stdlib.h>

SegLogLoader::~SegLogLoader()
{
    if (seeds)
        delete [] seeds;
    if (prids)
        delete [] prids;
}

SegLogLoader::SegLogLoader()
{

    seeds = NULL;
    seednum = 0;
    prids = NULL;
    pridnum = 0;
    pridbufnum = 0;
    seedidx = -1;
}

void SegLogLoader::loadSegLog(char *szFile)
{
    char			i_line[200];
    FILE			*segfp;

    int	*streamcnt = new int[MAXLABNUM];
    memset (streamcnt, 0, sizeof(int)*MAXLABNUM);
    int	*samplecnt = new int[MAXLABNUM];
    memset (samplecnt, 0, sizeof(int)*MAXLABNUM);

    segfp = fopen (szFile, "r");
    if (!segfp)
        return;

    pridnum = 0;
    seednum = 0;
    while (1) {
        if (fgets (i_line, 200, segfp) == NULL)
            break;
        if (strncasecmp(i_line, "prid", 4) == 0) {
            pridnum ++;
            continue;
        }
        if (strncasecmp (i_line, "seed", 4) == 0) {
            seednum ++;
            continue;
        }
    }
    if (!seednum || !pridnum)
        return;

    fseek (segfp, 0L, SEEK_SET);
    seeds = new ONESEED[seednum];
    memset (seeds, 0, sizeof(ONESEED)*seednum);
    pridbufnum = pridnum*2;
    prids = new ONEPRID[pridbufnum];
    memset (prids, 0, sizeof (ONEPRID)*pridbufnum);

    int streamtotal=0, sampletotal=0;
    int sno=0, pno=0, fnum;
    char *str;
    while (1) {
        if (fgets (i_line, 200, segfp) == NULL)
            break;
        if (strncasecmp (i_line, "prid", 4) == 0) {
            strtok (i_line, "= ,\t\n");
            prids[pno].prid = atoi (strtok (NULL, "= ,\t\n"));
            // start frame number
            prids[pno].sfn = atoi (strtok (NULL, "= ,\t\n"));
            // end frame number
            prids[pno].efn = atoi (strtok (NULL, "= ,\t\n"));
            prids[pno].snum = atoi (strtok (NULL, "= ,\t\n"));
            prids[pno].sid = sno;
            prids[pno].eid = sno+prids[pno].snum-1;
            str = strtok (NULL, "= ,\t\n");
            if (str) prids[pno].lab = atoi (str);
            streamcnt[prids[pno].lab]++;
            streamtotal++;

            map_prid_label[prids[pno].prid] = prids[pno].lab;

            if (++pno>=pridnum)
                break;

            continue;
        }
        if (strncasecmp (i_line, "seed", 4) == 0) {
            strtok (i_line, "= ,\t\n");
            seeds[sno].sno = sno;
            seeds[sno].fno = atoi (strtok (NULL, "= ,\t\n"));
            seeds[sno].milli = atoi (strtok (NULL, "= ,\t\n"));
            seeds[sno].rno  = atoi (strtok (NULL, "= ,\t\n"));
            seeds[sno].cp.x = atof (strtok (NULL, "= ,\t\n"));
            seeds[sno].cp.y = atof (strtok (NULL, "= ,\t\n"));
            seeds[sno].cp.z = atof (strtok (NULL, "= ,\t\n"));
            seeds[sno].ip.x = atoi (strtok (NULL, "= ,\t\n"));
            seeds[sno].ip.y = atoi (strtok (NULL, "= ,\t\n"));
            seeds[sno].pnum = atoi (strtok (NULL, "= ,\t\n"));
            seeds[sno].mind = atof (strtok (NULL, "= ,\t\n"));
            if (pno>0) {
                seeds[sno].prid = prids[pno-1].prid;
                seeds[sno].lab = prids[pno-1].lab;
            }
            samplecnt[seeds[sno].lab]++;
            sampletotal++;
            if (++sno>=seednum)
                break;
            continue;
        }
    }
    fclose (segfp);

    if (!seednum)
        return;
    qsort ((void *)seeds, seednum, sizeof(ONESEED), seed_compare_fno);
//    sFrmNo=seeds[0].fno;
//    eFrmNo=seeds[seednum-1].fno;
    for (int i=0; i<MAXLABNUM; i++) {
        if (!streamcnt[i]) continue;
        printf ("LABEL%d --- stream %d(%.2f%%) sample %d(%.2f%%)\n", i, streamcnt[i], streamcnt[i]/(double)streamtotal*100, samplecnt[i], samplecnt[i]/(double)sampletotal*100);
    }
    printf ("TOTAL --- stream %d sample %d\n", streamtotal, sampletotal);
}

void SegLogLoader::loadColorTabel(char *filename)
{
    memset (colorTable, 0, MAXLABNUM*4);

    char	i_line[80];
    FILE	*fp;

    fp = fopen (filename, "r");
    if (!fp)
        return;

    int n=0;
    while (1) {
        if (fgets (i_line, 80, fp) == NULL)
            break;
        colorTable[n][0] = atoi (strtok (i_line, ",\t\n"));
        colorTable[n][1] = atoi (strtok (NULL, ",\t\n"));
        colorTable[n][2] = atoi (strtok (NULL, ",\t\n"));
        colorTable[n][3] = atoi (strtok (NULL, ",\t\n"));
        n++;
        if (n>=MAXLABNUM)
            break;
    }
    fclose (fp);
}

int seed_compare_fno(const void *t1, const void *t2)
{
    ONESEED	*trj1, *trj2;

    trj1 = (ONESEED *)t1;
    trj2 = (ONESEED *)t2;

    if (trj1->fno>trj2->fno)
        return 1;
    else if (trj1->fno<trj2->fno)
        return -1;
    else
        return 0;
}
