import os
import sys
import tempfile
#import heapq

class heapnode:
    def __init__(self, item, fileHandler):
        self.item = item
        self.fileHandler = fileHandler

def heapify(list, mid, length):
    left = 2 * mid + 1
    right = 2 * mid + 2
    if left < length and list[left].item < list[mid].item:
        smallest = left
    else :
        smallest = mid
    if right < length and list[right].item < list[smallest].item:
        smallest = right
    if mid != smallest:
        (list[mid], list[smallest]) = (list[smallest], list[mid])
        heapify(list, smallest, length)

def construct_heap(list):
    length = len(list)
    mid = length / 2
    while(mid >= 0):
        heapify(list, mid, length)
        mid -= 1

#initialize
sortedTempFileHandlerList = []
cwd = os.getcwd()
print cwd
#the split file operations
#largeFileName = 'largefile'
#smallFileSize = 10

largeFileName = "input.txt"
smallFileSize = 1000


from random import randint
f=open(largeFileName,"w")
k=0
while True:
    n=randint(0, smallFileSize)
    f.write(str(n)+"\n")
    k=k+1
    if k==100:
        f.close()
        break

import mmap
with open(largeFileName, "r+b") as f:
    largeFileHandler = mmap.mmap(f.fileno(), 0)
    #largeFileHandler.close()
    #largeFileHandler = open(largeFileName) #to open the file withhout mmap, not needed anymore
    tempBuffer = []
    size = 0
    while True:
        number = largeFileHandler.readline()
        if not number:
            break
        tempBuffer.append(number)
        size += 1
        if size % smallFileSize == 0:
        	tempBuffer = sorted(tempBuffer, key = lambda no: int(no.strip()))
        	tempFile = tempfile.NamedTemporaryFile(dir = cwd + '/temp-files-python', delete = False)
        	tempFile.writelines(tempBuffer)
        	tempFile.seek(0)
        	sortedTempFileHandlerList.append(tempFile)
        	tempBuffer = []
    largeFileHandler.close()

    #merge sort
    list = []
    sorted_output = []
    for tempFileHandler in sortedTempFileHandlerList:
    	item = int(tempFileHandler.readline().strip())
    	list.append(heapnode(item, tempFileHandler))

    construct_heap(list)
    file = open('output.txt', 'w')
    file.close()
    while True:
        min = list[0]
        if min.item == sys.maxint:
            break
        sorted_output.append(min.item)
        fileHandler = min.fileHandler
        item = fileHandler.readline().strip()
        if not item:
            item = sys.maxint
        else:
            item = int(item)
        list[0] = heapnode(item, fileHandler)
        heapify(list, 0, len(list))
    for no in sorted_output:
        file = open('output.txt', 'a')
        file.write(str(no) + '\n')
        file.close()
'''
file = open('sortedCompleteData.txt', 'w')
file.close()
mergedNo = (map(int, tempFileHandler) for tempFileHandler in sortedTempFileHandlerList)
sortedCompleteData = heapq.merge( *mergedNo)
#for no in sorted_output:
for no in sortedCompleteData:
    file = open('sortedCompleteData.txt', 'a')
    file.write(str(no) + '\n')
    file.close()
    #largeFileHandler.close() #to close the mmap that was created earlier
'''