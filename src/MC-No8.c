/* Kernel includes. */
#include "../FreeRTOS.h"
#include "../task.h"
#include <xc.h>
//#include "stdafx.h"
//#include "Othello.h"
//#include "OthelloDlg.h"
//#include "afxdialogex.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GetTickCount() xTaskGetTickCount()
#define Sleep(x) vTaskDelay(x)

#define macrox(pos) ((pos)%9)	/* pos からＸ座標への変換 x=1～8	*/
#define macroy(pos) ((pos)/9)	/* pos からＹ座標への変換 y=1～8	*/
#define macropos(x,y) ((x)+(y)*9)	/* X,Y 座標から pos への変換	*/
#define macroinv(koma) ((koma)==ISHI_K ? ISHI_S : \
            ((koma)==ISHI_S ? ISHI_K : Dispbug("macroinv error k=%d\n",koma)))
#define abs1(x) ((x)<0 ? -(x) : (x))

#define ISHI_N		0
#define ISHI_K		1
#define ISHI_S		2
#define ISHI_O		3

int dir3[8]={-8,-9,-10,-1,1,8,9,10};

int tekazu;
int teban;
int white;							//0:cpu, 1:human
int black;							//0:cpu, 1:human
char goban[100];
char msg1[50];
char msg2[50];
char msg3[50];
int timesetting;
int brink_flag;

extern int vn_exec;
extern uint32_t video[262][10];          // 15,720 byte

long genrand_int31(void);
int human_input(int pos);
void dispban(char ban[100], int pos);
int Dispbug(const char *fmt, ...);
void msg_printf(const char *fmt, ...);
void msg_main_printf(const char *fmt, ...);
int gametop(long *pten);
void brinking(int k, int pos);

/********************************************************************************/
/*		initban																	*/
/********************************************************************************/
void initban(char *ban)
{
	int x, y, p;
	
	memset(ban, ISHI_O, 100);
	for(x=1; x<=8; x++){
		for(y=1; y<=8; y++){
			p = macropos(x,y);
			ban[p] = ISHI_N;
		}
	}
	ban[macropos(4,4)] = ISHI_S;
	ban[macropos(5,5)] = ISHI_S;
	ban[macropos(4,5)] = ISHI_K;
	ban[macropos(5,4)] = ISHI_K;
}
/********************************************************************************/
/*		MCrnd																	*/
/********************************************************************************/
long MCrnd(long i)
{
	return genrand_int31()%i;
}
/********************************************************************************/
/*		game_init																*/
/********************************************************************************/
void game_init()
{
	initban(goban);
	tekazu = 1;
	teban = ISHI_K;
//	white = 0;
//	black = 0;
}
/********************************************************************************/
/*		yomitst																	*/
/********************************************************************************/
int yomitst_s(char *ban, int k, int pos, int dir)
{
	int kk;
	
	if(ban[pos])							// 石無しでなければ
		return 0;							// 打てない
	pos += dir;
	if(ban[pos] != macroinv(k))				// 敵駒に接触していなければ
		return 0;							// 打てない
	for(;;){
		pos += dir;
		kk = ban[pos];
		if(kk==k)							// はさんでいる
			return 1;						// 打てる
		if(kk==ISHI_N || kk== ISHI_O)
			return 0;						// 打てない
	}
}

int yomitst(char *ban, int k, int pos)
{
	int i;
	
	if(ban[pos])							// 石無しでなければ
		return 0;							// 打てない
	for(i=0; i<8; i++){
		if(yomitst_s(ban, k, pos, dir3[i]))	// dir3 方向で打てるか？
			return 1;						// 打てる
	}
	return 0;								// 打てない
}

/********************************************************************************/
/*		get_upos																*/
/********************************************************************************/
void get_upos(char *ban, int k, char upos[60], int *upos_cnt)
{
	int p;
	
	*upos_cnt = 0;
	for(p=10; p<81; p++){
		if(yomitst(ban, k, p))				// 打てるか？
			upos[(*upos_cnt)++] = p;
	}
}

