#ifndef SAMPLEGENERATOR_H
#define SAMPLEGENERATOR_H
#include "types.h"
#include "seglogloader.h"
class SampleGenerator
{
public:
    SampleGenerator(RMAP *prm_);
    void ExtractSampleById(IDTYPE& rid, cv::Mat& outsample);

    bool FindMaxContourCenter(const cv::Mat& mask_mat, int& center_ix, int& center_iy, cv::Rect& bbox);

    bool IsInBbox(LABELLEDPONTS pt, cv::Point3f minBoxPt, cv::Point3f maxBoxPt);

    void setRangeMapPointer(RMAP *value);

    void OnMouse(int event, int x, int y);

    void GenerateAllSamplesInRangeImage(RMAP *prm_, SegLogLoader *seglog, cv::VideoWriter &out);

    void ExtractSampleByCenter(int center_x, int center_y, std::vector<LABELLEDPONTS> &labelpoints, cv::Mat &outsample);
    void ExtractPointsInBbox(const cv::Rect bbox, std::vector<LABELLEDPONTS> &points, double &center_x, double &center_y, double &center_z);
private:
    RMAP *prm;
    cv::Mat dataimg;
};

#endif // SAMPLEGENERATOR_H
