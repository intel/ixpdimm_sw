#!/bin/sh
. $(dirname $0)/INSTALLCONFIG.FILE

RESTART=0; \
	if [ -x /usr/sbin/cimserver ]; \
	then \
		cimserver --status &> /dev/null; \
		if [ $? -ne 0 ]; \
		then \
			RESTART=1; \
			cimserver enableHttpConnection=false enableHttpsConnection=false enableRemotePrivilegedUserAccess=false slp=false; \
		fi; \
		cimprovider -d -m intelwbemprovider &> /dev/null; \
		cimprovider -r -m intelwbemprovider &> /dev/null; \
		mofcomp -v -r -n root/intelwbem $PRODUCT_DATADIR/Pegasus/mof/intelwbem.mof; \
		if [[ $RESTART -gt 0 ]]; \
		then \
			cimserver -s; \
		fi; \
	fi
