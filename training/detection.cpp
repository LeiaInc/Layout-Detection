#include <iostream>
#include <chrono>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <memory>
#include <cxxopts.hpp>
#include <stdlib.h>

#include "feature.h"
#include "svm_util.h"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>

using namespace std;
using namespace cv;
namespace fs = std::filesystem;

int detect(string video_fpath, string model_fpath, int max_img_dim, string output_dir, int save) {
    cv::VideoCapture cap;
    Mat frame, gray;

    if(!cap.open(video_fpath)) {    
        cout << "Failed to open " <<  video_fpath << endl;
        return 0;
    }

    // select random frame index
    int length = cap.get(cv::CAP_PROP_FRAME_COUNT);
    srand (time(NULL));
    int frame_idx = rand() % length;
    cap.set(cv::CAP_PROP_POS_FRAMES, frame_idx);

    cap >> frame;
    if(frame.empty()) {
        cout << "Video frame is empty." << endl;
        return 0;
    }
    cap.release();

    // Grayscale
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    

    // SVM model loading
    cout << "\nLoad SVM model from file " << model_fpath << endl;
    std::ifstream ifs(model_fpath);
    std::string content;
    content.assign( (std::istreambuf_iterator<char>(ifs) ),
                    (std::istreambuf_iterator<char>()    ) );
    svm_model* model = loadSVMFromString(content);

    // Detection
    float feat[NUM_FEATURES];
    featureExtraction(gray, max_img_dim, feat);

    vector<double> prob;
    int y_pred;
    svmPrediction(model, feat, NUM_FEATURES, y_pred, prob);   

    svm_free_and_destroy_model(&model);

    if(save == 1) {
        fs::path out_path(output_dir);
        fs::create_directory(out_path);
        string filename = "frame_" + to_string(frame_idx) + "_pred_" + to_string(y_pred) + ".jpg";
        fs::path out_file = out_path / filename;
        cv::imwrite(out_file.string(), frame);
    }
    cout << "\n# Video file: " << video_fpath << endl;
    cout << "\n# Test frame index: " << frame_idx << " / " << length << endl;
    cout << "\n# Feature vector:" << endl;
    for(int i=0; i<NUM_FEATURES; i++) {
        cout << feat[i] << endl;
    }
    cout << "\n# Probability:" << endl;
    for(const auto& p : prob) {
        cout << p << endl;
    }
    cout << "\n# Predicted class label: " << y_pred << endl;
    
    
    return 1;
}

void showUsageExample() {
    cout << "\nUsage Examples:" << endl;
    cout << "./detection --video <video.mp4> --svm <svm>" << endl;        
}

int main(int argc, char **argv) {
	cxxopts::Options options("./detection", "dectection.");
	options.add_options("Input")
            ("video", "video file", cxxopts::value<std::string>())			
            ("svm", "svm model file.", cxxopts::value<std::string>())
			("max_img_dim", "Maximum dimension of image to use for processing", 
			 cxxopts::value<int>()->default_value("64"))
            ("output_dir", "output dir", cxxopts::value<std::string>())		
            ("save", "save missclassified samples.", cxxopts::value<int>()->default_value("0"))            
			("help", "Show help");

	auto result = options.parse(argc, argv);

	if (result.count("help")) {
		cout << options.help( {"Input"} ) << endl;
		showUsageExample();
		return 0;
	}

	std::string video = result["video"].as<std::string>();    	
	std::string svm_fpath = result["svm"].as<std::string>();
    std::string output_dir = result["output_dir"].as<std::string>();    	
	int max_img_dim = result["max_img_dim"].as<int>();
    int save = result["save"].as<int>();

	detect(video, svm_fpath, max_img_dim, output_dir, save);
	
	return 0;
}


