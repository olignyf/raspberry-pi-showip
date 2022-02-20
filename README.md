# ShowIP
Lxpanel plugin to show eth0/wlan0 IP address on top bar

# Installation
```
$> sudo apt-get --assume-yes install libfm-dev libfm-gtk-dev libgtk2.0-dev lxpanel-dev
$> git clone https://github.com/olignyf/raspberry-pi-showip.git
$> cd raspberry-pi-showip
$> make
$> sudo python install.py
```
# Manual Installation 

Compile source code (directly on Raspberry Pi) => showip.so is produced

## Raspberry 4
Paste it in /usr/lib/aarch64-linux-gnu/lxpanel/plugins/showip.so 
```
$> sudo cp showip.so /usr/lib/aarch64-linux-gnu/lxpanel/plugins/
```
## Raspberry 3
Paste it in /usr/lib/arm-linux-gnueabihf/lxpanel/plugins/showip.so 
```
$> sudo cp showip.so /usr/lib/arm-linux-gnueabihf/lxpanel/plugins/
```

## Restart lxpanel
Restart lxpanel with the following under X, or reboot:
```
%> sudo lxpanelctl restart
```

But the panel won't appear yet, you have to add it in place by right-clicking on the top bar.
Programmatically you can edit ~/.config/lxpanel/LXDE-pi/panels
There is a python install script which may work in your case:
```
$> sudo python install.py
```

# Troubleshooting
```
$> cat /home/pi/.cache/lxsession/LXDE-pi/run.log
```

# pisetup
showip is also part of pisetup
https://github.com/olignyf/pisetup

# Developpers Documentation
https://wiki.lxde.org/en/How_to_write_plugins_for_LXPanel

# License
5$US fee for commercial use per unit using.
20$US one-time fee for code reuse.
