#include <xc.h>
#include <sys/attribs.h>
#include <string.h>

#define PSET1(x,y) {if((x)>=0 && (x)<=232 && (y)>=0 && (y)<=160)video[(y)+56][(x)/32+2] |= (0x80000000>>((x)%32));}

//uint32_t h_sync[10];
//uint32_t v_sync[10];
uint32_t sync[262][10];
uint32_t video[262][10];                // 15,720 byte

/********************************************************************************/
/*		sync_data_set															*/
//
//      clock = 30MHz
//      shift clock = 30MHz/2/(2+1) = 5MHz  1bit:0.2μs
//      Timer2 30MHz/6/32 = 30MHz/192  1転送 32bit
//      同期信号 10転送 = 32x0.2x10 = 64μs
//      h_sync: 4.8μs(24bit) low, 59.2μs(296bit) high
//      v_sync: 59.2μs(296bit) high, 4.8μs(24bit) low
//      同期信号は v_sync:3cycle + h_sync:259cycle
/********************************************************************************/
void sync_data_set()
{
    int i, j, byte;
    uint32_t bit;
    
    for(j=0; j<262; j++){
        bit = 0x80000000;
        byte = 0;
        for(i=0; i<10*32; i++){
            if(j<3){
                if(i<296){
                    sync[j][byte] &= ~bit;
                }
                else{
                    sync[j][byte] |= bit;
                }
            }
            else{
                if(i<24){
                    sync[j][byte] &= ~bit;
                }
                else{
                    sync[j][byte] |= bit;
                }
            }
            if((bit>>=1)==0){
                bit = 0x80000000;
                ++byte;
            }
        }
    }
}

/********************************************************************************/
/*		spi_init																*/
/********************************************************************************/
void spi_init()
{
	int i;

	sync_data_set();

	/*** SPI setting	***/
	RPB2R = 4;								// SDO2
	SPI2CON = 0x1083C;						// 32bit
	SPI2CONSET = 0x8000;					// Enable
	SPI2BRG = 2;							// 30MHz/2/(2+1)=5MHz
	
	RPB13R = 3;								// SDO1
	SPI1CON = 0x1083C;						// 32bit
	SPI1CONSET = 0x8000;					// Enable
	SPI1BRG = 2;							// 30MHz/2/(2+1)=5MHz
//    IPC7bits.SPI1IP = 1;
//	IFS1bits.SPI1TXIF = 0;
//	IEC1bits.SPI1TXIE = 1;
	
//	for(i=0; i<4; i++)
//		SPI1BUF = 0x33333333;
	
	/*** DMA setting	***/
	DMACON = 0x8000;						// DMA Enable
	
	//channel 2
	DCH2CON = 0;
	DCH2CONbits.CHAEN = 1;					// channel automatic
	DCH2ECONbits.CHSIRQ = _SPI1_TX_IRQ;
	DCH2ECONbits.SIRQEN = 1;				// channel start IRQ enable bit
	DCH2SSA = ((uint32_t) sync)&0x1FFFFFFF;
	DCH2DSA = ((uint32_t) (&SPI1BUF))&0x1FFFFFFF;
	DCH2SSIZ = sizeof(sync);
	DCH2DSIZ = 4;
	DCH2CSIZ = 4;							// 1回の転送バイト数
	DCH2INT = 0;							// no interrupt

	//channel 3
	DCH3CON = 0;
	DCH3CONbits.CHAEN = 1;					// channel automatic
	DCH3ECONbits.CHSIRQ = _SPI2_TX_IRQ;
	DCH3ECONbits.SIRQEN = 1;				// channel start IRQ enable bit
	DCH3SSA = ((uint32_t) video)&0x1FFFFFFF;
	DCH3DSA = ((uint32_t) (&SPI2BUF))&0x1FFFFFFF;
	DCH3SSIZ = sizeof(video);
	DCH3DSIZ = 4;
	DCH3CSIZ = 4;							// 1回の転送バイト数
	DCH3INT = 0;							// no interrupt

	DCH2CONbits.CHEN = 1;					// DMA ch2 enable
	DCH3CONbits.CHEN = 1;					// DMA ch3 enable
	
//	/*** timer2 setting ***/
//	T2CON = 0;
//	PR2 = 127;
//	TMR2 = 0;
//	IPC2bits.T2IP = 4;
//	IFS0bits.T2IF = 0;
//	IEC0bits.T2IE = 1;
//	T2CONSET=0x8000;						// TIMER2 ON

}
/********************************************************************************/
/*		interrupt																*/
/********************************************************************************/
//void __ISR(_DMA1_VECTOR, IPL5AUTO) _DMA1Interrupt ()
//{
//	IFS1bits.DMA1IF = 0;					// Clear DMA ch1 IF
//}
//
//void __ISR(_TIMER_2_VECTOR, IPL5AUTO) T2Interrupt()
//{
//    static int word_cnt;
//    
//	SPI1BUF = h_sync[word_cnt];
//    if(++word_cnt==15)
//        word_cnt = 0;
//	IFS0bits.T2IF = 0;
//}
//
//void __ISR(_SPI_1_VECTOR, IPL5AUTO) SPI1Interrupt()
//{
//    static int word_cnt;
//    static int line_cnt;
//    static int video_cnt;
//    
//    if(line_cnt < 3)
//        SPI1BUF = v_sync[word_cnt];
//    else
//        SPI1BUF = h_sync[word_cnt];
//    if(++word_cnt==10){
//        word_cnt = 0;
//        if(++line_cnt==262)
//            line_cnt = 0;
//    }
//    SPI2BUF = ((uint32_t *)video)[video_cnt];
//    if(++video_cnt==10*262)
//        video_cnt = 0;
//	IFS1bits.SPI1TXIF = 0;
//}
