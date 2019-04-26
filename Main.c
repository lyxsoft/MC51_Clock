/*
	(*)	Clock

		LyxSoft 13.10.2015

*/

#include "STC\STC15F104E.H"
#include "DS1302.h"
#include "NTC.h"

/*                                      0    1	  2	   3	4	 5	  6	   7	8	 9    A    b    C    d    E    F    G    H    I    J         L         N    O    P    q    r    S    t    U    V    W         y    Z    [    \    ]
																																																						  DEG	 -	 8.
                                        0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18   19   20   21   22   23   24   25   26   27   28   29   30   31   32   33   34   35   36   37   38  39*/

unsigned char code tblNumbers[]    ={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0x88,0x83,0xC6,0xA1,0x86,0x8E,0xC2,0x89,0xF9,0xF1,0xFF,0xC7,0xFF,0xC8,0xC0,0x8C,0x98,0xAF,0x92,0x87,0xC1,0x8D,0x81,0xFF,0x91,0xA4,0x9C,0xBF,0x0,0xFF};
unsigned char code tblNumbersRev[] ={0xC0,0xCF,0xA4,0x86,0x8B,0x92,0x90,0xC7,0x80,0x82,0x81,0x98,0xF0,0x8C,0xB0,0xB1,0xD0,0x89,0xCF,0xCE,0xFF,0xF8,0xFF,0xC1,0xC0,0xA1,0x83,0xBD,0x92,0xB8,0xC8,0xA9,0x88,0xFF,0x8A,0xA4,0xA3,0xBF,0x0,0xFF};

// 									Mon					TUE					 WEd				 Thu				 Fri				 SAt				 Sun
//                                      n	 o	  N	   |,   E	 U	  T	   T,   d	 E	  U    |,	u	 h    T    T,	i	 r	  F		,t		 A	  S		,	n
unsigned char code tblWeek[]       ={0xAB,0xA3,0xC8,0xF9,0x86,0xC1,0xCE,0xFE,0xA1,0x86,0xC1,0xF9,0xE3,0x8B,0xCE,0xFE,0xFB,0xAF,0x8E,0xFF,0x87,0x88,0x92,0xFF,0xAB,0xE3,0x92,0xFF};
unsigned char code tblWeekRev[]    ={0x9D,0x9C,0xC1,0xCF,0xB0,0xC8,0xF1,0xF7,0x8C,0xB0,0xC8,0xCF,0xDC,0x99,0xF1,0xF7,0xDF,0xBD,0xB1,0xFF,0xB8,0x81,0x92,0xFF,0x9D,0xDC,0x92,0xFF};
/*                                      0    1	  2	   3	4	 5	  6	   7	8	 9    a    b    c    d    e    f   10   11   12*/
unsigned char code tblMonthDays[] = {0x00,0x31,0x28,0x31,0x30,0x31,0x30,0x31,0x31,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x31,0x30,0x31};

#define MAKEBCD(nValue)	((((nValue) / 10) << 4) + ((nValue) % 10))
#define GETBCD(nBCD)    ((((nBCD) >> 4) * 10) + ((nBCD) & 0xF))

#define KEY_S1	P31
#define KEY_S2	P30
#define KEY_OK	P30
#define KEY_SYNC P30

#define BEEP	P33

#define MODE_CLOCK		0
#define MODE_HOUR		1
#define MODE_MINUTE		2
#define MODE_HOUR24		3
#define MODE_MONTH		4
#define MODE_DAY		5
#define MODE_WEEK		6
#define MODE_YEAR		7
#define MODE_BEEP		8
#define MODE_ALARM_H	9
#define MODE_ALARM_M	10
#define MODE_UP			11
#define MODE_NIGHT		12
#define MODE_MAX		12

#define SMODE_CLOCK				0
#define SMODE_TEMP				1
#define SMODE_DATE				2
#define SMODE_WEEK				3
#define SMODE_YEAR				4
#define SMODE_SECOND			5
#define SMODE_MS				6
#define SMODE_COUNTDOWN1		7
#define SMODE_COUNTDOWN2		8
#define SMODE_ALARM				9
#define SMODE_MAX				9

#define SMODE_COUNTSEC			10	//10 seconds return to SMODE_CLOCK

