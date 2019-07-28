#pragma once
#define MAJOR 0
#define MINOR 0
#define RELEASE 0
#define BUILD 0

#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)

#define VER_FILE_VERSION_STR        STRINGIZE(MAJOR)        \
                                    "." STRINGIZE(MINOR)    \
                                    "." STRINGIZE(RELEASE) \
                                    "." STRINGIZE(BUILD) \

#define COMMENTS "© Open Systems Pharmacology Community 2019"
#define COMPANY_NAME "Open Systems Pharmacology Community"
#define FILE_DESCRIPTION "SimModel Kernel Calculation"
#define FILE_VERSION VER_FILE_VERSION_STR
#define INTERNAL_NAME "OSPSuite.SimModelNative"
#define LEGAL_COPYRIGHT "Copyright 2019"
#define LEGAL_TRADEMARKS "© Open Systems Pharmacology Community"
#define ORIGINAL_FILENAME "OSPSuite.SimModelNative.dll"
#define PRODUCT_NAME  "OSPSuite.SimModelNative"
#define PRODUCT_VERSION VER_FILE_VERSION_STR
