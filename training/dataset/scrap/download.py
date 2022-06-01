"""
Download youtube videos
The format is fixed to 134.

"""

import os
import csv
import argparse
from tqdm import tqdm

parser = argparse.ArgumentParser()
parser.add_argument("--csv", type=str, 
    default='/nethome/cvnas01/datasets/youtube_scrap/ytsearchall_vr_360.csv', 
    help='csv video link, label list')
parser.add_argument("--output_dir", type=str, 
    default='/nethome/cvnas01/datasets/youtube_scrap/', 
    help='output dir')
parser.add_argument('--class_list', nargs='+', default=['mono', 'stereo', 'vr360', 'vr360_stereo'])
args = parser.parse_args()

for class_name in args.class_list:
    os.makedirs(os.path.join(args.output_dir, class_name), exist_ok=True)

with open(args.csv) as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=',')
    line_count = 0
    for row in csv_reader:
        if line_count > 0 and len(row[1]) > 0:
            url = row[0]
            yid = url.replace('https://www.youtube.com/watch?v=', '')
            
            label = int(row[1])
            if label < 0:
                continue

            cmd = ['youtube-dl', '--ignore-errors', 
                '--download-archive', '"{}"'.format(os.path.join(args.output_dir, 'downloaded_video.txt')), 
                "--user-agent \"\"",
                '--format', '134', 
                '-o', '"{}"'.format(
                    os.path.join(args.output_dir, args.class_list[label], yid+".mp4") ),
                '"{}"'.format(url)]
            os.system(' '.join(cmd))           

        line_count += 1