#define DMODE_CLOCK				1
#define DMODE_DIGDOT			2
#define DMODE_DEGDOT			3
#define DMODE_DEGMIN			4
#define DMODE_DATE				5

#define DMODE_FLAG_PM				0x10

bit bBlockClock;

// Clock
static DS1302Time nCurrentTime; 
static unsigned char n100Ms, n10Ms, nMs;
static bit bBeep, bUp, bNightMode;
static unsigned char nAlarmH, nAlarmM;

// Temperature
static unsigned char nTemp, nTempL;

// Buttons
static bit bPressedS1, bPressedS2;

static bit bNight, bSleep;
static unsigned char nMode, nSubMode, nSubModeTime, nCountDownSec, nCountDownMin, nCountDownBeep;

// Display
static unsigned char nLED, nData0, nData1, nData2, nData3, nShow;
static bit bDot, bSound; //, bShow0, bShow1, bShow2, bShow3;


unsigned char BCDHour24 (unsigned char nBCD)
{
	if (nBCD & 0x80)	// 12 HOUR
	{
		if ((nBCD & 0xA0) == 0xA0)	//PM
		{
			nBCD = GETBCD (nBCD & 0x1F);
			if (nBCD != 12) //12 PM means 12 aclock
				nBCD += 12;
			return (MAKEBCD (nBCD));
		}
		else						//AM
		{
			nBCD &= 0x1F;
			if (nBCD == 0x12)		//12 AM means 0 aclock
				return (0);
		}
	}
	return (nBCD);
}

unsigned char BCDHour12 (unsigned char nBCD)
{
	if (nBCD & 0x80)	// 12 HOUR
		return (nBCD);

	if (nBCD >= 0x12)			//PM
	{
		nBCD = GETBCD (nBCD) - 12;
		if (nBCD == 0)
			return (0xB2);		//12 PM
		else
			return (0xA0 | MAKEBCD (nBCD));
	}
	else if (nBCD == 0)
		return (0x92);			// 12 AM
	else
		return (0x80 | nBCD);
}

//#define CHARINDEX(nChar) (nChar <= '9' ? (nChar-'9'):(nChar <= ']' ? (nChar-'A'+10):(nChar <= 'z' ? (nChar-'a'+10):0xFF)))
unsigned char CharIndex (unsigned char nChar)
{
	if (nChar <= '9')
		return (nChar - '0');
	if (nChar <= ']')
		return (nChar - 'A' + 10);
	if (nChar <= 'z')
		return (nChar - 'a' + 10);
	return 39; //NA
}

void DisplayChar (unsigned char nChar0, unsigned char nChar1, unsigned char nChar2, unsigned char nChar3)
{
	if (bUp)
	{
		nData0 = tblNumbers [CharIndex(nChar0)];
		nData1 = tblNumbersRev [CharIndex(nChar1)];
		nData2 = tblNumbers [CharIndex(nChar2)];
		nData3 = tblNumbers [CharIndex(nChar3)];
	}
	else
	{
		nData3 = tblNumbersRev [CharIndex(nChar0)];
		nData2 = tblNumbersRev [CharIndex(nChar1)];
		nData1 = tblNumbers [CharIndex(nChar2)];
		nData0 = tblNumbersRev [CharIndex(nChar3)];
	}
}

void DisplayDecNum (unsigned char nNumLow, unsigned char nNumHigh, unsigned char nMode)
{
	if (bUp)
	{
		nData0 = tblNumbers [nNumLow & 0xF];
		nData1 = tblNumbersRev [nNumLow >> 4];
		nData2 = tblNumbers [nNumHigh & 0xF];
		nData3 = tblNumbers [nNumHigh >> 4];
	}
	else
	{
		nData3 = tblNumbersRev [nNumLow & 0xF];
		nData2 = tblNumbersRev [nNumLow >> 4];
		nData1 = tblNumbers [nNumHigh & 0xF];
		nData0 = tblNumbersRev [nNumHigh >> 4];
	}
	switch (nMode & 0xF)
	{
	case DMODE_CLOCK:				  //:
		nData1 &= 0x7F;
		nData2 &= 0x7F;
		break;
	case DMODE_DIGDOT:				  //.
		if (bUp)
			nData2 &=0x7F;
		else
			nData1 &=0x7F;
		break;
	case DMODE_DEGDOT:				 //.Deg
		if (bUp)
		{
			nData0 = 0x9c;	 //Deg
			nData2 &=0x7F;
		}
		else
		{
			nData3 = 0xA3;	 //Deg
			nData1 &=0x7F;
		}
		break;
	case DMODE_DEGMIN:				//- Deg
		if (bUp)
		{
			nData0 = 0x9c;	 //Deg
			nData3 = 0xBF;	 //-
		}
		else
		{
			nData3 = 0xA3;	 //Deg
			nData1 = 0xBF;	 //-
		}
		break;
	case DMODE_DATE:				  //'
		if (bUp)
			nData1 &=0x7F;
		else
			nData2 &=0x7F;
		break;
	default:
		break;
	}
	if (nMode & DMODE_FLAG_PM)			 //' @ End
	{
		if (bUp)
			nData0 &=0x7F;
		else
			nData3 &=0x7F;
	}
}

