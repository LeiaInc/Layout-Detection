#include "include/feature.h"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;
using namespace std;

#define EPSILON 1e-10 // used for avoiding the division by zero

cv::Mat resizeImageForDFT(const cv::Mat &img, int max_img_dim) {
    // Performance of DFT calculation is better for some array size. 
    // It is fastest when array size is power of two. 
    // https://docs.opencv.org/4.5.0/de/dbc/tutorial_py_fourier_transform.html
    int new_rows{img.rows}, new_cols{img.cols};
    if(max_img_dim > 0) {
        if(img.rows > img.cols) {
            new_rows = max_img_dim;
            new_cols = int( img.cols * max_img_dim / img.rows );
        } else {
            new_cols = max_img_dim;
            new_rows = int( img.rows * max_img_dim / img.cols );            
        }
    }
    new_rows = cv::getOptimalDFTSize( new_rows );
    new_cols = cv::getOptimalDFTSize( new_cols );
    
    Mat output;
    cv::resize(img, output, cv::Size(new_cols, new_rows));

    return output;
}

cv::Mat1d crossCorrelation(const cv::Mat1f &img) {
    Mat cc;
    cv::dft(img, cc);    
    cv::mulSpectrums(cc, cc, cc, 0, true);
    dft(cc, cc, DFT_INVERSE + DFT_SCALE);

    cv::extractChannel(cc, cc, 0);

    // normalization of cc values
    //  1. subtract the mean ( cc = cc - mean(cc) )
    //  2. divide by maximum ( cc = cc / max(cc) )
    //  or these two steps can be formulated by this eq.
    //  cc = (cc - mean(cc)) / (max(cc) - mean(cc))
    double mu = cv::mean(cc).val[0];
    double min, max;
    cv::minMaxLoc(cc, &min, &max);
    double scale_rcp = max - mu;
    if(fabs(scale_rcp) > EPSILON) {
        cc.convertTo(cc, CV_64F, 1.0/scale_rcp, -mu/scale_rcp);
    } else {
        cc.convertTo(cc, CV_64F, 1.0, -mu);
    }

    return cc;
}

double horizontalEdge(const cv::Mat &img) {
    if (img.empty()) {
        throw std::invalid_argument("The input image is empty.");
    }
    
    int mid = img.rows / 2;
    int ksize = 3;
    Mat strip = img(Range(mid - ksize, mid + ksize + 1), Range::all());

    Mat grad, abs_grad;
    Sobel(strip, grad, CV_16S, 0, 1, ksize, 1.0/128, 0, BORDER_REPLICATE);
    convertScaleAbs(grad, abs_grad);

    cv::Scalar mean = cv::mean(abs_grad);

    return mean[0];
}

double verticalEdge(const cv::Mat &img) {
    if (img.empty()) {
        throw std::invalid_argument("The input image is empty.");
    }

    int mid = img.cols / 2;
    int ksize = 3;
    Mat strip = img(Range::all(), Range(mid - ksize, mid + ksize + 1));

    Mat grad, abs_grad;
    Sobel(strip, grad, CV_16S, 1, 0, ksize, 1.0/128, 0, BORDER_REPLICATE);
    convertScaleAbs(grad, abs_grad);

    cv::Scalar mean = cv::mean(abs_grad);

    return mean[0];
}

void featureExtraction(const cv::Mat &img, int max_img_dim, float feat[NUM_FEATURES]) {
    if (img.empty()) {
        throw std::invalid_argument("The input image is empty.");
    }

    if (img.channels() != 1) {
        throw std::invalid_argument("The number of channels of the image must be 1 (Grayscale).");
    }    
    
    // DFT is fastest when array size is power of two. 
    Mat img_resize = resizeImageForDFT(img, max_img_dim);    

    // DFT accepts only floating-point array as input.
    Mat img_float;
    img_resize.convertTo(img_float, CV_32F);     

    // cross-correlation
    Mat cc = crossCorrelation(img_float);
   
    // extract values at the four corners of the left-top quadrant of the cc
    int cx = cc.cols / 2;
    int cy = cc.rows / 2;
    feat[0] = cc.at<double>(0,0);
    feat[1] = cc.at<double>(0,cx);
    feat[2] = cc.at<double>(cy,0);
    feat[3] = cc.at<double>(cy,cx);

    // add the image's aspect ratio
    feat[4] = double(img.rows) / img.cols;  

    feat[5] = horizontalEdge(img);
    feat[6] = verticalEdge(img);
}