//		この盤面に打つ手があるか？
int upos_tst(char *ban, int k)
{
	char upos[60];
	int upos_cnt;
	
	get_upos(ban, k, upos, &upos_cnt);
	if(upos_cnt==0)
		return 0;								// 打てない
	return 1;									// 打てる
}
/********************************************************************************/
/*		yomiutu																	*/
/*		開放度を返す															*/
/********************************************************************************/
int yomiutu(char *ban, int k, int pos)
{
	int i, j, s, p, dir, kaiho;
	
	s = macroinv(k);
	kaiho = 0;
	for(i=0; i<8; i++){
		dir = dir3[i];
		if(yomitst_s(ban, k, pos, dir)){	// dir 方向で打てるか？
			p = pos;
			for(;;){
				p += dir;
				if(ban[p]==s){
					ban[p] = k;
					for(j=0; j<8; j++){
						if(ban[p+dir3[j]]==ISHI_N)
							++kaiho;
					}
				}
				else{
					break;
				}
			}
		}
	}
	ban[pos] = k;
	return kaiho;
}
/********************************************************************************/
/*		result																	*/
/*		石数を数える、黒+														*/
/********************************************************************************/
int result(char *ban)
{
	int ten, p;
	
	ten = 0;
	for(p=10; p<81; p++){
		if(ban[p]==ISHI_K)
			++ten;
		else if(ban[p]==ISHI_S)
			--ten;
	}
	return ten;
}
/********************************************************************************/
/*		game																	*/
/*		ゲーム進行のメインルーチン												*/
/********************************************************************************/
void game()
{
	int player;								// 0:cpu, 1:human
	int pos, ten;
	long eva;
	
    memset(video, 0, sizeof(video));
	strcpy(msg1, white ? "white: human" : "white: cpu");
	strcpy(msg2, black ? "black: human" : "black: cpu");
	pos = macropos(1,1);
	for(;;){
		/***	パスの処理	***/
		if(upos_tst(goban, teban)==0){		// 打てない
			teban = macroinv(teban);
			if(upos_tst(goban, teban)==0){	// 相手番でも打てない
				ten = result(goban);
				if(ten==0)
					strcpy(msg3, "End: Draw");
				else
					sprintf(msg3, "%s win %d", ten>0 ? "Black" : "White", abs1(ten));
				dispban(goban, pos);
				keyinwait();
				keywait();
				return;
			}
			strcpy(msg3, "=== utenai ===");
            dispban(goban, pos);
			keyinwait();
			keywait();
		}
		player = teban==ISHI_K ? black : white;
		if(player==0){						// cpu の手番
			sprintf(msg3, "%s thinking", teban==ISHI_K ? "B" : "W");
			dispban(goban, pos);
			pos = gametop(&eva);
			if(yomitst(goban, teban, pos)==0)
				Dispbug("game error [%d,%d]", macrox(pos), macroy(pos));
		}
		else{								// 人間の手番
			sprintf(msg3, "%s:Please play", teban == ISHI_K ? "B" : "W");
			for(;;){
				dispban(goban, pos);
				while ((pos = human_input(pos)) == 0) Sleep(100);
				if (yomitst(goban, teban, pos))
					break;
				strcpy(msg3, "Illegal move");
			}
		}
        if(brink_flag){
            brinking(teban, pos);
        }
		yomiutu(goban, teban, pos);
		teban = macroinv(teban);
		++tekazu;
	}
}
/********************************************************************************/
/*		gametop																	*/
/********************************************************************************/
#define INF 10000000

#define T1 8000
#define T2 (-250)
#define T3 30
#define T4 0
#define T5 (-2000)
#define T6 (-10)
#define T7 (-10)
#define T8 (-10)
#define T9 (-10)
#define Ta (-10)
long tenpat_std[]={0,0,0,0,0,0,0,0,0,0,
               T1,T2,T3,T4,T4,T3,T2,T1,0,
               T2,T5,T6,T7,T7,T6,T5,T2,0,
               T3,T6,T8,T9,T9,T8,T6,T3,0,
               T4,T7,T9,Ta,Ta,T9,T7,T4,0,
               T4,T7,T9,Ta,Ta,T9,T7,T4,0,
               T3,T6,T8,T9,T9,T8,T6,T3,0,
               T2,T5,T6,T7,T7,T6,T5,T2,0,
               T1,T2,T3,T4,T4,T3,T2,T1,0,
               };

#define U1 8000
#define U2 (-250)
#define U3 30
#define U4 0
#define U5 (-2000)
#define U6 (-10)
#define U7 (-10)
#define U8 (-10)
#define U9 (-10)
#define Ua (-10)
long tenpat_test[]={0,0,0,0,0,0,0,0,0,0,
               U1,U2,U3,U4,U4,U3,U2,U1,0,
               U2,U5,U6,U7,U7,U6,U5,U2,0,
               U3,U6,U8,U9,U9,U8,U6,U3,0,
               U4,U7,U9,Ua,Ua,U9,U7,U4,0,
               U4,U7,U9,Ua,Ua,U9,U7,U4,0,
               U3,U6,U8,U9,U9,U8,U6,U3,0,
               U2,U5,U6,U7,U7,U6,U5,U2,0,
               U1,U2,U3,U4,U4,U3,U2,U1,0,
               };

