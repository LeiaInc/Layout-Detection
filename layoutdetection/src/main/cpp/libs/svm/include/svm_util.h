#ifndef LAYOUTDETECTION_SVM_UTIL_H
#define LAYOUTDETECTION_SVM_UTIL_H

#include <string>
#include <sstream>
#include "svm.h"

// We use the 'malloc' memory allocation as in the libsvm example code.
#define Malloc(type,n) (type *)malloc((n)*sizeof(type))

using namespace std;

void svmPrediction(const svm_model *model, const float* feat, int num_feats, int& label, vector<double> &prob);
svm_model* loadSVMFromString(const string& model_str);
bool readSVMHeader(stringstream &ss, svm_model* model);
int getTypeIndex(const vector<string> &types, const string& target);

#endif //LAYOUTDETECTION_SVM_UTIL_H
