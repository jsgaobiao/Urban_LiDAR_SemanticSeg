#include "samplegenerator.h"
#include <unordered_set>
#include <algorithm>
#include <QDir>
SampleGenerator::SampleGenerator(RMAP *prm_)
{
    prm = prm_;
    dataimg = cv::Mat::zeros(prm->len * 4.5, prm->wid / 2, CV_8UC3);
}

void SampleGenerator::ExtractSampleById(IDTYPE &rid, cv::Mat &outsample)
{
    LABELLEDPONTS onelabelpt;
    std::vector<LABELLEDPONTS> labelpoints;
    cv::Mat mask_mat = cv::Mat::zeros(prm->len, prm->wid, CV_8UC1);

    int count = 0;
    labelpoints.clear();

    int regionid = rid.id;

    for (int y = 0; y < prm->len; y++) {
        for (int x = 0; x < prm->wid; x++) {
            if (!prm->pts[y*prm->wid + x].i)
                continue;

            if (regionid != prm->regionID[y*prm->wid + x])
                continue;

            int px = x / 2;
            int py = y * 4.5;

            if (px < 0 || px >= dataimg.cols || py < 0 || py >= dataimg.rows)
                continue;

            count++;

            //����ԭʼ��Ϣ��������������
            onelabelpt.img_x = px;
            onelabelpt.img_y = py;
            onelabelpt.intensity = prm->pts[y*prm->wid + x].i;
            onelabelpt.loc = cv::Point3f(prm->pts[y*prm->wid + x].x, prm->pts[y*prm->wid + x].y, prm->pts[y*prm->wid + x].z);
            labelpoints.push_back(onelabelpt);

            mask_mat.at<uchar>(y, x) = 255;

            //���ӻ�
            int val = prm->regionID[y*prm->wid + x] % COLORNUM;
            dataimg.at<cv::Vec3b>(py, px) = cv::Vec3b(LEGENDCOLORS[val]);
        }
    }

    rid.point_num = count;

    if (count == 0)
        return ;

    double mean_x, mean_y, mean_z;
    mean_x = mean_y = mean_z = 0.0;

    int mean_ix, mean_iy;
    mean_ix = mean_iy = 0;

    if (labelpoints.size()) {

        cv::Rect bbox;
        bool ret = FindMaxContourCenter(mask_mat, mean_ix, mean_iy, bbox);

        if (!ret) {
            return;
        }

        ExtractPointsInBbox(bbox, labelpoints, mean_x, mean_y, mean_z);
    }

    if (mean_ix > 0 || mean_iy > 0) {//��ͼ������ϵ�а������ĵ㣬ȡ�̶��ߴ�������

        rid.center.x = mean_ix;
        rid.center.y = mean_iy;

        ExtractSampleByCenter(mean_ix, mean_iy, labelpoints, outsample);
    }
}

