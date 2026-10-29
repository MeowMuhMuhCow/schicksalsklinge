;===========================================================================
; SEG013.ASM - Anfragebloecke fuer CD-Audio (MSCDEX), CD-Puffer, Stack
; Die Schicksalsklinge (DSA 1), Attic 1992 - Rekonstruktion, bytegleich mit
; SCHICKM.EXE 3.02 (Bright-Eyes, kommentiert und in Attics Schreibweise)
;
; Inhalt:    Segment CDDATA (FAR_DATA, 434h Bytes):
;              _req      9 Anfragebloecke (Request Header) fuer den
;                        CD-Geraetetreiber, je 28 Bytes (req[0..8])
;              _cd_buf1  Puffer fuer IOCTL-Antworten (Titelliste)
;              _cd_buf2  dahinter, wird nur ueber cd_buf1[324h..] benutzt
;            Segment stack2_seg: der 4-KB-Stack des Programms.
; Aufrufer:  seg001 (CD_driver_request: int 2Fh, AX=1510h, ES:BX = req[n],
;            CX = Laufwerk). Die C-Seite sieht die Bloecke als
;            struct driver_request far req[9] (seg013.h).
; Daten:     _cd_ioctl_reset_code (seg001, Wert 2) ist der Steuerblock fuer req[8].
;===========================================================================

; Anfrageblock fuer den CD-Treiber (Geraetetreiber-Request-Header nach der
; MSCDEX-Spezifikation), 28 Bytes, Aufbau wie struct driver_request:
;   +00h  Laenge des Blocks (fuer den Treiber)
;   +01h  Untereinheit (0)
;   +02h  Kommando
;   +03h  Status (Wort, vom Treiber; seg001 setzt ihn vor jedem Aufruf auf 0)
;   +05h  8 Bytes reserviert
;   +0Dh  Medium (IOCTL) bzw. Adressierungsart: 0 = HSG, 1 = Red Book
;   +0Eh  IOCTL: Pufferadresse (far), PLAY: Startsektor, SEEK: Pufferadresse
;   +12h  IOCTL: Anzahl Bytes, PLAY: Anzahl Sektoren (Dword, mit +14h)
;   +14h  IOCTL: Startsektor (0), SEEK: Startsektor
;   +16h  6 Bytes (IOCTL: Zeiger auf Datentraegerkennung, unbenutzt)
; Ohne DUP geschrieben, damit TASM wie im Original LEDATA erzeugt.
GEN_REQ	MACRO	rlen, cmd, mode, xfer, w12, w14
		db	rlen, 0, cmd
		dw	0
		db	0, 0, 0, 0, 0, 0, 0, 0
		db	mode
		dd	xfer
		dw	w12
		dw	w14
		db	0, 0, 0, 0, 0, 0
	ENDM

; Kommandos des Geraetetreibers
CMD_IOCTL_INPUT	 = 03h
CMD_IOCTL_OUTPUT = 0Ch
CMD_SEEK	 = 83h
CMD_PLAY_AUDIO	 = 84h
CMD_STOP_AUDIO	 = 85h
CMD_RESUME_AUDIO = 88h

	public _req
	public _cd_buf1
	public _cd_buf2
	extrn _cd_ioctl_reset_code:byte

cddata	segment byte public 'FAR_DATA'
	assume nothing

;		Laenge Kommando		  Modus	Zeiger	  +12h	+14h

	; req[0] STOP: haelt an (seg001 CD_audio_stop_hsg; der Name ist
	;        irrefuehrend, STOP hat keine Adressierungsart)
_req	label	byte
	GEN_REQ	16h, CMD_STOP_AUDIO,	  0,	0,	  0,	0

	; req[1] STOP: zweites STOP setzt die Fortsetzposition zurueck
	;        (seg001 CD_audio_stop)
req1	label	byte
	GEN_REQ	16h, CMD_STOP_AUDIO,	  1,	0,	  0,	0

	; req[2] IOCTL INPUT, 7 Bytes: Steuercode 0Ah (Audio-Disk-Info) bzw.
	;        0Bh (Audio-Titel-Info); Puffer setzt seg001 CD_0432
req2	label	byte
	GEN_REQ	1Ah, CMD_IOCTL_INPUT,	  0,	0,	  7,	0

	; req[3] IOCTL INPUT, 11 Bytes: Steuercode 0Ch (Audio-Q-Kanal-Info),
	;        seg001 CD_unused1
req3	label	byte
	GEN_REQ	1Ah, CMD_IOCTL_INPUT,	  0,	0,	  0Bh,	0

	; req[4] SEEK auf HSG-Sektor 2: von C aus nie benutzt
req4	label	byte
	GEN_REQ	18h, CMD_SEEK,		  0,	0,	  0,	2

	; req[5] PLAY AUDIO, Red Book: Start (+0Eh) und Laenge (+12h) setzt
	;        seg001 CD_audio_track_start
req5	label	byte
	GEN_REQ	16h, CMD_PLAY_AUDIO,	  1,	0,	  0,	0

	; req[6] STOP als Pause (seg001 CD_audio_pause)
req6	label	byte
	GEN_REQ	16h, CMD_STOP_AUDIO,	  1,	0,	  0,	0

	; req[7] RESUME AUDIO (seg001 CD_audio_resume)
req7	label	byte
	GEN_REQ	16h, CMD_RESUME_AUDIO,	  1,	0,	  0,	0

	; req[8] IOCTL OUTPUT, 1 Byte aus _cd_ioctl_reset_code (= 2: Laufwerk
	;        zuruecksetzen); von C aus nie benutzt
req8	label	byte
	GEN_REQ	16h, CMD_IOCTL_OUTPUT,	  0,	_cd_ioctl_reset_code, 1,	0

; Antwortpuffer fuer IOCTL INPUT (seg001):
;   cd_buf1[0..0Ah]          Q-Kanal-Info (req[3])
;   cd_buf1[0Ch + 8*n]       Titel-Info von Titel n (Code 0Bh, Titel, Start
;                            als Frame/Sekunde/Minute, Steuerbyte)
;   cd_buf1[324h..32Ah]      Disk-Info (Code 0Ah, erster und letzter Titel,
;                            Ende der Disk) - das liegt schon in cd_buf2
; 804 = 0Ch + 8 * 99 Bytes: Platz fuer die Titel-Eintraege 0..98.
_cd_buf1 db 804 dup (0)
_cd_buf2 db 20 dup (0)

cddata	ends

; Der 4-KB-Stack: Kombinationstyp STACK und Mitglied von DGROUP, wie im
; Original. Dann traegt der Linker selbst SS:SP = 236F:1000 in den MZ-Kopf
; ein, die Segmenttabelle in _EXEINFO_ fuehrt den _STACK aus C0L und dieses
; Segment als Teil von DGROUP (Groesse FFFF, Flag 4), und DGROUP schliesst
; den Stack ein.
stack2_seg	segment para stack 'STACK'
		db 1000h dup (?)
stack2_seg	ends
DGROUP	group stack2_seg
end
