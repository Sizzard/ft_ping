#!/bin/bash

sudo apt update && apt upgrade
sudo wget http://ftp.fr.debian.org/debian/pool/main/i/inetutils/inetutils-ping_2.0-1+deb11u2_amd64.deb
sudo apt install ./inetutils-ping_2.0-1+deb11u2_amd64.deb -y
sudo apt install tcpdump -y
sudo apt install make -y
sudo apt install gcc -y
sudo apt install valgrind -y