void DisplayErr (void)
{
	nData0 = 0;
	nData1 = nData0;
	nData2 = nData0;
	nData3 = nData0;
}

void PreDisplay (void)
{
	unsigned char nDisplay;
	unsigned char nShowMaskH, nShowMaskL;

	nShowMaskH = bUp ? 0xC : 0x3;
	nShowMaskL = bUp ? 0x3 : 0xC;
	nShow = 0xF; //Show All
	switch (nMode)
	{
	case MODE_CLOCK:
		nDisplay = nSubMode;
		break;

	case MODE_HOUR:
		if (!bPressedS1 && !bDot)
			nShow = nShowMaskL; //bUp ? 0x3 : 0xC;

		nDisplay = SMODE_CLOCK;
		break;
	case MODE_MINUTE:
		if (!bPressedS1 && !bDot)
			nShow = nShowMaskH; //bUp ? 0xC : 0x3;

		nDisplay = SMODE_CLOCK;
		break;
	case MODE_MONTH:
		if (!bPressedS1 && !bDot)
			nShow = nShowMaskL; //bUp ? 0x3 : 0xC;

		nDisplay = SMODE_DATE;
		break;
	case MODE_DAY:
		if (!bPressedS1 && !bDot)
			nShow = nShowMaskH; //bUp ? 0xC : 0x3;

		nDisplay = SMODE_DATE;
		break;
	case MODE_WEEK:
		if (!bPressedS1 && !bDot)
			nShow = 0;

		nDisplay = SMODE_WEEK;
		break;
	case MODE_YEAR:
		if (!bPressedS1 && !bDot)
			nShow = 0;

		nDisplay = SMODE_YEAR;
		break;
	case MODE_ALARM_H:
		if (!bPressedS1 && !bDot)
			nShow = nShowMaskL; //bUp ? 0x3 : 0xC;

		nDisplay = SMODE_ALARM;
		break;
	case MODE_ALARM_M:
		if (!bPressedS1 && !bDot)
			nShow = nShowMaskH; //bUp ? 0xC : 0x3;

		nDisplay = SMODE_ALARM;
		break;
	case MODE_HOUR24:
		if (!bPressedS1 && !bDot)
			nShow = 0;
		else
			nShow = bUp ? 0x7 : 0xE;
		
		if (nCurrentTime.Hour & 0x80)	// 12 HOUR
			DisplayChar ('H','2','1',0xFF);
		else
			DisplayChar ('H','4','2',0xFF);
		return;
	case MODE_BEEP:
		if (!bPressedS1 && !bDot)
			nShow = 0;
		
		if (bBeep)
			DisplayChar ('P','E','E','B');
		else
			DisplayChar ('\\','\\','\\','\\');
		return;
	case MODE_UP:
		if (!bPressedS1 && !bDot)
			nShow = 0;
		
		DisplayChar ('P','U',0xFF,0xFF);
		return;
	case MODE_NIGHT:
		if (bNightMode)
			DisplayChar ('N','O',0xFF,'N');
		else
			DisplayChar ('F','F','O','N');
		return;		
	default:
		if (!bPressedS1 && !bDot)
			nShow = 0;

		DisplayErr (); //DisplayChar (']',']',']',']');	//NA
		return;		
	}

	switch (nDisplay)
	{
	case SMODE_CLOCK:
		nData2 = nCurrentTime.Hour;

		if (nData2 & 0x80) //12 Hour mode
		{
			nData2 &= 0x1F;
			if (nData2 < 0x10)
				nShow &= bUp ? 0x7 : 0xE;
		}

		DisplayDecNum (nCurrentTime.Minute, nData2, (unsigned char)bDot | (((nCurrentTime.Hour & 0xA0) == 0xA0) ? DMODE_FLAG_PM : 0));	//12 Hour Mode and PM

		break;
	case SMODE_TEMP:
		if (nTemp >= 40 && nTempL < 10)
			DisplayDecNum (nTempL << 4, MAKEBCD(nTemp-40), DMODE_DEGDOT);
		else if (nTemp < 40)
		{
			nData0 = 40 - nTemp;
			DisplayDecNum ((nData0%10)<<4, nData0/10, DMODE_DEGMIN);
		}
		else
		{
			DisplayErr ();
		}
		break;

	case SMODE_DATE:
		DisplayDecNum (nCurrentTime.Day, nCurrentTime.Month, DMODE_DATE);
		if (nCurrentTime.Month < 0x10)
			nShow &= bUp ? 0x7 : 0xE;
		break;

	case SMODE_WEEK:
		if (nCurrentTime.Week <= 7 && nCurrentTime.Week >= 1)
		{
			unsigned char nPtr;

			nPtr = nCurrentTime.Week * 4 - 4;
			if (bUp)
			{
				nData0 = tblWeek [nPtr];
				nData1 = tblWeekRev [nPtr + 1];	
				nData2 = tblWeek [nPtr + 2];	
				nData3 = tblWeek [nPtr + 3];	
			}
			else
			{
				nData3 = tblWeekRev [nPtr];
				nData2 = tblWeekRev [nPtr + 1];	
				nData1 = tblWeek [nPtr + 2];	
				nData0 = tblWeekRev [nPtr + 3];	
			}
		}
		break;
	case SMODE_YEAR:
		DisplayDecNum (nCurrentTime.Year, 0x20, 0); 
		break;
	case SMODE_SECOND:
		DisplayDecNum (nCurrentTime.Second, nCurrentTime.Minute, bDot); 
		break;
	case SMODE_MS:
		DisplayDecNum (n100Ms<<4, nCurrentTime.Second, DMODE_DIGDOT);
		break;
	case SMODE_COUNTDOWN1:
		DisplayDecNum (MAKEBCD(nCountDownSec), MAKEBCD(nCountDownMin), DMODE_CLOCK);
		break;
	case SMODE_COUNTDOWN2:
		DisplayDecNum (MAKEBCD(nCountDownSec), MAKEBCD(nCountDownMin), (nCountDownBeep == 0 || bDot));
		break;
	case SMODE_ALARM:
		DisplayDecNum (nAlarmM, nAlarmH, DMODE_CLOCK | DMODE_FLAG_PM);
		break;
	default:
		DisplayErr ();
		break;
	}

	nDisplay = BCDHour24(nCurrentTime.Hour);
	bNight = nDisplay >= 0x20 || nDisplay <= 0x6;
	bSleep = nDisplay >= 0x21 || nDisplay <= 0x5;
}

