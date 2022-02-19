import select, sys
import time
from time import sleep
from datetime import datetime, timedelta
import inspect
import re


EOL = "\r\n"

def ExtractBetween(source, start, end):
    index = source.find(start)
    if index >= 0:
        result = source[index+len(start):]
        index = result.find(end)
        if index >= 0:
            return result[0:index]
    
    return None


def ReplaceBetween(source, start, end, replacement):
    i_start = source.find(start)
    if i_start >= 0:
        trailing = source[i_start+len(start):]
        i_end = trailing.find(end)
        print ('i_end:'+str(i_end))
        if i_end >= 0:
            print ('end:' + trailing[i_end:])
            return source[0:i_start+len(start)] + replacement + trailing[i_end:]
    
    return None



