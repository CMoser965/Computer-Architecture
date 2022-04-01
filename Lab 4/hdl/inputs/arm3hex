#!/usr/bin/python

from __future__ import print_function
import os, subprocess, sys
from optparse import OptionParser

parser = OptionParser(usage="usage: %prog /path/to/arm_asm input [output]")
(options,args) = parser.parse_args()

if len(args)!=2 and len(args)!=3:
    parser.error("Wrong number of args.")

arm_asm = args[0]
arm_file = args[1]

current_dir = os.getcwd() + '/'
file_path = current_dir + arm_file
file_exists = os.path.isfile(file_path)

if not file_exists:
    print('File not found.')
    sys.exit(1)

out_file_name = file_path.split('/')[-1].split('.')[0] + '.x'

p = subprocess.Popen([arm_asm, "-aln", "-mbig-endian", "-W", file_path, "-o4243temp.out"], stdout=subprocess.PIPE)

out, err = p.communicate()

os.remove(current_dir + '4243temp.out')
#q = subprocess.Popen(["rm", current_dir + '4243temp.out'], stderr=subprocess.PIPE)
#q.communicate()

hex_out = ''

for lines in out.decode("utf-8").split('\n'):
    current_line = lines.split('\t')[0].strip().split(' ')
    if len(current_line) > 1:
        line_length = len(current_line[2])
        start = 0
        while line_length - start >= 2:
            hex_out += current_line[2][start:start+2] + '\n'
            start += 2
            #hex_out += current_line[2] + '\n'

if len(args)==3:
  out_file_name=args[2]
else:
  out_file_name = file_path[:len(file_path)-len(file_path.split('/')[-1])] + out_file_name

out_file = open(out_file_name,'w+')
out_file.write(hex_out)
out_file.close()


