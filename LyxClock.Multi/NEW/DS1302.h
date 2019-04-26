/*

	(*) DS1302 Interface



*/

typedef unsigned char uint8;

typedef struct { 
	uint8 Second ; 
	uint8 Minute ; 
	uint8 Hour ; 
	uint8 Day ; 
	uint8 Month ; 
	uint8 Week ; 
	uint8 Year ; 
	uint8 WP;
}	DS1302Time ; 


/****************************************************************************** 

* Function: void ClockInit( void ) * 

* Description:	Initialize the Clock if this is the first time power on

* Parameter: * 

* * 

* Return: * 

******************************************************************************/ 
void ClockInit (void);

/****************************************************************************** 

* Function: void DS1302ReadTime( DS1302Time * )

* Description: Read the time from DS1302 and save to the data 

* Parameter: *DS1302Time 

* * 

* Return: TRUE  - Read OK

*         FALSE - Read Error 

******************************************************************************/ 
unsigned char DS1302ReadTime ( DS1302Time *);


/****************************************************************************** 

* Function: void DS1302WriteTime( DS1302Time * )

* Description: Write the time into DS1302

* Parameter: *DS1302Time 

* * 

* Return: TRUE  - Write OK

*         FALSE - Write Error 

******************************************************************************/ 
unsigned char DS1302WriteTime ( DS1302Time *);


/****************************************************************************** 

* Function: unsigned char DS1302ReadMyData (void)

* Description: Read my data

* Parameter:

* * 

* Return: Data stored

******************************************************************************/ 
unsigned char DS1302ReadMyData (void);

/****************************************************************************** 

* Function: void DS1302WriteMyData (unsigned char)

* Description: Write my data

* Parameter: My data

* * 

* Return:

******************************************************************************/ 
void DS1302WriteMyData (unsigned char);

