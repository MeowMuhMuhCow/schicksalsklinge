# Linking in the IDE (BC.EXE) – gives the original BLADEM.EXE byte for byte

TLINK 5.1 always writes the program name into `_EXEINFO_` in lower case
("bladem.exe"). The integrated linker of the Borland C++ 3.1 IDE writes it in
upper case ("BLADEM.EXE"), like the original. So Attic most probably linked in the IDE.

1. Build all OBJ files with build.bat as usual.
2. Set the date to 12.01.1994 (SETDATE), then start `BC BLADEM.PRJ`.
   BLADEM.PRJ contains the 111 OBJ files in the order of TLINK.RES, the overlay
   flag per module (all except SEG001-SEG010, SEG013 and SEG011) and these options:
   Application: DOS Overlay, model Large, Linker: map Detailed, Overlaid DOS EXE,
   Case-sensitive link OFF, Debugger: Source debugging None.
   Directories in the project: C:\BC\INCLUDE and C:\BC\LIB (adjust in Options|Directories).
3. Compile | Link.

Result: MD5 4ce035f95ead01a8b2553b4487544e5d = SCHICKM.EXE (GOG, CD 3.02 de),
without any post-processing.

With TLINK instead of the IDE, only the 9 bytes of the name differ.
