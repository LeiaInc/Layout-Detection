import os
import argparse
from glob import glob

from natsort import natsorted
from tqdm import tqdm
import subprocess
import numpy as np

FNULL = open(os.devnull, 'w')
def call_mvlib(mv_path, imgL, imgR, outfile):
    subprocess.call(
        [
            mv_path,
            "--left", imgL,
            "--right", imgR,            
            "--no-save-disp",
            "--no-save-views",
            "--gain-multiplier", "1.0",            
            "-o", outfile
        ],
        stderr=FNULL,
        stdout=FNULL
    )

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-m", "--mv_path", type=str, default="android-multiview/lib/build/MultiviewTest", help='android multiview app')
    parser.add_argument("-l", "--left_dir", type=str, default='dataset/Holopix50k/val/left/', help='left image dir')
    parser.add_argument("-r", "--right_dir", type=str, default='dataset/Holopix50k/val/right/', help='right image dir')    
    parser.add_argument("-o", "--output_dir", type=str, default='dataset/Holopix50k/val/quad', help='output dir')    
    args = parser.parse_args()
    
    left_paths = natsorted(glob( os.path.join(args.left_dir, '*') ))
    right_paths = natsorted(glob( os.path.join(args.right_dir, '*') ))

    os.makedirs(args.output_dir, exist_ok=True)

    for i in tqdm(range(len(left_paths))):
        outfile = os.path.join( args.output_dir, os.path.basename(left_paths[i]).replace('_left.jpg', '') )
        call_mvlib(args.mv_path, left_paths[i], right_paths[i], outfile)

    print('Done.')
