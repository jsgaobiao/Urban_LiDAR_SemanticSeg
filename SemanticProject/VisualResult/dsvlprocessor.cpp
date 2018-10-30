#include "dsvlprocessor.h"
#include "samplegenerator.h"
DsvlProcessor::DsvlProcessor(string filename)
{
   dsbytesiz = sizeof (point3d)*2 + sizeof (ONEVDNDATA);
   labbytesiz = sizeof(int)*LINES_PER_BLK*PNTS_PER_LINE;
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
                rm.rMap->imageData[y*rm.wid+x] = min(255,int(rng*10));
                rm.regionID[y*rm.wid+x] = onefrm->dsv[i].lab[j*PNTS_PER_LINE+k];
                rm.regnum = max (rm.regnum, rm.regionID[y*rm.wid+x]);

                unsigned int val;
                if (rm.regionID[y*rm.wid+x]==EDGEPT) {
                    rm.lMap->imageData[(y*rm.wid+x)*3+2]	= 128;
                    rm.lMap->imageData[(y*rm.wid+x)*3+1]	= 128;
                    rm.lMap->imageData[(y*rm.wid+x)*3+0]	= 128;
                }
                else if (rm.regionID[y*rm.wid+x]==GROUND)
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
    onefrm->ang.x = onefrm->ang.y = onefrm->ang.z = 0;
    onefrm->shv.x = onefrm->shv.y = onefrm->shv.z = 0;

    for (i=0; i<BKNUM_PER_FRM; i++) {
        dfp.read((char *)&onefrm->dsv[i], dsbytesiz);
        if (dfp.gcount() != dsbytesiz)
            break;
        dfp.read((char *)onefrm->dsv[i].lab, labbytesiz);
        if (dfp.gcount() != labbytesiz)
            break;

        onefrm->ang.x += onefrm->dsv[i].ang.x;
        onefrm->ang.y += onefrm->dsv[i].ang.y;
        onefrm->ang.z += onefrm->dsv[i].ang.z;
        onefrm->shv.x += onefrm->dsv[i].shv.x;
        onefrm->shv.y += onefrm->dsv[i].shv.y;
        onefrm->shv.z += onefrm->dsv[i].shv.z;
    }

    //ÿһ֡��һ��ƽ����λ�ˣ��������ݹ���
    onefrm->ang.x /= BKNUM_PER_FRM;
    onefrm->ang.y /= BKNUM_PER_FRM;
    onefrm->ang.z /= BKNUM_PER_FRM;
    onefrm->shv.x /= BKNUM_PER_FRM;
    onefrm->shv.y /= BKNUM_PER_FRM;
    onefrm->shv.z /= BKNUM_PER_FRM;

    if (i<BKNUM_PER_FRM)
        return 0;
    else
        return 1;
}

void DsvlProcessor::Processing()
{
    dfp.seekg(0, std::ios_base::end);
    dFrmNum = dfp.tellg() / 180 / dsbytesiz;

    dfp.seekg(0, std::ios_base::beg);

    InitRmap (&rm);
    onefrm= new ONEDSVFRAME[1];

    IplImage * out = cvCreateImage (cvSize (rm.wid/2, rm.len*4.5),IPL_DEPTH_8U,1);
    IplImage * col = cvCreateImage (cvSize (rm.wid/2, rm.len*4.5),IPL_DEPTH_8U,3);

    int	num = 0;

    while (ReadOneDsvlFrame () && isRunning)
    {
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

        char WaitKey;
        WaitKey = cvWaitKey();
        if (WaitKey == 27) {
            isRunning = 0;
            break;
        }
        else if(WaitKey == 'q'){
            SampleGenerator sampler(&rm);
            sampler.GeneratorSamples(&rm);
        }
    }
    ReleaseRmap (&rm);
    cvReleaseImage(&out);
    cvReleaseImage(&col);
    delete []onefrm;
}

std::string classnames[]={"people", "people_2_3", "people_3", "car", "bus", "tree_bar", "bush",
                           "sign1", "sign2", "sign3","pillar", "trash_can", "building", "unknown", "cyclist", "cone"};

