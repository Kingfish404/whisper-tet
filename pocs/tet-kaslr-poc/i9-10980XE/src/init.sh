#!/bin/bash
mkdir -p third_party
cd third_party
git clone https://github.com/misc0110/PTEditor
cd PTEditor
sudo insmod module/pteditor.ko
