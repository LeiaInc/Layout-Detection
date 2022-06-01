"""
Write a list of image file paths and corresponding class labels to a csv file 
"""

import os
import argparse
from glob import glob

from natsort import natsorted
from tqdm import tqdm
import numpy as np
import argparse
from enum import Enum

def helper(fp, dataset_dir, sub_dirname, label):
    search_dir = os.path.join(os.path.join(dataset_dir, sub_dirname))
    if os.path.exists(search_dir):
        files = [f for f in os.listdir(search_dir) if f.endswith('.jpg') or f.endswith('.png')]
        files.sort()
        for f in files:
            fp.write('%s,%d\n' % ( os.path.join(sub_dirname, f), label ) )

def write_dataset(dataset_dir, csv_filename):   
    fp = open(os.path.join(dataset_dir, csv_filename), 'w')
    helper(fp, dataset_dir, 'mono', 0)
    helper(fp, dataset_dir, 'stereo', 1)        
    fp.close() 
    print(f'Done.')
    
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-d", "--dataset_dir", type=str, default='/nethome/cvnas01/datasets/youtubeVR360/data/test', help='dataset directory path')
    parser.add_argument("-c", "--csv_filename", type=str, default='dataset.csv', help='dataset csv filename')        
    args = parser.parse_args()
    
    write_dataset(args.dataset_dir, args.csv_filename)