void DsvlProcessor::VisualClassifyResult(std::map<int, std::vector<CLASSIFYINFO> > &mapper)
{
    dfp.seekg(0, std::ios_base::end);
    dFrmNum = dfp.tellg() / 180 / dsbytesiz;

    dfp.seekg(0, std::ios_base::beg);

    InitRmap (&rm);
    onefrm= new ONEDSVFRAME[1];

    IplImage * out = cvCreateImage (cvSize (rm.wid/2, rm.len*4.5),IPL_DEPTH_8U,1);
    IplImage * col = cvCreateImage (cvSize (rm.wid/2, rm.len*4.5),IPL_DEPTH_8U,3);

    int	num = 0;

    cv::VideoWriter vwriter;

    while (ReadOneDsvlFrame () && isRunning)
    {
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

        //set mouseclick response
        SampleGenerator sampler(&rm);
        cv::setMouseCallback("segmentation", DsvlProcessor::MouseCallback, &sampler);

        char WaitKey;
        WaitKey = cvWaitKey(1);
        if (WaitKey == 27) {
            isRunning = 0;
            break;
        }

        std::map<int, std::vector<CLASSIFYINFO> >::iterator iter;
        iter = mapper.find(rm.millsec);
        if (iter != mapper.end()){
            cv::Mat dataimg = cv::Mat::zeros(rm.len, rm.wid, CV_8UC3);
            for (int y = 0; y < rm.len; y++) {
                for (int x = 0; x < rm.wid; x++) {
                    if (!rm.pts[y*rm.wid + x].i)
                        continue;
                    std::vector<CLASSIFYINFO> vec;
                    int regionid = rm.regionID[y*rm.wid+x];
                    vec = iter->second;
//                    if ((rm.millsec==27514266)&&(regionid==717)){
//                        std::cout<<regionid;
//                    }
                    for (int i=0; i<vec.size(); i++){
                        int label = vec[i].label;
                        if ((regionid == vec[i].regionid)&&(vec[i].scores[label-1]>=0.8))
                        //if ((regionid == vec[i].regionid) && (label!=7))
                        {
                            int val = label % COLORNUM;
                            dataimg.at<cv::Vec3b>(y, x) = cv::Vec3b(LEGENDCOLORS[val]);
                            break;
                        }
                        else{
                            dataimg.at<cv::Vec3b>(y, x) = cv::Vec3b(128,128,128);
                        }
                    }
                }
            }
            cv::Mat visualmat;
            cv::resize(dataimg, visualmat, cv::Size(rm.wid/2, rm.len*4.5));

            cv::Mat range_mat;
            cv::Mat seg_mat = cv::cvarrToMat(col, true);
            cv::cvtColor(cv::cvarrToMat(out,true), range_mat, CV_GRAY2BGR);

            int width = visualmat.cols;
            int height = visualmat.rows;

            int bar_width=60, bar_height=30;
            cv::Mat bar = cv::Mat::zeros(bar_height,visualmat.cols,CV_8UC3);
            for (int i=1; i<=classes_num; i++){
                cv::Mat color_bar = cv::Mat::zeros(bar_height, bar_width, CV_8UC3);
                color_bar.setTo(cv::Vec3b(LEGENDCOLORS[i]));
                int col = (i-1)*bar_width+bar_width+10;
                color_bar.copyTo(bar(cv::Rect(col,0,bar_width,bar_height)));
                cv::putText(bar, classnames[i-1], cv::Point(col,bar_height/2), CV_FONT_HERSHEY_PLAIN, 1,
                        cv::Scalar(0,0,0),1);
            }

            cv::Mat final_mat = cv::Mat::zeros(bar_height + height*3+40, width, CV_8UC3);

            bar.copyTo(final_mat(cv::Rect(0,0,bar.cols,bar.rows)));
            visualmat.copyTo(final_mat(cv::Rect(0,bar_height,width,height)));
            range_mat.copyTo(final_mat(cv::Rect(0,bar_height+height+10,width,height)));
            seg_mat.copyTo(final_mat(cv::Rect(0,bar_height+height*2,width,height)));

            if (!vwriter.isOpened()){
                vwriter.open("log0313.avi", CV_FOURCC('x','v','i','d'), 5, cv::Size(final_mat.cols, final_mat.rows));
            }
            vwriter<<final_mat;

            cv::imshow("class", final_mat);
            cv::waitKey(1);
        }
    }
    ReleaseRmap (&rm);
    cvReleaseImage(&out);
    cvReleaseImage(&col);
    delete []onefrm;
}

