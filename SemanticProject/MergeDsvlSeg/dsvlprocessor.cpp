#include "dsvlprocessor.h"
#include "samplegenerator.h"
#include <unordered_set>
DsvlProcessor::DsvlProcessor(string filename)
{
    dsvlbytesiz = sizeof (ONEDSVDATA);
    dsvbytesiz = dsvlbytesiz-sizeof(int)*LINES_PER_BLK*PNTS_PER_LINE;
    dFrmNum=0;

    dfp.open(filename.c_str(), std::ios_base::binary);
    if (!dfp.is_open()){
        printf("File open failure : %s\n", filename.c_str());
        isRunning = 0;
    }
    else{
        isRunning = 1;
    }
}

void DsvlProcessor::ProcessOneFrame()
{
    int x, y;
    double	rng;
    point3fi *p;

    //����һ֡vel32���ݡ�����������ϵ����

    //���ӻ�λͼ
    cvZero(rm.rMap);	//����ͼ��
    cvZero(rm.lMap);	//�ָ�ͼ��
    memset (rm.pts, 0, sizeof (point3fi)*rm.wid*rm.len);	//����ͼ����Ӧ�ļ���������
    memset (rm.regionID, 0, sizeof (int)*rm.wid*rm.len);	//����ͼ����Ӧ�ķָ���ǩ����
    rm.regnum = 0;
    //�ָ���ǩ��Ŀ
    rm.ang = onefrm->ang;
    rm.shv = onefrm->shv;

    rm.millsec = onefrm->dsv[0].millisec;

    //���ɾ���ͼ������Ӧ�����ݡ���180��12����32
    for (int i=0; i<BKNUM_PER_FRM; i++) {
        for (int j=0; j<LINES_PER_BLK; j++) {
            for (int k=0; k<PNTS_PER_LINE; k++) {
                p = &onefrm->dsv[i].points[j*PNTS_PER_LINE+k];
                if (!p->i)
                    continue;
                rng=sqrt(sqr(p->x)+sqr(p->y)+sqr(p->z));
                x=i*LINES_PER_BLK+j;
                y=k;
                rm.pts[y*rm.wid+x] = onefrm->dsv[i].points[j*PNTS_PER_LINE+k];
//                std::cout<<rm.pts[y*rm.wid+x].z<<' '<<rm.pts[y*rm.wid+x].y<<' '<<rm.pts[y*rm.wid+x].z<<std::endl;

                rm.rMap->imageData[y*rm.wid+x] = min(255,int(rng*10));
                rm.regionID[y*rm.wid+x] = onefrm->dsv[i].lab[j*PNTS_PER_LINE+k];
                rm.regnum = max (rm.regnum, rm.regionID[y*rm.wid+x]);

                unsigned int val;

                /*if (rm.regionID[y*rm.wid+x]==EDGEPT) {
                    rm.lMap->imageData[(y*rm.wid+x)*3+2]	= 128;
                    rm.lMap->imageData[(y*rm.wid+x)*3+1]	= 128;
                    rm.lMap->imageData[(y*rm.wid+x)*3+0]	= 128;
                }
                else */if (rm.regionID[y*rm.wid+x]==GROUND)
                {
                    rm.lMap->imageData[(y*rm.wid+x)*3+2]	= 0;
                    rm.lMap->imageData[(y*rm.wid+x)*3+1]	= 0;
                    rm.lMap->imageData[(y*rm.wid+x)*3+0]	= 128;
                }
                else if (rm.regionID[y*rm.wid+x]==NONVALID)
                {
                    rm.lMap->imageData[(y*rm.wid+x)*3+2]	= 255;
                    rm.lMap->imageData[(y*rm.wid+x)*3+1]	= 255;
                    rm.lMap->imageData[(y*rm.wid+x)*3+0]	= 255;
                }
                else if (rm.regionID[y*rm.wid+x]==UNKNOWN)
                {
                    rm.lMap->imageData[(y*rm.wid+x)*3+2]	= 64;
                    rm.lMap->imageData[(y*rm.wid+x)*3+1]	= 64;
                    rm.lMap->imageData[(y*rm.wid+x)*3+0]	= 64;
                }
                else if (rm.regionID[y*rm.wid+x]==BACKGROUND)
                {
                    val = rm.pts[y*rm.wid+x].i;
                    rm.lMap->imageData[(y*rm.wid+x)*3+2]	= val;
                    rm.lMap->imageData[(y*rm.wid+x)*3+1]	= val;
                    rm.lMap->imageData[(y*rm.wid+x)*3+0]	= val;
                }
                else {
                    if (rm.regionID[y*rm.wid+x] < 0) {
                        rm.regionID[y*rm.wid+x] *= -1;
                    }
                    val = (rm.regionID[y*rm.wid+x])%COLORNUM;
                    rm.lMap->imageData[(y*rm.wid+x)*3+2]	= LEGENDCOLORS[val][2];
                    rm.lMap->imageData[(y*rm.wid+x)*3+1]	= LEGENDCOLORS[val][1];
                    rm.lMap->imageData[(y*rm.wid+x)*3+0]	= LEGENDCOLORS[val][0];
                }
            }
        }
    }
}

