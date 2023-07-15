#include <xc.h>
#include <sys/attribs.h>
#include <string.h>

#define X_MAX 232
#define Y_MAX 160
#define PSET1(x,y) {if((x)>=0 && (x)<=X_MAX && (y)>=0 && (y)<=Y_MAX)video[(y)+56][(x)/32+2] |= (0x80000000>>((x)%32));}
#define PCLR1(x,y) {if((x)>=0 && (x)<=X_MAX && (y)>=0 && (y)<=Y_MAX)video[(y)+56][(x)/32+2] &= ~(0x80000000>>((x)%32));}

extern uint32_t video[262][10];          // 15,720 byte

void char_disp(int x, int y, int c, int rev_flag);
void display(int x, int y, const char * str);
void rev_display(int x, int y, const char * str);
void g_pset(int x, int y, int color);
void line1(int x1, int y1, int x2, int y2, int color);
void circle1(int x0, int y0, int r, int color);
void boxfill(int x1, int y1, int x2, int y2, int c);
void circlefill(int x0, int y0, int r, int c);

/********************************************************/
/*      charactor generator                             */
/********************************************************/
const unsigned char chara_gene[] = {  // 6x8 charactor, bit0 Ç™è„
0,0x00,0x00,0x00,0x00,0x00,	// ' '
0,0x00,0x00,0x5f,0x00,0x00,	// '!'
0,0x00,0x07,0x00,0x07,0x00,	// '"'
0,0x14,0x7f,0x14,0x7f,0x14,	// '#'
0,0x24,0x2a,0x7f,0x2a,0x12,	// '$'
0,0x23,0x13,0x08,0x64,0x62,	// '%'
0,0x36,0x49,0x55,0x22,0x50,	// '&'
0,0x00,0x05,0x03,0x00,0x00,	// '''
0,0x00,0x1c,0x22,0x41,0x00,	// '('
0,0x00,0x41,0x22,0x1c,0x00,	// ')'
0,0x14,0x08,0x3e,0x08,0x14,	// '*'
0,0x08,0x08,0x3e,0x08,0x08,	// '+'
0,0x00,0x50,0x30,0x00,0x00,	// ','
0,0x08,0x08,0x08,0x08,0x08,	// '-'
0,0x00,0x60,0x60,0x00,0x00,	// '.'
0,0x20,0x10,0x08,0x04,0x02,	// '/'

0,0x3e,0x51,0x49,0x45,0x3e,	// '0'
0,0x00,0x42,0x7f,0x40,0x00,	// '1'
0,0x42,0x61,0x51,0x49,0x46,	// '2'
0,0x21,0x41,0x45,0x4b,0x31,	// '3'
0,0x18,0x14,0x12,0x7f,0x10,	// '4'
0,0x27,0x45,0x45,0x45,0x39,	// '5'
0,0x3c,0x4a,0x49,0x49,0x30,	// '6'
0,0x01,0x71,0x09,0x05,0x03,	// '7'
0,0x36,0x49,0x49,0x49,0x36,	// '8'
0,0x06,0x49,0x49,0x29,0x1e,	// '9'
0,0x00,0x36,0x36,0x00,0x00,	// ':'
0,0x00,0x56,0x36,0x00,0x00,	// ';'
0,0x08,0x14,0x22,0x41,0x00,	// '<'
0,0x14,0x14,0x14,0x14,0x14,	// '='
0,0x00,0x41,0x22,0x14,0x08,	// '>'
0,0x02,0x01,0x51,0x09,0x06,	// '?'

0,0x32,0x49,0x79,0x41,0x3e,	// '@'
0,0x7e,0x11,0x11,0x11,0x7e,	// 'A'
0,0x7f,0x49,0x49,0x49,0x36,	// 'B'
0,0x3e,0x41,0x41,0x41,0x22,	// 'C'
0,0x7f,0x41,0x41,0x22,0x1c,	// 'D'
0,0x7f,0x49,0x49,0x49,0x41,	// 'E'
0,0x7f,0x09,0x09,0x09,0x01,	// 'F'
0,0x3e,0x41,0x49,0x49,0x7a,	// 'G'
0,0x7f,0x08,0x08,0x08,0x7f,	// 'H'
0,0x00,0x41,0x7f,0x41,0x00,	// 'I'
0,0x20,0x40,0x41,0x3f,0x01,	// 'J'
0,0x7f,0x08,0x14,0x22,0x41,	// 'K'
0,0x7f,0x40,0x40,0x40,0x40,	// 'L'
0,0x7f,0x02,0x0c,0x02,0x7f,	// 'M'
0,0x7f,0x04,0x08,0x10,0x7f,	// 'N'
0,0x3e,0x41,0x41,0x41,0x3e,	// 'O'

0,0x7f,0x09,0x09,0x09,0x06,	// 'P'
0,0x3e,0x41,0x51,0x21,0x5e,	// 'Q'
0,0x7f,0x09,0x19,0x29,0x46,	// 'R'
0,0x46,0x49,0x49,0x49,0x31,	// 'S'
0,0x01,0x01,0x7f,0x01,0x01,	// 'T'
0,0x3f,0x40,0x40,0x40,0x3f,	// 'U'
0,0x1f,0x20,0x40,0x20,0x1f,	// 'V'
0,0x3f,0x40,0x38,0x40,0x3f,	// 'W'
0,0x63,0x14,0x08,0x14,0x63,	// 'X'
0,0x07,0x08,0x70,0x08,0x07,	// 'Y'
0,0x61,0x51,0x49,0x45,0x43,	// 'Z'
0,0x00,0x7f,0x41,0x41,0x00,	// '['
0,0x15,0x16,0x3c,0x16,0x15,	// '\'
0,0x00,0x41,0x41,0x7f,0x00,	// ']'
0,0x04,0x02,0x01,0x02,0x04,	// '^'
0,0x40,0x40,0x40,0x40,0x40,	// '_'

0,0x00,0x01,0x02,0x04,0x00,	// '`'
0,0x20,0x54,0x54,0x54,0x78,	// 'a'
0,0x7f,0x48,0x44,0x44,0x38,	// 'b'
0,0x38,0x44,0x44,0x44,0x20,	// 'c'
0,0x38,0x44,0x44,0x48,0x7f,	// 'd'
0,0x38,0x54,0x54,0x54,0x18,	// 'e'
0,0x08,0x7e,0x09,0x01,0x02,	// 'f'
0,0x0c,0x52,0x52,0x52,0x3e,	// 'g'
0,0x7f,0x08,0x04,0x04,0x78,	// 'h'
0,0x00,0x44,0x7d,0x40,0x00,	// 'i'
0,0x20,0x40,0x44,0x3d,0x00,	// 'j'
0,0x7f,0x10,0x28,0x44,0x00,	// 'k'
0,0x00,0x00,0x7f,0x00,0x00,	// 'l'
0,0x7c,0x04,0x18,0x04,0x78,	// 'm'
0,0x7c,0x08,0x04,0x04,0x78,	// 'n'
0,0x38,0x44,0x44,0x44,0x38,	// 'o'

0,0x7c,0x14,0x14,0x14,0x08,	// 'p'
0,0x08,0x14,0x14,0x18,0x7c,	// 'q'
0,0x7c,0x08,0x04,0x04,0x08,	// 'r'
0,0x48,0x54,0x54,0x54,0x20,	// 's'
0,0x04,0x3f,0x44,0x40,0x20,	// 't'
0,0x3c,0x40,0x40,0x20,0x7c,	// 'u'
0,0x1c,0x20,0x40,0x20,0x1c,	// 'v'
0,0x3c,0x40,0x30,0x40,0x3c,	// 'w'
0,0x44,0x28,0x10,0x28,0x44,	// 'x'
0,0x0c,0x50,0x50,0x50,0x3c,	// 'y'
0,0x44,0x64,0x54,0x4c,0x44,	// 'z'
0,0x00,0x08,0x36,0x41,0x00,	// '{'
0,0x00,0x00,0x7f,0x00,0x00,	// '|'
0,0x00,0x41,0x36,0x08,0x00,	// '}'
0,0x08,0x04,0x08,0x10,0x08, // '~'
0,0x44,0x44,0x5f,0x44,0x44, // 'Å}'
};
/********************************************************************************/
/*		char_disp																*/
/********************************************************************************/
void char_disp(int x, int y, int c, int rev_flag)
{
	int x1, y1, c1;
	
	if(c<0x20 || c>0x7f)
		return;
	for(x1=0; x1<6; x1++){
		c1 = chara_gene[(c-0x20)*6+x1];
		for(y1=0; y1<8; y1++){
			if(rev_flag==0){
				if(c1 & (1<<y1))
					PSET1(x+x1, y+y1);
			}
			else{
				if((c1 & (1<<y1))==0)
					PSET1(x+x1, y+y1);
			}
		}
	}
}