void DsvlProcessor::VisualClassifyResult(std::map<int, std::vector<CLASSIFYINFO> > &mapper, std::map<int, std::vector<CLASSIFYINFO> > &pair_mapper)
{
    dfp.seekg(0, std::ios_base::end);
    dFrmNum = dfp.tellg() / 180 / dsbytesiz;

    dfp.seekg(0, std::ios_base::beg);

    InitRmap (&rm);
    onefrm= new ONEDSVFRAME[1];

    IplImage * out = cvCreateImage (cvSize (rm.wid/2, rm.len*4.5),IPL_DEPTH_8U,1);
    IplImage * col = cvCreateImage (cvSize (rm.wid/2, rm.len*4.5),IPL_DEPTH_8U,3);

    int	num = 0;

    cv::VideoWriter vwriter;

    while (ReadOneDsvlFrame () && isRunning)
    {
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

        //set mouseclick response
        SampleGenerator sampler(&rm);
        cv::setMouseCallback("segmentation", DsvlProcessor::MouseCallback, &sampler);

        char WaitKey;
        WaitKey = cvWaitKey(1);
        if (WaitKey == 27) {
            isRunning = 0;
            break;
        }

        std::map<int, std::vector<CLASSIFYINFO> >::iterator iter, pair_iter;
        iter = mapper.find(rm.millsec);
        pair_iter = pair_mapper.find(rm.millsec);

        if ((iter != mapper.end()) && (pair_iter != pair_mapper.end())){
            cv::Mat range_mat;
            cv::Mat seg_mat = cv::cvarrToMat(col, true);
            cv::cvtColor(cv::cvarrToMat(out,true), range_mat, CV_GRAY2BGR);

            cv::Mat dataimg = cv::Mat::zeros(rm.len, rm.wid, CV_8UC3);
            cv::Mat pair_dataimg = cv::Mat::zeros(rm.len, rm.wid, CV_8UC3);

            for (int y = 0; y < rm.len; y++) {
                for (int x = 0; x < rm.wid; x++) {
                    if (!rm.pts[y*rm.wid + x].i)
                        continue;
                    std::vector<CLASSIFYINFO> vec;
                    int regionid = rm.regionID[y*rm.wid+x];

                    if (regionid == GROUND){
                        dataimg.at<cv::Vec3b>(y, x) = cv::Vec3b(0,0,0);
                        pair_dataimg.at<cv::Vec3b>(y, x) = cv::Vec3b(0,0,0);
                        continue;
                    }
                    vec = iter->second;
                    for (int i=0; i<vec.size(); i++){
                        int label = vec[i].label;
                        if ((regionid == vec[i].regionid))
                        {
                            int val = label % COLORNUM;
                            dataimg.at<cv::Vec3b>(y, x) = cv::Vec3b(LEGENDCOLORS[val]);
                            val = vec[i].truth % COLORNUM;
                            range_mat.at<cv::Vec3b>(y*4.5, x/2) = cv::Vec3b(LEGENDCOLORS[val]);
                            break;
                        }
                        else{
                            dataimg.at<cv::Vec3b>(y, x) = cv::Vec3b(128,128,128);
                        }
                    }

                    vec.clear();
                    vec = pair_iter->second;
                    for (int i=0; i<vec.size(); i++){
                        int label = vec[i].label;
                        if ((regionid == vec[i].regionid))
                        {
                            int val = label % COLORNUM;
                            pair_dataimg.at<cv::Vec3b>(y, x) = cv::Vec3b(LEGENDCOLORS[val]);
                            break;
                        }
                        else{
                            pair_dataimg.at<cv::Vec3b>(y, x) = cv::Vec3b(128,128,128);
                        }
                    }
                }
            }

            cv::Mat visualmat, pair_visualmat;
            cv::resize(dataimg, visualmat, cv::Size(rm.wid/2, rm.len*4.5));
            cv::putText(visualmat, "No Pair-Constraint", cv::Point(20,20), CV_FONT_HERSHEY_PLAIN,
                        2, cv::Scalar(0,255,0), 2);
            cv::resize(pair_dataimg, pair_visualmat, cv::Size(rm.wid/2, rm.len*4.5));
            cv::putText(pair_visualmat, "Pair-Constraint", cv::Point(20,20), CV_FONT_HERSHEY_PLAIN,
                        2, cv::Scalar(0,255,0), 2);
            cv::putText(range_mat, "Input", cv::Point(20,20), CV_FONT_HERSHEY_PLAIN,
                        2, cv::Scalar(0,255,0), 2);
            cv::putText(seg_mat, "Segmentation", cv::Point(20,20), CV_FONT_HERSHEY_PLAIN,
                        2, cv::Scalar(0,255,0), 2);

            int width = visualmat.cols;
            int height = visualmat.rows;

            int bar_width=60, bar_height=30;
            cv::Mat bar = cv::Mat::zeros(bar_height,visualmat.cols,CV_8UC3);
            for (int i=1; i<=classes_num; i++){
                cv::Mat color_bar = cv::Mat::zeros(bar_height, bar_width, CV_8UC3);
                color_bar.setTo(cv::Vec3b(LEGENDCOLORS[i]));
                int row = (i-1)*bar_width+bar_width;
                color_bar.copyTo(bar(cv::Rect(row,0,bar_width,bar_height)));
                cv::putText(bar, classnames[i-1], cv::Point(row,bar_height/2), CV_FONT_HERSHEY_PLAIN, 1,
                        cv::Scalar(0,0,0),1);
            }

            cv::Mat final_mat = cv::Mat::zeros(bar_height + height*4+40, width, CV_8UC3);

            bar.copyTo(final_mat(cv::Rect(0,0,bar.cols,bar.rows)));
            visualmat.copyTo(final_mat(cv::Rect(0,bar_height,width,height)));
            pair_visualmat.copyTo(final_mat(cv::Rect(0,bar_height+height+10,width,height)));
            seg_mat.copyTo(final_mat(cv::Rect(0,bar_height+height*2,width,height)));
            range_mat.copyTo(final_mat(cv::Rect(0,bar_height+height*3,width,height)));

            if (!vwriter.isOpened()){
                vwriter.open("log0313.avi", CV_FOURCC('x','v','i','d'), 5, cv::Size(final_mat.cols, final_mat.rows));
            }
            vwriter<<final_mat;

            cv::imshow("class", final_mat);
            cv::waitKey(1);
        }
    }
    ReleaseRmap (&rm);
    cvReleaseImage(&out);
    cvReleaseImage(&col);
    delete []onefrm;}

