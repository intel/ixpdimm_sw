#!/bin/bash
ROOT=$1
BUILD_DIR=$2
WBEM_PREFIX_INPUT=Intel_

## -----------------------------
## Create registration mofs
## -----------------------------
rm -f $BUILD_DIR/profile_registration.mof
cp -av $ROOT/src/wbem/mof/profile_registration.mof  $BUILD_DIR/profile_registration.mof
sed -i "s/<NVM_WBEM_PREFIX>/$WBEM_PREFIX_INPUT/g" $BUILD_DIR/profile_registration.mof

cp -av $ROOT/src/wbem/cimom/cmpi/pegasus_register.mof  $BUILD_DIR/pegasus_register.mof
cp -av $ROOT/src/wbem/cimom/cmpi/sfcb_register.reg  $BUILD_DIR/INTEL_NVDIMM.reg
sed -i "s/<NVM_WBEM_PREFIX>/$WBEM_PREFIX_INPUT/g" $BUILD_DIR/pegasus_register.mof
sed -i "s/<NVM_WBEM_PREFIX>/$WBEM_PREFIX_INPUT/g" $BUILD_DIR/INTEL_NVDIMM.reg

rm  -f $BUILD_DIR/intelwbem.mof
echo // DMTF CIM Schema 2.44.1 > $BUILD_DIR/intelwbem.mof
cat $ROOT/src/wbem/mof/cim_schema_2.44.1_combined.mof >> $BUILD_DIR/intelwbem.mof
echo >> $BUILD_DIR/intelwbem.mof
# SNIA CIM Schema
echo // SNIA CIM Schema 16Rev4-Updated >> $BUILD_DIR/intelwbem.mof	
cat $ROOT/src/wbem/mof/snia_mofs_16Rev4-updated.mof >> $BUILD_DIR/intelwbem.mof
echo >> $BUILD_DIR/intelwbem.mof
# Intel CIM Schema
echo // Intel CIM Schema >> $BUILD_DIR/intelwbem.mof
cat $ROOT/src/wbem/mof/class_def.mof >> $BUILD_DIR/intelwbem.mof
sed -i "s/<NVM_WBEM_PREFIX>/$WBEM_PREFIX_INPUT/g" $BUILD_DIR/intelwbem.mof
	
# ---------------------------------------------------------------------------------------------
# Change mof file to be CIMOM appropriate.
# The following sed commands rely on the <WMI> and <CMPI> tags to indicate pieces that should
# be removed or stay. The opening and closing WMI/CMPI tags must be on the same line.
# Note: the ".bak" value to the -i option in sed seems to be required on Windows for some reason
# ---------------------------------------------------------------------------------------------

# get rid of WMI Specifics
sed -i  's/<WMI>\(.*\)<\/WMI>//' $BUILD_DIR/intelwbem.mof
# Keep CMPI Specifics
sed -i  's/<CMPI>\(.*\)<\/CMPI>/\1/' $BUILD_DIR/intelwbem.mof

#we need to remove "SubType" for cmpi which understands datetime intervals better than WMI
cat $BUILD_DIR/intelwbem.mof | sed 's/SubType("interval"), //' > $BUILD_DIR/intelwbem_new.mof
mv  $BUILD_DIR/intelwbem_new.mof $BUILD_DIR/intelwbem.mof

#TODO: check for esx
#ifdef BUILD_ESX
	# sfcb on ESX complains about qualifier redefinitions, so use only SFCB-friendly qualifiers
#	cat mof/sfcb_qualifiers.mof > $(BUILD_DIR)/sfcb_intelwbem.mof
#else
	
echo "" > $BUILD_DIR/sfcb_intelwbem.mof

#endif

cat $BUILD_DIR/intelwbem.mof >> $BUILD_DIR/sfcb_intelwbem.mof
	
# profile registration doesn't need qualifiers for sfcb
cp -av $BUILD_DIR/profile_registration.mof $BUILD_DIR/sfcb_profile_registration.mof
sed -i".bak" '1,8d' $BUILD_DIR/sfcb_profile_registration.mof
rm $BUILD_DIR/sfcb_profile_registration.mof.bak

# add qualifiers for linux/windows
cat $ROOT/src/wbem/mof/qualifiers.mof > $BUILD_DIR/intelwbem_with_qualifiers.mof
cat $BUILD_DIR/intelwbem.mof >> $BUILD_DIR/intelwbem_with_qualifiers.mof
mv $BUILD_DIR/intelwbem_with_qualifiers.mof $BUILD_DIR/intelwbem.mof