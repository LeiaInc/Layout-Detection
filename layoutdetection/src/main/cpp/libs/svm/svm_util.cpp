#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

#include "svm_util.h"
using namespace std;

void svmPrediction(const svm_model *model, const float* feat, int num_feats, int& label, vector<double> &prob) {
    auto *node = (svm_node *) malloc( (num_feats + 1) * sizeof(svm_node));
    for(int i=0; i<num_feats; i++) {
        node[i].value = feat[i];
        // the sv index starts from one so we add 1 to the index 'i'.
        node[i].index = i + 1;
    }
    node[num_feats].index = -1; // this is for indicating the end of sv elements.

    int svm_type = svm_get_svm_type(model);
    int nr_class = svm_get_nr_class(model);
    auto *prob_estimates = (double *) malloc(nr_class * sizeof(double));
    prob.clear();
    if (svm_type == C_SVC || svm_type == NU_SVC) {
        label = int( svm_predict_probability(model, node, prob_estimates) );
        for(int i=0; i<nr_class; i++) {
            prob.push_back(prob_estimates[i]);
        }
    } else {
        label = int( svm_predict(model, node) );
    }

    free(prob_estimates);
    free(node);
}

svm_model* loadSVMFromString(const string& model_str) {
    if(model_str.empty()) {
        return nullptr;
    }
    std::stringstream ss(model_str);

    auto *model = Malloc(struct svm_model, 1);
    model->rho = nullptr;
    model->probA = nullptr;
    model->probB = nullptr;
    model->sv_indices = nullptr;
    model->label = nullptr;
    model->nSV = nullptr;

    // read model parameters
    if(!readSVMHeader(ss, model)) {
         free(model->rho);
         free(model->label);
         free(model->nSV);
         free(model);
         return nullptr;
    }

    // read support vectors
    long pos = ss.tellg();  // remember the current position

    // count the number of sv elements to allocate the memory
    int num_elements = 0;
    string line;
    while(getline(ss, line)) {
        // Each line will contains sv_coefs and sv values in "index:value" format as follows:
        // "1 0 0 1:1 2:0.55032963 3:-0.25062907 4:-0.28756726 5:0.5625"
        // We count the number of sv elements by tokenizing the string by ':'.
        // Note that the counting includes an additional element. i.e. the total count
        // will be 6, not 5. This is for the last element with dummy index '-1' at the end.
        // See more details in https://github.com/cjlin1/libsvm
        // about the svm_problem structure.
        stringstream ss_line(line);
        string token;
        while(getline(ss_line, token, ':')) {
            num_elements++;
        }
    }

    // allocate memory
    int num_coefs = model->nr_class - 1;
    int num_svs = model->l;
    model->sv_coef = Malloc(double *, num_coefs);
    for(int i=0; i<num_coefs; i++) {
        model->sv_coef[i] = Malloc(double, num_svs);
    }

    model->SV = Malloc(svm_node*, num_svs);
    svm_node *x_space = nullptr;
    if(num_svs > 0) {
        // x_space is a flatten 1D array that will containing all sv values in the row-major order
        x_space = Malloc(svm_node, num_elements);
    }

    // return to the position whrere the sv data starts
    ss = stringstream(model_str); // initialize the stream from the model content string.
    ss.seekg(pos);

    // copy data to the created arrays
    int num_elements_t = 0; // this is for checking if the count is correct.
    int idx_sv = 0;
    while(getline(ss, line)) {
        // each ling will contain sv_coef and sv (index:value) pairs:
        // "1 0 0 1:1 2:0.55032963 3:-0.25062907 4:-0.28756726 5:0.5625"

        model->SV[idx_sv] = &x_space[num_elements_t];

        stringstream ss_line(line);
        string token;
        // read the first (num_classes-1) elements for sv_coef
        for(int j=0; j<num_coefs; j++) {
            ss_line >> token;
            model->sv_coef[j][idx_sv] = stod(token);
        }

        // read the sv (index:value) pairs
        while( (ss_line >> token) ) {
            std::size_t split = token.find(':');
            if(split == string::npos) {
                // if there is no ':' then something is wrong.
                free(model->rho);
                free(model->label);
                free(model->nSV);
                free(model);
                return nullptr;
            } else {
                x_space[num_elements_t].index = stoi( token.substr(0, split) );
                x_space[num_elements_t].value = stod( token.substr(split+1, token.size()) );
                num_elements_t++;
            }
        }
        x_space[num_elements_t++].index = -1; // dummy element at the end.

        idx_sv++;
    }

    // check the count match the previous one.
    if(num_elements != num_elements_t || idx_sv != num_svs) {
        free(model->rho);
        free(model->label);
        free(model->nSV);
        free(model);
        return nullptr;
    }

    model->free_sv = 1;

    return model;
}

