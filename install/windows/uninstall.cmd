	cd $(BUILD_DIR); regsvr32 //s //u $(CIM_LIB_SONAME).dll # unregister DLL 
	powershell gwmi -N:root -Query \"SELECT \* from __Namespace WHERE Name = \'intelwbem\'\" "|" rwmi 
	
	#unregister from default namespace
	for ns in $(NAMESPACES); do \
		powershell gwmi -namespace \"$$ns\" -list > /dev/null; \
		if [ $$? -eq 0 ]; \
		then \
			powershell rwmi -N:$$ns -class Intel_ElementConformsToProfile; \
			powershell rwmi -N:$$ns -class intel_registeredprofile; \
			break; \
		fi; \
	done
	
# might need to stop the WMI Service after registering the DLL and running a query against it.  
# Unregistering the dll doesn't work most of the time.  The following command will stop the WMI service. (must be admin)
# net stop "Windows Management Instrumentation"
