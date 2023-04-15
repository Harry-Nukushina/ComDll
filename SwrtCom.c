//SwrtCom.c
#include "SwrtCom.h"

//********************************************************************************
//DLLエントリーポイント
// 引数		: hinstDll : DLLのインスタンスが格納, fdwReason : 関数が呼び出された原因を表す定数, lpvReserved : 常に 0
// リターン	: true
//********************************************************************************
int WINAPI DllEntryPoint(HINSTANCE hInstance , DWORD fdwReason , PVOID pvReserved) {
	switch(fdwReason) {
	case DLL_PROCESS_ATTACH:		//	DLL が読み込まれました
		IniSwrtCom(xCh, xCon);
		break;
	case DLL_PROCESS_DETACH:		//	DLL が解放されました
		ExitSwrtCom();
		break;
	case DLL_THREAD_ATTACH:			//	既存プロセスが新しいスレッドを作成しています
		break;
	case DLL_THREAD_DETACH:			//	既存のスレッドが終了しようとしています
		break;
	}
	return TRUE;
}


//********************************************************************************
//DLL初期化
// 引数		: xch : Comチャンネル最大数 , xcon : 接続最大数
//			:	設定済みより大きい場合のみ更新、以下の場合は現状を維持
// リターン	: 構造体アドレス
//********************************************************************************
EXTR DLLAPI struct tbCom* IniSwrtCom(__UC xch, __UC xcon) {			//	DLL初期化
	if (xch > 0) {
		__UC nx;
		__UC ns = bCom.nCh;
		if (!bCom.szCom) SerchCom();
		if (xcon < xch) xcon = xch;
		if (ns < xch) {
			void* psCom = bCom.bsCom;
			bCom.bsCom = calloc(xch, sizeof(*bCom.bsCom));			//		base address sCom(Com port data)
			bCom.nCh = xch;											//		Com port numbers
			if (ns) {
				memcpy(bCom.bsCom, psCom, ns*sizeof(*bCom.bsCom));
				free(psCom);
			}
		}
		nx = bCom.nCon;
		if (nx < xcon) {
			struct txCom* pxCom = bCom.bxCom;
			struct txCom* pxComN;
			pxComN = calloc(xcon, sizeof(*bCom.bxCom));			//		base address xCom(Connect data)
			bCom.bxCom = pxComN;
			bCom.nCon = xcon;										//		Connect numbers
			if (nx) {
				memcpy(pxComN, pxCom, nx*sizeof(*bCom.bxCom));
				free(pxCom);
			}
			for( ; nx < xcon; ++nx) {
				(*(pxComN+nx)).ch = 255;
			}
		}
	}
	return (struct tbCom*)&bCom;
}

//********************************************************************************
//DLL終了
// 引数		: なし
// リターン	: なし
//********************************************************************************
EXTR DLLAPI void ExitSwrtCom(void) {								//	DLL終了
	int i;
	if (bCom.nCh > 0) {
		for(i = 0; i < bCom.nCon; ++i) {
			if ((*(bCom.bxCom+i)).ch < bCom.nCh) {
				CloseCom(i);
			}
		}
	}
	bCom.nCon = 0;
	if (bCom.bsCom) {
		free(bCom.bsCom);
		bCom.bsCom = 0;
	}
	if (bCom.bxCom) {
		free(bCom.bxCom);
		bCom.bxCom = 0;
	}
	bCom.nCh = 0;													//		Com port numbers
}

//********************************************************************************
//オープン
// 引数	iPortNum : ポート名, iRate : ボーレート
// リターン	エラー時<0(成功時はch番号)
//	成功時はsCom[].Hanにファイルハンドル
//********************************************************************************
EXTR DLLAPI int OpenComX(__UL iPortNum, __SL iRate) {
	int	i,j,ch,rt;
	struct tsCom* ps;
	struct txCom* px;
	if (!(__UC)iPortNum) {								//ch設定失敗(iPortNum=1-255)
		return -3;
	}
	rt = -1;
	for(ch = 0; ch < bCom.nCh; ++ch) {
		ps = bCom.bsCom+ch;
		j = (*ps).m.s.nCom;
		if (j == (__UC)iPortNum) {
			for(i = bCom.nCh; i < bCom.nCon; ++i) {
				px = bCom.bxCom+i;
				if ((*px).ch >= 255) {
					(*ps).m.ul |= iPortNum;				//		nCom,mdSwrt,hPage,nCon
					j = GetBuf(ps);
					if (j<0) return j;
					++(*ps).m.s.nSel;
					(*px).ch = ch;
					(*px).Err = 0;
					(*px).prRx = (*ps).pwRx;			//	受信バッファ読み出し先頭offset
					return i;
				}
			}
			return -2;
		} else if (!j && rt < 0) {
			rt = ch;
		}
	}
	if (rt >= 0) {
		HANDLE h = OpenCom(iPortNum, iRate, (__SL)rt);
		if (h < 0) rt = (int)(__int64)h;
	}
	return rt;
}