bool SampleGenerator::FindMaxContourCenter(const cv::Mat &mask_mat, int &center_ix, int &center_iy, cv::Rect &bbox)
{
    //��������ͨ����,ȷ������λ��
    std::vector<std::vector<cv::Point> > contours;
    //��ȡ���������������ڵ�����
    cv::findContours(mask_mat, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

    if (!contours.size())
        return 0;

    /// Draw contours
    cv::Mat result = cv::Mat::zeros(mask_mat.size(), CV_8UC3);
    int max_contour_id = 0;
    int contour_size = -9;
    for (int i = 0; i< contours.size(); i++)
    {
        int size = contours[i].size();
        if (size > contour_size) {
            contour_size = size;
            max_contour_id = i;
        }
    }

    double tx = 0, ty = 0, tz = 0;
    for (int i = 0; i < contours[max_contour_id].size(); i++) {
        tx += contours[max_contour_id][i].x;
        ty += contours[max_contour_id][i].y;
    }
    //����ͼ������ϵ�����ĵ���λ��
    int ix = tx / double(contours[max_contour_id].size());
    int iy = ty / double(contours[max_contour_id].size());

    center_ix = ix / 2;
    center_iy = iy * 4.5;
    bbox = cv::boundingRect(cv::Mat(contours[max_contour_id]));
    return 1;

    //���ӻ�
    /*
    cv::Mat drawing;
    result = cv::Mat::zeros(mask_mat.size(), CV_8UC3);
    drawContours(result, contours, max_contour_id, cv::Scalar(255, 0, 0));
    cv::resize(result, drawing, cv::Size(result.cols / 2, result.rows*4.5));
    cv::circle(drawing, cv::Point(center_ix, center_iy), 3, cv::Scalar(0, 0, 255), 2);
    cv::imshow("resultImage", drawing);
    */

}

void SampleGenerator::ExtractPointsInBbox(const cv::Rect bbox, std::vector<LABELLEDPONTS> &points, double &center_x, double &center_y, double &center_z)
{
    double tx, ty, tz;

    cv::Point left_top = bbox.tl();
    cv::Point right_bottom = bbox.br();

    left_top.x = left_top.x / 2;
    left_top.y = left_top.y*4.5;
    right_bottom.x = right_bottom.x / 2;
    right_bottom.y = right_bottom.y*4.5;

    tx = ty = tz = 0.0;

    //��������contourȷ����bbox�У�������������ϵ�µ�����λ��
    long count = 0;
    for (int i = 0; i < points.size(); i++) {
        if (points[i].img_x <= left_top.x || points[i].img_x >= right_bottom.x
            || points[i].img_y <= left_top.y || points[i].img_y >= right_bottom.y) {
            continue;
        }
        tx += points[i].loc.x;
        ty += points[i].loc.y;
        tz += points[i].loc.z;
        count++;
    }

    if (count) {
        center_x = tx / double(count);
        center_y = ty / double(count);
        center_z = tz / double(count);

        //����������ϵ��ȡ3ά��BBOX���ߴ��̶�
        cv::Point3f minBoxPt = cv::Point3f(center_x - 2.5, center_y - 2.5, center_z - 1.2);
        cv::Point3f maxBoxPt = cv::Point3f(center_x + 2.5, center_y + 2.5, center_z + 1.2);

        points.clear();
        LABELLEDPONTS onelabelpt;

        //����������ϵ�£����»�ȡ������
        for (int y = 0; y < prm->len; y++) {
            for (int x = 0; x < prm->wid; x++) {
                if (!prm->pts[y*prm->wid + x].i)
                    continue;

                int px = x / 2;
                int py = y * 4.5;

                onelabelpt.img_x = px;
                onelabelpt.img_y = py;
                onelabelpt.intensity = prm->pts[y*prm->wid + x].i;
                onelabelpt.loc = cv::Point3f(prm->pts[y*prm->wid + x].x, prm->pts[y*prm->wid + x].y, prm->pts[y*prm->wid + x].z);
                if (IsInBbox(onelabelpt, minBoxPt, maxBoxPt)) {
                    points.push_back(onelabelpt);
                }
            }
        }
    }
}

void SampleGenerator::ExtractSampleByCenter(int center_x, int center_y, std::vector<LABELLEDPONTS> &labelpoints, cv::Mat &outsample)
{
    cv::circle(dataimg, cv::Point(center_x, center_y), 2, cv::Scalar(255, 0, 0), 2);
    int width = 256;
    int height = 256;

    cv::Mat range_mat = cv::Mat::zeros(height, width, CV_8UC1);
    cv::Mat height_mat = cv::Mat::zeros(height, width, CV_8UC1);
    cv::Mat intensity_mat = cv::Mat::zeros(height, width, CV_8UC1);

    cv::Point left_top = cv::Point(center_x - width / 2, center_y - height / 2);
    cv::Point right_bottom = cv::Point(center_x + width / 2, center_y + height / 2);
    //��ȡ����
    std::vector<double> range_feat;
    double min_r = 999999;
    double max_r = -min_r;
    double min_z = min_r;
    double max_z = max_r;
    int count = 0;
    for (int i = 0; i < labelpoints.size(); i++) {
        if (labelpoints[i].img_x <= left_top.x || labelpoints[i].img_x >= right_bottom.x
            || labelpoints[i].img_y <= left_top.y || labelpoints[i].img_y >= right_bottom.y) {
            continue;
        }
        count++;
        double onerange = std::sqrt(sqr(labelpoints[i].loc.x) + sqr(labelpoints[i].loc.y) + sqr(labelpoints[i].loc.z));
        range_feat.push_back(onerange);
        min_r = std::min<double>(onerange, min_r);
        max_r = std::max<double>(onerange, max_r);
        min_z = std::min<double>(labelpoints[i].loc.z, min_z);
        max_z = std::max<double>(labelpoints[i].loc.z, max_z);
    }

    if (count > 0) {
        count = 0;
        for (int i = 0; i < labelpoints.size(); i++) {
            if (labelpoints[i].img_x <= left_top.x || labelpoints[i].img_x >= right_bottom.x
                || labelpoints[i].img_y <= left_top.y || labelpoints[i].img_y >= right_bottom.y) {
                continue;
            }
            int ix = labelpoints[i].img_x - center_x + width / 2;
            int iy = labelpoints[i].img_y - center_y + height / 2;

            range_mat.at<uchar>(iy, ix) = (range_feat[count] - min_r) / (max_r - min_r) * 255; //����
            int height = (labelpoints[i].loc.z + 3.0) * 60;
            //height_mat.at<uchar>(iy, ix) = (labelpoints[i].loc.z - min_z) / (max_z - min_z) * 255; //�߶�
            height_mat.at<uchar>(iy, ix) = std::min<int>(height, 255); //�߶�
            intensity_mat.at<uchar>(iy, ix) = labelpoints[i].intensity; //intensity

            count++;
        }
        cv::Mat container[3] = { range_mat, height_mat, intensity_mat };

        cv::merge(container, 3, outsample);

        cv::imshow("ColorSample", outsample);
        cv::imshow("range_mat", range_mat);
        cv::imshow("height_mat", height_mat);
        cv::imshow("intensity_mat", intensity_mat);
    }
}

bool SampleGenerator::IsInBbox(LABELLEDPONTS pt, cv::Point3f minBoxPt, cv::Point3f maxBoxPt)
{
    if (pt.loc.x>minBoxPt.x && pt.loc.x<maxBoxPt.x)
        if (pt.loc.y>minBoxPt.y && pt.loc.y<maxBoxPt.y)
            if (pt.loc.z > minBoxPt.z && pt.loc.z < maxBoxPt.z) {
                return 1;
            }
    return 0;
}

void SampleGenerator::setRangeMapPointer(RMAP *value)
{
    prm = value;
}

void SampleGenerator::GeneratorSamples(RMAP *prm_)
{
    prm = prm_;
    int count = 0;
    std::vector<cv::Mat> samples;

    //determine all region id, prm->regnum is not enough
    std::unordered_set<int> idx_set;
    for (int y = 0; y < prm->len; y++) {
        for (int x = 0; x < prm->wid; x++) {
            if (!prm->pts[y*prm->wid + x].i)
                continue;
            if (prm->regionID[y*prm->wid+x] > 0){
                idx_set.insert(prm->regionID[y*prm->wid+x]);
            }
        }
    }

    bool onesample = 0;
    std::unordered_set<int>::iterator iter;
    for (iter=idx_set.begin(); iter!=idx_set.end(); iter++){
        int regionid = *iter;
        count = 0;
        onesample = 0;
        for (int y = 0; y < prm->len; y++) {
            if (onesample)
                break;
            for (int x = 0; x < prm->wid; x++) {
                if (!prm->pts[y*prm->wid + x].i)
                    continue;
                if (prm->regionID[y*prm->wid+x] == regionid){
                    count += 1;
                }
                if (count > 20){
                    IDTYPE rid(regionid, 0);
                    cv::Mat outsample;
                    ExtractSampleById(rid, outsample);
                    if (outsample.data){
                        std::string imgname = "/home/pku-m/SemanticMap/Tools/SemanticProject/GenerateInput/campus1_test/";
                        QDir dir(QString(imgname.c_str()));
                        if (!dir.exists()){
                            if (!dir.mkpath(dir.absolutePath()))
                                return ;
                        }
                        QString foldername =QString("%1%2").arg(imgname.c_str()).arg(prm->millsec);

                        dir.mkdir(foldername);

                        imgname = foldername.toStdString() + "/" + std::to_string(prm->millsec) + "_" + std::to_string(regionid);
                        imgname += ".png";
                        cv::imwrite(imgname, outsample);
                        samples.push_back(outsample);
                    }
                    onesample = 1;
                    break;
                }
            }
        }
    }

    return ;
}
