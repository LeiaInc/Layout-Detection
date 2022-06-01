"""
Convert the youtube ids to youtube links
"""

import os

src_file = '/nethome/cvnas01/datasets/youtubeVR360/scraping/vr_360.csv'
dup_check_ref_file = '/nethome/cvnas01/datasets/youtubeVR360/scraping/vr_360_stereo.csv'
dst_file = '/nethome/cvnas01/datasets/youtubeVR360/scraping/vr_360_link.csv'


ids = []
with open(src_file, 'r') as fp:
    ids += fp.readlines()

dup_check_ref_ids = []
with open(dup_check_ref_file, 'r') as fp:
    dup_check_ref_ids += fp.readlines()

dup_removed = []
for id in ids:
    if id in dup_check_ref_ids:
        continue
    else:
        dup_removed.append(id)
ids = dup_removed

links = []
for id in ids:
    links.append( 'https://www.youtube.com/watch?v=' + id )

with open(dst_file, 'w') as fp:
    fp.writelines(links)
    
    