//********************************************************************************
//オープン
// 引数	iPortNum : ポート名, iRate : ボーレート, ch : チャンネル
// リターン	エラー時<=0(成功時はファイルハンドル)
//	成功時はsCom[].Han(x)にもファイルハンドル
//********************************************************************************
EXTR DLLAPI HANDLE OpenCom(__UL iPortNum, __SL iRate, __SL ch) {
	char	pn[11];
	HANDLE	rt;
	struct txCom* px = bCom.bxCom+ch;
	struct tsCom* ps = bCom.bsCom+ch;
		printf("OpenCom : ch = %d, Han = %x\n", ch, (*ps).Han);
	do {
		if (!bCom.nCh) IniSwrtCom(xCh, xCon);
		if (ch >= bCom.nCh) {								//ch設定失敗(ch=0-bCom.nCh-1)
			rt = (HANDLE)-2;
			break;
		}
		if (!(__UC)iPortNum) {								//ch設定失敗(iPortNum=1-255)
			rt = (HANDLE)-3;
			break;
		}
		if ((*ps).Han) {									//ch設定失敗(使用済み)
			rt = (HANDLE)-4;
			break;
		}

//シリアルポートをオープンする
		sprintf(pn, "\\\\.\\COM%d",(__UC)iPortNum);
		rt = CreateFile(pn, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
		if (rt == (HANDLE)-1) break;							//ポートのオープン失敗
		(*ps).Han = rt;
		(*ps).m.ul = iPortNum;									//		nCom,mdSwrt,hPage,nCon
		(*ps).m.s.nSel = 1;
		(*px).ch = ch;
		(*px).Err = 0;
		GetCommState((*ps).Han, &(*ps).Dcb);					//現在の設定値を読み込み
//***** 通信条件の設定 *****
		(*ps).Dcb.DCBlength = sizeof(DCB);						//[D]DCBのサイズ
		(*ps).Dcb.BaudRate = iRate;								//[D]ボーレート
		(*ps).Dcb.fBinary = 1;									//[D]バイナリモード(1)
		(*ps).Dcb.fParity = 1;									//[D]パリティチェックなし(1)
		(*ps).Dcb.fOutxCtsFlow = FALSE;							//[D]CTSフロー制御:フロー制御なし
		(*ps).Dcb.fOutxDsrFlow = FALSE;							//[D]DSRハードウェアフロー制御：使用しない
		(*ps).Dcb.fDtrControl = DTR_CONTROL_DISABLE;			//[D]DTR有効/無効:(0=DTR_CONTROL_DISABLE,1=DTR_CONTROL_ENABLE,2=DTR_CONTROL_HANDSHAKE)
		(*ps).Dcb.fDsrSensitivity = FALSE;						//[D]DSR がOFFの間は受信データを無視するか
		(*ps).Dcb.fTXContinueOnXoff = TRUE;						//[D] 受信バッファー満杯＆XOFF受信後の継続送信可否:送信可
		(*ps).Dcb.fOutX = FALSE;								//[D]送信時XON/XOFF制御の有無:なし
		(*ps).Dcb.fInX = FALSE;									//[D]受信時XON/XOFF制御の有無:なし
		(*ps).Dcb.fErrorChar = FALSE;							//[D]パリティエラー発生時のキャラクタ（ErrorChar）置換:なし
		(*ps).Dcb.fNull = TRUE;									//[D]NULLバイトの破棄:破棄する
		(*ps).Dcb.fRtsControl = RTS_CONTROL_DISABLE;			//[D]RTSフロー制御:RTS制御なし
		(*ps).Dcb.fAbortOnError = FALSE;						//[D]エラー時の読み書き操作終了:終了しない
//		(*ps).Dcb.fDummy2 : 17;									//[D]
//		(*ps).Dcb.wReserved;									//[W]
		(*ps).Dcb.XonLim = 512;									//[W]XONが送られるまでに格納できる最小バイト数:512
		(*ps).Dcb.XoffLim = 512;								//[W]XOFFが送られるまでに格納できる最小バイト数:512
		(*ps).Dcb.ByteSize = 8;									//[B]ビット長の指定
		(*ps).Dcb.Parity = NOPARITY;							//[B]パリティなし(0=NOPARITY,1=ODDPARITY,2=EVENPARITY,3=MARKPARITY,4=SPACEPARITY)
		(*ps).Dcb.StopBits = ONESTOPBIT;						//[B]ストップビット数を1指定(0=ONESTOPBIT,1=ONE5STOPBITS,2=TWOSTOPBITS)
		(*ps).Dcb.XonChar = 0x11;								//[C]送信時XON文字 ( 送信可：ビジィ解除 ) の指定:XON文字として11H ( デバイス制御１：DC1 )
		(*ps).Dcb.XoffChar = 0x13;								//[C]XOFF文字（送信不可：ビジー通告）の指定:XOFF文字として13H ( デバイス制御3：DC3 )
		(*ps).Dcb.ErrorChar = 0x00;								//[C]パリティエラー発生時の置換キャラクタ
		(*ps).Dcb.EofChar = 0x03;								//[C]データ終了通知キャラクタ:一般に0x03(ETX)がよく使われます。
		(*ps).Dcb.EvtChar = 0x02;								//[C]イベント通知キャラクタ:一般に0x02(STX)がよく使われます
//		(*ps).Dcb.wReserved1;									//[W]

//状態確認
		if (!SetCommState((*ps).Han, &(*ps).Dcb)) {
			rt = (HANDLE)-5;
			break;
		}

//バッファの設定
		if ((*ps).szBufRx < 256) (*ps).szBufRx = 1<<(__SL)ceilf(log2f(iRate * xRxRate));
		if ((*ps).szBufTx < 256) (*ps).szBufTx = 1<<(__SL)ceilf(log2f(iRate * xTxRate));
		printf("ch~%d, bRx=%x , bTx=%x\n" ,ch , (*ps).szBufRx, (*ps).szBufTx);
		if (!SetupComm((*ps).Han, (*ps).szBufRx, (*ps).szBufTx)) {
			rt = (HANDLE)-6;
			break;
		}
		if (!(*ps).Rx) {
			(*ps).Rx = (char*)malloc((*ps).szBufRx);
			if ((*ps).Rx == NULL) {
				rt = (HANDLE)-7;
				break;
			}
		}
		{
			int i = GetBuf(ps);							//	バッファ確保
			if (i < 0) {
				rt = (HANDLE)(__int64)i;
				break;
			}
		}
		(*px).prRx = (*ps).pwRx = 0;

//バッファのクリア
		if (!PurgeComm((*ps).Han, PURGE_TXCLEAR)) {
			rt = (HANDLE)-8;
			break;
		}

//タイムアウトの設定
		{
			int tm = (20000 / iRate) + 1;
			(*ps).TmOut.ReadIntervalTimeout = 0;				//文字の読み込み待ち時間
			(*ps).TmOut.ReadTotalTimeoutMultiplier = tm;		//読み込みの１文字あたりの時間
			(*ps).TmOut.ReadTotalTimeoutConstant = tm;			//読み込みの定数時間
			(*ps).TmOut.WriteTotalTimeoutMultiplier = tm;		//書き込みの１文字あたりの時間
			(*ps).TmOut.WriteTotalTimeoutConstant = tm;			//書き込みの定数時間
		}
		if (!SetCommTimeouts((*ps).Han, &(*ps).TmOut)) {
			rt = (HANDLE)-9;
			break;
		}
	} while(0);
	if (rt >= (HANDLE)-16) {
		if (rt < (HANDLE)-4) {
			if (rt < (HANDLE)-7) {
				free((*ps).Rx);
				(*ps).Rx = (char*)0;
			}
			CloseCom(ch);
		}
	}
	(*ps).Han = rt;
	return rt;
}

EXTR int GetBuf(struct tsCom* ps) {							//	バッファ確保
	int rt = 0;
	do {
		if (!(*ps).pSwrt) {
			if ((*ps).m.s.fSwrt || (*ps).m.s.fDac) {
				struct tSwrt* pt = malloc(sizeof(struct tSwrt));
				(*ps).pSwrt = pt;
				if (pt == NULL) {
					rt = -8;
					break;
				}
				memset(pt, 0, sizeof(struct tSwrt)-sizeof((*(struct tSwrt *)0).Vx));
			}
		}
		if (!(*ps).pDac) {
			if ((*ps).m.s.fDac) {
				struct tDac* pt = malloc(sizeof(struct tDac));
				(*ps).pDac = pt;
				if (pt == NULL) {
					rt = -9;
					break;
				}
				memset(pt, 0, sizeof(struct tDac)-sizeof((*(struct tDac *)0).dt));
			}
		}
		if (!(*ps).pTrap) {
			if ((*ps).m.s.fTrap) {
				struct tTrap* pt = malloc(sizeof(struct tTrap));
				(*ps).pTrap = pt;
				if (pt == NULL) {
					rt = -10;
					break;
				}
				(*pt).ofTrap = 0;
			}
		}
	} while(0);
	return rt;
}

//********************************************************************************
//クローズ
// 引数	ch : チャンネル(省略時は0)
// リターン	エラー時0
//	成功時はsCom[].Han(x)もクリア
//********************************************************************************
EXTR DLLAPI int CloseCom(__SL ch) {
	struct txCom* px;
	struct tsCom* ps;
	__UC pn;
	if (ch >= bCom.nCon) {							//ch設定失敗(ch=0-nCon)
		return -1;
	}
	px = bCom.bxCom+ch;
	pn = (*px).ch;
	if (pn >= bCom.nCh) {							//ch設定失敗(ch=0-nCh)
		return -2;
	}
	ps = bCom.bsCom+pn;
	if (!(*ps).Han || (*ps).Han >= (HANDLE)-16) {	//ch設定失敗(Not Open)
		return -3;
	}
	(*px).ch = 255;
	(*px).prRx = (*ps).pwRx;						//	受信バッファ読み出し先頭offset
	if (!--(*ps).m.s.nSel) {
		free((*ps).Rx);								//	バッファ解放
		(*ps).Rx = (char*)0;
		free((*ps).pSwrt);							//	バッファ解放
		(*ps).pSwrt = (struct tSwrt*)0;
		free((*ps).pDac);							//	バッファ解放
		(*ps).pDac = (struct tDac*)0;
		free((*ps).pTrap);							//	バッファ解放
		(*ps).pTrap = (struct tTrap*)0;
		CloseHandle((*ps).Han);
		(*ps).Han = 0;								//	ハンドルをクリア
		(*ps).m.s.nCom = 0;
	}
	return 0;
}

//********************************************************************************
//エラーメッセージ
// 引数	er : エラー番号
//********************************************************************************
EXTR DLLAPI void MsgErr(__SL er) {
	char* mErr;
//	mErr = Split(StrErr, ",");
//	MsgBox(mErr(-1 - er), vbOKCancel);
}

//********************************************************************************
//ステータスリード
// 引数	ch : チャンネル(省略時は0)
// リターン	エラー時0
//	ステータスは(*ps).Stat
//********************************************************************************
EXTR DLLAPI __SL GetComStatus(__SL ch) {
	struct tsCom* ps = bCom.bsCom+ch;
	if (ch >= bCom.nCon) {								//ch設定失敗(ch=0-nCon)
		return -16;
	}
	if (!(*ps).Han || (*ps).Han >= (HANDLE)-16) {		//ch設定失敗(Not Open)
		return -17;
	}

	return (__SL)ClearCommError((*ps).Han, &(*ps).Err, &(*ps).Stat);
}

//********************************************************************************
//Com クリア
// 引数	ch : チャンネル(省略時は0), md : mode(.0=Err, .1=Rx buffer, .5-2=PurgeComm
// リターン	エラー時0
//********************************************************************************
EXTR DLLAPI __SL ClearCom(__SL ch, __SL md) {		//	Com Clear
	int rt = 0;
	struct txCom* px;
	struct tsCom* ps;
	__UC pn;
	if (ch >= bCom.nCon) {							//ch設定失敗(ch=0-nCon)
		return -1;
	}
	px = bCom.bxCom+ch;
	pn = (*px).ch;
	if (pn >= bCom.nCh) {							//ch設定失敗(ch=0-nCh)
		return -2;
	}
	ps = bCom.bsCom+pn;
	if (!(*ps).Han || (*ps).Han >= (HANDLE)-16) {	//ch設定失敗(Not Open)
		return -3;
	}
	if (md & 1) (*px).Err = 0;
	if (md & 2) (*px).prRx = (*ps).pwRx;
	if (md >> 2) {
		__UC i0;
		rt = PurgeComm((*ps).Han, (DWORD)(md>>2));
		(*ps).Err = 0;
		(*ps).pwRx = 0;
		for(i0 = 0; i0 < bCom.nCon; ++i0) {
			px = bCom.bxCom+i0;
			if (pn == (*px).ch) {
				(*px).Err = 0;
				(*px).prRx = 0;
			}
		}
	}
	return rt;
}

//********************************************************************************
//リード
// 引数	mData : tm : 受信待ち時間(s)(省略時は0), ch : 受信チャンネル(省略時は0)
// 引数	mData : pRx : 受信バッファアドレス(NULLの時はDLL内バッファを使用し先頭アドレスを格納), len : 受信データ長(0の時はDLL内バッファを使用し受信バッファアドレスがNULL出ない場合は、そのアドレスにDLL内バッファアドレスをセット)
// リターン	受信文字数
//********************************************************************************
EXTR DLLAPI __SL Read(__SL tm, __SL ch, char* pRx, __SL len) {
	__SL sz = ReadByte(tm, ch);					//バイナリで受信
	if (sz >= 0) {
		struct txCom* px = bCom.bxCom+ch;
		struct tsCom* ps = bCom.bsCom+ch;
		__SL pw = (*ps).pwRx;
		__SL pr = (*px).prRx;
		__SL sb = (*ps).szBufRx;
		sz = pw - pr;
		if (sz > sb) {
			sz = sb;
			pr = pw - sz;
			(*px).Err |= 0x80;
		}
		if (sz) {
			__SL pw0 = pw & -sb;				//	Page offset0
			__SL sz0 = pw0 - pr;
			if (len <= 0) {						//	? DLLバッファ使用
				if (sz0 > 0) {					//		? Page change
					pw = pw0;					//			Next Page offset0
					sz = pw-pr;
				}
				if (pRx) *(char**)&pRx = (*ps).Rx+(pr & (sb-1));
			} else {
				char* pt = (*ps).Rx+(pr & (sb-1));
				if (sz0 > 0) {					//		? Page change
					memcpy(pRx, pt, sz0);
					pRx += sz0;
					pt = (*ps).Rx;
					sz0 = sz-sz0;
				} else {
					sz0 = sz;
				}
				if (sz0) memcpy(pRx, pt, sz0);
			}
			(*px).prRx = pw;
		}
	}
	return sz;
}
//********************************************************************************
//バイナリリード
// 引数	tm : 受信待ち時間(s)(省略時は0), ch : 受信チャンネル(省略時は0)
// リターン	受信バイト数(<0時はエラー)
//	受信データは(*ps).Rx(),受信データ長は(*ps).sRx
//********************************************************************************
EXTR DLLAPI __SL ReadByte(__SL tm, __SL ch) {
	__SL ret;
	int	sRx;
	struct txCom* px;
	struct tsCom* ps;
	__UC pn;
	if (ch >= bCom.nCon) {							//ch設定失敗(ch=0-nCon)
		return -1;
	}
	px = bCom.bxCom+ch;
	pn = (*px).ch;
	if (pn >= bCom.nCh) {							//ch設定失敗(ch=0-nCh)
		return -2;
	}
	if (tm > 0) Sleep (tm);
	ret = 0;
	if (GetComStatus(pn) < 0) {						//ch設定失敗
		return -3;
	};
	ps = bCom.bsCom+pn;
	sRx = (*ps).Stat.cbInQue;
	if (sRx > 0) do {
		DWORD sz = (*ps).szBufRx;
		DWORD pRx = (*ps).pwRx & (sz-1);
		DWORD wRx, i = sRx;
		__ULL ft;
		GetSystemTimeAsFileTime((struct _FILETIME*)&ft);
		wRx = (ft - (*ps).t.ull)>>13;				//	0.8ms step
		if (wRx < (*ps).m.s.tRep) break;
		(*ps).t.ull = *(__ULL*)&ft;
		{
			__UC er = (__UC)(*ps).Err;
			if (er) {
				struct txCom* px0;
				(*px).Err |= er;
				for (wRx=(DWORD)bCom.nCh; wRx < (DWORD)bCom.nCon; ++wRx) {
					px0 = bCom.bxCom+wRx;
					if ((*px0).ch == pn) (*px0).Err |= er;
				}
			}
		}
		do {
			wRx = pRx+i;
			if (wRx >= sz) wRx = sz;
			wRx -= pRx;
			if (!ReadFile((*ps).Han, (*ps).Rx+pRx, wRx, &wRx, 0)) {	//データの受信
				CloseCom(ch);
				return(-1);
			}
			ret += wRx;
			{										//	受信データ解析
				if ((*ps).pSwrt) {
					__UL ob = (*(*ps).pSwrt).pW;	//	Swrtバッファ追加先頭offset
					DWORD w0, w1 = wRx, w2 = pRx;
					__UL szL = xlenRx;				//	受信データ解析Min長
					if ((*ps).pDac) {
						__UL md = (*(*ps).pDac).tmDac;
						if (md) {
							__UL sz = ((__UC)md+1)*(((md>>8) & 15)+1)*4;
							if (szL > (sz+xlenRxL-4)) szL = (sz+xlenRxL-4);
						}
					}
					do {
						w0 = sizeof((*(struct tSwrt *)0).Wx) - ob;
						if (w0 > w1) w0 = w1;
						memcpy(&(*(*ps).pSwrt).Wx[ob], (*ps).Rx+w2, w0);
						ob += w0;
						if (ob >= szL) {
							__UC* pt = &(*(*ps).pSwrt).Wx[0];
							DWORD i0 = 0,i1 = 0;
							__UC nu, np;
							do {
								do {
									if (*pt++ != xMkSt) break;
									i1 =i0;
									if (*pt++ != xMkSt2) break;
									nu = *pt++;
									if (nu == xMkSt2) {
										++i1; nu = *pt++;
									}
									np = *pt++;
																	//	Swrt Dat
									if ((nu < bCom.nCh) && ((ob-i0)>=xlenRx) && (*(pt+(xlen+1)) == xMkEnd) && !CalSum(-nu-np, pt, xlen+1)) {			//	CheckSum
										__UL dt;
										++(*(*ps).pSwrt).Nr;					//	Swrt受信データ数(起動時から)
										++(*(*ps).pSwrt).Ns;					//	Swrt受信データ数(測定開始時から)
										if (!(*(*ps).pSwrt).Ts)					//	最初のSwrtデータ受信開始時刻[Now()]
											(*(*ps).pSwrt).Ts = ft;
										dt = (ft-(*(*ps).pSwrt).Ts)>>10;		//	今回のSwrtデータ受信開始時刻[Now()-Ts]
										if (!(*(*ps).pSwrt).Tr)
											(*(*ps).pSwrt).Tr = dt;
										(*(*ps).pSwrt).Tn = dt;					//	最新のSwrtデータ受信時刻[Now()-Ts]
										(*(*ps).pSwrt).Vt[np] = dt;				//	Swrt最終受信データ時刻（開始時からの経過時間）
										++(*(*ps).pSwrt).Vc[np];				//	Swrt受信データカウンタ
										memcpy((*(*ps).pSwrt).Vx[np], pt, xlen);
										i1 += xlenRx; i0 = i1 - 1; pt += xlen+2;
										break;
									}
									if (!(*ps).pDac) break;						//	Dac
									if (!nu && (np == 0xFF)) {					//	Old Dac format
										nu = 0xC0; np = 0;
									}
									if ((nu - 0xC0) < bCom.nCh) {
										__UL md = *(__UL*)pt;
										int sz = ((__UC)md+1)*(((md>>8) & 15)+1)*4;
										if (szL > (__UL)(sz+xlenRxL-4)) szL = (sz+xlenRxL-4);
										if ((int)(ob-i0)<(sz+xlenRxL-4)) break;
										pt += 4;
										if ((*(pt+sz+1) == xMkEnd) && !CalSum(0, pt-6, sz+7)) {
											__UL of = (*(*ps).pDac).ofDac;
											int i = sz;
											if ((of*sizeof(__US)+i) > sizeof((*(*ps).pDac).dt)) {
												do {
													i -= ((__UC)md+1)*4;
												} while ((of*sizeof(__US)+i) > sizeof((*(*ps).pDac).dt));
												memcpy(&((*(*ps).pDac).dt[of]), pt, i);
												i1 += i; pt += i; sz -= i;
												of = 0;
											}
											(*(*ps).pDac).te = ft;					//	最終受信時刻[Now()]
											++(*(*ps).pDac).Nr;						//	受信データ数(起動時から)
											++(*(*ps).pDac).Ns;						//	受信データ数(測定開始時から)
											(*(*ps).pDac).tmDac = md;				//	Dac Time Data
											(*(*ps).pDac).ofDac = of+(sz>>1);		//	Dacバッファ追加先頭offset
											memcpy(&((*(*ps).pDac).dt[of]), pt, sz);
											i1 += sz+xlenRxL-4; i0 = i1 - 1; pt += sz+2;
										}
									}
								} while (0);
								++i0;
							} while ((ob - i0) >= szL);
							if (!i1) i1 = i0;
							ob -= i1;
							if (ob > 0) memmove(&(*(*ps).pSwrt).Wx[0], &(*(*ps).pSwrt).Wx[i1], ob);
						}
						w2 += w0;
						w1 -= w0;
					} while (w1 > 0);
					(*(*ps).pSwrt).pW = ob;						//	Swrtバッファ追加先頭offset
				}
				if ((*ps).pTrap && (*ps).m.s.fTrap) {
					__UL ob = (*(*ps).pTrap).ofTrap;			//	Trapバッファ追加先頭offset
					DWORD w0, w1 = wRx, w2 = pRx, w3;
					do {
						w3 = (ob & (sizeof((*(struct tTrap *)0).dt)-1));
						w0 = sizeof((*(struct tTrap *)0).dt) - w3;
						if (w0 > w1) w0 = w1;
						memcpy(&(*(*ps).pTrap).dt[w3], (*ps).Rx+w2, w0);
						ob += w0;
						if (!(ob & (sizeof((*(struct tTrap *)0).dt)-1))) {
		printf("ob=%08x, te=%08x %08x\n,", ob, (*(*ps).pTrap).te>>32, (*(*ps).pTrap).te);
						}
						w2 += w0;
						w1 -= w0;
					} while (w1 > 0);
					(*(*ps).pTrap).ofTrap  = ob;			//	Trapバッファ追加先頭offset
					(*(*ps).pTrap).te = ft;						//	最終受信時刻[Now()]
				}
			}
			pRx += wRx;
			if (pRx >= sz) pRx -= sz;
			i -= wRx;
		} while (i > 0);
		(*ps).pwRx += ret;
	} while(0);
	return ret;
}

//********************************************************************************
//バイナリ送信
// 引数	dTx : 送信データ,szTx : 送信バイト数(省略時は-1,<0で全データ), tm : バッファフル時の待ち時間(s)(省略時は0), ch : 送信チャンネル(省略時は0)
// リターン	送信バイト数(<0時はエラー,送信バイト数が指定バイト数より小さい時は時間オーバで残りのデータは未送信)
//********************************************************************************
EXTR DLLAPI __SL SendByte(char* dTx, __SL szTx, __SL tm, __SL ch) {
	__SL	PACKETSIZE;							//	一度にまとめて送るデータのサイズ
	__SL	fLen;								//	送信されたバイト数
	__SL	ret;
	struct txCom* px;
	struct tsCom* ps;
	__UC pn;
	if (ch >= bCom.nCon) {						//ch設定失敗(ch=0-nCon)
		return -1;
	}
	px = bCom.bxCom+ch;
	pn = (*px).ch;
	if (pn >= bCom.nCh) {						//ch設定失敗(ch=0-nCh)
		return -2;
	}
	ps = bCom.bsCom+pn;
	if (!(*ps).Han || (*ps).Han >= (HANDLE)-16) {	//ch設定失敗(Not Open)
		return -3;
	}

	if (szTx < 0) szTx = strlen(dTx);
	PACKETSIZE = (*ps).szBufTx;					//	一度にまとめて送るサイズの設定(上限はPCスペックと受信側の都合で決まる)
	if (szTx < PACKETSIZE) PACKETSIZE = szTx;	//	残データ量がパケットサイズ以下ならパケットサイズ縮小する
	ret = szTx;
	if (ret) do {								//	送信処理
		if (!WriteFile((*ps).Han, dTx, PACKETSIZE, (DWORD*)&fLen, 0)) {	//	データの送信
			CloseCom(ch);
			ret = 0;
			break;
		}
		dTx += fLen;							//	次に送信するバイト位置
		szTx -= fLen;							//	残データサイズ更新
	} while(szTx > 0);							//	残データ量がなくなったら送信完了
	return ret;
}
//********************************************************************************
//文字列送信(文字はシステム内部形式,日本ではSJIS)
// 引数	TxS : 送信文字列, tm : バッファフル時の待ち時間(s)(省略時は0), ch : 送信チャンネル(省略時は0)
// リターン	送信バイト数(<0時はエラー,送信バイト数が指定バイト数より小さい時は時間オーバで残りのデータは未送信)
//********************************************************************************
EXTR DLLAPI __SL Send(char* TxS, __SL tm, __SL ch) {
											//	バイナリ送信
	return SendByte(TxS, (int)(TxS - (char*)strchr(TxS, 0)), tm, ch);
}

//********************************************************************************
//接続ポートの自動検索
// リターン	使用可能ポート番号をカンマで区切った文字列
//********************************************************************************
EXTR DLLAPI struct tbCom* SerchCom(void) {
	char	pn[11];
	int		i;
	HANDLE	fn;
	char name[MAX_PATH];
	char value[MAX_PATH];
	HKEY hkey;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\SERIALCOMM",0,KEY_READ, &hkey) == ERROR_SUCCESS) {
		int idx = 0;
		for(; ; idx++) {
			DWORD nameLen = MAX_PATH;
			DWORD valueLen = MAX_PATH;
			if (RegEnumValueA(hkey, idx, name, &nameLen, NULL, NULL, (LPBYTE)&value, &valueLen) != ERROR_SUCCESS) break;
			if (!strncmp(value, "COM", 3)) {
				i = atoi(&value[3])-1;
				if (!(i>>8)) {
					bCom.enCom[i>>5] |= 1<<(i & 31);
					++bCom.szCom;
				}
			}
		}
		RegCloseKey(hkey);
	}
	return (struct tbCom*)&bCom;
}
//********************************************************************************
//ファイルの最後にデータを追加
// 引数	fname : ファイル名, pt : 追加データのオフセットsz : 追加バイト数, md : 追加配列指定(省略時は0/0=(*ps).Rx/他はRxD)
// リターン	追加バイト数(<0時はエラー)
//********************************************************************************
EXTR DLLAPI __SL FileAdd(char* fname, char* pt, __SL sz) {
	__SL	fLen = -1;
	HANDLE	fn;
	fn = CreateFile(fname, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_ALWAYS, 0, 0);
	if (fn != (HANDLE)-1) {					//オープン成功
		fLen = SetFilePointer(fn, 0, 0, FILE_END);
		fLen = WriteFile(fn, pt, sz, (DWORD*)&fLen, 0);
		CloseHandle(fn);
	}
	return fLen;
}

