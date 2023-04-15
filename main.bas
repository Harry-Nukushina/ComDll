Option Explicit

Private Type tsCom								'	Com接続情報
			nCh As Byte							'		Com port numbers
			nCon As Byte						'		Connect numbers
End Type
Private Type txCom								'	Com接続情報
			nCh As Byte							'		Com port numbers
			nCon As Byte						'		Connect numbers
End Type
Private Type tbCom								'	Com接続情報
			nCh As Byte							'		Com port numbers
			nCon As Byte						'		Connect numbers
			bsCom As LongPtr					'		base address sCom(Com port data)
			bxCom As LongPtr					'		base address xCom(Connect data)
End Type
'	DLL初期化
Private Declare PtrSafe Function IniSwrtCom Lib "SwrtCom" (ByVal y As LongPtr, ByVal z As LongPtr) As LongPtr
'	DLL終了
Private Declare PtrSafe Sub  ExitSwrtCom Lib "SwrtCom" ()
'	オープン:引数1 COMポート名称("COM5"とか) 引数2:ボーレート
Private Declare PtrSafe Function OpenComX Lib "SwrtCom" (ByVal x As LongPtr, ByVal y As LongPtr) As Long
'	オープン:引数1 COMポート名称("COM5"とか) 引数2:ボーレート 引数3:ch指定
Private Declare PtrSafe Function OpenCom Lib "SwrtCom" (ByVal x As LongPtr, ByVal y As LongPtr, ByVal z As LongPtr) As LongPtr
'	クローズ
Private Declare PtrSafe Function CloseCom Lib "SwrtCom" (ByVal x As LongPtr) As Long
'	エラーメッセージ
Private Declare PtrSafe Sub MsgErr Lib "SwrtCom" (ByVal x As LongPtr)
'	ステータスリード
Private Declare PtrSafe Function GetStatus Lib "SwrtCom" (ByVal x As LongPtr) As Long
'	受信エラークリア
Private Declare PtrSafe Function ClearComErr Lib "SwrtCom" (ByVal x As LongPtr) As Long
'	受信ドライババッファクリア
Private Declare PtrSafe Function ClearCom Lib "SwrtCom" (ByVal x As LongPtr) As Long
'	受信バッファクリア
Private Declare PtrSafe Function ClearBuffer Lib "SwrtCom" (ByVal x As LongPtr) As Long
'	リード
Private Declare PtrSafe Function Read Lib "SwrtCom" (ByVal x As LongPtr, ByVal y As LongPtr, ByVal z As Any, ByVal u As LongPtr) As Long
'	バイナリリード
Private Declare PtrSafe Function ReadByte Lib "SwrtCom" (ByVal x As LongPtr, ByVal y As LongPtr) As Long
'	バイナリ送信
Private Declare PtrSafe Function SendByte Lib "SwrtCom" (ByVal x As Any, ByVal y As LongPtr, ByVal z As LongPtr, ByVal u As LongPtr) As Long
'	アスキー送信
Private Declare PtrSafe Function Send Lib "SwrtCom" (ByVal x As Any, ByVal y As LongPtr, ByVal z As LongPtr) As Long
'	接続ポートの自動検索
Private Declare PtrSafe Function SerchCom Lib "SwrtCom" () As LongPtr
'	ファイルの最後にデータを追加
Private Declare PtrSafe Function FileAdd Lib "SwrtCom" (ByVal x As Any, ByVal y As Any, ByVal z As LongPtr) As Long

'	指数整数変換(ne:指数桁,nv:仮数桁)
Private Declare PtrSafe Function Exp2Lng Lib "SwrtCom" (ByVal x As LongPtr, ByVal y As LongPtr, ByVal z As LongPtr) As Long
'	整数指数変換(ne:指数桁,nv:仮数桁)
Private Declare PtrSafe Function Lng2Exp Lib "SwrtCom" (ByVal x As LongPtr, ByVal y As LongPtr, ByVal z As LongPtr) As Long
'	32bit浮動少数[Float]
Private Declare PtrSafe Function Lng2Flt Lib "SwrtCom" (ByVal x As LongPtr) As Single
'	16bit浮動少数[binary16]
Private Declare PtrSafe Function HF2Flt Lib "SwrtCom" (ByVal x As LongPtr) As Single
'	16bit浮動少数[bFloat16]
Private Declare PtrSafe Function BF2Flt Lib "SwrtCom" (ByVal x As LongPtr) As Single
'	32bit浮動少数[Float]
Private Declare PtrSafe Function Flt2Lng Lib "SwrtCom" (ByVal x As Single) As Long
'	16bit浮動少数[binary16]
Private Declare PtrSafe Function Flt2HF Lib "SwrtCom" (ByVal x As Single) As Long
'	16bit浮動少数[bFloat16]
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
	'	32bit浮動少数[Float]
	f = Lng2Flt(n)
	a = Flt2Lng(f)
	Debug.Print "Lng2Flt : " , ie, iv, Hex(n), f, Hex(a)
	'	16bit浮動少数[binary16]
	n = &H3C00
	f = HF2Flt(n)
	a = Flt2HF(f)
	Debug.Print "HF2Flt : " , ie, iv, Hex(n), f, Hex(a)
	'	16bit浮動少数[bFloat16]
	f = BF2Flt(n)
	a = Flt2BF(f)
	Debug.Print "BF2Flt : " , ie, iv, Hex(n), f, Hex(a)
End Sub
