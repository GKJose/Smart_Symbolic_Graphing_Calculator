#!/bin/bash
sudo systemctl start systemd-timesyncd
sudo systemctl start dhcpcd.service
sudo systemctl start ssh.service
sudo systemctl start networking.service
fbcp &
con2fbmap 1 0 &
gpio mode 5 out &
tput civis &
setterm -blink off &
fbcon=map:1 &
./home/pi/Smart_Symbolic_Graphing_Calculator/build/out/SSGC
