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

//	�v���O�����p�萔
#define	xVer		0x0000						//	Version(.F-C=MAJOR,.B-8=MINOR,.7-0=PATCH version)
#define	xBuild		0x0000						//	Build Number
#define	xCh			4							//	Comm Channel Max - 1
#define	xCon		16							//	�ڑ��ő吔
#define	xRxRate		0.1							//	���̓o�b�t�@�T�C�Y(bps*dt���̃o�b�t�@�T�C�Y���m��)
#define	xTxRate		0.05						//	�o�̓o�b�t�@�T�C�Y(bps*dt���̃o�b�t�@�T�C�Y���m��)
#define	dsCom		bCom.bsCom					//	
#define	dxCom		bCom.bxCom					//	
#define	xlen			124						//	Data length(8bit)
#define	xlenRxL			14						//	Data Min length(8bit,st2+un+pg+tm4,dt4+cm+em=14)
#define	xlenCom			(xlen + 2 + 8)			//	DAT(SwrtXp) Data length(8bit)
#define	xlenRx			(xlen + 6)				//	RxD(SwrtXp) Data length(8bit)
#define	xt_com			1						//	comm recive error time (s)
#define	xBufLenFile		16384					//	�t�@�C�������݃o�b�t�@�T�C�Y(SSD�̃y�[�W�T�C�Y�ɍ��킹��)
#define	xBufLenRx		8192					//	comm buffer size(Byte) CommX�̎�M�o�b�t�@�T�C�Y
#define	xBufLenTx		2048					//	comm buffer size(Byte) CommX�̑��M�o�b�t�@�T�C�Y
#define	xCommandSwrt	0						//	Swrt command code
#define	xCommandDac		0xFF					//	Dac command code
#define	xMkSt			0xF8					//	Start Mark(1st)
#define	xMkSt2			0xF8					//	Start Mark(2nd)
#define	xMkEnd			0x0A					//	End Mark(1st)
#define	xDisp			2						//	Screen numbers
#define	xnDac			32768					//	Dac�f�[�^��
#define	xMaxGr			40						//	Send data Gr max
#define	xMaxSendDt		64						//	Send Data area max

#define	__UC	unsigned char			//	8bit�����Ȃ�����
#define	__SC	signed char				//	8bit�����t����
#define	__US	unsigned short			//	16bit�����Ȃ�����
#define	__SS	signed short			//	16bit�����t����
#define	__UL	unsigned long			//	32bit�����Ȃ�����
#define	__SL	signed long				//	32bit�����t����
#ifdef	_DEBUG
#define	__ULL	unsigned __int64		//	64bit�����Ȃ�����
#define	__SLL	__int64					//	64bit�����t����
#else
#define	__ULL	unsigned long long		//	64bit�����Ȃ�����
#define	__SLL	signed long long		//	64bit�����t����
#endif
#define	__DBL	double					//	�{���x��������(����1,�w��11,����52)
#define	__FL	float					//	�P���x��������(����1,�w��8,����23)
#define	__HF	signed short			//	�����x��������(����1,�w��5,����10)
#define	__BF	signed short			//	bfloat16(����1,�w��8,����7)

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

//	�v���O�����p�ϐ�
EXTR	struct tbCom {							//	Com�ڑ����
			char		nCh;					//		Com port numbers
			char		nCon;					//		Connect numbers
			short		szCom;					//	Com Numbers
			long		enCom[8];				//	Enable Com Port Number([0].0=COM1, .. [31].7=COM256)
			struct tsCom* bsCom;				//		base address sCom(Com port data)
			struct txCom* bxCom;				//		base address xCom(Connect data)
		} bCom;
