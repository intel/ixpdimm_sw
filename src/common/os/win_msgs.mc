MessageIdTypedef=DWORD

SeverityNames=(
    Informational=0x0:INFORMATIONAL_EVENT
    Warning=0x1:WARNING_EVENT
    Error=0x2:ERROR_EVENT
    )


FacilityNames=(
    Service=0x0:FACILITY_SERVICE
    )

LanguageNames=(EnglishUS=0x401:MSG00401)

; // The following are message definitions.

MessageId=0x0
Severity=Informational
Facility=Service
SymbolicName=NVMDIMM_INFORMATIONAL
Language=EnglishUS
%1
.

MessageId=0x1
Severity=Warning
Facility=Service
SymbolicName=NVMDIMM_WARNING
Language=EnglishUS
%1
.

MessageId=0x2
Severity=Error
Facility=Service
SymbolicName=NVMDIMM_ERROR
Language=EnglishUS
%1
.

; // A message file must end with a period on its own line
; // followed by a blank line.
