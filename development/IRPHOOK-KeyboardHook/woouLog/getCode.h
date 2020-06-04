#ifndef __ScanCode_h__
#define __ScanCode_h__


#include "Header.h"


#define INVALID 0X00 //scan code not supported by this driver
#define SPACE 0X01 //space bar
#define ENTER 0X02 //enter key
#define LSHIFT 0x03 //left shift key
#define RSHIFT 0x04 //right shift key
#define CTRL  0x05 //control key
#define ALT	  0x06 //alt key


void convertScanToKey(PMYDEVICE_EXTENSION pDevExt, KEY_DATA* kData, char* keys);

#endif
