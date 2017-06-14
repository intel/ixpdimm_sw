#!/bin/sh
. $(dirname $0)/INSTALLCONFIG.FILE

if [ -x /usr/sbin/cimserver ]; \
	then \
		cimserver --status &> /dev/null; \
		if [ $? -eq 0 ]; \
		then \
		CIMMOF=cimmof; \
		else \
		CIMMOF="cimmofl -R /var/lib/Pegasus"; \
		fi; \
		for ns in interop root/interop root/PG_Interop; do \
			$CIMMOF -E -n"$ns" $PRODUCT_DATADIR/Pegasus/mof/pegasus_register.mof &> /dev/null; \
			if [ $? -eq 0 ]; \
			then \
				$CIMMOF -uc -n$ns $PRODUCT_DATADIR/Pegasus/mof/pegasus_register.mof &> /dev/null; \
				$CIMMOF -uc -n$ns $PRODUCT_DATADIR/Pegasus/mof/profile_registration.mof &> /dev/null; \
				break; \
			fi; \
		done; \
		$CIMMOF -uc -aE -nroot/intelwbem $PRODUCT_DATADIR/Pegasus/mof/intelwbem.mof &> /dev/null; \
fi
