;===========================================================================
; SEG010.ASM - EMS (Expanded Memory ueber den EMM-Treiber, INT 67h)
; Die Schicksalsklinge (DSA 1), Attic 1992 - Rekonstruktion, bytegleich mit
; SCHICKM.EXE 3.02 (Bright-Eyes, kommentiert und mit eigenen Daten)
;
; Inhalt:    EMS_init (Treiber suchen, Seitenrahmen merken) und duenne
;            Huellen um die EMS-Funktionen 42h..45h: freie Seiten zaehlen,
;            Seiten anlegen, Seite einblenden, Seiten freigeben. Dazu
;            EMS_norm_ptr, das einen far-Zeiger normalisiert.
; Aufrufer:  seg120 (Start: EMS_init, Seiten anlegen), seg028 (Bilder,
;            Karten und Monsterbilder in den EMS auslagern und zurueckholen),
;            seg002 (Spielstand-Kopien im EMS, Programmende: freigeben).
; Aufruf:    far, C-Konvention, Stackrahmen bp, Parameter ab [bp+6].
;            Die beiden Hilfsroutinen EMS_installed und EMS_get_frame_ptr
;            sind near und nur von EMS_init aus erreichbar.
; Register:  DS, ES, SI, DI gesichert.
; Daten:     g_emm_sig und g_ems_frame_ptr (ds:4ba2..4bad). Bright-Eyes
;            hatte sie in seg007.cpp mit dem Vermerk "belong to seg010".
;            Sie stehen jetzt hier im eigenen _DATA, an derselben Adresse.
;===========================================================================
	locals

SEG010_TEXT	segment byte public 'CODE'
SEG010_TEXT	ends

DGROUP	group _DATA
	assume cs:SEG010_TEXT, ds:DGROUP

_DATA	segment word public 'DATA'
; Kennung, die ein EMM-Treiber 10 Bytes hinter dem Anfang seines
; Interrupt-67h-Segments traegt (Name im Treiberkopf)
_g_emm_sig		db	'EMMXXXX0'			; ds:4ba2
; Seitenrahmen des EMS (Segment:0000), 0 = kein EMS
_g_ems_frame_ptr	dd	0				; ds:4baa
_DATA	ends

SEG010_TEXT	segment byte public 'CODE'

	assume cs:SEG010_TEXT

; ---------------------------------------------------------------------------
; EMS_installed - ist ein EMM-Treiber geladen?
; Holt den Vektor von INT 67h (ES:BX) und vergleicht die 8 Bytes ab ES:000A
; mit "EMMXXXX0".
; Aus:  AX = 1 Treiber gefunden, 0 sonst
; ---------------------------------------------------------------------------
EMS_installed PROC NEAR
	push bp
	mov bp,sp
	push ds
	push es
	push si
	push di
	mov ax, 03567h		; DOS: Interruptvektor 67h holen -> ES:BX
	int 21h
	mov di, 0ah		; ES:DI = Name im Treiberkopf
	mov si, OFFSET _g_emm_sig	; DS:SI = "EMMXXXX0"
	mov cx, 9		; 8 Zeichen + 1: loope zaehlt cx vor der Pruefung
				; herunter. cx erreicht 0 nur, wenn alle 8
				; Zeichen gleich waren (das 9. ist egal)
@@vergleich:	cmpsb
	loope @@vergleich
	xor ax,ax
	or cx,cx
	jnz @@fertig
	mov ax, 1
@@fertig:
	pop di
	pop si
	pop es
	pop ds
	mov sp,bp
	pop bp
	ret
EMS_installed ENDP

; ---------------------------------------------------------------------------
; EMS_get_frame_ptr - Segment des EMS-Seitenrahmens (EMS 41h)
; Aus:  DX:AX = Seitenrahmen:0000, DX = 0 bei Fehler
; ---------------------------------------------------------------------------
EMS_get_frame_ptr PROC NEAR
	push bp
	mov bp,sp
	push ds
	push es
	push si
	push di
	mov ax, 4100h		; EMS: Seitenrahmen-Segment -> BX
	int 67h
	or ah,ah		; AH = Status, 0 = in Ordnung
	jz @@ok
	xor bx,bx
@@ok:	mov dx,bx
	xor ax,ax
	pop di
	pop si
	pop es
	pop ds
	mov sp,bp
	pop bp
	ret
EMS_get_frame_ptr ENDP

; ---------------------------------------------------------------------------
; signed int EMS_get_num_pages_unalloced(void) - freie EMS-Seiten (EMS 42h)
; Aus:  AX = Zahl der freien 16-KB-Seiten, 0 bei Fehler
; ---------------------------------------------------------------------------
_EMS_get_num_pages_unalloced PROC FAR
	push bp
	mov bp,sp
	push ds
	push es
	push si
	push di
	mov ax, 4200h		; EMS: Seitenzahl -> BX frei, DX gesamt
	int 67h
	or ah,ah
	jz @@ok
	xor bx,bx
@@ok:	mov ax,bx
	pop di
	pop si
	pop es
	pop ds
	mov sp,bp
	pop bp
	retf
_EMS_get_num_pages_unalloced ENDP

