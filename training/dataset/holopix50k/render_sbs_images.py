"""
Render 2x1, 2x1 half, 1x2 images from the holopix50k dataset.
"""

import os
import argparse
from glob import glob

from natsort import natsorted
from tqdm import tqdm
import numpy as np
import cv2
import random
import shutil

ASPECT_RATIO_LOWER_SHORT = 7.0/9.0
ASPECT_RATIO_LOWER_LONG = 8.0/10.0

def image_resize(img, max_dim):
    if img.shape[0] > img.shape[1]:
        new_rows = max_dim
        new_cols = int(img.shape[1] * max_dim / img.shape[0])
    else:
        new_rows = int(img.shape[0] * max_dim / img.shape[1])
        new_cols = max_dim

    return cv2.resize(img, (new_cols, new_rows))

def random_aspect_ratio(left, right):
    type = random.randint(0, 1)
    if type == 0:
        ratio = random.uniform(ASPECT_RATIO_LOWER_SHORT, 1.0)
        if left.shape[0] > left.shape[1]:
            new_rows = left.shape[0] 
            new_cols = int( left.shape[1] * ratio )
        else:
            new_rows = int( left.shape[0] * ratio )
            new_cols = left.shape[1] 
    else:
        ratio = random.uniform(ASPECT_RATIO_LOWER_LONG, 1.0)
        if left.shape[0] > left.shape[1]:
            new_rows = int( left.shape[0] * ratio )
            new_cols = left.shape[1]
        else:
            new_rows = left.shape[0]
            new_cols = int( left.shape[1] * ratio )
    
    left = left[0:new_rows, 0:new_cols, :]
    right = right[0:new_rows, 0:new_cols, :]

    return left, right


def main(dataset_dir, output_dir, sbs_half=False, augment = False, max_dim = 320):   
    assert dataset_dir != output_dir, "the output directory must be different from the source dataset directory to avoid overwriting."
    
    os.makedirs(os.path.join(output_dir, 'left'), exist_ok=True)
    os.makedirs(os.path.join(output_dir, 'right'), exist_ok=True)
    os.makedirs(os.path.join(output_dir, 'left_right'), exist_ok=True)
    os.makedirs(os.path.join(output_dir, 'left_right_half'), exist_ok=True)
    os.makedirs(os.path.join(output_dir, 'top_bottom'), exist_ok=True)
    
    left_paths = natsorted(glob(os.path.join(dataset_dir, 'left', '*.jpg')))
    for i in tqdm(range(len(left_paths))):
        try:        
            left = cv2.imread(left_paths[i])

            filename = os.path.basename(left_paths[i])        
            right = cv2.imread(os.path.join(dataset_dir, 'right', filename.replace('_left.jpg', '_right.jpg')))
            
            if augment:
                left, right = random_aspect_ratio(left, right)
            
            # resize the original images
            left = image_resize(left, max_dim)
            right = image_resize(right, max_dim)

            # Save files        
            cv2.imwrite(os.path.join(output_dir, 'left', filename), left)
            cv2.imwrite(os.path.join(output_dir, 'right', filename.replace('_left.jpg', '_right.jpg')), right)

            left_right = np.concatenate([left, right], axis=1)
            cv2.imwrite(os.path.join(output_dir, 'left_right', filename.replace('_left.jpg', '_left_right.jpg')), left_right)

            if sbs_half:
                left_right_half = cv2.resize(left_right, (left.shape[1], left.shape[0]))
                cv2.imwrite(os.path.join(output_dir, 'left_right_half', filename.replace('_left.jpg', '_left_right_half.jpg')), left_right_half)
            
            top_bottom = np.concatenate([left, right], axis=0)
            cv2.imwrite(os.path.join(output_dir, 'top_bottom', filename.replace('_left.jpg', '_top_bottom.jpg')), top_bottom)
        except Exception as e:
            print(e)

    print('Done.')


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-d", "--dataset_dir", type=str, default='/nethome/cvnas01/datasets/Holopix50k/test/')
    parser.add_argument("-o", "--output_dir", type=str, default='/nethome/cvnas01/datasets/Holopix50k/layout_detection/test/')    
    parser.add_argument("--sbs_half", type=bool, default=True, help='include 2x1 half class')
    parser.add_argument("-a", "--augment", type=bool, default=False, help='change the aspect ratio')   
    parser.add_argument('--max_dim', default=320, type=int, help='sample every n frames')
    args = parser.parse_args()

    main(args.dataset_dir, args.output_dir, sbs_half=args.sbs_half, augment=args.augment, max_dim=args.max_dim)
    
