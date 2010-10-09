import os
import errno
import codecs

def writeFile(filename, contents):
    out = codecs.open(filename, encoding='utf-8', mode='w+')
    out.write(contents)
    out.close()

def readFile(filename):
    fileHandle = codecs.open(filename, encoding='utf-8')
    fileContents = unicode(fileHandle.read())
    fileHandle.close()
    return fileContents

def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc:
        if exc.errno == errno.EEXIST:
            pass
        else: raise