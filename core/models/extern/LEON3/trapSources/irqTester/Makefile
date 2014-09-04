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
#   the Free Software Foundation; either version 2 of the License, or
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
#   (c) Luca Fossati, fossati@elet.polimi.it
#
####################################################################################

# Makefile for the compilation of the software program to be simulated on the ISS
# in order to test interrupts

# Here define your own location of the cross-compiler
CC_CROSS=/home/luke/cross/sparc/bin/sparc-elf-gcc

irqTester: osemu-irq-crt0.o exceptionTrap-irq.o irqTester.c
	$(CC_CROSS) -o irqTester -specs=osemu-irq.specs irqTester.c

osemu-irq-crt0.o: osemu-irq-crt0.S
	$(CC_CROSS) -o osemu-irq-crt0.o -c osemu-irq-crt0.S

exceptionTrap-irq.o: exceptionTrap-irq.S
	$(CC_CROSS) -o exceptionTrap-irq.o -c exceptionTrap-irq.S

clean:
	rm -rf irqTester osemu-irq-crt0.o exceptionTrap-irq.o
