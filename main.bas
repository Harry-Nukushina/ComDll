Option Explicit

Private Type tsCom								'	Com�ڑ����
			nCh As Byte							'		Com port numbers
			nCon As Byte						'		Connect numbers
End Type
Private Type txCom								'	Com�ڑ����
			nCh As Byte							'		Com port numbers
			nCon As Byte						'		Connect numbers
End Type
Private Type tbCom								'	Com�ڑ����
			nCh As Byte							'		Com port numbers
			nCon As Byte						'		Connect numbers
			bsCom As LongPtr					'		base address sCom(Com port data)
			bxCom As LongPtr					'		base address xCom(Connect data)
End Type
'	DLL������
Private Declare PtrSafe Function IniSwrtCom Lib "SwrtCom" (ByVal y As LongPtr, ByVal z As LongPtr) As LongPtr
'	DLL�I��
Private Declare PtrSafe Sub  ExitSwrtCom Lib "SwrtCom" ()
'	�I�[�v��:����1 COM�|�[�g����("COM5"�Ƃ�) ����2:�{�[���[�g
Private Declare PtrSafe Function OpenComX Lib "SwrtCom" (ByVal x As LongPtr, ByVal y As LongPtr) As Long
'	�I�[�v��:����1 COM�|�[�g����("COM5"�Ƃ�) ����2:�{�[���[�g ����3:ch�w��
Private Declare PtrSafe Function OpenCom Lib "SwrtCom" (ByVal x As LongPtr, ByVal y As LongPtr, ByVal z As LongPtr) As LongPtr
'	�N���[�Y
Private Declare PtrSafe Function CloseCom Lib "SwrtCom" (ByVal x As LongPtr) As Long
'	�G���[���b�Z�[�W
Private Declare PtrSafe Sub MsgErr Lib "SwrtCom" (ByVal x As LongPtr)
'	�X�e�[�^�X���[�h
Private Declare PtrSafe Function GetStatus Lib "SwrtCom" (ByVal x As LongPtr) As Long
'	��M�G���[�N���A
Private Declare PtrSafe Function ClearComErr Lib "SwrtCom" (ByVal x As LongPtr) As Long
'	��M�h���C�o�o�b�t�@�N���A
Private Declare PtrSafe Function ClearCom Lib "SwrtCom" (ByVal x As LongPtr) As Long
'	��M�o�b�t�@�N���A
Private Declare PtrSafe Function ClearBuffer Lib "SwrtCom" (ByVal x As LongPtr) As Long
'	���[�h
Private Declare PtrSafe Function Read Lib "SwrtCom" (ByVal x As LongPtr, ByVal y As LongPtr, ByVal z As Any, ByVal u As LongPtr) As Long
'	�o�C�i�����[�h
Private Declare PtrSafe Function ReadByte Lib "SwrtCom" (ByVal x As LongPtr, ByVal y As LongPtr) As Long
'	�o�C�i�����M
Private Declare PtrSafe Function SendByte Lib "SwrtCom" (ByVal x As Any, ByVal y As LongPtr, ByVal z As LongPtr, ByVal u As LongPtr) As Long
'	�A�X�L�[���M
Private Declare PtrSafe Function Send Lib "SwrtCom" (ByVal x As Any, ByVal y As LongPtr, ByVal z As LongPtr) As Long
'	�ڑ��|�[�g�̎�������
Private Declare PtrSafe Function SerchCom Lib "SwrtCom" () As LongPtr
'	�t�@�C���̍Ō�Ƀf�[�^��ǉ�
Private Declare PtrSafe Function FileAdd Lib "SwrtCom" (ByVal x As Any, ByVal y As Any, ByVal z As LongPtr) As Long

'	�w�������ϊ�(ne:�w����,nv:������)
Private Declare PtrSafe Function Exp2Lng Lib "SwrtCom" (ByVal x As LongPtr, ByVal y As LongPtr, ByVal z As LongPtr) As Long
'	�����w���ϊ�(ne:�w����,nv:������)
Private Declare PtrSafe Function Lng2Exp Lib "SwrtCom" (ByVal x As LongPtr, ByVal y As LongPtr, ByVal z As LongPtr) As Long
'	32bit��������[Float]
Private Declare PtrSafe Function Lng2Flt Lib "SwrtCom" (ByVal x As LongPtr) As Single
'	16bit��������[binary16]
Private Declare PtrSafe Function HF2Flt Lib "SwrtCom" (ByVal x As LongPtr) As Single
'	16bit��������[bFloat16]
Private Declare PtrSafe Function BF2Flt Lib "SwrtCom" (ByVal x As LongPtr) As Single
'	32bit��������[Float]
Private Declare PtrSafe Function Flt2Lng Lib "SwrtCom" (ByVal x As Single) As Long
'	16bit��������[binary16]
Private Declare PtrSafe Function Flt2HF Lib "SwrtCom" (ByVal x As Single) As Long
'	16bit��������[bFloat16]
Private Declare PtrSafe Function Flt2BF Lib "SwrtCom" (ByVal x As Single) As Long

Sub test()
	Dim a, n, ie, iv As Long
	Dim f As Single
	ChDir "C:\D\tmp\SwrtX\DLL\"
	n = -1024
	ie = 3
	iv = 4
	a = Lng2Exp(n, ie, iv)
	Debug.Print "Lng2Exp[" & ie & "," & iv & "] :", n, a, Exp2Lng(a, ie, iv)
	n = 1024
	a = Lng2Exp(n, ie, iv)
	Debug.Print "Lng2Exp[" & ie & "," & iv & "] :", n, a, Exp2Lng(a, ie, iv)
	n = &H39800000
	'	32bit��������[Float]
	f = Lng2Flt(n)
	a = Flt2Lng(f)
	Debug.Print "Lng2Flt : " , ie, iv, Hex(n), f, Hex(a)
	'	16bit��������[binary16]
	n = &H3C00
	f = HF2Flt(n)
	a = Flt2HF(f)
	Debug.Print "HF2Flt : " , ie, iv, Hex(n), f, Hex(a)
	'	16bit��������[bFloat16]
	f = BF2Flt(n)
	a = Flt2BF(f)
	Debug.Print "BF2Flt : " , ie, iv, Hex(n), f, Hex(a)
End Sub
