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

#define COMMENTS "© Open Systems Pharmacology Community 2017"
#define COMPANY_NAME "Open Systems Pharmacology Community"
#define FILE_DESCRIPTION "OSPSuite_SimModelComp"
#define FILE_VERSION VER_FILE_VERSION_STR
#define INTERNAL_NAME "SimModelComp"
#define LEGAL_COPYRIGHT "Copyright 2017"
#define LEGAL_TRADEMARKS "© Open Systems Pharmacology Community"
#define ORIGINAL_FILENAME "OSPSuite_SimModelComp.dll"
#define PRODUCT_NAME  "SimModelComp"
#define PRODUCT_VERSION VER_FILE_VERSION_STR