//********************************************************************************
//チェックサム計算
// 引数	ini : 初期値, pt : 先頭アドレス, sz : バイト数
// リターン	 CheckSum(8bit)
//********************************************************************************
EXTR DLLAPI __UC CalSum(__UC ini, __UC* pt, int sz) {				//	CheckSum
	if (sz > 0) do {
		ini -= *pt++;
	} while(--sz);
	return ini;
}
//********************************************************************************
//データ変換関数
//********************************************************************************
EXTR DLLAPI __SL Exp2Lng(__SL n, __SL ne, __SL nv) {	//	指数整数変換(ne:指数桁,nv:仮数桁)
	int		ik = 1;
	int		ir = 1 << nv;
	int		ie;
	int		il = ne + nv;
	__SLL	iw;
	n &= (2 << (ne + nv)) - 1;
	if ((il & 1) && (n & (1 << (ne + nv)))) {
		ik = -1;
		if (il < 31) {
			ie = 1 << (ne + nv);
			if (n & ie) n = ie - (n & (ie - 1));
			if (n == ie) n = ie - 1;
		} else {
			if (n < -2147483647) n = 2147483647;
			else n = -n;
		}
	}
	ie = (n >> nv) & ((1 << ne) - 1);
	if (ie >= 2) {
		iw = (__SLL)((n & (ir - 1)) + ir) << (ie - 1);
		if (iw > 2147483647) iw = 2147483647;
		n = (__SL)iw;
	}
	return (n * ik);
}

