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

def write_dataset(dataset_dir, csv_filename, sbs_half):    
    if not sbs_half:
        class Image(Enum):
            SINGLE = 0
            STEREO_LEFT_RIGHT = 1        
            STEREO_TOP_BOTTOM = 2
            QUAD = 3
    else:
        class Image(Enum):
            SINGLE = 0
            STEREO_LEFT_RIGHT = 1  
            STEREO_LEFT_RIGHT_HALF = 2      
            STEREO_TOP_BOTTOM = 3
            QUAD = 4

    fp = open(os.path.join(dataset_dir, csv_filename), 'w')

    helper(fp, dataset_dir, 'left', Image.SINGLE.value)
    helper(fp, dataset_dir, 'left_right', Image.STEREO_LEFT_RIGHT.value)
    if sbs_half:
        helper(fp, dataset_dir, 'left_right_half', Image.STEREO_LEFT_RIGHT_HALF.value)
    helper(fp, dataset_dir, 'top_bottom', Image.STEREO_TOP_BOTTOM.value)
    helper(fp, dataset_dir, 'quad', Image.QUAD.value)


    fp.close() 

    print(f'Done.')
    
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-d", "--dataset_dir", type=str, default='/Users/chelhwon.kim/Downloads/datasets/WSVD/test/', help='dataset directory path')
    parser.add_argument("-c", "--csv_filename", type=str, default='test.csv', help='dataset csv filename')     
    parser.add_argument("--sbs_half", type=bool, default=True, help='include 2x1 half class')    
    args = parser.parse_args()
    
    write_dataset(args.dataset_dir, args.csv_filename, args.sbs_half)