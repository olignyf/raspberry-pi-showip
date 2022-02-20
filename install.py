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
    print ("You pressed exit")
    exit(0)

def cleanAndExit():
    sys.exit(0)

# signal handler to handle Ctrl+C and exit server
signal.signal(signal.SIGINT, signal_handler)


# frameAmount is needed by videoClarity, it need to matches the clip length, or smaller if a shorter test is wanted.
def main():
    global HTML, allRows, makito_prompt
    
    beginTime = time.time()
    
    # old pi3 call(["cp", "showip.so", "/usr/lib/arm-linux-gnueabihf/lxpanel/plugins/"])
    call(["cp", "showip.so", "/usr/lib/aarch64-linux-gnu/lxpanel/plugins/"])
    print('after copying');
    CONFIG = None
    PATH = "/etc/xdg/lxpanel/LXDE/panels/panel"
    try:
        # 
        CONFIG = open(PATH, 'r').read() 
    except:
        print('FIRST FAIL OPENING CONFIG') # no problem yet
    print('after first read');
    print(CONFIG);
    if CONFIG == None:
        try:
            PATH = "/etc/xdg/lxpanel/LXDE-pi/panels/panel"
            CONFIG = open(PATH, 'r').read() 
        except:
            print ("Failed to open LXDE-pi/panels/panel"); 
            return -1
    
    MYPLUGIN = 'Plugin {\n  type=showip\n  Config {\n  }\n}'
    index = CONFIG.find(MYPLUGIN)
    if index >= 0:
        print ('Already done')
        return 0
    print(CONFIG)
    CONFIG = common.ReplaceBetween(CONFIG, 'VolumeUpKey = XF86AudioRaiseVolume\n    }\n}\n', 'Plugin {\n    type = tray\n}\n', MYPLUGIN+"\n")
    
    if (CONFIG == '' or CONFIG == None):
        print('ERROR: failed to parse config');
        exit(1);
    
    f = open(PATH, "w")
    f.write(CONFIG)
    f.close()
     
    call(["lxpanelctl", "restart"])
     
    # produce report
    elapsedTime = time.time() - beginTime
    print ("[FINISH] elapsedTime:" + str(elapsedTime)  )

if __name__ == '__main__':
    main()
