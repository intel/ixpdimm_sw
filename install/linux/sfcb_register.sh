#!/bin/sh
. $(dirname $0)/INSTALLCONFIG.FILE

if [ -x /usr/sbin/sfcbd ]; \
	then \
		RESTART=0; \
		systemctl is-active sblim-sfcb.service &> /dev/null; \
		if [ $? -eq 0 ]; \
		then \
			RESTART=1; \
			systemctl stop sblim-sfcb.service &> /dev/null; \
		fi; \
		sfcbstage -n root/intelwbem -r $PRODUCT_DATADIR/sfcb/INTEL_NVDIMM.reg $PRODUCT_DATADIR/sfcb/sfcb_intelwbem.mof; \
		sfcbrepos -f; \
		if [[ $RESTART -gt 0 ]]; \
		then \
			systemctl start sblim-sfcb.service &> /dev/null; \
		fi; \
fi