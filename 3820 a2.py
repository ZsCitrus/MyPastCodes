# -*- coding: utf-8 -*-
"""
COMP3820 A2
Xing Zhou 7869781
"""

import sys
import time
from collections import OrderedDict
from Bio import SeqIO
from Bio.Align import substitution_matrices

input=sys.argv[1]
query=sys.argv[2]
k=int(sys.argv[3])
threshold=int(sys.argv[4])
exThreshold=int(sys.argv[5])
hsp=int(sys.argv[6])
alphabet="ARNDCQEGHILKMFPSTWYVBZX"
b62=substitution_matrices.load("BLOSUM62")
seq=list(SeqIO.parse(input, "fasta"))
kmer=[]
neighbor=[]
outcome=OrderedDict()
temp=[]
preForm=OrderedDict()

#get score bwtween query and other items
def Score(a, b):
    score=0
    for i in range(len(a)):
        x=alphabet.index(a[i])
        y=alphabet.index(b[i])
        score+=b62[x][y]
    return score


#to find proper neighbor by sustitude char one by one
#this method is different from permutation, but has the same output
#and it is faster, it is power function, while permutation is exponential function  
def SearchNei(a, l):
    r=[]
    for i in range(len(l)):
        for j in range(len(l[i])):
            if l[i][j]==a[j]:
                for w in range(len(alphabet)):
                    if alphabet[w]!=a[j]:
                        add=l[i][:j]+alphabet[w]+l[i][j+1:]
                        check=Score(a,add)
                        if check>=threshold:             
                            r.append(add)
    return r


#preprocess of 1 char table
def PreOneChar(ol, a):
    for i in range(len(alphabet)):
        ol[alphabet[i]]=len(a) #first kmer as pattern
    keyList=list(ol)
    for i in range(1, len(a), 1): #update the preprocess table from right to left
        for j in range(len(ol)):
            if a[len(a)-1-i]==keyList[j] and i<ol[keyList[j]]:
                ol[keyList[j]]=i


#BMH 1 char search function
def SearchSeq(ol, text, a):
    r=[]
    index=len(a)-1
    shift=0
    match=0
    mis=False
    while shift+index<len(text): #while within the sequence
        anchor=text[shift+index] #record the last char of chars
        while match<len(a) and mis==False: #compare the text with pattern right to left
            if a[index]==text[shift+index]:
                match+=1
                index-=1
            else:
                mis=True
        if match==len(a): # if every char the same, record the string head loaction
            r.append(shift)
        shift+=ol[anchor] #shift the patter along the text
        index=len(a)-1 #clean relevant variables for next turn of comparison
        match=0
        mis=False
    return r


#real work begin here
start_time=time.time()

#produce the kmer
for i in range(len(query)-(k-1)):
    s=query[i:i+k]
    kmer.append(s)

#use kmer[0] as seed to search in the database
PreOneChar(preForm, kmer[0])
qHead=query.index(kmer[0][0])

#to find all proper neighbor
for i in range(len(kmer)):
    compare=kmer[i]
    candidate=[compare]
    result=SearchNei(compare, candidate) #start by substitute 1 char
    while len(result)>0: #if something can be kept after 1 char replace
        neighbor+=result #record the neighbors found by now
        temp=result
        result=None
        #further replace 2 char from the kmer until the score too low and nothing can be kept
        result=SearchNei(compare, temp) 
      
neighbor+=kmer
neighbor.sort()

#search seed in the database and do the extension
for i in range(len(seq)):
    hit=SearchSeq(preForm, seq[i].seq, kmer[0])
    if len(hit)>0:
        for j in range(len(hit)):
            highScore=Score(kmer[0], kmer[0])
            w=1
            q=query[qHead:qHead+k+w]
            test=seq[i].seq[hit[j]:hit[j]+k+w]
            #in case seed is in the end of the database sequence
            #then actually nothing to do with extension
            #the while loop condition show this logic
            if len(q)==len(test): 
                newScore=Score(q, test)
            else:
                newScore=highScore
            #the "==" and "<=" condition ensure nothing go beyong any string bound
            while highScore-newScore<exThreshold and len(q)==len(test) and w<=len(query):
                if newScore>highScore:
                    highScore=newScore
                w+=1
                q=query[qHead:qHead+k+w]
                test=seq[i].seq[hit[j]:hit[j]+k+w]
                if len(q)==len(test):
                    newScore=Score(q, test)
            cut=w-1
            q=query[qHead:qHead+k+cut]
            test=seq[i].seq[hit[j]:hit[j]+k+cut]
            lastScore=Score(q, test)  
            if lastScore>=hsp:
                key=str(i)+"/"+str(j)
                outcome[key]=[i, hit[j], k+cut, lastScore]

#all search steps end here
end_time=time.time()


print("Kmers:")
print(kmer)
print("Neighborhood:")
print(neighbor)
outList=list(outcome)
for i in range(len(outcome)):
    print("\n")
    theScore=outcome[outList[i]][3]
    num=outcome[outList[i]][0] #the index of the sequences have the findings
    qString=query[qHead:qHead+outcome[outList[i]][2]]
    #slice the proper string segment by the index
    sString=seq[num].seq[outcome[outList[i]][1]:outcome[outList[i]][1]+outcome[outList[i]][2]]
    print("Found one match with score %d in sequence #%d:" % (theScore, num))
    print(seq[num].description)
    print(sString)
    print(qString)
print("\n")
print("In total, %d hits were found in the database." %(len(outcome)))
print("The search was completed in %0.4f seconds." %(end_time-start_time))
