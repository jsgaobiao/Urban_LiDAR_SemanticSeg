#ifndef SAMPLEGENERATOR_H
#define SAMPLEGENERATOR_H
#include "types.h"

class SampleGenerator
{
public:
    SampleGenerator(RMAP *prm_);
    void ExtractSampleById(IDTYPE& rid, cv::Mat& outsample);

    bool FindMaxContourCenter(const cv::Mat& mask_mat, int& center_ix, int& center_iy, cv::Rect& bbox);

    void ExtractPointsInBbox(const cv::Rect bbox, std::vector<LABELLEDPONTS>& points,
                             double& center_x, double& center_y, double& center_z);

    void ExtractSampleByCenter(int center_x, int center_y,
                               std::vector<LABELLEDPONTS>& labelpoints, cv::Mat& outsample);

    bool IsInBbox(LABELLEDPONTS pt, cv::Point3f minBoxPt, cv::Point3f maxBoxPt);

    void setRangeMapPointer(RMAP *value);

    void OnMouse(int event, int x, int y);

    std::vector<cv::Mat>& GeneratorSamples(RMAP *prm_);

private:
    RMAP *prm;
    cv::Mat dataimg;
};

#endif // SAMPLEGENERATOR_H
