//SwrtCom.c
#include "SwrtCom.h"

//********************************************************************************
//DLL�G���g���[�|�C���g
// ����		: hinstDll : DLL�̃C���X�^���X���i�[, fdwReason : �֐����Ăяo���ꂽ������\���萔, lpvReserved : ��� 0
// ���^�[��	: true
//********************************************************************************
int WINAPI DllEntryPoint(HINSTANCE hInstance , DWORD fdwReason , PVOID pvReserved) {
	switch(fdwReason) {
	case DLL_PROCESS_ATTACH:		//	DLL ���ǂݍ��܂�܂���
		IniSwrtCom(xCh, xCon);
		break;
	case DLL_PROCESS_DETACH:		//	DLL ���������܂���
		ExitSwrtCom();
		break;
	case DLL_THREAD_ATTACH:			//	�����v���Z�X���V�����X���b�h���쐬���Ă��܂�
		break;
	case DLL_THREAD_DETACH:			//	�����̃X���b�h���I�����悤�Ƃ��Ă��܂�
		break;
	}
	return TRUE;
}


//********************************************************************************
//DLL������
// ����		: xch : Com�`�����l���ő吔 , xcon : �ڑ��ő吔
//			:	�ݒ�ς݂��傫���ꍇ�̂ݍX�V�A�ȉ��̏ꍇ�͌�����ێ�
// ���^�[��	: �\���̃A�h���X
//********************************************************************************
EXTR DLLAPI struct tbCom* IniSwrtCom(__UC xch, __UC xcon) {			//	DLL������
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
//DLL�I��
// ����		: �Ȃ�
// ���^�[��	: �Ȃ�
//********************************************************************************
EXTR DLLAPI void ExitSwrtCom(void) {								//	DLL�I��
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
//�I�[�v��
// ����	iPortNum : �|�[�g��, iRate : �{�[���[�g
// ���^�[��	�G���[��<0(��������ch�ԍ�)
//	��������sCom[].Han�Ƀt�@�C���n���h��
//********************************************************************************
EXTR DLLAPI int OpenComX(__UL iPortNum, __SL iRate) {
	int	i,j,ch,rt;
	struct tsCom* ps;
	struct txCom* px;
	if (!(__UC)iPortNum) {								//ch�ݒ莸�s(iPortNum=1-255)
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
					(*px).prRx = (*ps).pwRx;			//	��M�o�b�t�@�ǂݏo���擪offset
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
//�I�[�v��
// ����	iPortNum : �|�[�g��, iRate : �{�[���[�g, ch : �`�����l��
// ���^�[��	�G���[��<=0(�������̓t�@�C���n���h��)
//	��������sCom[].Han(x)�ɂ��t�@�C���n���h��
//********************************************************************************
EXTR DLLAPI HANDLE OpenCom(__UL iPortNum, __SL iRate, __SL ch) {
	char	pn[11];
	HANDLE	rt;
	struct txCom* px = bCom.bxCom+ch;
	struct tsCom* ps = bCom.bsCom+ch;
		printf("OpenCom : ch = %d, Han = %x\n", ch, (*ps).Han);
	do {
		if (!bCom.nCh) IniSwrtCom(xCh, xCon);
		if (ch >= bCom.nCh) {								//ch�ݒ莸�s(ch=0-bCom.nCh-1)
			rt = (HANDLE)-2;
			break;
		}
		if (!(__UC)iPortNum) {								//ch�ݒ莸�s(iPortNum=1-255)
			rt = (HANDLE)-3;
			break;
		}
		if ((*ps).Han) {									//ch�ݒ莸�s(�g�p�ς�)
			rt = (HANDLE)-4;
			break;
		}

//�V���A���|�[�g���I�[�v������
		sprintf(pn, "\\\\.\\COM%d",(__UC)iPortNum);
		rt = CreateFile(pn, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
		if (rt == (HANDLE)-1) break;							//�|�[�g�̃I�[�v�����s
		(*ps).Han = rt;
		(*ps).m.ul = iPortNum;									//		nCom,mdSwrt,hPage,nCon
		(*ps).m.s.nSel = 1;
		(*px).ch = ch;
		(*px).Err = 0;
		GetCommState((*ps).Han, &(*ps).Dcb);					//���݂̐ݒ�l��ǂݍ���
//***** �ʐM�����̐ݒ� *****
		(*ps).Dcb.DCBlength = sizeof(DCB);						//[D]DCB�̃T�C�Y
		(*ps).Dcb.BaudRate = iRate;								//[D]�{�[���[�g
		(*ps).Dcb.fBinary = 1;									//[D]�o�C�i�����[�h(1)
		(*ps).Dcb.fParity = 1;									//[D]�p���e�B�`�F�b�N�Ȃ�(1)
		(*ps).Dcb.fOutxCtsFlow = FALSE;							//[D]CTS�t���[����:�t���[����Ȃ�
		(*ps).Dcb.fOutxDsrFlow = FALSE;							//[D]DSR�n�[�h�E�F�A�t���[����F�g�p���Ȃ�
		(*ps).Dcb.fDtrControl = DTR_CONTROL_DISABLE;			//[D]DTR�L��/����:(0=DTR_CONTROL_DISABLE,1=DTR_CONTROL_ENABLE,2=DTR_CONTROL_HANDSHAKE)
		(*ps).Dcb.fDsrSensitivity = FALSE;						//[D]DSR ��OFF�̊Ԃ͎�M�f�[�^�𖳎����邩
		(*ps).Dcb.fTXContinueOnXoff = TRUE;						//[D] ��M�o�b�t�@�[���t��XOFF��M��̌p�����M��:���M��
		(*ps).Dcb.fOutX = FALSE;								//[D]���M��XON/XOFF����̗L��:�Ȃ�
		(*ps).Dcb.fInX = FALSE;									//[D]��M��XON/XOFF����̗L��:�Ȃ�
		(*ps).Dcb.fErrorChar = FALSE;							//[D]�p���e�B�G���[�������̃L�����N�^�iErrorChar�j�u��:�Ȃ�
		(*ps).Dcb.fNull = TRUE;									//[D]NULL�o�C�g�̔j��:�j������
		(*ps).Dcb.fRtsControl = RTS_CONTROL_DISABLE;			//[D]RTS�t���[����:RTS����Ȃ�
		(*ps).Dcb.fAbortOnError = FALSE;						//[D]�G���[���̓ǂݏ�������I��:�I�����Ȃ�
//		(*ps).Dcb.fDummy2 : 17;									//[D]
//		(*ps).Dcb.wReserved;									//[W]
		(*ps).Dcb.XonLim = 512;									//[W]XON��������܂łɊi�[�ł���ŏ��o�C�g��:512
		(*ps).Dcb.XoffLim = 512;								//[W]XOFF��������܂łɊi�[�ł���ŏ��o�C�g��:512
		(*ps).Dcb.ByteSize = 8;									//[B]�r�b�g���̎w��
		(*ps).Dcb.Parity = NOPARITY;							//[B]�p���e�B�Ȃ�(0=NOPARITY,1=ODDPARITY,2=EVENPARITY,3=MARKPARITY,4=SPACEPARITY)
		(*ps).Dcb.StopBits = ONESTOPBIT;						//[B]�X�g�b�v�r�b�g����1�w��(0=ONESTOPBIT,1=ONE5STOPBITS,2=TWOSTOPBITS)
		(*ps).Dcb.XonChar = 0x11;								//[C]���M��XON���� ( ���M�F�r�W�B���� ) �̎w��:XON�����Ƃ���11H ( �f�o�C�X����P�FDC1 )
		(*ps).Dcb.XoffChar = 0x13;								//[C]XOFF�����i���M�s�F�r�W�[�ʍ��j�̎w��:XOFF�����Ƃ���13H ( �f�o�C�X����3�FDC3 )
		(*ps).Dcb.ErrorChar = 0x00;								//[C]�p���e�B�G���[�������̒u���L�����N�^
		(*ps).Dcb.EofChar = 0x03;								//[C]�f�[�^�I���ʒm�L�����N�^:��ʂ�0x03(ETX)���悭�g���܂��B
		(*ps).Dcb.EvtChar = 0x02;								//[C]�C�x���g�ʒm�L�����N�^:��ʂ�0x02(STX)���悭�g���܂�
//		(*ps).Dcb.wReserved1;									//[W]

//��Ԋm�F
		if (!SetCommState((*ps).Han, &(*ps).Dcb)) {
			rt = (HANDLE)-5;
			break;
		}

//�o�b�t�@�̐ݒ�
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
			int i = GetBuf(ps);							//	�o�b�t�@�m��
			if (i < 0) {
				rt = (HANDLE)(__int64)i;
				break;
			}
		}
		(*px).prRx = (*ps).pwRx = 0;

//�o�b�t�@�̃N���A
		if (!PurgeComm((*ps).Han, PURGE_TXCLEAR)) {
			rt = (HANDLE)-8;
			break;
		}

//�^�C���A�E�g�̐ݒ�
		{
			int tm = (20000 / iRate) + 1;
			(*ps).TmOut.ReadIntervalTimeout = 0;				//�����̓ǂݍ��ݑ҂�����
			(*ps).TmOut.ReadTotalTimeoutMultiplier = tm;		//�ǂݍ��݂̂P����������̎���
			(*ps).TmOut.ReadTotalTimeoutConstant = tm;			//�ǂݍ��݂̒萔����
			(*ps).TmOut.WriteTotalTimeoutMultiplier = tm;		//�������݂̂P����������̎���
			(*ps).TmOut.WriteTotalTimeoutConstant = tm;			//�������݂̒萔����
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

EXTR int GetBuf(struct tsCom* ps) {							//	�o�b�t�@�m��
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
//�N���[�Y
// ����	ch : �`�����l��(�ȗ�����0)
// ���^�[��	�G���[��0
//	��������sCom[].Han(x)���N���A
//********************************************************************************
EXTR DLLAPI int CloseCom(__SL ch) {
	struct txCom* px;
	struct tsCom* ps;
	__UC pn;
	if (ch >= bCom.nCon) {							//ch�ݒ莸�s(ch=0-nCon)
		return -1;
	}
	px = bCom.bxCom+ch;
	pn = (*px).ch;
	if (pn >= bCom.nCh) {							//ch�ݒ莸�s(ch=0-nCh)
		return -2;
	}
	ps = bCom.bsCom+pn;
	if (!(*ps).Han || (*ps).Han >= (HANDLE)-16) {	//ch�ݒ莸�s(Not Open)
		return -3;
	}
	(*px).ch = 255;
	(*px).prRx = (*ps).pwRx;						//	��M�o�b�t�@�ǂݏo���擪offset
	if (!--(*ps).m.s.nSel) {
		free((*ps).Rx);								//	�o�b�t�@���
		(*ps).Rx = (char*)0;
		free((*ps).pSwrt);							//	�o�b�t�@���
		(*ps).pSwrt = (struct tSwrt*)0;
		free((*ps).pDac);							//	�o�b�t�@���
		(*ps).pDac = (struct tDac*)0;
		free((*ps).pTrap);							//	�o�b�t�@���
		(*ps).pTrap = (struct tTrap*)0;
		CloseHandle((*ps).Han);
		(*ps).Han = 0;								//	�n���h�����N���A
		(*ps).m.s.nCom = 0;
	}
	return 0;
}

//********************************************************************************
//�G���[���b�Z�[�W
// ����	er : �G���[�ԍ�
//********************************************************************************
EXTR DLLAPI void MsgErr(__SL er) {
	char* mErr;
//	mErr = Split(StrErr, ",");
//	MsgBox(mErr(-1 - er), vbOKCancel);
}

//********************************************************************************
//�X�e�[�^�X���[�h
// ����	ch : �`�����l��(�ȗ�����0)
// ���^�[��	�G���[��0
//	�X�e�[�^�X��(*ps).Stat
//********************************************************************************
EXTR DLLAPI __SL GetComStatus(__SL ch) {
	struct tsCom* ps = bCom.bsCom+ch;
	if (ch >= bCom.nCon) {								//ch�ݒ莸�s(ch=0-nCon)
		return -16;
	}
	if (!(*ps).Han || (*ps).Han >= (HANDLE)-16) {		//ch�ݒ莸�s(Not Open)
		return -17;
	}

	return (__SL)ClearCommError((*ps).Han, &(*ps).Err, &(*ps).Stat);
}

//********************************************************************************
//Com �N���A
// ����	ch : �`�����l��(�ȗ�����0), md : mode(.0=Err, .1=Rx buffer, .5-2=PurgeComm
// ���^�[��	�G���[��0
//********************************************************************************
EXTR DLLAPI __SL ClearCom(__SL ch, __SL md) {		//	Com Clear
	int rt = 0;
	struct txCom* px;
	struct tsCom* ps;
	__UC pn;
	if (ch >= bCom.nCon) {							//ch�ݒ莸�s(ch=0-nCon)
		return -1;
	}
	px = bCom.bxCom+ch;
	pn = (*px).ch;
	if (pn >= bCom.nCh) {							//ch�ݒ莸�s(ch=0-nCh)
		return -2;
	}
	ps = bCom.bsCom+pn;
	if (!(*ps).Han || (*ps).Han >= (HANDLE)-16) {	//ch�ݒ莸�s(Not Open)
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
//���[�h
// ����	mData : tm : ��M�҂�����(s)(�ȗ�����0), ch : ��M�`�����l��(�ȗ�����0)
// ����	mData : pRx : ��M�o�b�t�@�A�h���X(NULL�̎���DLL���o�b�t�@���g�p���擪�A�h���X���i�[), len : ��M�f�[�^��(0�̎���DLL���o�b�t�@���g�p����M�o�b�t�@�A�h���X��NULL�o�Ȃ��ꍇ�́A���̃A�h���X��DLL���o�b�t�@�A�h���X���Z�b�g)
// ���^�[��	��M������
//********************************************************************************
EXTR DLLAPI __SL Read(__SL tm, __SL ch, char* pRx, __SL len) {
	__SL sz = ReadByte(tm, ch);					//�o�C�i���Ŏ�M
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
			if (len <= 0) {						//	? DLL�o�b�t�@�g�p
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
//�o�C�i�����[�h
// ����	tm : ��M�҂�����(s)(�ȗ�����0), ch : ��M�`�����l��(�ȗ�����0)
// ���^�[��	��M�o�C�g��(<0���̓G���[)
//	��M�f�[�^��(*ps).Rx(),��M�f�[�^����(*ps).sRx
//********************************************************************************
EXTR DLLAPI __SL ReadByte(__SL tm, __SL ch) {
	__SL ret;
	int	sRx;
	struct txCom* px;
	struct tsCom* ps;
	__UC pn;
	if (ch >= bCom.nCon) {							//ch�ݒ莸�s(ch=0-nCon)
		return -1;
	}
	px = bCom.bxCom+ch;
	pn = (*px).ch;
	if (pn >= bCom.nCh) {							//ch�ݒ莸�s(ch=0-nCh)
		return -2;
	}
	if (tm > 0) Sleep (tm);
	ret = 0;
	if (GetComStatus(pn) < 0) {						//ch�ݒ莸�s
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
			if (!ReadFile((*ps).Han, (*ps).Rx+pRx, wRx, &wRx, 0)) {	//�f�[�^�̎�M
				CloseCom(ch);
				return(-1);
			}
			ret += wRx;
			{										//	��M�f�[�^���
				if ((*ps).pSwrt) {
					__UL ob = (*(*ps).pSwrt).pW;	//	Swrt�o�b�t�@�ǉ��擪offset
					DWORD w0, w1 = wRx, w2 = pRx;
					__UL szL = xlenRx;				//	��M�f�[�^���Min��
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
										++(*(*ps).pSwrt).Nr;					//	Swrt��M�f�[�^��(�N��������)
										++(*(*ps).pSwrt).Ns;					//	Swrt��M�f�[�^��(����J�n������)
										if (!(*(*ps).pSwrt).Ts)					//	�ŏ���Swrt�f�[�^��M�J�n����[Now()]
											(*(*ps).pSwrt).Ts = ft;
										dt = (ft-(*(*ps).pSwrt).Ts)>>10;		//	�����Swrt�f�[�^��M�J�n����[Now()-Ts]
										if (!(*(*ps).pSwrt).Tr)
											(*(*ps).pSwrt).Tr = dt;
										(*(*ps).pSwrt).Tn = dt;					//	�ŐV��Swrt�f�[�^��M����[Now()-Ts]
										(*(*ps).pSwrt).Vt[np] = dt;				//	Swrt�ŏI��M�f�[�^�����i�J�n������̌o�ߎ��ԁj
										++(*(*ps).pSwrt).Vc[np];				//	Swrt��M�f�[�^�J�E���^
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
											(*(*ps).pDac).te = ft;					//	�ŏI��M����[Now()]
											++(*(*ps).pDac).Nr;						//	��M�f�[�^��(�N��������)
											++(*(*ps).pDac).Ns;						//	��M�f�[�^��(����J�n������)
											(*(*ps).pDac).tmDac = md;				//	Dac Time Data
											(*(*ps).pDac).ofDac = of+(sz>>1);		//	Dac�o�b�t�@�ǉ��擪offset
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
					(*(*ps).pSwrt).pW = ob;						//	Swrt�o�b�t�@�ǉ��擪offset
				}
				if ((*ps).pTrap && (*ps).m.s.fTrap) {
					__UL ob = (*(*ps).pTrap).ofTrap;			//	Trap�o�b�t�@�ǉ��擪offset
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
					(*(*ps).pTrap).ofTrap  = ob;			//	Trap�o�b�t�@�ǉ��擪offset
					(*(*ps).pTrap).te = ft;						//	�ŏI��M����[Now()]
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
//�o�C�i�����M
// ����	dTx : ���M�f�[�^,szTx : ���M�o�C�g��(�ȗ�����-1,<0�őS�f�[�^), tm : �o�b�t�@�t�����̑҂�����(s)(�ȗ�����0), ch : ���M�`�����l��(�ȗ�����0)
// ���^�[��	���M�o�C�g��(<0���̓G���[,���M�o�C�g�����w��o�C�g����菬�������͎��ԃI�[�o�Ŏc��̃f�[�^�͖����M)
//********************************************************************************
EXTR DLLAPI __SL SendByte(char* dTx, __SL szTx, __SL tm, __SL ch) {
	__SL	PACKETSIZE;							//	��x�ɂ܂Ƃ߂đ���f�[�^�̃T�C�Y
	__SL	fLen;								//	���M���ꂽ�o�C�g��
	__SL	ret;
	struct txCom* px;
	struct tsCom* ps;
	__UC pn;
	if (ch >= bCom.nCon) {						//ch�ݒ莸�s(ch=0-nCon)
		return -1;
	}
	px = bCom.bxCom+ch;
	pn = (*px).ch;
	if (pn >= bCom.nCh) {						//ch�ݒ莸�s(ch=0-nCh)
		return -2;
	}
	ps = bCom.bsCom+pn;
	if (!(*ps).Han || (*ps).Han >= (HANDLE)-16) {	//ch�ݒ莸�s(Not Open)
		return -3;
	}

	if (szTx < 0) szTx = strlen(dTx);
	PACKETSIZE = (*ps).szBufTx;					//	��x�ɂ܂Ƃ߂đ���T�C�Y�̐ݒ�(�����PC�X�y�b�N�Ǝ�M���̓s���Ō��܂�)
	if (szTx < PACKETSIZE) PACKETSIZE = szTx;	//	�c�f�[�^�ʂ��p�P�b�g�T�C�Y�ȉ��Ȃ�p�P�b�g�T�C�Y�k������
	ret = szTx;
	if (ret) do {								//	���M����
		if (!WriteFile((*ps).Han, dTx, PACKETSIZE, (DWORD*)&fLen, 0)) {	//	�f�[�^�̑��M
			CloseCom(ch);
			ret = 0;
			break;
		}
		dTx += fLen;							//	���ɑ��M����o�C�g�ʒu
		szTx -= fLen;							//	�c�f�[�^�T�C�Y�X�V
	} while(szTx > 0);							//	�c�f�[�^�ʂ��Ȃ��Ȃ����瑗�M����
	return ret;
}
//********************************************************************************
//�����񑗐M(�����̓V�X�e�������`��,���{�ł�SJIS)
// ����	TxS : ���M������, tm : �o�b�t�@�t�����̑҂�����(s)(�ȗ�����0), ch : ���M�`�����l��(�ȗ�����0)
// ���^�[��	���M�o�C�g��(<0���̓G���[,���M�o�C�g�����w��o�C�g����菬�������͎��ԃI�[�o�Ŏc��̃f�[�^�͖����M)
//********************************************************************************
EXTR DLLAPI __SL Send(char* TxS, __SL tm, __SL ch) {
											//	�o�C�i�����M
	return SendByte(TxS, (int)(TxS - (char*)strchr(TxS, 0)), tm, ch);
}

//********************************************************************************
//�ڑ��|�[�g�̎�������
// ���^�[��	�g�p�\�|�[�g�ԍ����J���}�ŋ�؂���������
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
//�t�@�C���̍Ō�Ƀf�[�^��ǉ�
// ����	fname : �t�@�C����, pt : �ǉ��f�[�^�̃I�t�Z�b�gsz : �ǉ��o�C�g��, md : �ǉ��z��w��(�ȗ�����0/0=(*ps).Rx/����RxD)
// ���^�[��	�ǉ��o�C�g��(<0���̓G���[)
//********************************************************************************
EXTR DLLAPI __SL FileAdd(char* fname, char* pt, __SL sz) {
	__SL	fLen = -1;
	HANDLE	fn;
	fn = CreateFile(fname, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_ALWAYS, 0, 0);
	if (fn != (HANDLE)-1) {					//�I�[�v������
		fLen = SetFilePointer(fn, 0, 0, FILE_END);
		fLen = WriteFile(fn, pt, sz, (DWORD*)&fLen, 0);
		CloseHandle(fn);
	}
	return fLen;
}

//********************************************************************************
//�`�F�b�N�T���v�Z
// ����	ini : �����l, pt : �擪�A�h���X, sz : �o�C�g��
// ���^�[��	 CheckSum(8bit)
//********************************************************************************
EXTR DLLAPI __UC CalSum(__UC ini, __UC* pt, int sz) {				//	CheckSum
	if (sz > 0) do {
		ini -= *pt++;
	} while(--sz);
	return ini;
}
//********************************************************************************
//�f�[�^�ϊ��֐�
//********************************************************************************
EXTR DLLAPI __SL Exp2Lng(__SL n, __SL ne, __SL nv) {	//	�w�������ϊ�(ne:�w����,nv:������)
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

EXTR DLLAPI __SL Lng2Exp(__SL n, __SL ne, __SL nv) {	//	�����w���ϊ�(ne:�w����,nv:������)
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

EXTR DLLAPI __FL Lng2Flt(__SL x) {	//	32bit��������[Float]
	union tVar32 z;
	z.sl = x;
	return (z.fl);
}

EXTR DLLAPI __FL HF2Flt(__HF x) {	//	16bit��������[binary16]
	union tVar32 z;
	z.sl = (((x & 0x7FFF)+((127-15)<<10))<<(23-10))+((x>>15)<<31);
	return (z.fl);
}

EXTR DLLAPI __FL BF2Flt(__BF x) {	//	16bit��������[bFloat16]
	union tVar32 z;
	z.hf[1] = x;
	z.hf[0] = 0;
	return (z.fl);
}

EXTR DLLAPI __SL Flt2Lng(__FL x) {	//	32bit��������[Float]
	union tVar32 z;
	z.fl = x;
	return (z.sl);
}

EXTR DLLAPI __HF Flt2HF(__FL x) {	//	16bit��������[binary16]
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

EXTR DLLAPI __BF Flt2BF(__FL x) {	//	16bit��������[bFloat16]
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
