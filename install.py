#!/usr/bin/env python

import sys, os, traceback
from time import sleep
import time
import json
from datetime import datetime, timedelta
import re
from subprocess import call
import signal
import select # for select.select
import inspect
import common


EOL = "\r\n"

def signal_handler(signal, frame):
    print "You pressed exit"
    exit(0)

def cleanAndExit():
    sys.exit(0)

# signal handler to handle Ctrl+C and exit server
signal.signal(signal.SIGINT, signal_handler)


# frameAmount is needed by videoClarity, it need to matches the clip length, or smaller if a shorter test is wanted.
def main():
    global HTML, allRows, makito_prompt
    
    beginTime = time.time()
    
    call(["cp", "showip.so", "/usr/lib/arm-linux-gnueabihf/lxpanel/plugins/"])
   
    CONFIG = None
    PATH = "../../.config/lxpanel/LXDE-pi/panels/panel"
    try:
        # 
        CONFIG = open(PATH, 'r').read() 
    except:
        print '' # no problem yet
    
    if CONFIG == None:
        try:
            PATH = "../.config/lxpanel/LXDE-pi/panels/panel"
            CONFIG = open(PATH, 'r').read() 
        except:
            print "Failed to open LXDE-pi/panels/panel"; 
            return -1
    
    MYPLUGIN = 'Plugin {\n  type=showip\n  Config {\n  }\n}'
    index = CONFIG.find(MYPLUGIN)
    if index >= 0:
        print 'Already done'
        return 0
    
    CONFIG = common.ReplaceBetween(CONFIG, 'Plugin {\n  type=volumealsa\n  Config {\n  }\n}\n', 'Plugin', MYPLUGIN+"\n")
    
    f = open(PATH, "w")
    f.write(CONFIG)
    f.close()
     
    call(["lxpanelctl", "restart"])
     
    # produce report
    elapsedTime = time.time() - beginTime
    print "[FINISH] elapsedTime:" + str(elapsedTime)  

if __name__ == '__main__':
    main()