; ---------------------------------------------------------------------------
; signed int EMS_alloc_pages(int16_t pages) - Seiten anlegen (EMS 43h)
; Ein:  [bp+6] = Zahl der Seiten
; Aus:  AX = EMS-Handle, 0 bei Fehler
; ---------------------------------------------------------------------------
_EMS_alloc_pages PROC FAR
	push bp
	mov bp,sp
	push ds
	push es
	push si
	push di
	mov ax, 4300h		; EMS: BX Seiten anlegen -> Handle in DX
	mov bx,[bp+6h]
	int 67h
	or ah,ah
	jz @@ok
	xor dx,dx
@@ok:
	mov ax,dx
	pop di
	pop si
	pop es
	pop ds
	mov sp,bp
	pop bp
	retf
_EMS_alloc_pages ENDP

; ---------------------------------------------------------------------------
; signed int EMS_free_pages(int16_t handle) - Handle freigeben (EMS 45h)
; Ein:  [bp+6] = Handle
; Aus:  AL = EMS-Status (0 = in Ordnung)
; ---------------------------------------------------------------------------
_EMS_free_pages PROC FAR
	push bp
	mov bp,sp
	push ds
	push es
	push si
	push di
	mov ax, 4500h		; EMS: Handle DX freigeben
	mov dx,[bp+6h]
	int 67h
	mov al,ah
	pop di
	pop si
	pop es
	pop ds
	mov sp,bp
	pop bp
	retf
_EMS_free_pages ENDP

; ---------------------------------------------------------------------------
; int16_t EMS_map_memory(int16_t handle, int16_t lpage, int16_t ppage)
; Logische Seite eines Handles in eine physische Seite des Rahmens
; einblenden (EMS 44h).
; Ein:  [bp+6] = Handle, [bp+8] = logische Seite, [bp+0Ah] = physische
;       Seite 0..3
; Aus:  AL = EMS-Status (0 = in Ordnung)
; ---------------------------------------------------------------------------
_EMS_map_memory PROC FAR
	push bp
	mov bp,sp
	push ds
	push es
	push si
	push di
	mov ax, 4400h		; EMS: einblenden, AL = physische Seite
	mov al,[bp+0ah]
	mov bx,[bp+08h]		; BX = logische Seite
	mov dx,[bp+06h]		; DX = Handle
	int 67h
	mov al,ah
	pop di
	pop si
	pop es
	pop ds
	mov sp,bp
	pop bp
	retf
_EMS_map_memory ENDP

; ---------------------------------------------------------------------------
; uint8_t* EMS_norm_ptr(uint8_t* p) - far-Zeiger normalisieren
; Rechnet Segment:Offset in eine lineare Adresse um und zurueck, so dass
; der Offset danach 0..15 ist. Dann kann memmove einen grossen Block ab
; diesem Zeiger kopieren, ohne am Segmentende umzulaufen.
; Ein:  [bp+6] = Offset, [bp+8] = Segment
; Aus:  DX:AX = normalisierter Zeiger
; Hinweis: rechnet mit F_LXLSH@ (long << cl) aus der Borland-Bibliothek.
;          Die zweite Verschiebung (um 12) liefert in DX das neue Segment.
; ---------------------------------------------------------------------------
_EMS_norm_ptr PROC FAR
	push bp
	mov bp,sp
	push ds
	push es
	push si
	push di
	xor dx,dx
	mov ax, [bp+8h]		; DX:AX = Segment
	mov cx, 4

	call far ptr F_LXLSH@	; DX:AX = Segment * 16

	mov [bp+8], word ptr 0h	; Parameter wird zum long: Offset, High-Wort 0
	add ax,[bp+6]
	adc dx,[bp+8]		; DX:AX = lineare Adresse
	mov cx, 0ch

	call far ptr F_LXLSH@	; DX = lineare Adresse / 16 = Segment

	mov ax,[bp+6]
	and ax, 0fh		; AX = Offset 0..15
	pop di
	pop si
	pop es
	pop ds
	mov sp,bp
	pop bp
	retf
_EMS_norm_ptr ENDP

; ---------------------------------------------------------------------------
; signed int EMS_init(void) - EMS suchen und Seitenrahmen merken
; Aus:  AX = 1 EMS vorhanden (g_ems_frame_ptr gesetzt), 0 sonst
; ---------------------------------------------------------------------------
_EMS_init PROC FAR
	push bp
	mov bp,sp
	push ds
	push es
	push si
	push di
	call EMS_installed
	or ax,ax
	jz @@fertig
	call EMS_get_frame_ptr

	mov word ptr [_g_ems_frame_ptr + 2], dx

	mov word ptr [_g_ems_frame_ptr], word ptr 0h

	mov ax, 1
@@fertig:
	pop di
	pop si
	pop es
	pop ds
	mov sp,bp
	pop bp
	retf
_EMS_init ENDP

SEG010_TEXT ends

	public _EMS_get_num_pages_unalloced
	public _EMS_alloc_pages
	public _EMS_free_pages
	public _EMS_map_memory
	public _EMS_norm_ptr
	public _EMS_init
	public _g_emm_sig
	public _g_ems_frame_ptr
	extrn F_LXLSH@:far

	end
