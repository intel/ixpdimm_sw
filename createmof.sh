#!/bin/bash
ROOT=$1
OUTPUT_DIR=$2
WBEM_PREFIX_INPUT=Intel_

echo WBEM_PREFIX_INTEL: $WBEM_PREFIX_INPUT
## -----------------------------
## Create registration mofs
## -----------------------------
rm -f $OUTPUT_DIR/profile_registration.mof
cp -av $ROOT/src/wbem/mof/profile_registration.mof  $OUTPUT_DIR/profile_registration.mof
sed -i "s/<NVM_WBEM_PREFIX>/$WBEM_PREFIX_INPUT/g" $OUTPUT_DIR/profile_registration.mof

cp -av $ROOT/src/wbem/cimom/cmpi/pegasus_register.mof  $OUTPUT_DIR/pegasus_register.mof
cp -av $ROOT/src/wbem/cimom/cmpi/sfcb_register.reg  $OUTPUT_DIR/INTEL_NVDIMM.reg
sed -i "s/<NVM_WBEM_PREFIX>/$WBEM_PREFIX_INPUT/g" $OUTPUT_DIR/pegasus_register.mof
sed -i "s/<NVM_WBEM_PREFIX>/$WBEM_PREFIX_INPUT/g" $OUTPUT_DIR/INTEL_NVDIMM.reg

echo $OUTPUT_DIR
rm  -f $OUTPUT_DIR/intelwbem.mof
echo // DMTF CIM Schema 2.44.1 > $OUTPUT_DIR/intelwbem.mof
cat $ROOT/src/wbem/mof/cim_schema_2.44.1_combined.mof >> $OUTPUT_DIR/intelwbem.mof
echo >> $OUTPUT_DIR/intelwbem.mof
# SNIA CIM Schema
echo // SNIA CIM Schema 16Rev4-Updated >> $OUTPUT_DIR/intelwbem.mof	
cat $ROOT/src/wbem/mof/snia_mofs_16Rev4-updated.mof >> $OUTPUT_DIR/intelwbem.mof
echo >> $OUTPUT_DIR/intelwbem.mof
# Intel CIM Schema
echo // Intel CIM Schema >> $OUTPUT_DIR/intelwbem.mof
cat $ROOT/src/wbem/mof/class_def.mof >> $OUTPUT_DIR/intelwbem.mof
sed -i "s/<NVM_WBEM_PREFIX>/$WBEM_PREFIX_INPUT/g" $OUTPUT_DIR/intelwbem.mof
	
# ---------------------------------------------------------------------------------------------
# Change mof file to be CIMOM appropriate.
# The following sed commands rely on the <WMI> and <CMPI> tags to indicate pieces that should
# be removed or stay. The opening and closing WMI/CMPI tags must be on the same line.
# Note: the ".bak" value to the -i option in sed seems to be required on Windows for some reason
# ---------------------------------------------------------------------------------------------

# get rid of WMI Specifics
sed -i  's/<WMI>\(.*\)<\/WMI>//' $OUTPUT_DIR/intelwbem.mof
# Keep CMPI Specifics
sed -i  's/<CMPI>\(.*\)<\/CMPI>/\1/' $OUTPUT_DIR/intelwbem.mof

#we need to remove "SubType" for cmpi which understands datetime intervals better than WMI
cat $OUTPUT_DIR/intelwbem.mof | sed 's/SubType("interval"), //' > $OUTPUT_DIR/intelwbem_new.mof
mv  $OUTPUT_DIR/intelwbem_new.mof $OUTPUT_DIR/intelwbem.mof

#TODO: check for esx
#ifdef BUILD_ESX
	# sfcb on ESX complains about qualifier redefinitions, so use only SFCB-friendly qualifiers
#	cat mof/sfcb_qualifiers.mof > $(BUILD_DIR)/sfcb_intelwbem.mof
#else
	
echo "" > $OUTPUT_DIR/sfcb_intelwbem.mof

#endif

cat $OUTPUT_DIR/intelwbem.mof >> $OUTPUT_DIR/sfcb_intelwbem.mof
	
# profile registration doesn't need qualifiers for sfcb
cp -av $OUTPUT_DIR/profile_registration.mof $OUTPUT_DIR/sfcb_profile_registration.mof
sed -i".bak" '1,8d' $OUTPUT_DIR/sfcb_profile_registration.mof
rm $OUTPUT_DIR/sfcb_profile_registration.mof.bak

# add qualifiers for linux/windows
cat $ROOT/src/wbem/mof/qualifiers.mof > $OUTPUT_DIR/intelwbem_with_qualifiers.mof
cat $OUTPUT_DIR/intelwbem.mof >> $OUTPUT_DIR/intelwbem_with_qualifiers.mof
mv $OUTPUT_DIR/intelwbem_with_qualifiers.mof $OUTPUT_DIR/intelwbem.mof