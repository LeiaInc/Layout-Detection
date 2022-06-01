import os
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
from sklearn.metrics import confusion_matrix, classification_report
import argparse

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-o", "--output_file", type=str, 
        default='/nethome/cvnas01/chelhwon/Projects/layout_detection/result/svm_models/svm_v06_sbs_half.txt', 
        help='classification output file')
    parser.add_argument('--num_classes',  default=5, type=int, help='set the number of classes in the model')
    parser.add_argument('--conf_score_th',  default=0.9, type=float, help='set the confidence score threshold to compute false discovery/miss rate')
    args = parser.parse_args()

    num_classes = args.num_classes
    conf_score_th = args.conf_score_th

    with open(args.output_file, newline='') as fp:
        data = fp.readlines()

    data_correct = []
    data_incorrect = []
    y_trues = []
    y_preds = []
    for line in data:
        tokens = line.split(" ")
        y_true = int(tokens[0])
        y_pred = int(tokens[1])
        probs = [float(p) for p in tokens[2:2+num_classes]]

        y_trues.append(y_true)
        y_preds.append(y_pred)

        if y_true == y_pred:
            data_correct.append(probs[y_pred])
        else:
            data_incorrect.append(probs[y_pred])
    
    cm = confusion_matrix(y_trues, y_preds)
    print(classification_report(y_trues, y_preds, digits = 5))
    print("Confusion matrix")
    print(cm) 
    
    data_correct = np.array(data_correct)
    data_incorrect = np.array(data_incorrect)
    tp = (data_correct > conf_score_th)
    fp = (data_incorrect > conf_score_th)
    fn = (data_correct <= conf_score_th)
    tn = (data_incorrect <= conf_score_th)

    false_discovery_rate = np.sum(fp) / (np.sum(tp) + np.sum(fp))

    miss_rate = np.sum(fn) / (np.sum(fn) + np.sum(tp))

    print("Threshold the confidence score at ", conf_score_th)
    print("false discovery rate: ", false_discovery_rate)
    print("miss rate: ", miss_rate)