EXTR DLLAPI __SL Lng2Exp(__SL n, __SL ne, __SL nv) {	//	整数指数変換(ne:指数桁,nv:仮数桁)
	int		ik = 1;
	int		ir = 2 << nv;
	int		ie;
	if (n < 0) {
		ik = -1;
		if (n < -2147483647) n = -2147483647;
		n = -n;
	}
	if (n >= ir) {
		ie = 0;
		do {
			++ie;
			n = n >> 1;
		} while(n >= ir);
		n = ie * (ir >> 1) + n;
		ie = (1 << (ne + nv)) - 1;
		if (n > ie) n = ie;
	}
	return (n * ik);
}

EXTR DLLAPI __FL Lng2Flt(__SL x) {	//	32bit浮動少数[Float]
	union tVar32 z;
	z.sl = x;
	return (z.fl);
}

EXTR DLLAPI __FL HF2Flt(__HF x) {	//	16bit浮動少数[binary16]
	union tVar32 z;
	z.sl = (((x & 0x7FFF)+((127-15)<<10))<<(23-10))+((x>>15)<<31);
	return (z.fl);
}

EXTR DLLAPI __FL BF2Flt(__BF x) {	//	16bit浮動少数[bFloat16]
	union tVar32 z;
	z.hf[1] = x;
	z.hf[0] = 0;
	return (z.fl);
}

EXTR DLLAPI __SL Flt2Lng(__FL x) {	//	32bit浮動少数[Float]
	union tVar32 z;
	z.fl = x;
	return (z.sl);
}

EXTR DLLAPI __HF Flt2HF(__FL x) {	//	16bit浮動少数[binary16]
	union tVar32 z;
	int i;
	z.fl = x;
	i = z.us[1] & 0x8000;
	z.sl = ((z.ul & 0x7FFFFFFF) >> (23-10))-((127-15)<<10);
	if (z.sl < 0) {
		z.sl = 0;
	} else if (z.sl >= ((127+16)<<10)) {
		z.sl = 0x7F7F;
	}
	return (__HF)(z.sl+i);
}

EXTR DLLAPI __BF Flt2BF(__FL x) {	//	16bit浮動少数[bFloat16]
	union tVar32 z;
	z.fl = x;
	return (z.bf[1]);
}

/*
EXTR DLLAPI BYTE* Dec2Hex(ByVal dT As LongPtr, ByVal sz As Long) {
	return = Right("000000000000000" + Hex(dT), sz)
}

EXTR DLLAPI __SL Hex2Dec(dT As String) {
	return (__SL)("&H0" + dT)
}

*/
