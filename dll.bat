rem cls
gcc -shared -DBUILD_DLL -o %1SwrtCom.dll SwrtCom.c
IF ERRORLEVEL 1 GOTO ex
IF NOT %1L==L GOTO ex
gcc -L./ -lSwrtCom -o main.exe main.c
IF ERRORLEVEL 1 GOTO ex
main
:ex
