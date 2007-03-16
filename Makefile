#***************************************************************************
#*   Copyright (C) 2004 by Anthony DeChiaro                                *
#*   axd6491@njit.edu                                                      *
#*                                                                         *
#*   This program is free software; you can redistribute it and/or modify  *
#*   it under the terms of the GNU General Public License as published by  *
#*   the Free Software Foundation; either version 2 of the License, or     *
#*   (at your option) any later version.                                   *
#*                                                                         *
#*   This program is distributed in the hope that it will be useful,       *
#*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
#*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
#*   GNU General Public License for more details.                          *
#*                                                                         *
#*   You should have received a copy of the GNU General Public License     *
#*   along with this program; if not, write to the                         *
#*   Free Software Foundation, Inc.,                                       *
#*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
#***************************************************************************

# Makefile

OSLOADERDIR	= ./osloader
KERNELDIR	= ./kernel
SNAPSHOTDIR = ./snapshots
BINDIR		= ./bin
BACKUPDIR = /cygdrive/T/coding/ktux

##############
# Ktux files #
##############

BOOTSECT	= ${OSLOADERDIR}/bootsect.dat
BOOTSTUBBIN	= ${OSLOADERDIR}/ktuxldr
KERNELBIN	= ${KERNELDIR}/ktuxkrnl

#################################################
# Bootdisk related files (disk image or device) #
#################################################

CLEANIMG	= a-blank.img
SCRATCHIMG	= a.img
LOOPBACKDEV	= /dev/loop0
IMGMOUNTPT	= /mnt/floppyimg

SNAPSHOT	= ${BINDIR}/make_snapshot.sh
VFD			= ${BINDIR}/vfd

#############################
# Start of Makefile targets #
#############################

default:
	make -C kernel default
	make -C osloader default

all:
	make -C kernel all
	make -C osloader all
	make -C . install

clean:
	make -C kernel clean
	make -C osloader clean
	-${VFD} stop	
	-rm ${SCRATCHIMG}	
	
snapshot:
	make -C . clean
	${SNAPSHOT}	${SNAPSHOTDIR} ${BACKUPDIR}

### Modify this according to your environment ###
install: installcygwin
### Modify this according to your environment ###

linux: installlinux
cygwin: installcygwin


installlinux:
ifneq (${shell id -u},0)
	@echo "Error: Makefile requires root privledges for install target"
	@exit 1
endif
	-rm ${SCRATCHIMG}
	cp ${CLEANIMG} ${SCRATCHIMG}
	-umount ${LOOPBACKDEV}
	-losetup -d ${LOOPBACKDEV}
	losetup ${LOOPBACKDEV} ${SCRATCHIMG}
	mkfs.msdos ${LOOPBACKDEV}
	mount ${LOOPBACKDEV} -o loop ${IMGMOUNTPT}
	dd if=${BOOTSECT} bs=512 count=1 of=${LOOPBACKDEV}
	cp ${BOOTSTUBBIN} ${IMGMOUNTPT}
	cp ${KERNELBIN} ${IMGMOUNTPT}
	umount ${LOOPBACKDEV}

installcygwin:
	-${VFD} stop
	-rm ${SCRATCHIMG}
	cp ${CLEANIMG} ${SCRATCHIMG}
	@echo "Writing bootsector..."
	dd if=${BOOTSECT} bs=512 count=1 conv=notrunc of=${SCRATCHIMG}
	-${VFD} install
	-${VFD} start
	${VFD} ulink 1
	${VFD} link 1 b
	${VFD} open b: ${SCRATCHIMG} /w /144
	cp ${BOOTSTUBBIN} /cygdrive/b
	cp ${KERNELBIN} /cygdrive/b
	-${VFD} stop
	