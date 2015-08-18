#!/usr/bin/env python
# -*- coding: iso-8859-1 -*-
####################################################################################
#         ___        ___           ___           ___
#        /  /\      /  /\         /  /\         /  /\
#       /  /:/     /  /::\       /  /::\       /  /::\
#      /  /:/     /  /:/\:\     /  /:/\:\     /  /:/\:\
#     /  /:/     /  /:/~/:/    /  /:/~/::\   /  /:/~/:/
#    /  /::\    /__/:/ /:/___ /__/:/ /:/\:\ /__/:/ /:/
#   /__/:/\:\   \  \:\/:::::/ \  \:\/:/__\/ \  \:\/:/
#   \__\/  \:\   \  \::/~~~~   \  \::/       \  \::/
#        \  \:\   \  \:\        \  \:\        \  \:\
#         \  \ \   \  \:\        \  \:\        \  \:\
#          \__\/    \__\/         \__\/         \__\/
#
#   This file is part of TRAP.
#
#   TRAP is free software; you can redistribute it and/or modify
#   it under the terms of the GNU Lesser General Public License as published by
#   the Free Software Foundation; either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public License
#   along with this TRAP; if not, write to the
#   Free Software Foundation, Inc.,
#   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
#   or see <http://www.gnu.org/licenses/>.
#
#   (c) Luca Fossati, fossati@elet.polimi.it, fossati.l@gmail.com
#
####################################################################################

# This program executes all the specified benchmarks and it extracts, for
# each of them, the number of cycles executed, the number of insstructions,
# the number of load and store operations and the presence of load/store
# double. A part from the cycles executed, all the other information is
# taken from the profiling result.

from __future__ import division
from __future__ import print_function
from builtins import str
from past.utils import old_div
import sys, os, csv

commandLine = '--cycles_range main-_exit --profiler profOut --prof_range main-_exit -n'

load_instr = ['LDSB_imm', 'LDSB_reg', 'LDSH_imm', 'LDSH_reg', 'LDUB_imm', 'LDUB_reg', 'LDUH_imm', 'LDUH_reg', 'LD_imm', 'LD_reg', 'LDD_imm', 'LDD_reg', 'LDSBA_reg', 'LDSHA_reg', 'LDUBA_reg', 'LDUHA_reg', 'LDA_reg', 'LDDA_reg']
store_instr = ['STB_imm', 'STB_reg', 'STH_imm', 'STH_reg', 'ST_imm', 'ST_reg', 'STD_imm', 'STD_reg', 'STBA_reg', 'STHA_reg', 'STA_reg', 'STDA_reg']
double_instr = ['LDD_imm', 'LDD_reg', 'LDDA_reg', 'STD_imm', 'STD_reg', 'STDA_reg']

if __name__ == "__main__":

    print('\n')

    if len(sys.argv) < 3:
        raise Exception('Error, the command line for the benchmarking program is: processorExecutable benchmark1 benchmark2 ...')
    if not os.path.exists(sys.argv[1]):
        raise Exception('Error, processorExecutable ' + sys.argv[1] + ' not existing')

    fileHandle = open('timeValidation.txt', 'w')
    orderedBenchmarks = sorted(sys.argv[2:])
    for benchmark in orderedBenchmarks:
        if not os.path.exists(benchmark):
            raise Exception('Error, benchmark ' + benchmark + ' not existing')
        print('Executing benchmark ' + benchmark)
        result = os.popen(sys.argv[1] + ' ' + commandLine + ' -a ' + benchmark).readlines()
        # First lets read the program output
        if not 'Program exited with value 0\n' in result:
            fileHandle.write('Benchmark ' + benchmark + ' failed since it has wrong exit value\n')
            continue
        for res in result:
            if 'Cycles between addresses' in res:
                cycles = int(res.split()[6])
                break
        # and now the profiling result
        profResFile = open('profOut_instr.csv', 'r')
        profResFile.readline()
        profResReader = csv.reader(profResFile, delimiter = ';')
        load = 0
        store = 0
        doubleOps = 0
        for row in profResReader:
            if row[0] in load_instr:
                load += int(row[1])
            if row[0] in store_instr:
                store += int(row[1])
            if row[0] in double_instr:
                doubleOps += int(row[1])
            if row[0] == 'Total calls':
                instructions = int(row[2])
        profResFile.close()

        # finally we can store the summary on the output file
        fileHandle.write(benchmark + ' ' + str(cycles) + ' ' + str(instructions) + ' ' + str(load) + ' ' + str(store) + ' ' + str(old_div(float(doubleOps),float(load + store))) + '\n')
        fileHandle.flush()

    fileHandle.close()
