
. $(dirname $0)/INSTALLCONFIG.FILE

BUILD_DIR=$1
BUILD_SIM=$2

# uninstall files from LIB_DIR	
$RM ${LIB_FILES[@]/#/$LIB_DIR/}

# uninstall files from CIM_LIB_DIR	
$RM ${CIM_LIB_FILES[@]/#/$CIM_LIB_DIR/}
	
# uninstall files from INCLUDE_DIR
$RM ${INCLUDE_FILES[@]/#/$INCLUDE_DIR/}
		
# uninstall files from BIN_DIR
$RM ${BIN_FILES[@]/#/$BIN_DIR/}
	
# uninstall monitor service
$RM $RPM_ROOT$UNIT_DIR/$MONITOR_NAME	
	
#uninstall Pegasus files
$RM $PEGASUS_MOF_FILES
$RMDIR $RPM_ROOT$PEGASUS_MOFDIR
$RMDIR $RPM_ROOT$PRODUCT_DATADIR/Pegasus
	
#uninstall SFCB files
$RM $SFCB_DIR/$SFCB_MOF_FILES
$RM $SFCB_DIR/$SFCB_REG_FILE
$RMDIR $RPM_ROOT$SFCB_DIR
	
$RM $RPM_ROOT$SYSCONF_DIR/ld.so.conf.d/$LINUX_PRODUCT_NAME-$HW_ARCH.conf
	
# uninstall shared files
$RM ${DATADIR_FILES[@]/#/$PRODUCT_DATADIR/}
$RMDIR $RPM_ROOT$PRODUCT_DATADIR
	
#uninstall manpage files
$RM $MANPAGE_GZ_FILES
mandb
		
if [ $BUILD_SIM -eq 1 ]
then 
	# remove simulator DB
	$RM $RPM_ROOT$PRODUCT_DATADIR/sim_system.db
fi