void display(int x, int y, const char * str)
{
	while(*str){
		char_disp(x, y, *str++, 0);
		x += 6;
	}
}

void rev_display(int x, int y, const char * str)
{
	while(*str){
		char_disp(x, y, *str++, 1);
		x += 6;
	}
}
/********************************************************************************/
/*		g_pset																	*/
/********************************************************************************/
void g_pset(int x, int y, int color)
{
	if(color){
		PSET1(x,y);
    }
	else{
		PCLR1(x,y);
    }
}
/********************************************************************************/
/*		line1																	*/
/********************************************************************************/
void line1(int x1, int y1, int x2, int y2, int color)
{
	int sx,sy,dx,dy,i;
	int e;

	if(x2>x1){
		dx=x2-x1;
		sx=1;
	}
	else{
		dx=x1-x2;
		sx=-1;
	}
	if(y2>y1){
		dy=y2-y1;
		sy=1;
	}
	else{
		dy=y1-y2;
		sy=-1;
	}
	if(dx>=dy){
		e=-dx;
		for(i=0;i<=dx;i++){
			g_pset(x1,y1,color);
			x1+=sx;
			e+=dy*2;
			if(e>=0){
				y1+=sy;
				e-=dx*2;
			}
		}
	}
	else{
		e=-dy;
		for(i=0;i<=dy;i++){
			g_pset(x1,y1,color);
			y1+=sy;
			e+=dx*2;
			if(e>=0){
				x1+=sx;
				e-=dy*2;
			}
		}
	}
}
/********************************************************************************/
/*		erase_line																*/
/********************************************************************************/
void erase_line(int start_line, int end_line)
{
	if(start_line < 0) start_line = 0;
	if(start_line > Y_MAX) start_line = Y_MAX;
	if(end_line < start_line) end_line = start_line;
	if(end_line > Y_MAX) end_line = Y_MAX;
	for(; start_line<=end_line; ++start_line){
		memset(video[start_line+56], 0, sizeof(uint32_t)*10);
	}
}
/********************************************************************************/
/*		circle1																	*/
/*		(x0,y0) ÇíÜêSÇ…ÅAîºåa r ÇÃâ~Çï`Ç≠										*/
/********************************************************************************/
void circle1(int x0, int y0, int r, int color)
{
	int x,y,f;
	x=r;
	y=0;
	f=-2*r+3;
	while(x>=y){
		g_pset(x0-x,y0-y,color);
		g_pset(x0-x,y0+y,color);
		g_pset(x0+x,y0-y,color);
		g_pset(x0+x,y0+y,color);
		g_pset(x0-y,y0-x,color);
		g_pset(x0-y,y0+x,color);
		g_pset(x0+y,y0-x,color);
		g_pset(x0+y,y0+x,color);
		if(f>=0){
			x--;
			f-=x*4;
		}
		y++;
		f+=y*4+2;
	}
}
/********************************************************************************/
/*		circle2																	*/
/*		(x0,y0) ÇíÜêSÇ…ÅAîºåa r ÇÃâ~Çï`Ç≠										*/
/********************************************************************************/
void circle2(int x0, int y0, int rx, int ry, int color)
{
	int x,y,f;
	x=rx;
	y=0;
	f=(3-rx-ry)*rx*ry;
//    f = -(rx+ry)*rx*ry/2;
	while(x*ry*ry >= y*rx*rx){
		g_pset(x0-x,y0-y,color);
		g_pset(x0-x,y0+y,color);
		g_pset(x0+x,y0-y,color);
		g_pset(x0+x,y0+y,color);
		if(f>=0){
			x--;
			f-=x*4*ry*ry;
		}
		y++;
		f+=y*4*rx*rx;
	}
	g_pset(x0-x,y0-y,color);
	g_pset(x0-x,y0+y,color);
	g_pset(x0+x,y0-y,color);
	g_pset(x0+x,y0+y,color);

	y=ry;
	x=0;
	f=(3-rx-ry)*rx*ry;
//    f = -(rx+ry)*rx*ry/2;
	while(y*rx*rx >= x*ry*ry){
		g_pset(x0-x,y0-y,color);
		g_pset(x0-x,y0+y,color);
		g_pset(x0+x,y0-y,color);
		g_pset(x0+x,y0+y,color);
		if(f>=0){
			y--;
			f-=y*4*rx*rx;
		}
		x++;
		f+=x*4*ry*ry;
	}
	g_pset(x0-x,y0-y,color);
	g_pset(x0-x,y0+y,color);
	g_pset(x0+x,y0-y,color);
	g_pset(x0+x,y0+y,color);
}
/********************************************************************************/
/*		boxfill																	*/
/*		(x1,y1),(x2,y2)ÇëŒäpê¸Ç∆Ç∑ÇÈÉJÉâÅ[cÇ≈ìhÇÁÇÍÇΩí∑ï˚å`Çï`âÊ				*/
/********************************************************************************/
void boxfill(int x1, int y1, int x2, int y2, int c)
{
	int temp;
	if(x1>x2){
		temp=x1;
		x1=x2;
		x2=temp;
	}
	if(x2<0 || x1>X_MAX) return;
	if(y1>y2){
		temp=y1;
		y1=y2;
		y2=temp;
	}
	if(y2<0 || y1>Y_MAX) return;
	if(y1<0) y1=0;
	if(y2>Y_MAX) y2=Y_MAX-1;
	while(y1<=y2){
		line1(x1,y1,x2,y1,c);
        y1++;
	}
}
/********************************************************************************/
/*		circlefill																*/
/*		(x0,y0)ÇíÜêSÇ…ÅAîºåarÅAÉJÉâÅ[cÇ≈ìhÇÁÇÍÇΩâ~Çï`âÊ						*/
/********************************************************************************/
void circlefill(int x0, int y0, int r, int c)
{
	int x,y,f;
	x=r;
	y=0;
	f=-2*r+3;
	while(x>=y){
		line1(x0-x,y0-y,x0+x,y0-y,c);
		line1(x0-x,y0+y,x0+x,y0+y,c);
		line1(x0-y,y0-x,x0+y,y0-x,c);
		line1(x0-y,y0+x,x0+y,y0+x,c);
		if(f>=0){
			x--;
			f-=x*4;
		}
		y++;
		f+=y*4+2;
	}
}
/********************************************************************************/
/*		circlefill																*/
/*		(x0,y0)ÇíÜêSÇ…ÅAîºåarÅAÉJÉâÅ[cÇ≈ìhÇÁÇÍÇΩâ~Çï`âÊ						*/
/********************************************************************************/
void circlefill2(int x0, int y0, int rx, int ry, int c)
{
	int x,y,f;
	x=rx;
	y=0;
//	f=(3-rx-ry)*rx*ry;
    f = -(rx+ry)*rx*ry/2;
	while(x*ry*ry >= y*rx*rx){
		line1(x0-x,y0-y,x0+x,y0-y,c);
		line1(x0-x,y0+y,x0+x,y0+y,c);
		if(f>=0){
			x--;
			f-=x*4*ry*ry;
		}
		y++;
		f+=y*4*rx*rx;
	}
	line1(x0-x,y0-y,x0+x,y0-y,c);
	line1(x0-x,y0+y,x0+x,y0+y,c);

	y=ry;
	x=0;
//	f=(3-rx-ry)*rx*ry;
    f = -(rx+ry)*rx*ry/2;
	while(y*rx*rx >= x*ry*ry){
		line1(x0-x,y0-y,x0+x,y0-y,c);
		line1(x0-x,y0+y,x0+x,y0+y,c);
		if(f>=0){
			y--;
			f-=y*4*rx*rx;
		}
		x++;
		f+=x*4*ry*ry;
	}
	line1(x0-x,y0-y,x0+x,y0-y,c);
	line1(x0-x,y0+y,x0+x,y0+y,c);
}
