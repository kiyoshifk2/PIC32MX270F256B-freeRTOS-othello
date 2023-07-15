/* Kernel includes. */
#include "../FreeRTOS.h"
#include "../task.h"
#include <xc.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define macrox(pos) ((pos)%9)	/* pos からＸ座標への変換 x=1～8	*/
#define macroy(pos) ((pos)/9)	/* pos からＹ座標への変換 y=1～8	*/
#define macropos(x,y) ((x)+(y)*9)	/* X,Y 座標から pos への変換	*/

#define ISHI_N		0
#define ISHI_K		1
#define ISHI_S		2
#define ISHI_O		3

struct menu {
	int line_numb;
	int cmd;
	char *msg;
};

extern uint32_t video[262][10];          // 15,720 byte
extern int tekazu;
extern int teban;
extern int white;						//0:cpu, 1:human
extern int black;						//0:cpu, 1:human
extern char goban[100];
extern char msg1[50];
extern char msg2[50];
extern char msg3[50];
extern int timesetting;
extern int brink_flag;

void char_disp(int x, int y, int c, int rev_flag);
void display(int x, int y, const char * str);
void rev_display(int x, int y, const char * str);
void g_pset(int x, int y, int color);
void line1(int x1, int y1, int x2, int y2, int color);
void erase_line(int start_line, int end_line);
void circle1(int x0, int y0, int r, int color);
void circle2(int x0, int y0, int rx, int ry, int color);
void boxfill(int x1, int y1, int x2, int y2, int c);
void circlefill(int x0, int y0, int r, int c);
void circlefill2(int x0, int y0, int rx, int ry, int c);
int keyin();
void keywait();
int menu_select(struct menu *tbl);
void game_init();
void game();
int keyinwait();

/********************************************************************************/
/*		MAINtask																*/
/*      円の縦横比 : 1.19                                                        */
/********************************************************************************/
struct menu play_menu[] = {
	{10,1,"Black:cpu,   White:cpu"},
	{20,2,"Black:human, White:human"},
	{30,3,"Black:cpu,   white:human"},
	{40,4,"Black:hunam, White:cpu"},
	{0,0,0}
};
struct menu strong_menu[] = {
	{10,1,"strong"},
	{20,2,"normal"},
	{30,3,"weak"},
	{0,0,0}
};
struct menu brink_menu[] = {
    {10,1,"brink ON"},
    {20,2,"brink OFF"},
    {0,0,0}
};

void MAINtask(void *pvParameters)
{
	int cmd;

#if 0
    char str[50];
    for(;;){
        sprintf(str,"RPB5R=%x",RPB5R);
        display(0,0,str);
        sprintf(str,"RPB7R=%x",RPB7R);
        display(0,10,str);
        sprintf(str,"RPB8R=%x",RPB8R);
        display(0,20,str);
        sprintf(str,"PMCON=%x",PMCON);
        display(0,30,str);
        sprintf(str,"RB7=%x",PORTBbits.RB7);
        display(0,40,str);
        sprintf(str,"RB8=%x",PORTBbits.RB8);
        display(0,50,str);
        sprintf(str,"RB5=%x",PORTBbits.RB5);
        display(0,60,str);
		vTaskDelay(10);
    }
#endif
    
    for(;;){
        memset(video, 0, sizeof(video));
		display(0, 10, "Othello Game");
		display(0, 30, "SW1: cursol move");
		display(0, 40, "SW2: cursol move");
		display(0, 50, "SW3: execute");
		keyinwait();
		keywait();
		
		cmd = menu_select(play_menu);
		if(cmd==1){
			black = 0;
			white = 0;
		}
		else if(cmd==2){
			black = 1;
			white = 1;
		}
		else if(cmd==3){
			black = 0;
			white = 1;
		}
		else{
			black = 1;
			white = 0;
		}
		
		cmd = menu_select(strong_menu);
		if(cmd==1){
			timesetting = 1000;
		}
		else if(cmd==2){
			timesetting = 100;
		}
		else{
			timesetting = 10;
		}
        
        cmd = menu_select(brink_menu);
        if(cmd==1){
            brink_flag = 1;
        }
        else{
            brink_flag = 0;
        }
		
		game_init();
		game();
	}
    
	for(;;){
		vTaskDelay(1);
	}
}
/********************************************************************************/
/*		koma0																	*/
/*		石無しの駒																*/
/********************************************************************************/
void koma0(int x, int y)
{
	if(x<1 || x>8 || y<1 || y>8)
		return;
	boxfill(x*14+90, y*16, x*14+104, y*16+16, 0);	// clear
	/*** 枠		***/
	line1(x*14+90, y*16, x*14+90, y*16+16, 1);
	line1(x*14+104, y*16, x*14+104, y*16+16, 1);
	line1(x*14+90, y*16, x*14+104, y*16, 1);
	line1(x*14+90, y*16+16, x*14+104, y*16+16, 1);
}

/********************************************************************************/
/*		koma1																	*/
/*		黒駒																	*/
/********************************************************************************/
void koma1(int x, int y)
{
	if(x<1 || x>8 || y<1 || y>8)
		return;
	koma0(x, y);
	circle2(x*14+90+7, y*16+8, 6, 7, 1);
}

/********************************************************************************/
/*		koma2																	*/
/*		白駒																	*/
/********************************************************************************/
void koma2(int x, int y)
{
	if(x<1 || x>8 || y<1 || y>8)
		return;
	koma0(x, y);
	circlefill2(x*14+90+7, y*16+8, 6, 7, 1);
}

/********************************************************************************/
/*		koma3																	*/
/*		小黒丸																	*/
/********************************************************************************/
void koma3(int x, int y)
{
	if(x<1 || x>8 || y<1 || y>8)
		return;
	circlefill(x*14+90+7, y*16+8, 3, 0);
}

