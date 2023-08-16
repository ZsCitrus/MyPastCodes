# -*- coding: utf-8 -*-
"""
COMP3820 A1
Xing Zhou 7869781
"""

import sys
import time
from collections import OrderedDict

fasta=sys.argv[1]
mode=int(sys.argv[3])
pattern=sys.argv[2]
alphabet="ACGT"
aLen=len(alphabet)
pLen=len(pattern)
text=""
preForm=OrderedDict()
header=""

r=open(fasta,'r')
for line in r:
    if line[0]=='>':
        header=line.rstrip() #get rid of the header
    else:
        s=line.rstrip()
        text+=s #combine all lines as one sequence
r.close()

# preprocess of 1 char table
def PreOneChar(ol):
    for i in range(aLen):
        ol[alphabet[i]]=pLen # initialize as the length of the pattern
    keyList=list(ol)
    for i in range(1, pLen, 1): # update the preprocess table from right to left
        for j in range(len(ol)):
            if pattern[pLen-1-i]==keyList[j] and i<ol[keyList[j]]:
                ol[keyList[j]]=i

# preprocess of 3 char table
def PreThreeChar(ol):
    for i in range(aLen):
        for j in range(aLen):
            for k in range(aLen):
                key=alphabet[i]+alphabet[j]+alphabet[k] # enumerate all char combination
                ol[key]=pLen # initialize as the length of the pattern
                if key[1]==pattern[0]: # -1 for first char in the pattern
                    ol[key]=pLen-1
                if key[2]==pattern[1]: # -2 for second char in the pattern
                    ol[key]=pLen-2
    keyList=list(ol)
    for i in range(pLen-3): # update the preprocess table from left to right
        sub=pattern[i:i+3] # 3 char
        for j in range(len(ol)):    
            if sub==keyList[j] and pLen-(i+3)<ol[keyList[j]]:
                ol[keyList[j]]=pLen-(i+3)

# the search function
def Search(mode, ol):
    num=0
    index=pLen-1
    shift=0
    match=0
    mis=False
    while shift+index<len(text): #while within the sequence
        if mode==1:
            anchor=text[shift+index] #record the last char/triple of chars
        if mode==2:
            anchor=text[shift+index-2:shift+index+1]
        while match<pLen and mis==False: #compare the text with pattern right to left
            if pattern[index]==text[shift+index]:
                match+=1
                index-=1
            else:
                mis=True
        if match==pLen: # if every char the same, occurrence plus 1
            num+=1
        shift+=ol[anchor] #shift the patter along the text
        index=pLen-1 #clean relevant variables for next turn of comparison
        match=0
        mis=False
    if mode==1:
        print("BMH with 1 char search completed.")
    if mode==2:
        print("BMH with 3 char search completed.")
    print("Pattern %s was found %d times." % (pattern, num)) #print the output

# the real work is done here
start_time=time.time() # record time
if mode==1:
    PreOneChar(preForm) # preprocess table
    Search(1, preForm) # search
if mode==2:
    PreThreeChar(preForm)
    Search(2, preForm)
print("The search was completed in %0.4f seconds." % (time.time() - start_time))
