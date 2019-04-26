/*
	(*) DS1302 Supporting Functions


*/

#include "STC\STC15F104E.H"
#include "DS1302.h"

sbit io_DS1302_RST = P0^0 ; 

sbit io_DS1302_IO = P0^1 ; 

sbit io_DS1302_SCLK = P3^2 ; 


//-------------------------------------³£Êýºê---------------------------------// 

#define DS1302_SECOND_WRITE	0x80 //Ð´Ê±ÖÓÐ¾Æ¬µÄ¼Ä´æÆ÷Î»ÖÃ 
#define DS1302_MINUTE_WRITE 0x82 
#define DS1302_HOUR_WRITE	0x84 
#define DS1302_WEEK_WRITE	0x8A 
#define DS1302_DAY_WRITE	0x86 
#define DS1302_MONTH_WRITE	0x88 
#define DS1302_YEAR_WRITE	0x8C 
#define DS1302_WP			0x8E
#define DS1302_CLOCK_WRITE	0xBE


#define DS1302_SECOND_READ	0x81 //¶ÁÊ±ÖÓÐ¾Æ¬µÄ¼Ä´æÆ÷Î»ÖÃ 
#define DS1302_MINUTE_READ	0x83 
#define DS1302_HOUR_READ	0x85 
#define DS1302_WEEK_READ	0x8B 
#define DS1302_DAY_READ		0x87 
#define DS1302_MONTH_READ	0x89 
#define DS1302_YEAR_READ	0x8D 
#define DS1302_WP_READ		0x8F
#define DS1302_CLOCK_READ	0xBF

#define DS1302_RAM0_WRITE	0xC0
#define DS1302_RAM1_WRITE	0xC2

#define DS1302_RAM0_READ	0xC1
#define DS1302_RAM1_READ	0xC3


//-----------------------------------²Ù×÷ºê----------------------------------// 

#define DS1302_SCLK_HIGH io_DS1302_SCLK = 1 ; 

#define DS1302_SCLK_LOW io_DS1302_SCLK = 0 ; 

#define DS1302_IO_HIGH io_DS1302_IO = 1 ; 

#define DS1302_IO_LOW io_DS1302_IO = 0 ; 

#define DS1302_IO io_DS1302_IO 



#define DS1302_RST_HIGH io_DS1302_RST = 1 ; 

#define DS1302_RST_LOW io_DS1302_RST = 0 ; 

static bit bGate = 0;


/****************************************************************************** 

* Function: static void DS1302Write( uint8 Content ) * 

* Description: Write one BYTE

* Parameter:uint8 Content : Wite Value

* * 

******************************************************************************/ 
static void DS1302Write (uint8 nData) 
{
	uint8 nMask;

	for (nMask = 1; nMask; nMask <<= 1)
	{
		DS1302_IO = nData & nMask;

		DS1302_SCLK_HIGH 
		DS1302_SCLK_LOW 
	}
	DS1302_IO_HIGH	// Release I/O
} 

/****************************************************************************** 

* Function: static uint8 DS1302Read( void ) * 

* Description: Read one BYTE 

* Parameter: * 

* Return: Value Read 

******************************************************************************/ 

static uint8 DS1302Read (void) 

{
	uint8 nMask, nReadValue = 0;

	for (nMask = 1; nMask; nMask <<= 1)
	{
		if (DS1302_IO)		//Data is ready at the end of command bytes
			nReadValue |= nMask;

		DS1302_SCLK_HIGH 			//Start for next bit
		DS1302_SCLK_LOW 
	}

	return nReadValue ; 
} 



/****************************************************************************** 

* Function: void DS1302WriteByte( uint8 Address, uint8 Content ) * 

* Description: Write one BYTE 

* Parameter: Address

* Content: Data to write

* Return: * 

******************************************************************************/ 
void DS1302WriteByte (uint8 nAddress, uint8 nData)
{ 
	DS1302_RST_LOW		//Cancel all data
	DS1302_SCLK_LOW		//Set SCLK to low to enable RST
	DS1302_RST_HIGH		//Start data transfering 

	DS1302Write (nAddress);
	DS1302Write (nData); 

	DS1302_RST_LOW		//Stop data transfering
	DS1302_SCLK_HIGH 
} 

/****************************************************************************** 

* Function: uint8 DS1302ReadByte (uint8 nAddress)

* Description:	Read a BYTE from the address

* Parameter: Address

* 

* Return: Readed value

******************************************************************************/ 

uint8 DS1302ReadByte (uint8 nAddress) 

{ 
	uint8 nReadValue ; 

	if (bGate)
		return 0;

	bGate = 1;

	DS1302_RST_LOW		//Cancel all data 
	DS1302_SCLK_LOW 	//Set SCLK to low to enable RST
	DS1302_RST_HIGH		//Start data transfering 


	DS1302Write (nAddress);
	nReadValue = DS1302Read ();

	DS1302_RST_LOW		//Stop data transfering 
	DS1302_SCLK_HIGH 

	bGate = 0;

	return nReadValue ; 
} 

/****************************************************************************** 

* Function: void ClockInit( void ) * 

* Description:	Initialize the Clock if this is the first time power on

* Parameter: * 

* * 

* Return: * 

******************************************************************************/ 