void OnTimer () interrupt 1
{
	static unsigned char nLastSec;
	static unsigned char nShowCount = 0;
		
	// Timer reset
	TL0 = 0xD7; //0xCD;
	TH0 = 0xFD; //0xD4;			// 0.05ms

	// Clock
	nMs ++;
	nShowCount ++;
	if (nMs >= 200)
	{
		nMs = 0;
		n10Ms ++;

		if (n10Ms >= 10)
		{
			n10Ms = 0;
			n100Ms ++;
			if (n100Ms == 5)
			{
				bDot = 1;
			}
			else if (n100Ms >= 10)
			{
				bDot = 0;
				n100Ms = 0;
			}

			// Close Tick Sound
			if (bSound)
			{
				if (bBeep && BEEP)
					BEEP = 0;
				else
				{
					BEEP = 1;
					bSound = 0;
				}
			}

			// Read DS1302
			nLastSec = nCurrentTime.Second;
	
			if (!bBlockClock)
				if (!DS1302ReadTime (&nCurrentTime))	//Read Time
					return;
	
			// If time is changed
			if (nCurrentTime.Second != nLastSec)
			{
				n100Ms = 0;
				n10Ms = 0;
				nMs = 0;
				bDot = 0;
		
				if (nMode == MODE_CLOCK)
				{
					switch (nSubMode)
					{
					case SMODE_COUNTDOWN2:
						if (!nCountDownSec)
						{
							if (nCountDownMin)
							{
								nCountDownMin --;
								nCountDownSec = 59;
							}
							else if (nCountDownBeep)
							{
								bSound = 1;
								nCountDownBeep --;
							}
						}
						else
							nCountDownSec --;
						break;
					case SMODE_TEMP:
						if (nSubModeTime)
						{
							nTemp = ReadTemperature (&nTempL);
							nSubModeTime --;
						}
						else
							nSubMode = SMODE_CLOCK;
						break;
					case SMODE_CLOCK:
						if (nSubModeTime)
							nSubModeTime --;
						break;
					default:
						break;
					}
				}
				
				if (BCDHour24(nCurrentTime.Hour) == nAlarmH && nCurrentTime.Minute == nAlarmM && nCurrentTime.Second < 5)
					bSound = 1;
			}  // Time Changed

			PreDisplay ();

		}	// Per 100 MS
	}	// Per 10 MS


	if (nMode == MODE_CLOCK && nSubMode == SMODE_CLOCK && bNightMode && (( bSleep && !nSubModeTime) ||
		 (bNight && nShowCount < 10)))
	{
		// Hide Display
		P37 = 1;
		P36 = 1;
		P35 = 1;
		P34 = 1;
	}
	else if (nShowCount >= 10)
	{
		// Display
		nShowCount = 0;
	
		switch (nLED)
		{
		case 0:
			P34 = 1;
			if (nShow & 0x1)
			{
				P2 = nData0;
				P37 = 0; //Lighton LED
			}
			nLED ++;
			break;
		case 1:
			P37 = 1;
			if (nShow & 0x2)
			{
				P2 = nData1;
				P36 = 0;
			}
			nLED ++;
			break;
		case 2:
			P36 = 1;
			if (nShow & 0x4)
			{
				P2 = nData2;
				P35 = 0;
			}
			nLED ++;
			break;
		case 3:
			P35 = 1;
			if (nShow & 0x8)
			{
				P2 = nData3;
				P34 = 0;
			}
			nLED = 0;
			break;
		}
	}
}