struct tsCom {									//	Com�ڑ����
			union tmCom {
				__UL	ul;
				__US	us[2];
				__UC	uc[4];
				struct {
					__UL		nCom	: 8;	//	Com�|�[�g�ԍ�
					__UL		nSel	: 8;	//	sCom�ڑ���
					__UL		fSwrt	: 1;	//	Swrt Enable
					__UL		fDac	: 1;	//	Swrt Dac Enable
					__UL		fTrap	: 2;	//	Swrt Trap
					__UL		fRec	: 4;	//	Swrt Recode
					__UL		tRep	: 8;	//	Port Read Interval(dt*0.8ms)
				} s;
			} m;
			__SL				pwRx;			//	��M�o�b�t�@�ǉ��擪offset
			__SL				szBufRx;		//	��M�o�b�t�@�T�C�Y(�h���C�o)
			__SL				szBufTx;		//	���M�o�b�t�@�T�C�Y(�h���C�o)
			char*				Rx;				//	��M�o�b�t�@(malloc�Ŋm��)
			struct tSwrt*		pSwrt;			//	Swrt�o�b�t�@(malloc�Ŋm��)
			struct tDac*		pDac;			//	Dac�o�b�t�@(malloc�Ŋm��)
			struct tTrap*		pTrap;			//	Trap�o�b�t�@(malloc�Ŋm��)
			HANDLE					Han;		//	�n���h��
			union tVar64			t;			//	GetSystemTimeAsFileTime (1601/01/01(UTC)�ȍ~��100 �i�m�b�Ԋu64bit�l)
			__UL					Err;		//	�G���[�̎�ނ������}�X�N���󂯎�� 32 �r�b�g�ϐ�(lpErrors)
												//		.0:CE_RXOVER(���̓o�b�t�@�I�[�o�[�t���[)
												//		.1:CE_OVERRUN(�����o�b�t�@�I�[�o�[����)
												//		.2:CE_RXPARITY(�p���e�B�G���[)
												//		.3:CE_FRAME(�t���[�~���O�G���[)
												//		.4:CE_BREAK(Break���o)
												//		.8:CE_TXFULL
												//		.9:CE_PTO
												//		.10:CE_IOE
												//		.11:CE_DNS
												//		.12:CE_OOP
												//		.15:CE_MODE
			struct _COMMTIMEOUTS	TmOut;		//	�^�C���A�E�g�ݒ�\����
			struct _COMSTAT			Stat;		//	�f�o�C�X�X�e�[�^�X�\����
												//	fCtsHold : 1;TRUE �̏ꍇ�A�`���� CTS (clear-to-send) �V�O�i���̑��M��ҋ@���Ă��܂��B
												//	fDsrHold : 1;TRUE �̏ꍇ�A�`���� DSR (�f�[�^�E�Z�b�g�Ή�) �V�O�i���̑��M��ҋ@���Ă��܂��B
												//	fRlsdHold : 1TRUE �̏ꍇ�A�`���� RLSD (receive-line-signal-detect) �M���̑��M��ҋ@���Ă��܂��B
												//	fXoffHold : 1TRUE �̏ꍇ�AXOFF ��������M���ꂽ���߁A�`���͑ҋ@���Ă��܂��B
												//	fXoffSent : 1TRUE �̏ꍇ�AXOFF ���������M���ꂽ���߁A�`���͑ҋ@���Ă��܂��B (XOFF �������A���ۂ̕����Ɋ֌W�Ȃ��AXON �Ƃ��Ď��̕������󂯎��V�X�e���ɑ��M�����ƁA�`���͒�~���܂��B
												//	fEof : 1;TRUE �̏ꍇ�A�t�@�C���̖��� (EOF) ��������M����܂����B
												//	fTxim : 1;TRUE �̏ꍇ�A TransmitCommChar �֐����g�p���ĒʐM�f�o�C�X�ɑ��M����镶�����L���[�ɓ�����܂��B �ʐM�f�o�C�X�́A�f�o�C�X�̏o�̓o�b�t�@�[���̑��̕����̑O�ɂ��̂悤�ȕ����𑗐M���܂��B
												//	fReserved : 25;
												//	cbInQue;�V���A�� �v���o�C�_�[����M�������A ReadFile ����ł܂��ǂݎ���Ă��Ȃ��o�C�g���B
												//	cbOutQue;���ׂĂ̏������ݑ���ő��M�����c��̃��[�U�[ �f�[�^�̃o�C�g���B ���̒l�́A�I�[�o�[���b�v����Ă��Ȃ��������݂̏ꍇ�� 0 �ɂȂ�܂��B
			struct _DCB				Dcb;		//	�f�o�C�X����u���b�N�\����
};
struct txCom {
			__UC				ch;				//	sCom channel
			__UC				Err;			//	sCom channel
												//		.0:CE_RXOVER(���̓o�b�t�@�I�[�o�[�t���[)
												//		.1:CE_OVERRUN(�����o�b�t�@�I�[�o�[����)
												//		.2:CE_RXPARITY(�p���e�B�G���[)
												//		.3:CE_FRAME(�t���[�~���O�G���[)
												//		.4:CE_BREAK(Break���o)
			__SL				prRx;			//	��M�o�b�t�@�ǂݏo���擪offset
};
struct tSwrt {
			__UL				Nr;				//	Swrt��M�f�[�^��(�N��������)
			__UL				Ns;				//	Swrt��M�f�[�^��(����J�n������)
			__ULL				Ts;				//	�ŏ���Swrt�f�[�^��M�J�n����[Now()]
			__UL				Tr;				//	�����Swrt�f�[�^��M�J�n����[Now()-Ts]
			__UL				Tn;				//	�ŐV��Swrt�f�[�^��M����[Now()-Ts]
			__UC				pW;				//	Work pointer
			__UC				Wx[255];		//	Work buffer
			__UL				Vt[256];		//	Swrt�ŏI��M�f�[�^�����i�J�n������̌o�ߎ��ԁj
			__UL				Vc[256];		//	Swrt��M�f�[�^�J�E���^
			__UC				Vx[256][128];	//	Swrt��M�f�[�^:256Page*(124Byte+1Single)
};
struct tDac {
			__ULL				te;				//	Dac�ŏI��M����[Now()]
			__UL				Nr;				//	��M�f�[�^��(�N��������)
			__UL				Ns;				//	��M�f�[�^��(����J�n������)
			__UL				tmDac;			//	Dac Time Data
			__UL				ofDac;			//	Dac�o�b�t�@�ǉ��擪offset
			__US				dt[32768];		//	Dac data
};
struct tTrap {
			__ULL				te;				//	�ŏI��M����[Now()]
			__UL				ofTrap;			//	Trap�o�b�t�@�ǉ��擪offset
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

EXTR DLLAPI struct tbCom* IniSwrtCom(__UC xch, __UC xcon);			//	DLL������
EXTR DLLAPI void ExitSwrtCom(void);									//	DLL�I��
EXTR DLLAPI int OpenComX(__UL iPortNum, __SL iRate);				//	�I�[�v��:����1 COM�|�[�g����("COM5"�Ƃ�) ����2:�{�[���[�g
EXTR DLLAPI HANDLE OpenCom(__UL iPortNum, __SL iRate, __SL ch);		//	�I�[�v��:����1 COM�|�[�g����("COM5"�Ƃ�) ����2:�{�[���[�g ����3:ch�w��
EXTR		int GetBuf(struct tsCom* ps);							//	�o�b�t�@�m��
EXTR DLLAPI int CloseCom(__SL ch);									//	�N���[�Y
EXTR DLLAPI void MsgErr(__SL er);									//	�G���[���b�Z�[�W
EXTR DLLAPI __SL GetComStatus(__SL ch);								//	�X�e�[�^�X���[�h
EXTR DLLAPI __SL ClearCom(__SL ch, __SL md);						//	Com Clear
EXTR DLLAPI __SL Read(__SL tm, __SL ch, char* pRx, __SL len);		//	���[�h
EXTR DLLAPI __SL ReadByte(__SL tm, __SL ch);						//	�o�C�i�����[�h
EXTR DLLAPI __SL SendByte(char* dTx, __SL szTx, __SL tm, __SL ch);	//	�o�C�i�����M
EXTR DLLAPI __SL Send(char* TxS, __SL tm, __SL ch);					//	�A�X�L�[���M
EXTR DLLAPI struct tbCom* SerchCom(void);							//	�ڑ��|�[�g�̎�������
EXTR DLLAPI __SL FileAdd(char* fname, char* pt, __SL sz);			//	�t�@�C���̍Ō�Ƀf�[�^��ǉ�
EXTR DLLAPI __UC CalSum(__UC ini, __UC* pt, int sz);				//	CheckSum

//	�֐���`
EXTR DLLAPI __SL Exp2Lng(__SL, __SL, __SL);	//	�w�������ϊ�(ne:�w����,nv:������)
EXTR DLLAPI __SL Lng2Exp(__SL, __SL, __SL);	//	�����w���ϊ�(ne:�w����,nv:������)
EXTR DLLAPI __FL Lng2Flt(__SL);				//	32bit��������[Float]
EXTR DLLAPI __FL HF2Flt(__HF);				//	16bit��������[binary16]
EXTR DLLAPI __FL BF2Flt(__BF);				//	16bit��������[bFloat16]
EXTR DLLAPI __SL Flt2Lng(__FL);				//	32bit��������[Float]
EXTR DLLAPI __HF Flt2HF(__FL);				//	16bit��������[binary16]
EXTR DLLAPI __BF Flt2BF(__FL);				//	16bit��������[bFloat16]
#endif