long tenpat_end[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				T1, T1, T1, T1, T1, T1, T1, T1, 0,
				T1, T1, T1, T1, T1, T1, T1, T1, 0,
				T1, T1, T1, T1, T1, T1, T1, T1, 0,
				T1, T1, T1, T1, T1, T1, T1, T1, 0,
				T1, T1, T1, T1, T1, T1, T1, T1, 0,
				T1, T1, T1, T1, T1, T1, T1, T1, 0,
				T1, T1, T1, T1, T1, T1, T1, T1, 0,
				T1, T1, T1, T1, T1, T1, T1, T1, 0,
};
long eva_cnt;

void sort_upos(char *ban, int k, int depth, int maxdepth, char *upos, int upos_cnt);

long evaluate(char *ban, int k, long *tenpat, long gamma, int kaiho)
{
	long ten;
	int p;
	
	++eva_cnt;
	ten = 0;
	for(p=10; p<81; p++){
		if(ban[p]==ISHI_K)
			ten += tenpat[p];
		else if(ban[p]==ISHI_S)
			ten -= tenpat[p];
	}
	if(teban <= 15){
		ten += MCrnd(40)-20;				// 乱数加算
	}
	ten = k==ISHI_K ? ten : -ten;

	ten += 20*gamma;
	ten += 10*kaiho;
	return ten;
}

long negamax(char *ban, int k, int depth, int maxdepth, int *pos, long alpha, long beta, int pass, long gamma, int kaiho)
{
	int i, upos_cnt, dummy, kho;
	char ban1[100], upos[60];
	long ten, max;
	
	if(depth>=maxdepth){
		return evaluate(ban, k, tenpat_std, gamma, kaiho);	/* 中間評価関数						*/
	}
	
	max = -INF;
	*pos = 0;
	get_upos(ban, k, upos, &upos_cnt);
	if (upos_cnt == 0){
		if (pass){												// 2回連続パス
			return evaluate(ban, k, tenpat_end, gamma, kaiho);	/* 中間評価関数						*/
		}
		return -negamax(ban, macroinv(k), depth + 1, maxdepth, &dummy, -beta, -alpha, 1, -gamma, -kaiho);
	}
	if(maxdepth-depth >= 4 && upos_cnt >= 2)
		sort_upos(ban, k, depth+4, maxdepth, upos, upos_cnt);
	gamma = gamma/2 + upos_cnt;
	for(i=0; i<upos_cnt; i++){
		memcpy(ban1, ban, 100);
		if(yomitst(ban1, k, upos[i])==0)			/* 打てない					*/
			continue;
		kho = yomiutu(ban1, k, upos[i]);
		if(depth<=1)
			kho = kho-kaiho;
		else
			kho = -kaiho;

		ten = - negamax(ban1, macroinv(k), depth+1, maxdepth, &dummy, -beta, -alpha, 0, -gamma, kho);
		if(ten > beta)							/* 枝刈							*/
			return ten;
		if(max < ten){
			*pos = upos[i];
			max = ten;
			if(max > alpha)
				alpha = max;
		}
	}
	return max;
}

int gametop(long *pten)
{
	int maxdepth, pos;
	long time1, ten;
	
	eva_cnt = 0;
//	maxdepth = 7;
	time1 = GetTickCount();
	for(maxdepth=1; maxdepth<=10; ++maxdepth){
		ten = negamax(goban, teban, 0, maxdepth, &pos, -INF, INF, 0, 0, 0);
		if(GetTickCount()-time1 > timesetting)
			break;
	}
	msg_main_printf("depth=%d %ldms ten=%ld", maxdepth, GetTickCount()-time1, ten);
	*pten = ten;
	return pos;
}
/********************************************************************************/
/*		sort_upos																*/
/********************************************************************************/
struct data {long ten; char upos;};

int cmp_upos(const void *a, const void *b)
{
	struct data *aa = (struct data *)a;
	struct data *bb = (struct data *)b;
	
	if(aa->ten==bb->ten)
		return 0;
	if(aa->ten > bb->ten)
		return -1;
	return 1;
}

void sort_upos(char *ban, int k, int depth, int maxdepth, char *upos, int upos_cnt)
{
	struct data data[60];
	int i, dummy, kaiho;
	char ban1[100];
	
	for(i=0; i<upos_cnt; i++){
		data[i].upos = upos[i];
		memcpy(ban1, ban, 100);
		kaiho = yomiutu(ban1, k, upos[i]);
		data[i].ten = -negamax(ban1, macroinv(k), 1, maxdepth-depth+1, &dummy, -INF, INF, 0, 0, kaiho);
	}
	qsort(data, upos_cnt, sizeof(struct data), cmp_upos);
	for(i=0; i<upos_cnt; i++){
		upos[i] = data[i].upos;
	}
}