void Delay10ms()
{
	unsigned char n10MsSave, nMsSave;

	n10MsSave = n10Ms;
	nMsSave = nMs;
	while (n10Ms == n10MsSave);
	n10MsSave = n10Ms;
	while (nMsSave != nMs && n10Ms == n10MsSave);
}

void HourInc ()
{
	unsigned char nHour;

	nHour = nCurrentTime.Hour;

	if (nHour & 0x80) // 12 Hour mode
	{
		if ((nHour & 0xA0) == 0xA0)	//PM
		{
			nHour = GETBCD (nHour & 0x1F);
			if (nHour != 12) //12 PM means 12 aclock
				nHour += 12;
		}
		else					   //AM
		{
			nHour = GETBCD (nHour & 0x1F);
			if (nHour == 12)			//12 AM means 0 aclock
				nHour = 0;
		}

		if (++nHour > 23)
			nCurrentTime.Hour = 0x92;		//12AM
		else if (nHour >= 12)				//PM
		{
			nHour -= 12;
			if (nHour == 0)
				nCurrentTime.Hour = 0xB2;	//12 PM
			else
				nCurrentTime.Hour = 0xA0 | MAKEBCD (nHour);
		}
		else
			nCurrentTime.Hour = 0x80 | MAKEBCD (nHour);
	}
	else
	{
		nHour = GETBCD (nHour);
		if (++nHour > 23)
			nHour = 0;
		nCurrentTime.Hour = MAKEBCD (nHour);
	}
}

