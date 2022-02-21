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
    
    # pi3
    if os.path.isdir('/usr/lib/aarch64-linux-gnu/lxpanel/plugins/'):
        call(["cp", "showip.so", "/usr/lib/aarch64-linux-gnu/lxpanel/plugins/"])
    else:
        call(["cp", "showip.so", "/usr/lib/arm-linux-gnueabihf/lxpanel/plugins/"])
    print('after copying');
    CONFIG = None
    
    PATH = "/home/pi/.config/lxpanel/LXDE-pi/panels/panel"
    if not os.path.isfile(PATH):
        print('Selecting global panel config')
        PATH = '/etc/xdg/lxpanel/LXDE/panels/panel'
        
    try:
        # 
        CONFIG = open(PATH, 'r').read() 
    except:
        print('Failed opening lxpanel config') # no problem yet
        
    print('Config content:\n');
    print(CONFIG);
    
    if (CONFIG == '' or CONFIG == None):
        print('ERROR: failed to open lx panel config');
        exit(1);
    
        
    MYPLUGIN = 'Plugin {\n  type=showip\n  Config {\n  }\n}'
    index = CONFIG.find(MYPLUGIN)
    if index >= 0:
        print ('Already done')
        call(["lxpanelctl", "restart"])
        return 0
    print(CONFIG)
    TEST = common.ReplaceBetween(CONFIG, 'type=tray\n  Config {\n  }\n}\n', 'Plugin {\n', MYPLUGIN+"\n")
    
    if (TEST == '' or TEST == None):
        TEST = common.ReplaceBetween(CONFIG, 'VolumeUpKey = XF86AudioRaiseVolume\n    }\n}\n', 'Plugin {\n    type = tray\n}\n', MYPLUGIN+"\n")
    
    if (TEST == '' or TEST == None):
        print('ERROR: failed to parse config');
        exit(1);

    CONFIG = TEST
    
    f = open(PATH, "w")
    f.write(CONFIG)
    f.close()
     
    call(["lxpanelctl", "restart"])
     
    # produce report
    elapsedTime = time.time() - beginTime
    print ("[FINISHED] elapsedTime:" + str(elapsedTime)  )

if __name__ == '__main__':
    main()
