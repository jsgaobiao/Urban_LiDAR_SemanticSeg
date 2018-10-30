import cv2
import os
import numpy as np
import matplotlib.pyplot as plot
import matplotlib.image as mpimg
import pdb

PATH = '/home/gaobiao/Documents/SemanticSeg_IV2019/results/1014_segloss/vis/'
NAME = '1014_segloss'
IMAGE_WIDTH = 1080
IMAGE_HEIGHT = 32
VIS_IMAGE_HEIGHT = 144
NUM_OF_CLASSESS = 7

def ColorMap(data, img):
    if img[1] == 0:         # intensity == 0, invalid data
        data[:] = [img[0], img[0], img[0]]
        return data

    if data[0] == 1:        # people
        data = [0, 0,255]
    elif data[0] == 2:      # car
        data = [255,0,0]
    elif data[0] == 3:      # tree
        data = [0,255,0]
    elif data[0] == 4:      # sign
        data = [255,0,255]
    elif data[0] == 5:      # building
        data = [255,255,0]
    # elif data[0] == 6:      # cyclist
    #     data = [0,128,255]
    # elif data[0] == 7:      # stop bicycle
    #     data = [128,64,0]
    elif data[0] == 6:      # road
        data = [208,149,117]
    else:
        data[:] = [img[0], img[0], img[0]]
    return data

def LabelColor(img, gt, pre):
    table = np.zeros((NUM_OF_CLASSESS, NUM_OF_CLASSESS))
    height, width, channel = gt.shape
    for i in range(height):
        for j in range(width):
            # caculate accuracy
            table[gt[i,j,0]][pre[i,j,0]] += 1
            # Label color of gt
            gt[i,j] = ColorMap(gt[i,j], img[i,j])
            # Label color of prediction
            pre[i,j] = ColorMap(pre[i,j], img[i,j])
    return table

def OutputResult(table):
    fout = open(NAME+'.result', 'w')
    for i in range(NUM_OF_CLASSESS):
        for j in range(NUM_OF_CLASSESS):
            fout.write('%d\t' % table[i,j])
        fout.write('\n')
    fout.write('---------------------------\n')
    fout.write('label\ttp\tfp\tfn\tIoU\tmPA\n')
    for i in range(1,NUM_OF_CLASSESS):
        tp = fp = fn = cn = 0
        tp = table[i,i].astype(int)
        for j in range(1, NUM_OF_CLASSESS):
            if i != j:
                fp += table[j,i].astype(int)
                fn += table[i,j].astype(int)
        if (tp + fp + fn != 0):
            IoU = float(tp) / float(tp + fp + fn)
        else:
            IoU = 0
        fout.write('%d\t%d\t%d\t%d\t%.6f\t%.6f\n' % (int(i), int(tp), int(fp), int(fn), float(IoU), float(tp)/float(tp+fp)))
    fout.close()

listName = os.listdir(PATH)
imgList = []
gtList = []
preList = []
cntTable = np.zeros((NUM_OF_CLASSESS, NUM_OF_CLASSESS))

for name in listName:
    if name.split('/')[-1][0] == 'i':
        imgList.append(name)
    elif name.split('/')[-1][0] == 'g':
        gtList.append(name)
    elif name.split('/')[-1][0] == 'p':
        preList.append(name)

imgList.sort()
gtList.sort()
preList.sort()

videoWriter = cv2.VideoWriter(NAME+'.avi', cv2.cv.CV_FOURCC('M', 'J', 'P', 'G'), 5, (IMAGE_WIDTH, VIS_IMAGE_HEIGHT * 2), True)
#videoWriter = cv2.VideoWriter('test.avi', cv2.VideoWriter_fourcc(*'XVID'), 5, (IMAGE_WIDTH, IMAGE_HEIGHT * 2), True)

for i in range(len(imgList)):
    img = cv2.imread(PATH + imgList[i], cv2.IMREAD_COLOR)
    gt = cv2.imread(PATH + gtList[i], cv2.IMREAD_COLOR)
    pre = cv2.imread(PATH + preList[i], cv2.IMREAD_COLOR)
    table = LabelColor(img, gt, pre)
    cntTable += table
    mergeImg = np.concatenate((pre,gt), axis=0)
    mergeImg = cv2.resize(mergeImg, (IMAGE_WIDTH, VIS_IMAGE_HEIGHT * 2), interpolation=cv2.INTER_NEAREST)
    videoWriter.write(mergeImg)
    print('Frame: %d' % i)

OutputResult(cntTable)

videoWriter.release()
