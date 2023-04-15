//SwrtCom.h
#ifndef	_SwrtCom
#define	_SwrtCom
#ifdef	_DEBUG
#define DLLAPI
#define EXTR
#define log2f(x)	(log(x)/log(2))
#define ceilf(x)	ceil(x)
#else
#ifdef BUILD_DLL
#define DLLAPI	__declspec(dllexport)
#define EXTR
#else
#define DLLAPI	__declspec(dllimport)
#define EXTR	extern
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <string.h>
#include <tchar.h>
#include <math.h>

//	プログラム用定数
#define	xVer		0x0000						//	Version(.F-C=MAJOR,.B-8=MINOR,.7-0=PATCH version)
#define	xBuild		0x0000						//	Build Number
#define	xCh			4							//	Comm Channel Max - 1
#define	xCon		16							//	接続最大数
#define	xRxRate		0.1							//	入力バッファサイズ(bps*dt分のバッファサイズを確保)
#define	xTxRate		0.05						//	出力バッファサイズ(bps*dt分のバッファサイズを確保)
#define	dsCom		bCom.bsCom					//	
#define	dxCom		bCom.bxCom					//	
#define	xlen			124						//	Data length(8bit)
#define	xlenRxL			14						//	Data Min length(8bit,st2+un+pg+tm4,dt4+cm+em=14)
#define	xlenCom			(xlen + 2 + 8)			//	DAT(SwrtXp) Data length(8bit)
#define	xlenRx			(xlen + 6)				//	RxD(SwrtXp) Data length(8bit)
#define	xt_com			1						//	comm recive error time (s)
#define	xBufLenFile		16384					//	ファイル書込みバッファサイズ(SSDのページサイズに合わせる)
#define	xBufLenRx		8192					//	comm buffer size(Byte) CommXの受信バッファサイズ
#define	xBufLenTx		2048					//	comm buffer size(Byte) CommXの送信バッファサイズ
#define	xCommandSwrt	0						//	Swrt command code
#define	xCommandDac		0xFF					//	Dac command code
#define	xMkSt			0xF8					//	Start Mark(1st)
#define	xMkSt2			0xF8					//	Start Mark(2nd)
#define	xMkEnd			0x0A					//	End Mark(1st)
#define	xDisp			2						//	Screen numbers
#define	xnDac			32768					//	Dacデータ数
#define	xMaxGr			40						//	Send data Gr max
#define	xMaxSendDt		64						//	Send Data area max

#define	__UC	unsigned char			//	8bit符号なし整数
#define	__SC	signed char				//	8bit符号付整数
#define	__US	unsigned short			//	16bit符号なし整数
#define	__SS	signed short			//	16bit符号付整数
#define	__UL	unsigned long			//	32bit符号なし整数
#define	__SL	signed long				//	32bit符号付整数
#ifdef	_DEBUG
#define	__ULL	unsigned __int64		//	64bit符号なし整数
#define	__SLL	__int64					//	64bit符号付整数
#else
#define	__ULL	unsigned long long		//	64bit符号なし整数
#define	__SLL	signed long long		//	64bit符号付整数
#endif
#define	__DBL	double					//	倍精度浮動小数(符号1,指数11,仮数52)
#define	__FL	float					//	単精度浮動小数(符号1,指数8,仮数23)
#define	__HF	signed short			//	半精度浮動小数(符号1,指数5,仮数10)
#define	__BF	signed short			//	bfloat16(符号1,指数8,仮数7)

union tVar16 {
	__UC	uc[2];
	__SC	sc[2];
	__US	us;
	__SS	ss;
	__HF	hf;
	__BF	bf;
};

union tVar32 {
	__UC	uc[4];
	__SC	sc[4];
	__US	us[2];
	__SS	ss[2];
	__UL	ul;
	__SL	sl;
	__FL	fl;
	__HF	hf[2];
	__BF	bf[2];
};

union tVar64 {
	__UC	uc[8];
	__SC	sc[8];
	__US	us[4];
	__SS	ss[4];
	__UL	ul[2];
	__SL	sl[2];
	__ULL	ull;
	__SLL	sll;
	__DBL	dbl;
	__FL	fl[2];
	__HF	hf[4];
	__BF	bf[4];
	struct _FILETIME ft; 
};

