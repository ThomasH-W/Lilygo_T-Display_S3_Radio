#
# file: TFT_eSPI_cp-user-files.py
#
# copy  User_Setup_Select.h User_Setup.h 
# from  ./README/TFT_eSPI
# to    .pio/lipdeps/lilygo-t-display-s3/TFT_eSPI
#
#   cp ./README/TFT_eSPI/* .pio/lipdeps/lilygo-t-display-s3/TFT_eSPI
#
print
print("---------------------------------------------------- TFT_eSPI_cp-user-files.py")
print("cp user setup files for library TFT_eSPI")
print

src_dir="./README/TFT_eSPI/"
file1=src_dir + "User_Setup_Select.h"
file2=src_dir + "User_Setup.h"
dest_dir1="./.pio/libdeps/"
dest_dir2="/TFT_eSPI/"

import shutil
import os

Import("env")
x=env['PIOENV']
print (x)

dest_dir=dest_dir1 + x + dest_dir2
print ("dest_dir: " + dest_dir)

def cpFile(file):
    try:
        with open(file, 'r') as f:
            print(file + " ok")
            shutil.copy(file, dest_dir)
    except FileNotFoundError:
        print("### ERR: file not found: " + file )

if not os.path.isdir(dest_dir):
    raise ValueError("### ERR Invalid directory!" + dest_dir)

cpFile(file1)
cpFile(file2)

print
print("---------------------------------------------------- done")