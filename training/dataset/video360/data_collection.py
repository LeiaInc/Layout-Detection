"""
Generate mono and stereo video frames extracted from the downloaded 
YouTube 360 videos from https://github.com/pedro-morgado/spatialaudiogen 
"""
import os
from tqdm import tqdm
import numpy as np
import argparse
import glob
import cv2

# ratios to determine the start and end frame idx  
START_IDX_RATIO = 0.2 
END_IDX_RATIO = 0.8

def image_resize(img, max_dim):
    if img.shape[0] > img.shape[1]:
        new_rows = max_dim
        new_cols = int(img.shape[1] * max_dim / img.shape[0])
    else:
        new_rows = int(img.shape[0] * max_dim / img.shape[1])
        new_cols = max_dim

    return cv2.resize(img, (new_cols, new_rows))

def collect(dataset_dir, split, nsamples_per_video, max_dim):  
    # output folders
    output_dir = os.path.join(dataset_dir, 'data', split)
    if os.path.exists(output_dir):
        print(output_dir, 'is already exist. Do you want to delete them? (y/n)')
        y_or_n = input()
        if y_or_n == 'y':            
            os.system('rm -rf ' + output_dir)
    os.makedirs(output_dir, exist_ok=True)
    os.makedirs( os.path.join(output_dir, 'mono'), exist_ok=True)
    os.makedirs( os.path.join(output_dir, 'stereo'), exist_ok=True)     
 
    # create a dictionary of pairs (video id, stereopsis)
    video_formats = os.path.join(dataset_dir, 'scraping/video_formats_lowres.txt')
    with open(video_formats, 'r') as fp:
        lines = fp.readlines()
    video_dict = {}
    for line in lines:
        line = line.rstrip('\n')
        video_id, format, stereopsis, projection = line.split(' ')
        video_dict[video_id] = stereopsis
    
    # lists of video_ids
    subsets = ['meta/subsets/REC-Street.{}.1.lst'.format(split), 'meta/subsets/YT-All.{}.1.lst'.format(split)]    
    video_ids = []
    for f in subsets:
        with open(os.path.join(dataset_dir, f), 'r') as fp:
            video_ids += fp.readlines()
    
    # video frame extraction
    for video_id in tqdm(video_ids):
        video_id = video_id.rstrip('\n')      
        # video file path
        video_path = glob.glob( os.path.join(dataset_dir, 'data/orig', video_id + ".video.mp4") )
        if len(video_path) != 1:
            continue        
        video_path = video_path[0]

        # video stereopsis
        if not video_id in video_dict.keys():
            continue
        stereopsis = video_dict[video_id]

        # extract video frames
        cap = cv2.VideoCapture(video_path)
        if cap is None:
            print("Failed to open ", video_path)
            continue

        video_len = int( cap.get(cv2.CAP_PROP_FRAME_COUNT) )

        start_idx = int(video_len * START_IDX_RATIO)
        end_idx = int(video_len * END_IDX_RATIO)
        frame_idices = np.linspace(start_idx, end_idx, nsamples_per_video).astype(np.int)       
        for i in frame_idices:
            cap.set(cv2.CAP_PROP_POS_FRAMES, i)
            ret, image = cap.read()
            if ret is None or image is None:
                continue            
            if stereopsis == 'STEREO':
                fname = os.path.join(output_dir, 'stereo', video_id + '_' + str(i) + '.jpg')                    
            else:
                fname = os.path.join(output_dir, 'mono', video_id + '_' + str(i) + '.jpg')
            
            image = image_resize(image, max_dim)
            cv2.imwrite(fname, image)        
        cap.release()


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("--dataset_dir", type=str, default='/nethome/cvnas01/datasets/youtubeVR360/', help='dataset dir')  
    parser.add_argument("--split", type=str, default='train', help='train / test')   
    parser.add_argument('--num', default=10, type=int, help='number of samples per video')
    parser.add_argument('--max_dim', default=320, type=int, help='sample every n frames')
    args = parser.parse_args()
    
    collect(args.dataset_dir, args.split, args.num, args.max_dim)
