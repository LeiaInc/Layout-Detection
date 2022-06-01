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

def write_dataset(dataset_dir, csv_filename, class_type):   
    fp = open(os.path.join(dataset_dir, csv_filename), 'w')

    if class_type == 0:
        class Image(Enum):
            SINGLE = 0
            STEREO_LEFT_RIGHT = 1  
            STEREO_LEFT_RIGHT_HALF = 2      
            STEREO_TOP_BOTTOM = 3
            QUAD = 4
        helper(fp, dataset_dir, 'left', Image.SINGLE.value)
        helper(fp, dataset_dir, 'left_right', Image.STEREO_LEFT_RIGHT.value)    
        helper(fp, dataset_dir, 'left_right_half', Image.STEREO_LEFT_RIGHT_HALF.value)
        helper(fp, dataset_dir, 'top_bottom', Image.STEREO_TOP_BOTTOM.value)
        helper(fp, dataset_dir, 'quad', Image.QUAD.value)
        
    elif class_type == 1:
        class Image(Enum):
            SINGLE = 0
            STEREO_LEFT_RIGHT = 1        
            STEREO_TOP_BOTTOM = 2
            QUAD = 3
        helper(fp, dataset_dir, 'left', Image.SINGLE.value)
        helper(fp, dataset_dir, 'left_right', Image.STEREO_LEFT_RIGHT.value)        
        helper(fp, dataset_dir, 'top_bottom', Image.STEREO_TOP_BOTTOM.value)
        helper(fp, dataset_dir, 'quad', Image.QUAD.value)
        
    else:
        class Image(Enum):
            SINGLE = 0
            STEREO_LEFT_RIGHT_HALF = 1
        
        helper(fp, dataset_dir, 'left', Image.SINGLE.value)
        helper(fp, dataset_dir, 'left_right_half', Image.STEREO_LEFT_RIGHT_HALF.value)
        
    fp.close() 

    print(f'Done.')
    
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-d", "--dataset_dir", type=str, default='/nethome/cvnas01/datasets/WSVD/test/', help='dataset directory path')
    parser.add_argument("-c", "--csv_filename", type=str, default='dataset.csv', help='dataset csv filename')     
    parser.add_argument("--class_type", type=int, default=0, 
        help='0: [mono, 2x1, 2x1 half, 1x2, quad], 1: [mono, 2x1, 1x2, quad], 2: [mono, 2x1 half]')    
    args = parser.parse_args()
    
    write_dataset(args.dataset_dir, args.csv_filename, args.class_type)