void HourDec ()
{
	unsigned char nHour;

	nHour = nCurrentTime.Hour;

	if (nHour & 0x80) // 12 Hour mode
	{
		if ((nHour & 0xA0) == 0xA0)	//PM
		{
			nHour = GETBCD (nHour & 0x1F);
			if (nHour != 12) //12 PM means 12 aclock
				nHour += 12;
		}
		else					   //AM
		{
			nHour = GETBCD (nHour & 0x1F);
			if (nHour == 12)			//12 AM means 0 aclock
				nHour = 0;
		}

		if (nHour > 0)
			nHour --;
		else
			nHour = 23;

		if (nHour == 0)
			nCurrentTime.Hour = 0x92;		//12AM
		else if (nHour >= 12)				//PM
		{
			nHour -= 12;
			if (nHour == 0)
				nCurrentTime.Hour = 0xB2;	//12 PM
			else
				nCurrentTime.Hour = 0xA0 | MAKEBCD (nHour);
		}
		else
			nCurrentTime.Hour = 0x80 | MAKEBCD (nHour);
	}
	else
	{
		nHour = GETBCD (nHour);

		if (nHour > 0)
			nHour --;
		else
			nHour = 23;
		nCurrentTime.Hour = MAKEBCD (nHour);
	}
}

unsigned char BCDInc (unsigned char nBCD, unsigned char nBCDLimit, unsigned char nBCDLow)
{
	if (nBCD >= nBCDLimit || nBCD == 0x99)
		return nBCDLow;

	if ((nBCD & 0xF) == 9)
		return (nBCD & 0xF0) + 0x10;
	return (nBCD + 1);
}

unsigned char BCDDec (unsigned char nBCD, unsigned char nBCDLimit, unsigned char nBCDLow)
{
	if (nBCD <= nBCDLow || nBCD == 0)
		return nBCDLimit;

	if ((nBCD & 0xF) == 0)
		return nBCD - 0x7; //0x10 + 0x9
	return (nBCD - 1);
}

void AdjustTime (bit bIncrease)
{
	bit bModeChange = 0;

	bBlockClock = 1;

	switch (nMode)
	{
	case MODE_HOUR:
		if (bIncrease)
			HourInc ();
		else
			HourDec ();
		break;
	case MODE_MINUTE:
		if (bIncrease)
			nCurrentTime.Minute = BCDInc (nCurrentTime.Minute, 0x59, 0);
		else
			nCurrentTime.Minute = BCDDec (nCurrentTime.Minute, 0x59, 0);
		nCurrentTime.Second = 0;
		break;
	case MODE_MONTH:
		if (bIncrease)
			nCurrentTime.Month = BCDInc (nCurrentTime.Month, 0x12, 1);
		else
			nCurrentTime.Month = BCDDec (nCurrentTime.Month, 0x12, 1);
		break;
	case MODE_DAY:
		if (bIncrease)
			nCurrentTime.Day = BCDInc (nCurrentTime.Day, tblMonthDays[nCurrentTime.Month] + (nCurrentTime.Month == 2 && (GETBCD(nCurrentTime.Year) % 4) == 0), 1);
		else
			nCurrentTime.Day = BCDDec (nCurrentTime.Day, tblMonthDays[nCurrentTime.Month] + (nCurrentTime.Month == 2 && (GETBCD(nCurrentTime.Year) % 4) == 0), 1);
		break;
	case MODE_WEEK:
		if (bIncrease)
			nCurrentTime.Week = BCDInc (nCurrentTime.Week, 7, 1);
		else
			nCurrentTime.Week = BCDDec (nCurrentTime.Week, 7, 1);
		break;
	case MODE_YEAR:
		if (bIncrease)
			nCurrentTime.Year = BCDInc (nCurrentTime.Year, 0x99, 0);
		else
			nCurrentTime.Year = BCDDec (nCurrentTime.Year, 0x99, 0);
		break;
	case MODE_HOUR24:
		if (nCurrentTime.Hour & 0x80)	// 12 HOUR
			nCurrentTime.Hour = BCDHour24 (nCurrentTime.Hour);
		else
			nCurrentTime.Hour = BCDHour12 (nCurrentTime.Hour);
		break;
	case MODE_ALARM_H:
		if (bIncrease)
			nAlarmH = BCDInc (nAlarmH, 0x23, 0);
		else
			nAlarmH = BCDDec (nAlarmH, 0x23, 0);
		DS1302WriteMyData (2, nAlarmH);
		bBlockClock = 0;
		return;
	case MODE_ALARM_M:
		if (bIncrease)
			nAlarmM = BCDInc (nAlarmM, 0x59, 0);
		else
			nAlarmM = BCDDec (nAlarmM, 0x59, 0);
		DS1302WriteMyData (3, nAlarmM);
		bBlockClock = 0;
		return;
	case MODE_BEEP:
		bBeep = !bBeep;
		bModeChange = 1;
		break;
	case MODE_UP:
		bUp = !bUp;
		bModeChange = 1;
		break;
	case MODE_NIGHT:
		bNightMode = !bNightMode;
		bModeChange = 1;
		break;

	default:
		bBlockClock = 0;
		return;
	}

	if (bModeChange)
	{
		DS1302WriteMyData (1, (unsigned char)bBeep | ((unsigned char)bUp << 1) | ((unsigned char)bNightMode << 2));
		bBlockClock = 0;
		return;
	}
	DS1302WriteTime (&nCurrentTime);//Write Time
	n100Ms = 0;
	n10Ms = 0;
	nMs = 0;
	bBlockClock = 0;
}