//	プログラム用変数
EXTR	struct tbCom {							//	Com接続情報
			char		nCh;					//		Com port numbers
			char		nCon;					//		Connect numbers
			short		szCom;					//	Com Numbers
			long		enCom[8];				//	Enable Com Port Number([0].0=COM1, .. [31].7=COM256)
			struct tsCom* bsCom;				//		base address sCom(Com port data)
			struct txCom* bxCom;				//		base address xCom(Connect data)
		} bCom;
struct tsCom {									//	Com接続情報
			union tmCom {
				__UL	ul;
				__US	us[2];
				__UC	uc[4];
				struct {
					__UL		nCom	: 8;	//	Comポート番号
					__UL		nSel	: 8;	//	sCom接続数
					__UL		fSwrt	: 1;	//	Swrt Enable
					__UL		fDac	: 1;	//	Swrt Dac Enable
					__UL		fTrap	: 2;	//	Swrt Trap
					__UL		fRec	: 4;	//	Swrt Recode
					__UL		tRep	: 8;	//	Port Read Interval(dt*0.8ms)
				} s;
			} m;
			__SL				pwRx;			//	受信バッファ追加先頭offset
			__SL				szBufRx;		//	受信バッファサイズ(ドライバ)
			__SL				szBufTx;		//	送信バッファサイズ(ドライバ)
			char*				Rx;				//	受信バッファ(mallocで確保)
			struct tSwrt*		pSwrt;			//	Swrtバッファ(mallocで確保)
			struct tDac*		pDac;			//	Dacバッファ(mallocで確保)
			struct tTrap*		pTrap;			//	Trapバッファ(mallocで確保)
			HANDLE					Han;		//	ハンドル
			union tVar64			t;			//	GetSystemTimeAsFileTime (1601/01/01(UTC)以降の100 ナノ秒間隔64bit値)
			__UL					Err;		//	エラーの種類を示すマスクを受け取る 32 ビット変数(lpErrors)
												//		.0:CE_RXOVER(入力バッファオーバーフロー)
												//		.1:CE_OVERRUN(文字バッファオーバーラン)
												//		.2:CE_RXPARITY(パリティエラー)
												//		.3:CE_FRAME(フレーミングエラー)
												//		.4:CE_BREAK(Break検出)
												//		.8:CE_TXFULL
												//		.9:CE_PTO
												//		.10:CE_IOE
												//		.11:CE_DNS
												//		.12:CE_OOP
												//		.15:CE_MODE
			struct _COMMTIMEOUTS	TmOut;		//	タイムアウト設定構造体
			struct _COMSTAT			Stat;		//	デバイスステータス構造体
												//	fCtsHold : 1;TRUE の場合、伝送は CTS (clear-to-send) シグナルの送信を待機しています。
												//	fDsrHold : 1;TRUE の場合、伝送は DSR (データ・セット対応) シグナルの送信を待機しています。
												//	fRlsdHold : 1TRUE の場合、伝送は RLSD (receive-line-signal-detect) 信号の送信を待機しています。
												//	fXoffHold : 1TRUE の場合、XOFF 文字が受信されたため、伝送は待機しています。
												//	fXoffSent : 1TRUE の場合、XOFF 文字が送信されたため、伝送は待機しています。 (XOFF 文字が、実際の文字に関係なく、XON として次の文字を受け取るシステムに送信されると、伝送は停止します。
												//	fEof : 1;TRUE の場合、ファイルの末尾 (EOF) 文字が受信されました。
												//	fTxim : 1;TRUE の場合、 TransmitCommChar 関数を使用して通信デバイスに送信される文字がキューに入れられます。 通信デバイスは、デバイスの出力バッファー内の他の文字の前にこのような文字を送信します。
												//	fReserved : 25;
												//	cbInQue;シリアル プロバイダーが受信したが、 ReadFile 操作でまだ読み取られていないバイト数。
												//	cbOutQue;すべての書き込み操作で送信される残りのユーザー データのバイト数。 この値は、オーバーラップされていない書き込みの場合は 0 になります。
			struct _DCB				Dcb;		//	デバイス制御ブロック構造体
};
struct txCom {
			__UC				ch;				//	sCom channel
			__UC				Err;			//	sCom channel
												//		.0:CE_RXOVER(入力バッファオーバーフロー)
												//		.1:CE_OVERRUN(文字バッファオーバーラン)
												//		.2:CE_RXPARITY(パリティエラー)
												//		.3:CE_FRAME(フレーミングエラー)
												//		.4:CE_BREAK(Break検出)
			__SL				prRx;			//	受信バッファ読み出し先頭offset
};
struct tSwrt {
			__UL				Nr;				//	Swrt受信データ数(起動時から)
			__UL				Ns;				//	Swrt受信データ数(測定開始時から)
			__ULL				Ts;				//	最初のSwrtデータ受信開始時刻[Now()]
			__UL				Tr;				//	今回のSwrtデータ受信開始時刻[Now()-Ts]
			__UL				Tn;				//	最新のSwrtデータ受信時刻[Now()-Ts]
			__UC				pW;				//	Work pointer
			__UC				Wx[255];		//	Work buffer
			__UL				Vt[256];		//	Swrt最終受信データ時刻（開始時からの経過時間）
			__UL				Vc[256];		//	Swrt受信データカウンタ
			__UC				Vx[256][128];	//	Swrt受信データ:256Page*(124Byte+1Single)
};
struct tDac {
			__ULL				te;				//	Dac最終受信時刻[Now()]
			__UL				Nr;				//	受信データ数(起動時から)
			__UL				Ns;				//	受信データ数(測定開始時から)
			__UL				tmDac;			//	Dac Time Data
			__UL				ofDac;			//	Dacバッファ追加先頭offset
			__US				dt[32768];		//	Dac data
};
struct tTrap {
			__ULL				te;				//	最終受信時刻[Now()]
			__UL				ofTrap;			//	Trapバッファ追加先頭offset
			__UC				dt[32768];		//	Trap data
};
struct tMath {
			__US				ad2;
			__US				ad3;
			__FL				dt1;
			__FL				dt2;
			__FL				dt3;
			__UC				md3;
			__UC				length;
			char				dt_format[14];
};
EXTR	char				nFile[256];			//	file name

