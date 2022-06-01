# Leia Image Layout Autodetection Model Training

## Installation
TODO

## Dataset generation from Holopix50k dataset
1. Download Holopix50k dataset from [https://github.com/LeiaInc/holopix50k](https://github.com/LeiaInc/holopix50k), or the dataset has been already downloaded in ```/cvnas01/datasets/Holopix50k/```
2. Render 2x1, 2x1 half, 1x2 images from Holopix50k dataset    
    - run ```python dataset/holopix50k/render_sbs_images.py -d /cvnas01/datasets/Holopix50k/test/ -o /cvnas01/datasets/Holopix50k/layout_detection/test/ --max_dim 320```
    - This will generate 2x1 and 1x2 images under ```/cvnas01/datasets/Holopix50k/layout_detection/test/```
    - To generate 2x1_half class images, add ```--sbs_half``` option.
    - To generate images with random aspect ratio, add ```-a``` option.

3. Render 2x2 quad images from Holopix50k dataset
    - Build android_multiview app from [https://github.com/LeiaInc/android-multiview](https://github.com/LeiaInc/android-multiview)    
    - run ```python dataset/holopix50l/run_multiviewlib.py -m /android-multiview/lib/build/MultiviewTest -l /cvnas01/datasets/Holopix50k/layout_detection/test/left/ -r /cvnas01/datasets/Holopix50k/layout_detection/test/right/ -o /cvnas01/datasets/Holopix50k/layout_detection/test/quad```

4. The generate images are under this directory structure
    ```
       |- path/to/output_dir/
       |    |-left_/ 
       |    |-right_/
       |    |-left_right/  # 2x1
       |    |-left_right_half/  # 2x1 half
       |    |-top_bottom/  # 1x2 
       |    |-quad/  # 2x2 
    ```          

5. Write a list of image file paths and corresponding class labels to a csv file    
    - run ```python dataset/write_dataset.py -d /cvnas01/datasets/Holopix50k/layout_detection/test/ -c dataset.csv --class_type {0 or 1 or 2}```
    - ```--class_type``` selects differnt combination of image layout types. ```0: [mono, 2x1, 2x1 half, 1x2, quad], 1: [mono, 2x1, 1x2, quad], 2: [mono, 2x1 half]```    
    - ```/cvnas01/datasets/Holopix50k/layout_detection/test/dataset.csv``` will be generated.
    
## Dataset generation from [WSVD dataset](https://sites.google.com/view/wsvd/home)
1. Follow the instruction in the WSVD dataset webpage to download all youtube video files and the frame ID list files, or the dataset has been already downloaded in ```/cvnas01/datasets/WSVD/```
2. All the files shoud be in the following directory structure.
    ```
       |- /cvnas01/datasets/WSVD/     # e.g. a path where your dataset is located
       |    |-wsvd/  # all downloaded youtube videos
       |        |-video1.mp4
       |        |-video2.mp4 
       |            :
       |    |-wsvd_test_clip_frame_ids.pkl     # list of the test video frames 
       |    |-wsvd_train_clip_frame_ids.pkl    # list of the train video frames  
    ``` 
2. Extract image frames from video clips    
    - run ```python video/video2images.py --frame_id /cvnas01/datasets/WSVD/wsvd_train_clip_frame_ids.pkl --video_dir /cvnas01/datasets/WSVD/wsvd --step 300 --max_dim 320 --output_dir /cvnas01/datasets/WSVD/train```
    - this will extract video frames at every other 300 frames from all ```test``` sections marked by ```wsvd_train_clip_frame_ids.pkl``` frame ID file.
    - all video frames will be resized by ```320-width```.
3. Render 2x2 quad images
    - Build android_multiview app from [https://github.com/LeiaInc/android-multiview](https://github.com/LeiaInc/android-multiview)    
    - run ```python dataset/holopix50l/run_multiviewlib.py -m /android-multiview/lib/build/MultiviewTest -l /cvnas01/datasets/WSVD/train/left/ -r /cvnas01/datasets/WSVD/train/right/ -o /cvnas01/datasets/WSVD/train/quad```
4. You can find the generated images at:
    ```
       |- /cvnas01/datasets/WSVD/train/
       |    |-left_/ 
       |    |-right_/
       |    |-left_right/  # 2x1
       |    |-left_right_half/  # 2x1 half
       |    |-top_bottom/  # 1x2        
    ```              
5. Write a list of image file paths and corresponding class labels to a csv file    
    - run ```python dataset/write_dataset.py -d /cvnas01/datasets/WSVD/train/ -c dataset.csv --class_type {0 or 1 or 2}```
    - ```--class_type``` selects differnt combination of image layout types. ```0: [mono, 2x1, 2x1 half, 1x2, quad], 1: [mono, 2x1, 1x2, quad], 2: [mono, 2x1 half]```    
    - ```/cvnas01/datasets/WSVD/train/dataset.csv``` will be generated.

## Dataset generation from YouTube 360 video dataset
1. Follow the instruction in [this webpage](https://github.com/pedro-morgado/spatialaudiogen) to download YouTube 360 mono / stereo videos, or the dataset has been already downloaded in ```/cvnas01/datasets/youtubeVR360/```
2. Extract video frames
    - Run ```python dataset/video360/data_collection.py --dataset_dir /cvnas01/datasets/youtubeVR360/ --split {train or test} --num 10 --max_dim 320```
    - This will generate ```--num``` frames per video and stored the frames in ```mono``` and ```stereo``` folders. 
    - You can find the generated images at:
    ```
       |- /cvnas01/datasets/youtubeVR360/data/{train or test}
       |    |-mono/ 
       |    |-stereo/       
    ```              
3. Write a list of image file paths and corresponding class labels to a csv file    
    - run ```python dataset/video360/write_dataset.py -d /cvnas01/datasets/youtubeVR360/data/{train or test} -c dataset.csv```
    - You can find the generated csv file in ```/cvnas01/datasets/youtubeVR360/data/{train or test}/dataset.csv```.