/********************************************************************************/
/*		koma4																	*/
/*		小白丸																	*/
/********************************************************************************/
void koma4(int x, int y)
{
	if(x<1 || x>8 || y<1 || y>8)
		return;
	circlefill(x*14+90+7, y*16+8, 3, 1);
}

/********************************************************************************/
/*		dispban																	*/
/********************************************************************************/
void dispban(char ban[100], int pos)
{
	int x, y, p, k;
	
//	memset(video, 0, sizeof(video));
	for(x=1; x<=8; x++){
		for(y=1; y<=8; y++){
			p = macropos(x,y);
			k = ban[p];
			if(k==ISHI_N)
				koma0(x,y);
			else if(k==ISHI_K)
				koma1(x,y);
			else if(k==ISHI_S)
				koma2(x,y);
		}
	}
    for(x=1; x<=8; x++)
        char_disp(x*14+90+4, 16-12, 'A'+x-1, 0);
    for(y=1; y<=8; y++)
        char_disp(14+90-10, y*16+4, '1'+y-1, 0);
    x = macrox(pos);
    y = macroy(pos);
    if(ban[pos]==ISHI_K || ban[pos]==ISHI_N)
    	koma4(x,y);							// 小白丸
    else if(ban[pos]==ISHI_S)
    	koma3(x,y);							// 小黒丸
    
    for(y=40; y<60+8; y++){
		for(x=0; x<14+90-10; x++){
			g_pset(x,y,0);					// clear
		}
	}
    display(0,40,msg1);
    display(0,50,msg2);
    display(0,60,msg3);
}
/************************************************/
/*      keyin                                   */
/*      bit0 : sw1                              */
/*      bit1 : sw2                              */
/*      bit2 : sw3                              */
/************************************************/
int keyins()
{
    int i = 0;

    if(PORTBbits.RB8==0)                   // sw1
        i = 1;
    if(PORTBbits.RB7==0)                   // sw2
        i |= 2;
    if(PORTBbits.RB5==0)
        i |= 4;                            // sw3
    return i;
}
int keyin()
{
    int tmp1, tmp2, i;

    for(i=10; --i; ){
        if(tmp1 != (tmp2=keyins()))
            ++i;
        tmp1 = tmp2;
        vTaskDelay(1);
    }
    return tmp1;
}

// キーから離されるのを待つ
void keywait()
{
    int i;

    for(i=0; i<5; ++i){
        if(keyin())                // まだキーが押されている
            i = 0;
    }
}

// キーが押されるのを待つ
int keyinwait()
{
    int key;
    
    while((key=keyin())==0)vTaskDelay(10);
    return key;
}
/********************************************************************************/
/*		menu_select																*/
/*		return: cmd を返す														*/
/********************************************************************************/
int menu_select(struct menu *tbl)
{
	int i, key;
	
	memset(video, 0, sizeof(video));		// clear
	for(i=0; tbl[i].msg ; ++i){
		display(0, tbl[i].line_numb, tbl[i].msg);
	}
	i = 0;
	for(;;){
		erase_line(tbl[i].line_numb, tbl[i].line_numb+7);
		rev_display(0, tbl[i].line_numb, tbl[i].msg);		// 反転表示
        keywait();
		key = keyinwait();
		if(key & 1){						// sw1: down
			erase_line(tbl[i].line_numb, tbl[i].line_numb+7);
			display(0, tbl[i].line_numb, tbl[i].msg);		// 非反転表示
			if(tbl[++i].msg==0)
				--i;
		
		}
		else if(key & 2){					// sw2: up
			erase_line(tbl[i].line_numb, tbl[i].line_numb+7);
			display(0, tbl[i].line_numb, tbl[i].msg);		// 非反転表示
			if(--i < 0)
				i = 0;
		}
		else if(key & 4){					// sw3: execute
			keywait();						// キーから離されるのを待つ
			return tbl[i].cmd;
		}
	}
}
/********************************************************************************/
/*		Dispbug																	*/
/********************************************************************************/
int Dispbug(const char *fmt, ...)
{
	char buf[50];
	va_list ap;

	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);				/* 文字列作成サブルーチン				*/
	va_end(ap);
	
    memset(video, 0, sizeof(video));
    display(0,40,"FATAL error");
    display(0,60,buf);
    
	return 0;
}
/********************************************************************************/
/*		msg_main_printf															*/
/********************************************************************************/
void msg_main_printf(const char *fmt, ...)
{
	char buf[50];
	va_list ap;
	
	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);				/* 文字列作成サブルーチン				*/
	va_end(ap);

    erase_line(150,160);
    display(0,150,buf);
}
/********************************************************************************/
/*		human_input																*/
/********************************************************************************/
int human_input(int pos)
{
	int x, y, p, key;
	
	x = macrox(pos);
	y = macroy(pos);
	for(;;){
		p = macropos(x,y);
		dispban(goban, p);
        keywait();
		key = keyinwait();
		if(key & 1){
			if(++x==9)
				x = 1;
		}
		else if(key & 2){
			if(++y==9)
				y = 1;
		}
		else if(key & 4){
			return macropos(x,y);
		}
	}
}
/********************************************************************************/
/*      brinking                                                                */
/********************************************************************************/
void brinking(int k, int pos)
{
    int i, x, y;
    
    x = macrox(pos);
    y = macroy(pos);
    for(i=0; i<3; i++){
        if(k==ISHI_K)
            koma1(x,y);
        else
            koma2(x,y);
		vTaskDelay(500);
        koma0(x,y);
		vTaskDelay(500);
    }
}