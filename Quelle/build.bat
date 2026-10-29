@ECHO OFF
@REM These are the files, which produce the same code as the originals.
@REM They shall not change, so the are compared with every commit.
@REM
@REM -- here start the complete
..\BIN\BCC.EXE -mlarge -O- -c -1 -Y SEG001.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Y -IAIL SEG002.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Y SEG003.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Y SEG004.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Y SEG005.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Y SEG006.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Y SEG007.C
..\BIN\TASM.EXE /os /z SEG008.asm SEG008.OBJ
..\BIN\TASM.EXE /os /z SEG009.asm SEG009.OBJ
..\BIN\TASM.EXE /os /z SEG010.asm SEG010.OBJ
..\BIN\TASM.EXE /m /w+ /ml /iAIL AIL\AIL.ASM SEG011.OBJ
..\BIN\TASM.EXE /os /z SEG013.asm SEG013.OBJ
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG024.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG025.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG026.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG027.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG028.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG029.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG030.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG031.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG032.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG033.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG034.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG035.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG036.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG037.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG038.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG039.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG040.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG041.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG042.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG043.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG044.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG045.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG046.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG047.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG048.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG049.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG050.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG051.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG052.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG053.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG054.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG055.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG056.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG057.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG058.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG059.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG060.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG061.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG062.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG063.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG064.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG065.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG066.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG067.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG068.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG069.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG070.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG071.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG072.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG073.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG074.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG075.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG076.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG077.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG078.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG079.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG080.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG081.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG082.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG083.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG084.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG085.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG086.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG087.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG088.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG089.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG090.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG091.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG092.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG093.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG094.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG095.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG096.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG097.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG098.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG099.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG100.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG101.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG102.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG103.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG104.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG105.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG106.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG107.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG108.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG109.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG110.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG111.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG112.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG113.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG114.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG115.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG116.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG117.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG118.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG119.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG120.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG121.C
..\BIN\BCC.EXE -mlarge -O- -c -1 -Yo SEG122.C

@REM Linken: in der Entwicklungsumgebung wie Attic, Datum vorher auf 12.01.1994 setzen
@REM (steht in der EXE). Das Bau-Paket macht das mit DATUM.COM:
@REM ..\BIN\BC.EXE /b BLADEM.PRJ