void ClockInit (void)
{
	if (DS1302ReadByte (DS1302_RAM0_READ) != 0xF0)		//Detect Flag
	{ 
		DS1302WriteByte (DS1302_WP, 0x00);				//Enable Write 

		DS1302WriteByte (DS1302_SECOND_WRITE, 0x00);
		DS1302WriteByte (DS1302_MINUTE_WRITE, 0x10);
		DS1302WriteByte (DS1302_HOUR_WRITE, 0x10);
		DS1302WriteByte (DS1302_DAY_WRITE, 0x10);
		DS1302WriteByte (DS1302_MONTH_WRITE, 0x10);
		DS1302WriteByte (DS1302_WEEK_WRITE, 0x05);
		DS1302WriteByte (DS1302_YEAR_WRITE, 0x15);

		//DS1302WriteByte( 0x90, 0xa5 ) ; //Charging 
		DS1302WriteByte( DS1302_RAM0_WRITE, 0xF0 );		//Set Flag
		DS1302WriteByte( DS1302_RAM1_WRITE, 0 );		//Default My Data
		DS1302WriteByte( DS1302_WP, 0x80 );				//Disable Write 
	}
	
	if (DS1302ReadByte (DS1302_SECOND_READ) & 0x80)		//Clock Paused
	{
		DS1302WriteByte (DS1302_WP, 0x00);				//Enable Write 

		DS1302WriteByte (DS1302_SECOND_WRITE, DS1302ReadByte (DS1302_SECOND_READ) & 0x7F); //Clear CH bit

		DS1302WriteByte( DS1302_WP, 0x80 );				//Disable Write 
	}
} 

/****************************************************************************** 

* Function: void DS1302ReadTime (DS1302Time *)

* Description: Read the time from DS1302 and save to the data 

* Parameter: *DS1302Time 

* * 

* Return: TRUE  - Read OK

*         FALSE - Read Error 

******************************************************************************/ 
uint8 DS1302ReadTime (DS1302Time *pCurrentTime)
{
	//Burst Read
	uint8 nReadValue, nCount, nMask;

	if (bGate)
		return 0;

	bGate = 1;

	DS1302_RST_LOW		//Cancel all data 
	DS1302_SCLK_LOW 	//Set SCLK to low to enable RST
	DS1302_RST_HIGH		//Start data transfering 


	DS1302Write (DS1302_CLOCK_READ);
	for (nCount = 0; nCount < 8; nCount ++)
	{
		nReadValue = 0;
		for (nMask = 1; nMask; nMask <<= 1)
		{
			if (DS1302_IO)		//Data is ready at the end of command bytes
				nReadValue |= nMask;
	
			DS1302_SCLK_HIGH 			//Start for next bit
			DS1302_SCLK_LOW 
		}
		((uint8 *)pCurrentTime) [nCount] = nReadValue;
	}

	DS1302_RST_LOW		//Stop data transfering 
	DS1302_SCLK_HIGH 

	bGate = 0;
	return 1;
}


/****************************************************************************** 

* Function: void DS1302WriteTime (DS1302Time *)

* Description: Write the time into DS1302

* Parameter: *DS1302Time 

* * 

* Return: TRUE  - Write OK

*         FALSE - Write Error 

******************************************************************************/ 
unsigned char DS1302WriteTime (DS1302Time *pCurrentTime)
{
	//Burst Write
	uint8 nData, nCount, nMask;

	if (bGate)
		return 0;

	bGate = 1;

	DS1302WriteByte (DS1302_WP, 0x00); //Enable Write

	DS1302_RST_LOW		//Cancel all data 
	DS1302_SCLK_LOW 	//Set SCLK to low to enable RST
	DS1302_RST_HIGH		//Start data transfering 

	DS1302Write (DS1302_CLOCK_WRITE);
	for (nCount = 0; nCount < 8; nCount ++)
	{
		nData = ((uint8 *)pCurrentTime) [nCount];
		for (nMask = 1; nMask; nMask <<= 1)
		{
			DS1302_IO = nData & nMask;
	
			DS1302_SCLK_HIGH 			//Start for next bit
			DS1302_SCLK_LOW 
		}
	}

	DS1302WriteByte (DS1302_WP, 0x80); //Disable Write

	bGate = 0;
	return 1;
}


/****************************************************************************** 

* Function: unsigned char DS1302ReadMyData (void)

* Description: Read my data

* Parameter:

* * 

* Return: Data stored

******************************************************************************/ 
unsigned char DS1302ReadMyData (void)
{
	return DS1302ReadByte (DS1302_RAM1_READ);
}

/****************************************************************************** 

* Function: void DS1302WriteMyData (unsigned char)

* Description: Write my data

* Parameter: My data

* * 

* Return:

******************************************************************************/ 
void DS1302WriteMyData (unsigned char nData)
{
	if (bGate)
		return;

	bGate = 1;

	DS1302WriteByte (DS1302_WP, 0x00); //Enable Write 
	DS1302WriteByte (DS1302_RAM1_WRITE, nData) ; //My Data
	DS1302WriteByte (DS1302_WP, 0x80 ); //Disable Write

	bGate = 0;
}