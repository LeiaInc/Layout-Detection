"""
Extract image frames from WSVD dataset (https://sites.google.com/view/wsvd/home)
"""

import numpy as np
import cv2
import os
import argparse
import random
import pickle
from tqdm import tqdm

def image_resize(img, max_dim):
    if img.shape[0] > img.shape[1]:
        new_rows = max_dim
        new_cols = int(img.shape[1] * max_dim / img.shape[0])
    else:
        new_rows = int(img.shape[0] * max_dim / img.shape[1])
        new_cols = max_dim

    return cv2.resize(img, (new_cols, new_rows))


def extract(frame_id, video_dir, output_dir, step, max_dim):
    os.makedirs(output_dir, exist_ok=True)
    
    left_dir = os.path.join(output_dir, 'left')
    os.makedirs(left_dir, exist_ok=True)
    
    right_dir = os.path.join(output_dir, 'right')
    os.makedirs(right_dir, exist_ok=True)

    left_right_dir = os.path.join(output_dir, 'left_right')
    os.makedirs(left_right_dir, exist_ok=True)

    left_right_half_dir = os.path.join(output_dir, 'left_right_half')
    os.makedirs(left_right_half_dir, exist_ok=True)

    top_bottom_dir = os.path.join(output_dir, 'top_bottom')
    os.makedirs(top_bottom_dir, exist_ok=True)

    with open(frame_id, 'rb') as f:
        videos = pickle.load(f)
        
    for video in tqdm(videos):
        video_fpath = os.path.join(video_dir, video['name'] + '.mp4')
        if not os.path.exists(video_fpath):
            continue
        
        cap = cv2.VideoCapture(video_fpath)

        clips = video['clips']
        for clip in clips:
            label = clip['label']
            frames = clip['frames']
            for i in range(0, frames.shape[0]-1, step):
                frame_idx = frames[i] - 1 # the WSVD frame index starts from 1 so subtract 1 from it
                cap.set(cv2.CAP_PROP_POS_FRAMES, frame_idx)

                ret, image = cap.read()
                if ret:
                    # We assume that all youtube sbs image frame was suqeezed in half width
                    # so we upscale the width first to recover the original aspect ratio           
                    left_right = cv2.resize(image, (0,0), fx=2, fy=1)

                    # split sbs into left and right 
                    left, right = np.split(left_right, 2, axis=1)

                    # resize left and right images to small images
                    left = image_resize(left, max_dim=max_dim)
                    right = image_resize(right, max_dim=max_dim)

                    # concatnate them to generate resized sbs image
                    left_right = np.concatenate([left, right], axis=1)
                    top_bottom = np.concatenate([left, right], axis=0)

                    # finally, sbs half image
                    left_right_half = cv2.resize(left_right, (0,0), fx=0.5, fy=1)
                                                        
                    img_fname = video['name'] + '_{}.jpg'.format(frame_idx)
                    
                    cv2.imwrite(os.path.join(left_dir, img_fname), left)
                    cv2.imwrite(os.path.join(right_dir, img_fname), right)

                    cv2.imwrite(os.path.join(left_right_dir, img_fname), left_right)
                    cv2.imwrite(os.path.join(left_right_half_dir, img_fname), left_right_half)
                    cv2.imwrite(os.path.join(top_bottom_dir, img_fname), top_bottom)

        cap.release()
     

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument(        
        '--frame_id',
        default='/nethome/cvnas01/datasets/WSVD/wsvd_train_clip_frame_ids.pkl', 
        help='frame id pkl file')
    parser.add_argument(        
        '--video_dir',
        default='/nethome/cvnas01/datasets/WSVD/wsvd', 
        help='directory containing all youtube video clips')
    parser.add_argument(        
        '--output_dir',
        default='/nethome/cvnas01/datasets/WSVD/train', 
        help='output directory path')
    parser.add_argument(
        '--step',
        default=300,
        type=int,
        help='sample every n frames')
    parser.add_argument(
        '--max_dim',
        default=320,
        type=int,
        help='sample every n frames')

    args = parser.parse_args()

    extract(args.frame_id, args.video_dir, args.output_dir, args.step, args.max_dim)