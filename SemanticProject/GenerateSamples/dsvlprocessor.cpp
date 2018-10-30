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

    // Initialization
    cvZero(rm.rMap);    // range map
    cvZero(rm.lMap);    // label map
    memset (rm.pts, 0, sizeof (point3fi)*rm.wid*rm.len);
    memset (rm.regionID, 0, sizeof (int)*rm.wid*rm.len);
    rm.regnum = 0;
    rm.ang = onefrm->ang;
    rm.shv = onefrm->shv;
    rm.millsec = onefrm->dsv[0].millisec;

    // 遍历所有激光点(velodyne3)，计算对应pixel的range值等
    for (int i=0; i<BKNUM_PER_FRM; i++) {
        for (int j=0; j<LINES_PER_BLK; j++) {
            for (int k=0; k<PNTS_PER_LINE; k++) {
                p = &onefrm->dsv[i].points[j*PNTS_PER_LINE+k];
                if (!p->i)
                    continue;
                // range in 'meter'
                rng=sqrt(sqr(p->x)+sqr(p->y)+sqr(p->z));
                x=i*LINES_PER_BLK+j;
                y=k;
                rm.pts[y*rm.wid+x] = onefrm->dsv[i].points[j*PNTS_PER_LINE+k];
                // 距离系数归一化到0-255
                rm.rMap->imageData[y*rm.wid+x] = min(255,int(rng*3));
                // 像素对应的类别标签(最详细的标签类别)
                rm.regionID[y*rm.wid+x] = onefrm->dsv[i].lab[j*PNTS_PER_LINE+k];
                // 这句话好像没有用到
                rm.regnum = max (rm.regnum, rm.regionID[y*rm.wid+x]);

                unsigned int val;

                /*if (rm.regionID[y*rm.wid+x]==EDGEPT) {
                    rm.lMap->imageData[(y*rm.wid+x)*3+2]	= 128;
                    rm.lMap->imageData[(y*rm.wid+x)*3+1]	= 128;
                    rm.lMap->imageData[(y*rm.wid+x)*3+0]	= 128;
                }
                else */if (rm.regionID[y*rm.wid+x]==GROUND)
                {

                    rm.lMap->imageData[(y*rm.wid+x)*3+2]	= 117; //0;
                    rm.lMap->imageData[(y*rm.wid+x)*3+1]	= 149; //0;
                    rm.lMap->imageData[(y*rm.wid+x)*3+0]	= 208; //128;
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
                    // 随机赋值颜色，用于区分不同的分割块
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

void DsvlProcessor::Processing()
{
    dfp.seekg(0, std::ios_base::end);
    dFrmNum = dfp.tellg() / 180 / dsvlbytesiz;

    dfp.seekg(0, std::ios_base::beg);

    InitRmap (&rm);
    onefrm= new ONEDSVFRAME[1];

    IplImage * out = cvCreateImage (cvSize (WIDTH, HEIGHT),IPL_DEPTH_8U,1);
    IplImage * col = cvCreateImage (cvSize (VIS_WIDTH, VIS_HEIGHT),IPL_DEPTH_8U,3);

    int	num = 0;

    // gt.avi
    // Ground truth visualization
    cv::VideoWriter vout;
    vout.open(videoName, CV_FOURCC('D','I','V','X'), 15, cv::Size(VIS_WIDTH,VIS_HEIGHT), true);

    // seg.avi
    // Segmentation visualization
    cv::VideoWriter voutSeg;
    voutSeg.open(videoNameSeg, CV_FOURCC('D','I','V','X'), 15, cv::Size(VIS_WIDTH,VIS_HEIGHT), true);

    while (ReadOneDsvlFrame () && isRunning)
    {
        if (num%100==0) printf("%d (%d)\n",num,dFrmNum);
        num++;

        ProcessOneFrame ();

        cvResize(rm.lMap, col);
        voutSeg << cvarrToMat(col);
        //InteractiveSelect(col, &rm);
        cvShowImage("segmentation",col);

        cvResize (rm.rMap, out);
        cvShowImage("range image",out);
        cv::waitKey(1);

        /* Mei Jilin's visualization */
        //        cv::Mat visual = cvarrToMat(col, true);
        //        for (int i=0; i<seglog->seednum; i++){
        //            if (seglog->seeds[i].milli == rm.millsec){
        //                int x = seglog->seeds[i].ip.x / 2;
        //                int y = seglog->seeds[i].ip.y * FAC_HEIGHT;
        //                cv::circle(visual, cv::Point(x,y), 2, cv::Scalar(0,0,0), 2);
        //            }
        //        }

        /*code for check data*/
        //        cv::Mat rangeimg = cvarrToMat(rm.rMap, true);
        //        cv::Mat checkimg;
        //        cv::cvtColor(rangeimg, checkimg, CV_GRAY2BGR);
        //        CheckStreamByPrid(checkimg, 10208);
        //        cv::resize(checkimg, checkimg, cv::Size(rm.wid/2, rm.len*4.5));
        //        cv::imshow("check", checkimg);

        //cv::imshow("visual", visual);

        char WaitKey;
        WaitKey = cvWaitKey(1);
        if (WaitKey == 27) {
            isRunning = 0;
            break;
        }
        //else if (WaitKey == 't')
        {
            SampleGenerator sampler(&rm);
            cv::setMouseCallback("segmentation", DsvlProcessor::MouseCallback, &sampler);
            // 生成输入数据的样本
            sampler.GenerateAllSamplesInRangeImage(&rm, seglog, vout);
            std::cout<<"Generate Samples: "<<rm.millsec<<std::endl;
        }

    }
    vout.release();
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
