#pragma once

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define PROJECT_VERSION = 1.18.1
#define PROJECT_RELDATE = "2009 - 12 - 08"
#define BBLEAN_NUMVERSION = 0, 1, 18, 1

#define FONTDATE = 20091208
#define ZIPDATE = "2009-12-08 1:17"
#define ZIPVERSION = 1.17.1



#define BBLEAN_VERSION PROJECT_VERSION
#define BBLEAN_RELDATE PROJECT_RELDATE

	// FIXME: fill BBLEAN_NUMVERSION corectly (used for blackbox/resource.rc)
#define BBLEAN_NUMVERSION 0, 1, 17, 1
#define BBAPPNAME "BlackboxZero"
#define BBAPPVERSION "BlackboxZero " STR(PROJECT_VERSION)
