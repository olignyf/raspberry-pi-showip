# raspberry-showip
Lxpanel plugin to show eth0 IP address on top bar

# Installation
```
$> sudo apt-get --assume-yes install libfm-dev libfm-gtk-dev
$> git clone https://github.com/olignyf/raspberry-pi-showip.git
$> cd raspberry-pi-showip
$> make
$> sudo python install.py
```
# Manual Installation 
Compile source code (directly on Raspberry Pi) => showip.so is produced
Paste it in /usr/lib/arm-linux-gnueabihf/lxpanel/plugins/showip.so
Restart lxpanel with the following under X, or reboot:
```
%> lxpanelctl restart
```
But the panel won't appear yet, you have to add it in place by right-clicking on the top bar.
Programatically you can edit ~/.config/lxpanel/LXDE-pi/panels

# pisetup
showip is also part of pisetup
https://github.com/olignyf/pisetup

# Developpers Documentation
https://wiki.lxde.org/en/How_to_write_plugins_for_LXPanel

# Some music to keep you entertained
https://youtu.be/RarDSAz_jGs

# License
Free for educational use.
5$ fee for redistribution and/or commercial use.
