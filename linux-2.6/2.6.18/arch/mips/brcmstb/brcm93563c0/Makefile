#
# Makefile for BCM973563 board
#
# Copyright (C) 2005 Broadcom Corporation
#
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
# Makefile for BCM973563 board
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#

obj-y	 += irq.o bcmuart.o hw_init.o bchip.o bcm93563c0.o
ifdef CONFIG_DISCONTIGMEM
obj-$(CONFIG_DISCONTIGMEM) += board-ddr1.o
else
obj-y	+= board.o
endif
