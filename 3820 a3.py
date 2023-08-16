# -*- coding: utf-8 -*-
"""
COMP3820 A3
Xing Zhou 7869781
This program is based on the assumption that:
each non-leaf node in the tree has exactly 2 children 
"""

import sys
from Bio import Phylo
from Bio import SeqIO

treeFile=sys.argv[1]
seqFile=sys.argv[2]
tree=Phylo.read(treeFile, "newick") #read the tree
record=list(SeqIO.parse(seqFile, "fasta")) #read the sequences
alphabet="ACGT"
transition=["AG", "GA", "CT", "TC"] #used to build the subsititute matrix
outcome=["A:", "C:", "G:", "T:"] #store the outcome

#build the substitute matrix
subMatrix={}
for i in range(len(alphabet)):
    for j in range(len(alphabet)):
        key=alphabet[i]+alphabet[j]
        if alphabet[i]==alphabet[j]:
            subMatrix[key]=0
        else:
            if key in transition:
                subMatrix[key]=1
            else:
                subMatrix[key]=2

#build the table at each node for calculation
#the first 4 elements are for ACGT score
#the 5th element is to control whether this table is updated to valid in run time
#the 6th/last element is only for leaves to hold the sequences letter ACGT
nodeList=tree.get_terminals()+tree.get_nonterminals()
nodeDict={}
for i in range(len(nodeList)):
    nodeDict[nodeList[i].name]=[-1,-1, -1, -1, -1, -1]

#calculate the score in the table of each node
def CalScore(parent, children, bottom):
    left=[-1, -1, -1, -1] #temp table for left child
    right=[-1, -1, -1, -1] #temp table for right child
    temp=[-1, -1, -1, -1] # 1 more temp table to hold substitute score
    pair=0 #for the key to search substitute matrix
    if(bottom): #if child node is leaf
        for i in range(len(alphabet)):
            pair=alphabet[i]+nodeDict[children[0]][5] #use the 6th sequence letter in the leaf
            left[i]=subMatrix[pair]
            pair=alphabet[i]+nodeDict[children[1]][5]
            right[i]=subMatrix[pair]
            nodeDict[parent][i]=left[i]+right[i]
        nodeDict[parent][4]=True
    else:
        for i in range(len(alphabet)):
            for j in range(len(alphabet)):
                pair=alphabet[i]+alphabet[j]
                temp[j]=subMatrix[pair] #the substitute score at the node
                left[j]=temp[j]+nodeDict[children[0]][j] #add the corresponding score in the child
                right[j]=temp[j]+nodeDict[children[1]][j]
            nodeDict[parent][i]=min(left)+min(right) #to min together to be the score in this node
        nodeDict[parent][4]=True

#recursive method to go down the tree and update the score table
def GoDown(node, pList):
    bottom=True #control whether or not leaf
    parent=0 #the parent parameter for CalScore method
    pList.append(node) #record the past path until current node
    cList=[] #the chirdren parameter for CalScore method
    for child in node: #start from root
        if(not nodeDict[child.name][4]): #if child score table is not valid at the beginning
            bottom=False #it must be internal node
            node=child
            GoDown(node, pList) #go down the tree to update relevent score table
        assert nodeDict[child.name][4]==True #make sure child score table is valid
        cList.append(child.name) #add the proper child
    if(bottom): #now we have proper children
        parent=node #if children are leaves, node is not changed recursively, parent is the node
        pList.remove(pList[len(pList)-1]) #this node table is done, delete it from the path
    else:
        parent=pList[len(pList)-1] #parent node has changed due to the recursion
        pList.remove(pList[len(pList)-1]) #so we use path to determine the right parent
    CalScore(parent.name, cList, bottom) #proper children and parent to update parent's score table

# the main
for i in range(len(record[0].seq)):
    index=-1
    
    #clean all score tables for a new iteration
    for k in range(len(nodeList)):
        nodeDict[nodeList[k].name][4]=False
        for w in range(len(alphabet)):
            nodeDict[nodeList[k].name][w]=sys.maxsize
            
    #initialize the leaf node information including initialize the leaf score table 
    for j in range(len(record)):
        nodeDict[record[j].name][4]=True
        nodeDict[record[j].name][5]=record[j].seq[i] #read and update the sequence letter in leaf
        index=alphabet.index(record[j].seq[i]) #the score of sequence letter should be 0
        nodeDict[record[j].name][index]=0 #others are all max int
    node=tree.root #then we can begin a new iteration
    parentList=[] #path list for the go down method
    GoDown(node, parentList)
    for x in range(len(outcome)): #record the outcome before next iteration
        outcome[x]+=" "
        outcome[x]+=str(nodeDict[tree.root.name][x])

#print the outcome        
for i in range(len(outcome)):
    print(outcome[i])
