#!/bin/bash

sudo apt update && apt upgrade
sudo apt-get install inetutils-ping=2.0 -y
sudo apt install tcpdump -y
sudo apt install make -y
sudo apt install gcc -y
sudo apt install valgrind -y