void DsvlProcessor::VisualNegSampleResult(std::map<int, std::vector<CLASSIFYINFO> > &mapper)
{
    dfp.seekg(0, std::ios_base::end);
    dFrmNum = dfp.tellg() / 180 / dsbytesiz;

    dfp.seekg(0, std::ios_base::beg);

    InitRmap (&rm);
    onefrm= new ONEDSVFRAME[1];

    IplImage * out = cvCreateImage (cvSize (rm.wid/2, rm.len*4.5),IPL_DEPTH_8U,1);
    IplImage * col = cvCreateImage (cvSize (rm.wid/2, rm.len*4.5),IPL_DEPTH_8U,3);

    int	num = 0;

    while (ReadOneDsvlFrame () && isRunning)
    {
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

        //set mouseclick response
        SampleGenerator sampler(&rm);
        cv::setMouseCallback("segmentation", DsvlProcessor::MouseCallback, &sampler);

        char WaitKey;
        WaitKey = cvWaitKey(1);
        if (WaitKey == 27) {
            isRunning = 0;
            break;
        }

        std::map<int, std::vector<CLASSIFYINFO> >::iterator iter;
        iter = mapper.find(rm.millsec);
        if (iter != mapper.end()){
            cv::Mat dataimg = cv::Mat::zeros(rm.len, rm.wid, CV_8UC3);
            for (int y = 0; y < rm.len; y++) {
                for (int x = 0; x < rm.wid; x++) {
                    if (!rm.pts[y*rm.wid + x].i)
                        continue;
                    std::vector<CLASSIFYINFO> vec;
                    int regionid = rm.regionID[y*rm.wid+x];
                    vec = iter->second;
//                    if ((rm.millsec==27514266)&&(regionid==717)){
//                        std::cout<<regionid;
//                    }
                    for (int i=0; i<vec.size(); i++){
                        int label = vec[i].label;
                        //if ((regionid == vec[i].regionid)&&(vec[i].scores[label-1]>=0.9))
                        if (regionid == vec[i].regionid)
                        {
                            if (label>0)
                                dataimg.at<cv::Vec3b>(y, x) = cv::Vec3b(LEGENDCOLORS[1]);
                            else
                                dataimg.at<cv::Vec3b>(y, x) = cv::Vec3b(LEGENDCOLORS[2]);
                            break;
                        }
                        else{
                            dataimg.at<cv::Vec3b>(y, x) = cv::Vec3b(128,128,128);
                        }
                    }
                }
            }
            cv::Mat visualmat;
            cv::resize(dataimg, visualmat, cv::Size(rm.wid/2, rm.len*4.5));
            cv::imshow("blue-negsample", visualmat);
            cv::waitKey();
        }
    }
    ReleaseRmap (&rm);
    cvReleaseImage(&out);
    cvReleaseImage(&col);
    delete []onefrm;
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