void InitTimer ()
{
	unsigned char nMyData;

	bBlockClock = 1;

	bDot = 0;
	n100Ms = 0;
	n10Ms = 0;
	nMs = 0;
	nLED = 0;

	ClockInit ();

	nMyData = DS1302ReadMyData (1);
	bBeep = nMyData & 0x1;
	bUp = (nMyData & 0x2);
	bNightMode = (nMyData & 0x4);
	nAlarmH = DS1302ReadMyData (2);
	nAlarmM = DS1302ReadMyData (3);
	nAlarmH = (nAlarmH >= 24) ? 0 : nAlarmH;
	nAlarmM = (nAlarmM >= 60) ? 0 : nAlarmM;

//	DS1302ReadTime (&nCurrentTime);
//	PreDisplay ();

	bBlockClock = 0;

	AUXR |= 0x80;		// 1T mode
	TL0 = 0xCD;
	TH0 = 0xD4;			// 1ms
	TMOD = 1;			// Timer Mode
	TCON = 0x10;		// Timer
	IE = 0x82;			// INT
}

void main()
{
	unsigned char nPressedSec, nPressedMs, nDiffMs, nLimitMs;
	bit bIncrease;

	nTemp = ReadTemperature (&nTempL);
	InitTimer ();

	BEEP = 1;
	nMode = 0;
	bPressedS1 = 0;
	bPressedS2 = 0;
	bIncrease = 1;


	while (1)
	{
		if (!KEY_S1)
		{
			Delay10ms ();
			if (!KEY_S1)
			{
				if (nMode == MODE_CLOCK)
				{
					while (!KEY_S1);

					if (nSubMode != SMODE_CLOCK || !bSleep || nSubModeTime) //Not Sleep
						if (++nSubMode > SMODE_MAX)
							nSubMode = 0;
					if (nSubMode == SMODE_COUNTDOWN1)
					{
						nCountDownMin = 1;
						nCountDownSec = 0;
						nCountDownBeep = 3;
					}
					bSound = 1;
					nSubModeTime = SMODE_COUNTSEC;
				}
				else
				{
					nLimitMs = 20;  //1.5sec
					nPressedSec = nCurrentTime.Second;
					nPressedMs = n100Ms;
					bPressedS1 = 0;
					while (!KEY_S1)
					{				
						if (!KEY_OK)	// Press OK & S1
						{
							while (!KEY_OK || !KEY_S1);	//Until all released
							bIncrease = !bIncrease;
							break;
						}

						nDiffMs = (((nCurrentTime.Second < nPressedSec) ? 60 : 0) + nCurrentTime.Second - nPressedSec) * 10 + ((nCurrentTime.Second == nPressedSec && n100Ms < nPressedMs) ? 10 : 0) + n100Ms - nPressedMs;
						//nDiffMs = 0.1sec
	
						if (nDiffMs > nLimitMs)
						{
							AdjustTime (bIncrease);
							bPressedS1 = 1;
							nPressedSec = nCurrentTime.Second;
							nPressedMs = n100Ms;
							nLimitMs = 5;  //0.3 sec
							bSound = 1;
						}
					}
					if (!bPressedS1) //Single Click
					{
						AdjustTime (bIncrease);
						bSound = 1;
					}
					else
						bPressedS1 = 0;
				}
			}
		}
		if (!KEY_OK)
		{
			Delay10ms ();
			if (!KEY_OK)
			{
				while (!KEY_OK);

				// Press S2
				if (++nMode > MODE_MAX)
				{
					nMode = 0;
					nSubMode = 0;
				}
				bIncrease = 1;	//Reset to Increasing
				bSound = 1;
			}
		}
	}
}