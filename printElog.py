import sys

from os import listdir
from os.path import isfile, join, isdir, exists

import os.path, time


print "This is the name of the script: ", sys.argv[0]
print "Number of arguments: ", len(sys.argv)
print "The arguments are: " , str(sys.argv)

mypath=str(sys.argv[1])

print "My path is ", mypath

onlydirs = [f for f in listdir(mypath) if isdir(join(mypath, f))]
onlydirs.sort()

for dir in onlydirs:
    filename=mypath+'/'+dir+'/Elog.txt'
    #    print filename
    if exists(filename):
        f = open (filename, 'r')
        file_contents = f.readline()
        file_contents.rstrip()
        creation_time=time.ctime(os.path.getctime(filename))
        sys.stdout.write(creation_time+" , "+dir+" , "+file_contents)
        f.close()
    else:
        print dir+" : No elog"

