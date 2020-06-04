#include "getCode.h"




#define INVALID 0X00 //scan code not supported by this driver
#define SPACE 0X01 //space bar
#define ENTER 0X02 //enter key
#define LSHIFT 0x03 //left shift key
#define RSHIFT 0x04 //right shift key
#define CTRL  0x05 //control key
#define ALT	  0x06 //alt key

char KeyMap[84] = {
	INVALID, //0
	INVALID, //1
	'1', //2
	'2', //3
	'3', //4
	'4', //5
	'5', //6
	'6', //7
	'7', //8
	'8', //9
	'9', //A
	'0', //B
	'-', //C
	'=', //D
	INVALID, //E
	INVALID, //F
	'q', //10
	'w', //11
	'e', //12
	'r', //13
	't', //14
	'y', //15
	'u', //16
	'i', //17
	'o', //18
	'p', //19
	'[', //1A
	']', //1B
	ENTER, //1C
	CTRL, //1D
	'a', //1E
	's', //1F
	'd', //20
	'f', //21
	'g', //22
	'h', //23
	'j', //24
	'k', //25
	'l', //26
	';', //27
	'\'', //28
	'`', //29
	LSHIFT,	//2A
	'\\', //2B
	'z', //2C
	'x', //2D
	'c', //2E
	'v', //2F
	'b', //30
	'n', //31
	'm' , //32
	',', //33
	'.', //34
	'/', //35
	RSHIFT, //36
	INVALID, //37
	ALT, //38
	SPACE, //39
	INVALID, //3A
	INVALID, //3B
	INVALID, //3C
	INVALID, //3D
	INVALID, //3E
	INVALID, //3F
	INVALID, //40
	INVALID, //41
	INVALID, //42
	INVALID, //43
	INVALID, //44
	INVALID, //45
	INVALID, //46
	'7', //47
	'8', //48
	'9', //49
	INVALID, //4A
	'4', //4B
	'5', //4C
	'6', //4D
	INVALID, //4E
	'1', //4F
	'2', //50
	'3', //51
	'0', //52
};

///////////////////////////////////////////////////////////////////////
//The Extended Key Map is used for those scan codes that can map to
//more than one key.  This mapping is usually determined by the 
//states of other keys (ie. the shift must be pressed down with a letter
//to make it uppercase).
///////////////////////////////////////////////////////////////////////
char ExtendedKeyMap[84] = {
	INVALID, //0
	INVALID, //1
	'!', //2
	'@', //3
	'#', //4
	'$', //5
	'%', //6
	'^', //7
	'&', //8
	'*', //9
	'(', //A
	')', //B
	'_', //C
	'+', //D
	INVALID, //E
	INVALID, //F
	'Q', //10
	'W', //11
	'E', //12
	'R', //13
	'T', //14
	'Y', //15
	'U', //16
	'I', //17
	'O', //18
	'P', //19
	'{', //1A
	'}', //1B
	ENTER, //1C
	INVALID, //1D
	'A', //1E
	'S', //1F
	'D', //20
	'F', //21
	'G', //22
	'H', //23
	'J', //24
	'K', //25
	'L', //26
	':', //27
	'"', //28
	'~', //29
	LSHIFT,	//2A
	'|', //2B
	'Z', //2C
	'X', //2D
	'C', //2E
	'V', //2F
	'B', //30
	'N', //31
	'M' , //32
	'<', //33
	'>', //34
	'?', //35
	RSHIFT, //36
	INVALID, //37
	INVALID, //38
	SPACE, //39
	INVALID, //3A
	INVALID, //3B
	INVALID, //3C
	INVALID, //3D
	INVALID, //3E
	INVALID, //3F
	INVALID, //40
	INVALID, //41
	INVALID, //42
	INVALID, //43
	INVALID, //44
	INVALID, //45
	INVALID, //46
	'7', //47
	'8', //48
	'9', //49
	INVALID, //4A
	'4', //4B
	'5', //4C
	'6', //4D
	INVALID, //4E
	'1', //4F
	'2', //50
	'3', //51
	'0', //52
};

void convertScanToKey(PMYDEVICE_EXTENSION pDevExt, PKEY_DATA kData, char* keys)
{
	char key = 0;
	key = KeyMap[kData->keyScan];


	switch (key)
	{
	case LSHIFT:
		if (kData->keyFlags == KEY_MAKE)
			pDevExt->kState.kSHIFT = TRUE;
		else
			pDevExt->kState.kSHIFT = FALSE;
		break;

	case RSHIFT:
		if (kData->keyFlags == KEY_MAKE)
			pDevExt->kState.kSHIFT = TRUE;
		else
			pDevExt->kState.kSHIFT = FALSE;
		break;

		///////////////////////////////////////
		//Get and update state of CONTROL key
		///////////////////////////////////////
	case CTRL:
		if (kData->keyFlags == KEY_MAKE)
			pDevExt->kState.kCTRL = TRUE;
		else
			pDevExt->kState.kCTRL = FALSE;
		break;

		///////////////////////////////////////
		//Get and update state of ALT key
		///////////////////////////////////////
	case ALT:
		if (kData->keyFlags == KEY_MAKE)
			pDevExt->kState.kALT = TRUE;
		else
			pDevExt->kState.kALT = FALSE;
		break;

		///////////////////////////////////////
		//If the space bar was pressed
		///////////////////////////////////////
	case SPACE:
		if ((pDevExt->kState.kALT != TRUE) && (kData->keyFlags == KEY_BREAK)) //the space bar does not leave 
			keys[0] = 0x20;				//a space if pressed with the ALT key
		break;

		///////////////////////////////////////
		//If the enter key was pressed
		///////////////////////////////////////
	case ENTER:
		if ((pDevExt->kState.kALT != TRUE) && (kData->keyFlags == KEY_BREAK)) //the enter key does not leave 
		{								 //move to the next line if pressed
			keys[0] = 0x0D;				 //with the ALT key
			keys[1] = 0x0A;
		}//end if
		break;

		///////////////////////////////////////////
		//For all other alpha numeric keys
		//If the ALT or CTRL key is pressed, do not
		//convert. If the SHIFT or CAPS LOCK
		//keys are pressed, switch to the
		//extended key map. Otherwise return
		//the current key.
		////////////////////////////////////////////
	default:
		if ((pDevExt->kState.kALT != TRUE) && (pDevExt->kState.kCTRL != TRUE) && (kData->keyFlags == KEY_BREAK)) //don't convert if ALT or CTRL is pressed
		{
			if ((key >= 0x21) && (key <= 0x7E)) //don't convert non alpha numeric keys
			{
				if (pDevExt->kState.kSHIFT == TRUE)
					keys[0] = ExtendedKeyMap[kData->keyScan];
				else
					keys[0] = key;
			}//end if
		}//end if
		break;
	}//end switch(keys)
}//end ConvertScanCodeToKeyCode

