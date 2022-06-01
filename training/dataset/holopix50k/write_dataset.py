import os
import argparse
from glob import glob

from natsort import natsorted
from tqdm import tqdm
import numpy as np
import argparse
from enum import Enum

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

    files = natsorted(glob(dataset_dir + '/left_/*'))
    for f in tqdm(files):
        fp.write('%s,%d\n' % ( os.path.join('left_', os.path.basename(f)), Image.SINGLE.value ) )

    files = natsorted(glob(dataset_dir + '/left_right/*'))
    for f in tqdm(files):
        fp.write('%s,%d\n' % ( os.path.join('left_right', os.path.basename(f)), Image.STEREO_LEFT_RIGHT.value ) )

    if sbs_half:
        files = natsorted(glob(dataset_dir + '/left_right_half/*'))
        for f in tqdm(files):
            fp.write('%s,%d\n' % ( os.path.join('left_right_half', os.path.basename(f)), Image.STEREO_LEFT_RIGHT_HALF.value ) )

    files = natsorted(glob(dataset_dir + '/top_bottom/*'))
    for f in tqdm(files):
        fp.write('%s,%d\n' % ( os.path.join('top_bottom', os.path.basename(f)), Image.STEREO_TOP_BOTTOM.value ) )

    files = natsorted(glob(dataset_dir + '/quad/*'))
    for f in tqdm(files):
        fp.write('%s,%d\n' % ( os.path.join('quad', os.path.basename(f)), Image.QUAD.value ) )

    fp.close() 

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-d", "--dataset_dir", type=str, default='dataset/Holopix50k/', help='dataset directory path')
    parser.add_argument("-c", "--csv_filename", type=str, default='dataset.csv', help='dataset csv filename')     
    parser.add_argument("--sbs_half", type=bool, default=False, help='include 2x1 half class')    
    args = parser.parse_args()
    
    write_dataset(args.dataset_dir, args.csv_filename, args.sbs_half)