EXTR DLLAPI struct tbCom* IniSwrtCom(__UC xch, __UC xcon);			//	DLL初期化
EXTR DLLAPI void ExitSwrtCom(void);									//	DLL終了
EXTR DLLAPI int OpenComX(__UL iPortNum, __SL iRate);				//	オープン:引数1 COMポート名称("COM5"とか) 引数2:ボーレート
EXTR DLLAPI HANDLE OpenCom(__UL iPortNum, __SL iRate, __SL ch);		//	オープン:引数1 COMポート名称("COM5"とか) 引数2:ボーレート 引数3:ch指定
EXTR		int GetBuf(struct tsCom* ps);							//	バッファ確保
EXTR DLLAPI int CloseCom(__SL ch);									//	クローズ
EXTR DLLAPI void MsgErr(__SL er);									//	エラーメッセージ
EXTR DLLAPI __SL GetComStatus(__SL ch);								//	ステータスリード
EXTR DLLAPI __SL ClearCom(__SL ch, __SL md);						//	Com Clear
EXTR DLLAPI __SL Read(__SL tm, __SL ch, char* pRx, __SL len);		//	リード
EXTR DLLAPI __SL ReadByte(__SL tm, __SL ch);						//	バイナリリード
EXTR DLLAPI __SL SendByte(char* dTx, __SL szTx, __SL tm, __SL ch);	//	バイナリ送信
EXTR DLLAPI __SL Send(char* TxS, __SL tm, __SL ch);					//	アスキー送信
EXTR DLLAPI struct tbCom* SerchCom(void);							//	接続ポートの自動検索
EXTR DLLAPI __SL FileAdd(char* fname, char* pt, __SL sz);			//	ファイルの最後にデータを追加
EXTR DLLAPI __UC CalSum(__UC ini, __UC* pt, int sz);				//	CheckSum

//	関数定義
EXTR DLLAPI __SL Exp2Lng(__SL, __SL, __SL);	//	指数整数変換(ne:指数桁,nv:仮数桁)
EXTR DLLAPI __SL Lng2Exp(__SL, __SL, __SL);	//	整数指数変換(ne:指数桁,nv:仮数桁)
EXTR DLLAPI __FL Lng2Flt(__SL);				//	32bit浮動少数[Float]
EXTR DLLAPI __FL HF2Flt(__HF);				//	16bit浮動少数[binary16]
EXTR DLLAPI __FL BF2Flt(__BF);				//	16bit浮動少数[bFloat16]
EXTR DLLAPI __SL Flt2Lng(__FL);				//	32bit浮動少数[Float]
EXTR DLLAPI __HF Flt2HF(__FL);				//	16bit浮動少数[binary16]
EXTR DLLAPI __BF Flt2BF(__FL);				//	16bit浮動少数[bFloat16]
#endif
