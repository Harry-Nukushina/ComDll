//main.c
#include <stdio.h>
#include "SwrtCom.h"

#define	fin	"C:\\D\\tmp\\debug\\221205_001_B.RxD"
//#define	fin	"C:\\D\\tmp\\debug\\230414_001_1.RxD"

int main(int argc, char *argv[]) {
	__SL	a,b,n,ie,iv,num;
	__FL	f;
	num = 0;
	n = -1024;
	ie = 3;
	iv = 4;
	a = Lng2Exp(n, ie, iv);
	b = Exp2Lng(a, ie, iv);
	if (n != b) ++num;
	printf("Lng2Exp(%d,%d) : %d , %d , %d\n", ie, iv, n, a, b);
	n = 1024;
	a = Lng2Exp(n, ie, iv);
	b = Exp2Lng(a, ie, iv);
	if (n != b) ++num;
	printf("Lng2Exp(%d,%d) : %d , %d , %d\n", ie, iv, n, a, b);
	n = 0x39800000;
	f = Lng2Flt(n);				//	32bit浮動少数[Float]
	a = Flt2Lng(f);				//	32bit浮動少数[Float]
	if (n != a) ++num;
	printf("Lng2Flt : %x , %f, %x , %x\n", n, f, *(__SL *)&f, a);
	n = 0x3C00;
	f = HF2Flt(n);				//	16bit浮動少数[binary16]
	a = Flt2HF(f);				//	16bit浮動少数[binary16]
	if (n != a) ++num;
	printf("HF2Flt : %x , %f, %x , %x\n", n, f, *(__SL *)&f, a);
	f = BF2Flt(n);				//	16bit浮動少数[bFloat16]
	a = Flt2BF(f);				//	16bit浮動少数[bFloat16]
	if (n != a) ++num;
	printf("BF2Flt : %x , %f, %x , %x\n", n, f, *(__SL *)&f, a);

	{
		struct tsCom* ps;
		struct txCom* px;
		struct _SYSTEMTIME tm;
		__int64 t0,t1;
		HANDLE pc[1],hhp;
		__UC	nc[xCon] = {255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,};
		struct tbCom* bp = IniSwrtCom(xCh, xCon);
		nc[0] = OpenComX(0x70001, 115200);
		pc[0] = (*((*bp).bsCom+nc[0])).Han;
		nc[1] = OpenComX(2, 115200);
		pc[1] = (*((*bp).bsCom+nc[1])).Han;
		nc[2] = OpenComX(0x70002, 9600);
		nc[3] = OpenComX(2, 19200);
		printf("nc sz=%x , %08x,%08x,%08x,%08x\n,", sizeof(nc), *(__UL*)&nc[12], *(__UL*)&nc[8], *(__UL*)&nc[4], *(__UL*)&nc[0]);
		printf("bCom sz=%x , %08x, en=%08x,%08x,%08x,%08x,%08x,%08x,%08x,%08x\n,", sizeof(*bp), *(__UL*)bp, (*bp).enCom[7], (*bp).enCom[6], (*bp).enCom[5], (*bp).enCom[4], (*bp).enCom[3], (*bp).enCom[2], (*bp).enCom[1], (*bp).enCom[0]);
		if (sizeof(void*) > 4) {
			printf("	bs=%08x %08x, bx=%08x %08x\n", (__int64)(*bp).bsCom>>32, (*bp).bsCom, (__int64)(*bp).bxCom>>32, (*bp).bxCom);
			printf("Com1 : %08x %08x , Com2 : %08x %08x\n" , (__int64)pc[0]>>32 , pc[0], (__int64)pc[1]>>32, pc[1]);
		} else {
			printf("	bs=%08x, bx=%08x\n", (*bp).bsCom, (*bp).bxCom);
			printf("Com1 : %08x , Com2 : %08x\n" , pc[0], pc[1]);
		}
		ps = (*bp).bsCom+nc[0];
		printf("SystemTime%d : %08x %08x\n", nc[0], (*ps).t.ul[1], (*ps).t.ul[0]);
		FileTimeToSystemTime(&(*ps).t.ft, &tm);
		t0 = (*ps).t.ull;
		printf("SystemTime%d : %04d/%02d/%02d %02d:%02d:%02d.%03d\n", 0, tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond, tm.wMilliseconds);
		ps = (*bp).bsCom+nc[1];
		printf("SystemTime%d : %08x %08x\n", nc[1], (*ps).t.ul[1], (*ps).t.ul[0]);
		FileTimeToSystemTime(&(*ps).t.ft, &tm);
		t1 = (*ps).t.ull;
		printf("SystemTime%d : %04d/%02d/%02d %02d:%02d:%02d.%03d\n", 1, tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond, tm.wMilliseconds);
		printf("dT = %d\n", t1-t0);
		printf("size : tsCom=%x, char*=%x, HANDLE=%x, _COMMTIMEOUTS=%x , _COMSTAT=%x, _DCB=%x\n", sizeof(*ps), sizeof((*ps).Rx), sizeof((*ps).Han), sizeof((*ps).TmOut), sizeof((*ps).Stat), sizeof((*ps).Dcb));
		printf("Offset : tsCom=0 , char*=%x, HANDLE=%x, _COMMTIMEOUTS=%x , _COMSTAT=%x, _DCB=%x\n", &(*(struct tsCom*) 0).Rx, &(*(struct tsCom*) 0).Han, &(*(struct tsCom*) 0).TmOut, &(*(struct tsCom*) 0).Stat, &(*(struct tsCom*) 0).Dcb);
		if (pc[0] < (HANDLE)-16 && pc[1] < (HANDLE)-16) {
			int i;
			char *recievedData;
			DWORD	errors;									// エラーが起きた場合、エラーコードが入る
			COMSTAT	comStat;								// 通信状態バッファ構造体
			const char *sentData = "dummy data\n\0";		// 送信する文字列
			DWORD numberOfPut;								// 実際に送信したバイト数が入る
			DWORD numberOfGot;								// 受信バッファから実際に読み取ったバイト数が入る
			if (1) {
#ifdef	fin
				FILE * fp = NULL;
				px = (*bp).bxCom+nc[1];
				puts(fin);
				fp = fopen(fin, "rb");	//	memo.txtファイルを書き込み可能でオープン
				if (fp) {
					struct tSwrt* pw = (*ps).pSwrt;
					struct tDac* pd = (*ps).pDac;
					int szp = (*ps).szBufTx;				//	一度にまとめて送るサイズの設定(上限はPCスペックと受信側の都合で決まる)
					char* p0 = (char*)(*(*((*bp).bsCom+nc[0])).pSwrt).Vx;
					if (szp > sizeof((*pw).Vx)) szp = sizeof((*pw).Vx);
					do {
						int sz = fread((*pw).Vx, sizeof(unsigned char), szp, fp);
						if (sz > 0) SendByte((char*)(*pw).Vx, sz, 0, 0);
						if (1) {
							i = Read(500, 1, NULL, 0);		//	リード(データ転送あり)
						} else {
							int j = sz;
							do {
								i = Read(500, 1, p0, 32768);	//	リード(データ転送あり)
								if (i < 0) break;
								p0 += i;
								j -= i;
							} while (j > 0);
						}
						if (i >= 0) printf("pwRx = %d, prRx = %d, sz = %d, DatN = %d , DacN = %d\n", (*ps).pwRx, (*px).prRx, i, (*pw).Nr, (*pd).Nr);
					} while(i> 0);
					fclose(fp);
				}
#else
				SendByte((char *)sentData, -1, 0, 0);
//				WriteFile(pc[0], sentData, strlen(sentData), &numberOfPut, NULL); // ポートへ送信
				if (1) {
					char RxD[20];
					i = Read(500, 1, &RxD[0], sizeof(RxD)-1);	//	リード(データ転送あり)
					RxD[i] = 0; puts(RxD);						// 受信したデータを表示
				} else {
					char* pRxD = 0;
					int j;
					i = Read(500, 1, (char*)&pRxD, 0);			//	リード(データ転送なし)
					for(j=0; j<i; ++j) putchar(*(pRxD+j));		// 受信したデータを表示(DLL内バッファを使用するので\0を追加しない)
				}
#endif
			} else {
				int lengthOfRecieved = 0;						// 通信状態バッファ内の受信したメッセージ長を取得する
				WriteFile(pc[0], sentData, strlen(sentData), &numberOfPut, NULL); // ポートへ送信
				do {
					i = lengthOfRecieved;
					Sleep(500);									// 受信バッファにメッセージが溜まるまで待つ
					ClearCommError(pc[1], &errors, &comStat);	// 入出力バッファの情報を通信状態バッファへ取り込む
					lengthOfRecieved = comStat.cbInQue;						// 通信状態バッファ内の受信したメッセージ長を取得する
				} while (i != lengthOfRecieved);
				if (lengthOfRecieved) {
					hhp = HeapCreate(0, 0, 0);					// ヒープハンドルを生成
					recievedData = (char *)HeapAlloc(hhp, 0, sizeof(char) * (lengthOfRecieved + 1)); // 受信したメッセージ長 + 終端文字分のデータバッファをヒープ上に確保
					ReadFile(pc[1], recievedData, lengthOfRecieved, &numberOfGot, NULL); // 受信バッファからデータバッファへ取り込む
					recievedData[lengthOfRecieved] = '\0';		// 受信したデータに終端文字を追加
					puts(recievedData);							// 受信したデータを表示
					HeapDestroy(hhp);							// ヒープを解放
				}
			}
			i = CloseCom(0);
			i = CloseCom(1);
		}
#ifdef	_DEBUG
		ExitSwrtCom();
#endif
	}

	printf("Error : %d ?" , num);
	getc(stdin);
	return num;
}


#ifdef	_DEBUG
#include	"SwrtCom.c"
#endif