int getTypeIndex(const vector<string> &types, const string& target) {
    int type = -1;
    for(int i=0; i<types.size(); i++) {
        if(types[i] == target) {
            type = i;
            break;
        }
    }
    return type;
}

bool readSVMHeader(stringstream &ss, svm_model* model) {
    /*
    Example of the svm model file content:

    svm_type c_svc
    kernel_type rbf
    gamma 0.2
    nr_class 4
    total_sv 892
    rho 2.54 -1.77 0.04 0.12 -2.08 0.29
    label 0 1 2 3
    probA -3.58 -4.37 -4.18 -5.11 -4.72 -7.43
    probB -0.14 0.07 0.34 0.53 -0.16 -1.08
    nr_sv 282 275 177 158
    SV
    1 0 0 1:1 2:0.55 3:-0.25 4:-0.28 5:0.56
    ...
    ...

    */
    svm_parameter& param = model->param;
    // parameters for training only won't be assigned, but arrays are assigned as NULL for safety
    param.nr_weight = 0;
    param.weight_label = nullptr;
    param.weight = nullptr;

    // predefined keywords in svm.h file.
    vector<string> svm_type_table{"c_svc", "nu_svc", "one_class", "epsilon_svr", "nu_svr"};
    vector<string> kernel_type_table{"linear", "polynomial", "rbf", "sigmoid", "precomputed"};

    while(!ss.eof()) {
        string token;
        ss >> token;

        if(token == "svm_type") {
            ss >> token;
            param.svm_type = getTypeIndex(svm_type_table, token);
            if(param.svm_type == -1) {
                cout << "unknown svm type.\n";
                return false;
            }
        } else if(token == "kernel_type") {
            ss >> token;
            param.kernel_type = getTypeIndex(kernel_type_table, token);
            if(param.kernel_type == -1) {
                cout << "unknown kernel type.\n";
                return false;
            }
        } else if(token == "degree") {
            ss >> token;
            param.degree = stoi(token);
        } else if(token == "gamma") {
            ss >> token;
            param.gamma = stod(token);
        } else if(token == "coef0") {
            ss >> token;
            param.coef0 = stod(token);
        } else if(token == "nr_class") {
            ss >> token;
            model->nr_class = stoi(token);
        } else if(token == "total_sv") {
            ss >> token;
            model->l = stoi(token);
        } else if(token == "rho") {
            int n = model->nr_class * (model->nr_class-1)/2;
            model->rho = Malloc(double, n);
            for(int i=0; i<n; i++) {
                ss >> token;
                model->rho[i] = stod(token);
            }
        } else if(token == "label") {
            int n = model->nr_class;
            model->label = Malloc(int, n);
            for(int i=0; i<n; i++) {
                ss >> token;
                model->label[i] = stoi(token);
            }
        } else if(token == "probA") {
            int n = model->nr_class * (model->nr_class-1)/2;
            model->probA = Malloc(double, n);
            for(int i=0; i<n; i++) {
                ss >> token;
                model->probA[i] = stod(token);
            }
        } else if(token == "probB") {
            int n = model->nr_class * (model->nr_class-1)/2;
            model->probB = Malloc(double, n);
            for(int i=0; i<n; i++) {
                ss >> token;
                model->probB[i] = stod(token);
            }
        } else if(token == "nr_sv") {
            int n = model->nr_class;
            model->nSV = Malloc(int, n);
            for(int i=0; i<n; i++) {
                ss >> token;
                model->nSV[i] = stoi(token);
            }
        } else if(token == "SV") {
            // Stop reading where the support vector data starts
            getline(ss, token); // need this to go to the next line and skip the '\n'
            break;
        } else{
            cout << "unknown text in model file: " << token << endl;
            return false;
        }
    }
    return true;
}

