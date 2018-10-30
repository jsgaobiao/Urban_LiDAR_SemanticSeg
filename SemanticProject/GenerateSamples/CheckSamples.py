# -*- coding: utf-8 -*-
"""
Created on Thu Mar 29 11:53:21 2018

@author: pku-m
"""
import os
import re
seglogfile = open('/home/pku-m/SemanticMap/Data/FineAnnotation/2-1/2-1-merged-new.log', 'r')
lines = seglogfile.readlines()
prid = {}
total_seeds = 0
for line in lines:
    if (line[0]=='p'):
        items = [s for s in re.split('[prid=,\r\n]',line) if len(s) >0]
        key = items[0]+'_'+items[4]
        total_seeds += int(items[3])
        prid[key] = int(items[3])
print 'num of seeds: ',total_seeds

datapath = "/home/pku-m/SemanticMap/Data/campus_2_1_samples"
first_level_dirs = os.listdir(datapath)

labels = set()
for folder in first_level_dirs:
    labels.add(int(folder.split('_')[1]))
print sorted(labels)

print "num of regionid: ", len(first_level_dirs)

total_samples = 0
for item in first_level_dirs:
    num = len(os.listdir(datapath+'/'+item))
    total_samples += num
    if not prid.has_key(item):
        print 'not have :',item
        
#    if prid.has_key(item) and prid[item] != num:
#        print 'seeds error :',item, num
print "num of samples: ", total_samples

folders = {}
for item in first_level_dirs:
    folders[item] = 1

errorfile = open('error.txt', 'w')
untracked_label = {}
for key,item in prid.items():
    if not folders.has_key(key):
        errorfile.write(key+'\n')
        label = int(key.split('_')[1])
        if not untracked_label.has_key(label):
            untracked_label[label] = []
        untracked_label[label].append(key)
        
        #print key
for key, value in untracked_label.items():
    print key, len(value)
#print unique_untracked_label
errorfile.close()