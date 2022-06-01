#include <iostream>
#include <chrono>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <memory>
#include <cxxopts.hpp>

#include "feature.h"
#include "svm_util.h"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/core.hpp>

using namespace std;
using namespace cv;
namespace fs = std::filesystem;


// confidence interval const values
// 1.64 (90%)
// 1.96 (95%)
// 2.33 (98%)
// 2.58 (99%)
#define CONF_INTER_CONST 1.96  

// printProgress code from
// https://stackoverflow.com/questions/14539867/how-to-display-a-progress-indicator-in-pure-c-c-cout-printf
#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60
void printProgress(double percentage) {
    int val = (int) (percentage * 100);
    int lpad = (int) (percentage * PBWIDTH);
    int rpad = PBWIDTH - lpad;
    printf("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
    fflush(stdout);
}

// This reads a csv file and outputs a list of image file paths and class labels
void parseCSV(const string& csv_fpath, vector<string> &fpaths, vector<int> &labels) {
    string line;
    ifstream fs (csv_fpath);
    if (!fs.is_open()) {
        throw invalid_argument("Unable to open file " + csv_fpath);
    }

    fs::path dataset_dir(csv_fpath);
    dataset_dir = dataset_dir.parent_path();

    while ( getline(fs, line) ) {
        // line will contain the image file path and class label
        // e.g. line = "left/-LMudfNzcf8L_S_GHKkk_left.jpg,0"
        istringstream ss(line);
        string token;
        getline(ss, token, ',');
        fs::path full_p = dataset_dir / token;
        fpaths.push_back( full_p.string() );

        getline(ss, token);
        labels.push_back( stoi(token) );
    }
    fs.close();
}

void confidence_interval(double error, int num_samples, double& lower, double& upper) {
	// Interpretation of confidence interval (lower, upper) output:
	// There is a X% likelihood that the confidence interval [lower, upper] covers 
	// the true classification error of the model on unseen data.
	// where X % is determined by CONF_INTER_CONST constant number
	// 1.64 (90%)
	// 1.96 (95%)
	// 2.33 (98%)
	// 2.58 (99%)
	// See: https://machinelearningmastery.com/report-classifier-performance-confidence-intervals/	
	double interval = CONF_INTER_CONST * std::sqrt( (error * (1.0 - error)) / num_samples);
	
	// the confidence intervals on the classification error must be clipped to the values 0.0 and 1.0.
	lower = std::max(error - interval, 0.0);
	upper = std::min(error + interval, 1.0);	
}
	
void defaultParam(struct svm_parameter &param) {
    // https://github.com/cjlin1/libsvm
    // -s svm_type : set type of SVM (default 0)
    //     0 -- C-SVC		(multi-class classification)
    //     1 -- nu-SVC		(multi-class classification)
    //     2 -- one-class SVM
    //     3 -- epsilon-SVR	(regression)
    //     4 -- nu-SVR		(regression)
    // -t kernel_type : set type of kernel function (default 2)
    //     0 -- linear: u'*v
    //     1 -- polynomial: (gamma*u'*v + coef0)^degree
    //     2 -- radial basis function: exp(-gamma*|u-v|^2)
    //     3 -- sigmoid: tanh(gamma*u'*v + coef0)
    //     4 -- precomputed kernel (kernel values in training_set_file)
    // -d degree : set degree in kernel function (default 3)
    // -g gamma : set gamma in kernel function (default 1/num_features)
    // -r coef0 : set coef0 in kernel function (default 0)
    // -c cost : set the parameter C of C-SVC, epsilon-SVR, and nu-SVR (default 1)
    // -n nu : set the parameter nu of nu-SVC, one-class SVM, and nu-SVR (default 0.5)
    // -p epsilon : set the epsilon in loss function of epsilon-SVR (default 0.1)
    // -m cachesize : set cache memory size in MB (default 100)
    // -e epsilon : set tolerance of termination criterion (default 0.001)
    // -h shrinking : whether to use the shrinking heuristics, 0 or 1 (default 1)
    // -b probability_estimates : whether to train a SVC or SVR model for probability estimates, 0 or 1 (default 0)
    // -wi weight : set the parameter C of class i to weight*C, for C-SVC (default 1)
    // -v n: n-fold cross validation mode
    // -q : quiet mode (no outputs)

    param.svm_type = C_SVC; 
    param.kernel_type = RBF;
    param.degree = 3;       /* for poly */
    param.gamma = 0;        /* for poly/rbf/sigmoid */
    param.coef0 = 0;        /* for poly/sigmoid */
    param.nu = 0.5;         /* for NU_SVC, ONE_CLASS, and NU_SVR */
    param.cache_size = 100; /* in MB */
    param.C = 1;            /* for C_SVC, EPSILON_SVR, and NU_SVR */
    param.eps = 1e-3;       /* stopping criteria */
    param.p = 0.1;          /* for EPSILON_SVR */
    param.shrinking = 1;    /* use the shrinking heuristics */
    param.probability = 0;  /* do probability estimates */
    param.nr_weight = 0;    /* for C_SVC */
    param.weight_label = nullptr;   /* for C_SVC */
    param.weight = nullptr;         /* for C_SVC */
}

// See more details about svm problem structure and paramters here: 
// https://github.com/cjlin1/libsvm
void trainSVM(const vector<string>& csv_fpath, const string& model_fpath, int max_img_dim) {
    vector<string> fpaths;
    vector<int> labels;
    for(auto& f : csv_fpath) {
        parseCSV(f, fpaths, labels);        
    }
    
    int num_samples = fpaths.size();
    if(num_samples <= 0) {
        throw invalid_argument("No training data from files.");
    }

    struct svm_parameter param{};
    struct svm_problem prob{};
    struct svm_model *model;
    struct svm_node *x_space;

    defaultParam(param);
    // We use 1.0 for gamma to get better performance (OpenCV's default value).
    // We also set probability to 1 to enable the confidence computation. 
    param.gamma = 1.0;      
    param.probability = 1; 
    
    // init. problem parameters
    prob.l = num_samples;
    prob.y = Malloc(double, num_samples);
    prob.x = Malloc(struct svm_node *, num_samples);
    // x_space is a flat 1D array containing all the svm_nodes in the row-major order.
    // Note that each row has (NUM_FEATURES + 1) nodes, and the last one for indicating 
    // the end of the row. See the description above for more detail.
    x_space = Malloc(struct svm_node, num_samples * (NUM_FEATURES + 1));

    cout << "\nBuilding Problem (" << num_samples << " samples)" << endl;
    for(int i=0; i<num_samples; i++) {
        // feature extraction
        Mat img = imread(fpaths[i], cv::IMREAD_GRAYSCALE);
        float feat[NUM_FEATURES];
        featureExtraction(img, max_img_dim, feat);

        // assign the feature values to x_space[]
        for(int j=0; j<NUM_FEATURES; j++) {
            x_space[(NUM_FEATURES + 1) * i + j].value = feat[j];
            // the index starts from one so we add '1' to the index 'j'
            x_space[(NUM_FEATURES + 1) * i + j].index = j + 1;
        }
        // dummy element to indicate the end of the row.
        x_space[(NUM_FEATURES + 1) * i + NUM_FEATURES].index = -1;

        // prob.x[i] points to the first element of the i-th row
        prob.x[i] = &x_space[(NUM_FEATURES + 1) * i];

        // labels
        prob.y[i] = double(labels[i]);

        printProgress(double(i)/num_samples);
    }

    const char *error_msg = svm_check_parameter(&prob, &param);
    if(error_msg) {
        throw invalid_argument(string(error_msg));
    }

    cout << "Training SVM classifier..." << endl;
    model = svm_train(&prob, &param);
    if( svm_save_model(model_fpath.c_str(), model) ) {
        throw invalid_argument("can't save model to file " + model_fpath);
    }

    svm_free_and_destroy_model(&model);

    svm_destroy_param(&param);
    free(prob.y);
    free(prob.x);
    free(x_space);

    cout << "Done." << endl;
}

void evalSVM(const vector<string>& csv_fpath, string model_fpath, int max_img_dim, int save) {
    vector<string> fpaths;
    vector<int> y_true;
    for(auto& f : csv_fpath) {
        parseCSV(f, fpaths, y_true);
        cout << fpaths.size() << endl;
        
    }    
    int num_samples = fpaths.size();
    if(num_samples <= 0) {
        throw invalid_argument("No training data from files.");
    }

    cout << "\nLoad SVM model from file " << model_fpath << endl;
    std::ifstream ifs(model_fpath);
    std::string content;
    content.assign( (std::istreambuf_iterator<char>(ifs) ),
                    (std::istreambuf_iterator<char>()    ) );
    svm_model* model = loadSVMFromString(content);

    // save the prediction result to a file
    // the output file name is the model_fpath + ".txt"
    std::ofstream fp;
    fp.open(model_fpath + ".txt", std::ofstream::out);

    // save misclassified samples
    fs::path error_dir(csv_fpath[0]);
    error_dir = error_dir.parent_path() / "misclassified";
    if(save == 1) {        
        fs::create_directory(error_dir);
        for(int i=0; i<model->nr_class; i++) {
            fs::create_directory(error_dir / to_string(i));
        }
    }
    
    
    int num_correct{0}, num_correct_portrait{0}, num_portraits{0};		
    cout << "Evaluating svm model... (" << num_samples << ")" << endl;
    for(int i=0; i<num_samples; i++) {
        // feature extraction
        Mat img = imread(fpaths[i], cv::IMREAD_GRAYSCALE);
        float feat[NUM_FEATURES];
        featureExtraction(img, max_img_dim, feat);

        vector<double> prob;
        int y_pred;
        svmPrediction(model, feat, NUM_FEATURES, y_pred, prob);

        if(y_pred == y_true[i]) {
            num_correct++;
            if(img.rows > img.cols) {
                num_correct_portrait++;
            }			
        }

        if((save == 1) && (y_pred != y_true[i])) {
            fs::path img_fname(fpaths[i]);
            fs::path parent_path = img_fname.parent_path().filename();
            string new_fname = to_string(y_true[i]) + "_" + img_fname.parent_path().filename().string() + "_" + img_fname.filename().string();
            fs::path save_path = error_dir / to_string(y_pred) / new_fname;
            cv::imwrite(save_path.string(), img);
        } 

        if(img.rows > img.cols) {
            num_portraits++;
        }

        // write the prediction result in a line
        fp << to_string(y_true[i]) << " " << to_string(y_pred);
        for(const auto& p : prob) {
            fp << " " << to_string(p);
        }
        fp << endl;


        printProgress(double(i)/num_samples);
    }

    // close the output file
    fp.close();

	double acc = double(num_correct) / num_samples;
	printf("\nAccuracy: %d / %d = %.4f", num_correct, num_samples, acc);
	printf("\nAccuracy (Portrait): %d / %d = %.6f", num_correct_portrait, num_portraits, 
		double(num_correct_portrait) / num_portraits);
	printf("\nAccuracy (Landscape): %d / %d = %.6f", num_correct-num_correct_portrait, num_samples-num_portraits, 
		double(num_correct-num_correct_portrait) / (num_samples-num_portraits));

	// confidence interval
	double conf_lower, conf_upper;
	confidence_interval(1.0 - acc, num_samples, conf_lower, conf_upper);
	printf("\nerror = %f. There is a 95 percent likelihood that the confidence interval [%f, %f] covers", 1.0 - acc, conf_lower, conf_upper); 
	printf("\nthe true classification error of the model on unseen data.\n");
	
	svm_free_and_destroy_model(&model);
}

void showUsageExample() {
    cout << "\nUsage Examples:" << endl;
    cout << "./layoutDetection --csv <dataset.csv> --mode <train or eval> --svm <svm.xml>" << endl;        
}

int main(int argc, char **argv) {
	cxxopts::Options options("./layoutDectection", "layoutDectection.");
	options.add_options("Input")
            ("csv", "csv file", cxxopts::value<std::vector<std::string>>())
			("mode", "(train, eval)", cxxopts::value<std::string>()->default_value("train"))
            ("svm", "svm model file.", cxxopts::value<std::string>())
			("max_img_dim", "Maximum dimension of image to use for processing", 
			 cxxopts::value<int>()->default_value("64"))
            ("save", "save missclassified samples.", cxxopts::value<int>()->default_value("0"))            
			("help", "Show help");

	auto result = options.parse(argc, argv);

	if (result.count("help")) {
		cout << options.help( {"Input"} ) << endl;
		showUsageExample();
		return 0;
	}

	std::vector<std::string> csv_fpaths = result["csv"].as<std::vector<std::string>>();
    
	std::string mode = result["mode"].as<std::string>();	
	std::string svm_fpath = result["svm"].as<std::string>();
	int max_img_dim = result["max_img_dim"].as<int>();
    int save = result["save"].as<int>();

	if(mode == "train") {
		trainSVM(csv_fpaths, svm_fpath, max_img_dim);
	} else if(mode == "eval") {
		evalSVM(csv_fpaths, svm_fpath, max_img_dim, save);
	} else {
		throw invalid_argument("Not supported mode. (train/eval)");
	}	
	
	return 0;
}
