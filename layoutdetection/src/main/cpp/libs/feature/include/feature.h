#ifndef LAYOUTDETECTION_FEATURE_H_
#define LAYOUTDETECTION_FEATURE_H_

#include <vector>
#include <memory>
#include <opencv2/core.hpp>

#define NUM_FEATURES 7

void featureExtraction(const cv::Mat &img, int max_img_dim, float feat[NUM_FEATURES]);
cv::Mat resizeImageForDFT(const cv::Mat &img, int max_img_dim);
cv::Mat1d crossCorrelation(const cv::Mat1f &img);
double horizontalEdge(const cv::Mat &img);
double verticalEdge(const cv::Mat &img);

#endif  // LAYOUTDETECTION_FEATURE_H_