bool DsvlProcessor::ReadOneDsvlFrame()
{
    int		i;
    for (i=0; i<BKNUM_PER_FRM; i++) {
        dfp.read((char *)&onefrm->dsv[i], dsvlbytesiz);
        if (dfp.gcount() != dsvlbytesiz)
            break;
    }

    if (i<BKNUM_PER_FRM)
        return 0;
    else
        return 1;
}

void DsvlProcessor::Processing(DsvlProcessor &dsvl_bg)
{
    dfp.seekg(0, std::ios_base::end);
    dFrmNum = dfp.tellg() / 180 / dsvlbytesiz;

    dfp.seekg(0, std::ios_base::beg);

    InitRmap (&rm);
    dsvl_bg.InitRmap(&(dsvl_bg.rm));
    onefrm= new ONEDSVFRAME[1];

    IplImage * out = cvCreateImage (cvSize (rm.wid/2, rm.len*4.5),IPL_DEPTH_8U,1);
    IplImage * col = cvCreateImage (cvSize (rm.wid/2, rm.len*4.5),IPL_DEPTH_8U,3);

    int	num = 0;

    std::string videoName = "/home/gaobiao/Documents/2-1/ladybug.avi";
    cv::VideoWriter vout;
    vout.open(videoName, CV_FOURCC('D','I','V','X'), 15, cv::Size(1080,144), true);

    while (ReadOneDsvlFrame () && isRunning)
    {
        dsvl_bg.ReadOneDsvlFrame();
        if (num%100==0)
            printf("%d (%d)\n",num,dFrmNum);

        num++;

        ProcessOneFrame ();

        cvResize(rm.lMap, col);

        //InteractiveSelect(col, &rm);
        cvShowImage("segmentation",col);

        //���ӻ�
        cvResize (rm.rMap, out);
        cvShowImage("range image",out);
//        cv::waitKey(0);


        SampleGenerator sampler(&rm);
        cv::setMouseCallback("segmentation", DsvlProcessor::MouseCallback, &sampler);
        sampler.GenerateAllSamplesInRangeImage(&rm, seglog, vout);
        std::cout<<"Generate Samples: "<<rm.millsec<<std::endl;

    }
    ReleaseRmap (&rm);
    cvReleaseImage(&out);
    cvReleaseImage(&col);
    delete []onefrm;
}

void DsvlProcessor::CheckStreamByPrid(Mat &img, int prid)
{
    for (int y = 0; y < rm.len; y++) {
        for (int x = 0; x < rm.wid; x++) {
            if (rm.regionID[y*rm.wid+x] == prid){
                std::cout<<rm.millsec<<std::endl;
                int px = x;
                int py = y;
                img.at<cv::Vec3b>(py,px) = cv::Vec3b(0,0,255);
            }
        }
    }
}

void DsvlProcessor::InitRmap(RMAP *rm)
{
    rm->wid = LINES_PER_BLK*BKNUM_PER_FRM;
    rm->len = PNTS_PER_LINE;
    rm->pts = new point3fi[rm->wid*rm->len];
    rm->regionID = new int[rm->wid*rm->len];
    rm->rMap = cvCreateImage(cvSize(rm->wid,rm->len), IPL_DEPTH_8U, 1);
    rm->lMap = cvCreateImage(cvSize(rm->wid,rm->len), IPL_DEPTH_8U, 3);
}

void DsvlProcessor::ReleaseRmap(RMAP *rm)
{
    delete []rm->pts;
    delete []rm->regionID;
    cvReleaseImage(&rm->rMap);
    cvReleaseImage(&rm->lMap);
}

void DsvlProcessor::MouseCallback(int event, int x, int y, int, void *userdata)
{
    SampleGenerator* psg = reinterpret_cast<SampleGenerator*>(userdata);
    psg->OnMouse(event, x, y);
}

void DsvlProcessor::setSeglog(SegLogLoader *value)
{
    seglog = value;
}
