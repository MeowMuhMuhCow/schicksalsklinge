;===========================================================================
; SEG008.ASM - VGA-Grafikroutinen (und ein ungenutzter OPL-Musiktreiber)
; Die Schicksalsklinge (DSA 1), Attic 1992 - Rekonstruktion, bytegleich mit
; SCHICKM.EXE 3.02 (Bright-Eyes, kommentiert und in Attics Schreibweise)
;
; Inhalt:    32 Assemblerroutinen. 14 davon ruft das Spiel auf (oeffentlich,
;            Namen von Bright-Eyes), 18 sind unbenutzt (nicht oeffentlich,
;            nirgends aufgerufen) und tragen Namen nach ihrer Funktion:
;              BIOS:  _swap_u16, _set_video_mode, _set_video_page,
;                     _save_display_stat, vga_unused_set_cursor_pos,
;                     vga_unused_get_cursor_pos, vga_unused_set_cursor_shape,
;                     vga_unused_set_ega_palette
;              DAC:   _set_color, _set_palette
;              Linien und Spalten im Bildspeicher (A000h, 320x200):
;                     _draw_h_line, _draw_h_spaced_dots,
;                     vga_unused_get_line, vga_unused_put_line,
;                     vga_unused_get_column, vga_unused_put_column
;              Rechtecke: _pic_copy, _save_rect, _fill_rect,
;                     _copy_solid_permuted, _copy_solid
;              Entpacken: _decomp_rle (NVF-Bilder, Lauflaengen)
;              OPL2 (AdLib) Musik ueber den Timer-Interrupt, ganz ungenutzt:
;                     opl_unused_write_reg, opl_unused_calibrate,
;                     opl_unused_reset, opl_unused_timer_isr,
;                     opl_unused_restore_timer, opl_unused_install_timer,
;                     opl_unused_open, opl_unused_close, opl_unused_stop,
;                     opl_unused_play
; Aufrufer:  seg002 (swap_u32, main, process_nvf_extraction), seg003
;            door_frame, seg004 (schick_set_video, schick_reset_video,
;            do_h_line, do_v_line, do_pic_copy, do_save_rect, do_fill_rect),
;            seg029 draw_playmask, seg066 load_town_texture, seg075
;            DNG_draw_image, seg120 refresh_colors; set_palette aus vielen
;            Segmenten (siehe dort).
; Aufruf:    alle oeffentlichen Routinen far, C-Konvention (Argumente ab
;            [bp+6], der Aufrufer raeumt den Stack ab). Die OPL-Routinen
;            haben eigene Registerkonventionen (siehe dort).
; Register:  Die Grafikroutinen sichern DS, ES, SI, DI (und BP), AX/BX/CX/DX
;            sind frei. Bildschirmbreite ist ueberall fest 320.
; Daten:     _pic_copy liest g_pic_copy_rect (ds:2990h) und g_pic_copy_flag
;            (ds:4A92h) ueber CS mit festen Adressen: Im Linkbild beginnt
;            SEG008_TEXT bei 0F18:0008, DGROUP bei 14FC:0000. cs:87D0h ist
;            also dieselbe Speicherstelle wie ds:2990h. Vermutlich hat Attic
;            "cs:_g_pic_copy_rect" geschrieben und der Linker hat den Abstand
;            zum CS-Rahmen eingesetzt; das geht nur, weil DGROUP zufaellig
;            weniger als 64 KB hinter diesem Rahmen liegt.
;            Die OPL-Routinen haben ihre Variablen im Codesegment (cs:).
; Attics Schreibweise: Das Original wurde mit TASM und JUMPS uebersetzt.
;            Vorwaerts-Spruenge bekommen dabei Platz fuer einen langen
;            Sprung; reicht ein kurzer, fuellt TASM mit NOPs auf (jcc: drei,
;            jmp: eins, jcxz: fuenf). Zu weite bedingte Spruenge setzt TASM
;            selbst in Gegensprung plus jmp um. Bright-Eyes hatte die NOPs und
;            Umwege von Hand ausgeschrieben; hier stehen wieder die einfachen
;            Befehle. Die 16-fach entrollten Schleifen stehen als Makro.
;===========================================================================
.186
.model large
JUMPS
.code

	public _swap_u16
	public _set_video_mode
	public _set_video_page
	public _save_display_stat
	public _set_color
	public _set_palette
	public _draw_h_line
	public _draw_h_spaced_dots
	public _pic_copy
	public _save_rect
	public _fill_rect
	public _copy_solid_permuted
	public _copy_solid
	public _decomp_rle

		assume cs:@code

; Lage im Linkbild (siehe Kopf "Daten")
SEG008_START	= 8			; SEG008_TEXT beginnt bei 0F18:0008
CS_TO_DGROUP	= (14FCh - 0F18h) * 16	; = 5E40h: ds:x ist cs:x+5E40h
CS_PIC_COPY_RECT = CS_TO_DGROUP + 2990h	; g_pic_copy_rect: y1, x1, y2, x2
CS_PIC_COPY_FLAG = CS_TO_DGROUP + 4A92h	; g_pic_copy_flag

INT08_VEC	= 8 * 4			; Interruptvektor 08h (Timer)

SCREEN_W	= 320			; Breite einer Bildschirmzeile
FRAME_COLOR	= 0C8h			; ab hier Farben des Rahmens/der Oberflaeche

; ---------------------------------------------------------------------------
; uint16_t swap_u16(uint16_t val)
; Vertauscht die beiden Bytes eines Wortes (Motorola <-> Intel).
; Aufrufer: seg002 swap_u32.
; Aus: AX = val mit vertauschten Bytes
; ---------------------------------------------------------------------------
_swap_u16	proc far

		push	bp
		mov	bp, sp

		mov	ax, [bp+6]
		xchg	ah, al

		mov	sp, bp
		pop	bp
		retf

_swap_u16	endp


; ---------------------------------------------------------------------------
; void set_video_mode(int16_t mode)
; Schaltet den Videomodus um (BIOS int 10h, AH=00h).
; Aufrufer: seg004 schick_set_video (13h), schick_reset_video (alter Modus).
; ---------------------------------------------------------------------------
_set_video_mode	proc far

		push	bp
		mov	bp, sp
		push	ds
		push	es
		push	si
		push	di

		mov	ah, 0
		mov	al, [bp+6]
		int	10h

		pop	di
		pop	si
		pop	es
		pop	ds
		mov	sp, bp
		pop	bp
		retf

_set_video_mode	endp


; ---------------------------------------------------------------------------
; void set_video_page(int16_t page)
; Waehlt die angezeigte Bildschirmseite (BIOS int 10h, AH=05h).
; Aufrufer: seg004 schick_reset_video.
; ---------------------------------------------------------------------------
_set_video_page	proc far

		push	bp
		mov	bp, sp
		push	ds
		push	es
		push	si
		push	di

		mov	al, [bp+6]
		mov	ah, 5
		int	10h

		pop	di
		pop	si
		pop	es
		pop	ds
		mov	sp, bp
		pop	bp
		retf

_set_video_page	endp


; ---------------------------------------------------------------------------
; void save_display_stat(int16_t *p)
; Merkt sich den Bildschirmzustand beim Programmstart, als vier Woerter:
;   p[0] = Bildschirmseite, p[1] = Videomodus, p[2] = Spalten (int 10h/0Fh),
;   p[3] = Zeilen (int 10h/1130h mit BH=2, DL = Zeilen - 1).
; Aufrufer: seg002 main (&g_video_page_bak; p[1] ist g_video_mode_bak,
;   p[2..3] liegen in g_unkn_083).
; Besonderheit: int 10h/1130h liefert in ES:BP einen Zeiger auf den
;   Zeichensatz, deshalb werden BP und ES zusaetzlich gesichert.
; ---------------------------------------------------------------------------
_save_display_stat proc far

		push	bp
		mov	bp, sp
		push	ds
		push	es
		push	si
		push	di
		push	bp

		les	di, [bp+6]
		mov	ah, 0Fh			; Modus, Spalten, Seite holen
		int	10h
		mov	dx, ax
		xor	ax, ax
		mov	al, bh
		stosw				; Seite
		mov	al, dl
		stosw				; Modus
		mov	al, dh
		stosw				; Spalten
		push	es
		mov	ax, 1130h		; Zeichensatzinfo (8x14)
		mov	bh, 2
		int	10h
		xor	dh, dh
		inc	dx
		pop	es
		mov	es:[di], dx		; Zeilen

		pop	bp
		pop	di
		pop	si
		pop	es
		pop	ds
		mov	sp, bp
		pop	bp
		retf

_save_display_stat endp


; ---------------------------------------------------------------------------
; unbenutzt: void vga_unused_set_cursor_pos(int page, int row, int col)
; Setzt die Textcursorposition (BIOS int 10h, AH=02h).
; (Bright-Eyes: unused_func01)
; ---------------------------------------------------------------------------
vga_unused_set_cursor_pos proc far

		push	bp
		mov	bp, sp
		push	ds
		push	es
		push	si
		push	di

		mov	ah, 2
		mov	bh, [bp+6]		; Seite
		mov	dh, [bp+8]		; Zeile
		mov	dl, [bp+0Ah]		; Spalte
		int	10h

		pop	di
		pop	si
		pop	es
		pop	ds
		mov	sp, bp
		pop	bp
		retf

vga_unused_set_cursor_pos endp


; ---------------------------------------------------------------------------
; unbenutzt: int vga_unused_get_cursor_pos(int page)
; Liest die Textcursorposition (BIOS int 10h, AH=03h).
; Aus: AL = Zeile, AH = Spalte
; (Bright-Eyes: unused_func02)
; ---------------------------------------------------------------------------
vga_unused_get_cursor_pos proc far

		push	bp
		mov	bp, sp
		push	ds
		push	es
		push	si
		push	di

		mov	ah, 3
		mov	bh, [bp+6]		; Seite
		int	10h

		mov	al, dh
		mov	ah, dl

		pop	di
		pop	si
		pop	es
		pop	ds
		mov	sp, bp
		pop	bp
		retf

vga_unused_get_cursor_pos endp


; ---------------------------------------------------------------------------
; unbenutzt: void vga_unused_set_cursor_shape(int start, int end)
; Setzt Anfangs- und Endzeile des Textcursors (BIOS int 10h, AH=01h).
; (Bright-Eyes: unused_func03)
; ---------------------------------------------------------------------------
vga_unused_set_cursor_shape proc far

		push	bp
		mov	bp, sp
		push	ds
		push	es
		push	si
		push	di

		mov	ch, [bp+6]		; Anfangszeile
		mov	cl, [bp+8]		; Endzeile
		and	cx, 1FFFh
		mov	ah, 1
		int	10h

		pop	di
		pop	si
		pop	es
		pop	ds
		mov	sp, bp
		pop	bp
		retf

vga_unused_set_cursor_shape endp


; ---------------------------------------------------------------------------
; unbenutzt: void vga_unused_set_ega_palette(uint8_t *regs)
; Setzt die 16 Palettenregister des Attribut-Controllers aus regs[0..15]
; (BIOS int 10h, AX=1000h, einzeln).
; (Bright-Eyes: unused_func04)
; ---------------------------------------------------------------------------
vga_unused_set_ega_palette proc far

		push	bp
		mov	bp, sp
		push	ds
		push	es
		push	si
		push	di

		lds	si, [bp+6]
		mov	cx, 10h
		xor	bl, bl			; Register 0..15

@@ega_reg:
		mov	bh, [si]
		mov	ax, 1000h
		int	10h
		inc	si
		inc	bl
		loop	@@ega_reg

		pop	di
		pop	si
		pop	es
		pop	ds
		mov	sp, bp
		pop	bp
		retf
vga_unused_set_ega_palette endp

; ---------------------------------------------------------------------------
; void set_color(uint8_t *rgb, uint8_t color)
; Setzt eine Farbe der VGA-DAC-Palette (3 Bytes R, G, B, je 0..63).
; Vor dem Blauanteil wird auf den Strahlruecklauf gewartet (3DAh Bit 0).
; Aufrufer: seg004 schick_set_video, seg029 draw_playmask,
;   seg120 refresh_colors.
; ---------------------------------------------------------------------------
_set_color	proc far

;rgb		= dword	ptr  6
;color		= byte ptr  0Ah

		push	bp
		mov	bp, sp
		push	ds
		push	es
		push	si
		push	di

		cli
		mov	dx, 3C8h		; DAC-Schreibindex
		mov	al, [bp+0ah]
		out	dx, al
		lds	si, [bp+6]
		mov	dx, 3C9h		; DAC-Daten
		lodsb
		out	dx, al			; R
		lodsb
		out	dx, al			; G
		mov	dx, 3DAh		; Eingabestatus 1

@@color_retrace:
		in	al, dx
		and	al, 1
		jz	@@color_retrace

		mov	dx, 3C9h
		lodsb
		out	dx, al			; B

		sti

		pop	di
		pop	si
		pop	es
		pop	ds
		mov	sp, bp
		pop	bp
		retf
_set_color	endp


; ---------------------------------------------------------------------------
; void set_palette(uint8_t *rgb, int first_color, int colors)
; Setzt 'colors' Farben der VGA-DAC-Palette ab 'first_color' (je 3 Bytes
; R, G, B). Wie set_color wird vor jedem Blauanteil auf den Strahlruecklauf
; gewartet.
; Aufrufer: seg002, seg004, seg024, seg025, seg028, seg029, seg032, seg046,
;   seg049, seg063, seg065, seg066, seg075, seg093, seg094, seg106, seg120.
; ---------------------------------------------------------------------------
_set_palette	proc far

;rgb		= dword	ptr  6
;first_color	= byte ptr  0Ah
;colors		= word ptr  0Ch

		push	bp
		mov	bp, sp
		push	ds
		push	es
		push	si
		push	di

		cli

		mov	dx, 3C8h
		mov	al, [bp+0ah]		; erste Farbe
		out	dx, al
		lds	si, [bp+6]		; DS:SI = Farbdaten
		mov	dx, 3C9h
		mov	cx, [bp+0ch]		; Anzahl Farben

@@palette_color:
		lodsb
		out	dx, al			; R
		lodsb
		out	dx, al			; G
		mov	dx, 3DAh

@@palette_retrace:
		in	al, dx
		and	al, 1
		jz	@@palette_retrace

		mov	dx, 3C9h
		lodsb
		out	dx, al			; B
		loop	@@palette_color

		sti

		pop	di
		pop	si
		pop	es
		pop	ds
		mov	sp, bp
		pop	bp
		retf
_set_palette	endp


; ---------------------------------------------------------------------------
; void draw_h_line(uint8_t *dst, int count, uint8_t color)
; Fuellt 'count' Bytes ab dst mit 'color' (erst ein Byte, falls ungerade,
; dann wortweise).
; Aufrufer: seg004 do_h_line.
; ---------------------------------------------------------------------------
_draw_h_line	proc far

;dst		= dword	ptr  6
;count		= word ptr  0Ah
;color		= byte ptr  0Ch

		push	bp
		mov	bp, sp
		push	ds
		push	es
		push	si
		push	di

		les	di, [bp+6]
		mov	cx, [bp+0ah]
		mov	al, [bp+0ch]
		mov	ah, al
		shr	cx, 1
		jnb	@@h_line_words
		stosb

@@h_line_words:
		rep stosw

		pop	di
		pop	si
		pop	es
		pop	ds
		mov	sp, bp
		pop	bp
		retf

_draw_h_line	endp


; ---------------------------------------------------------------------------
; unbenutzt: void vga_unused_get_line(uint16_t off, uint8_t *dst)
; Kopiert eine Bildschirmzeile (320 Bytes) ab A000:off nach dst.
; (Bright-Eyes: unused_func05)
; ---------------------------------------------------------------------------
vga_unused_get_line proc far

		push	bp
		mov	bp, sp
		push	ds
		push	es
		push	si
		push	di

		mov	cx, SCREEN_W / 2
		mov	ax, 0A000h
		mov	ds, ax
		assume ds:nothing
		mov	si, [bp+6]
		les	di, [bp+8]
		rep movsw
		pop	di
		pop	si
		pop	es
		pop	ds
		assume ds:@DATA
		mov	sp, bp
		pop	bp
		retf

vga_unused_get_line endp


; ---------------------------------------------------------------------------
; unbenutzt: void vga_unused_put_line(uint16_t off, uint8_t *src)
; Kopiert 320 Bytes von src als Bildschirmzeile nach A000:off.
; (Bright-Eyes: unused_func06)
; ---------------------------------------------------------------------------
vga_unused_put_line proc far

		push	bp
		mov	bp, sp
		push	ds
		push	es
		push	si
		push	di

		mov	cx, SCREEN_W / 2
		mov	ax, 0A000h
		mov	es, ax
		assume es:nothing
		mov	di, [bp+6]
		lds	si, [bp+8]
		rep movsw

		pop	di
		pop	si
		pop	es
		assume es:nothing
		pop	ds
		mov	sp, bp
		pop	bp
		retf

vga_unused_put_line endp


; ---------------------------------------------------------------------------
; void draw_h_spaced_dots(uint8_t *dst, int count, uint8_t color, int space)
; Setzt 'count' Punkte der Farbe 'color' im Abstand 'space'. Mit space = 320
; ist das eine senkrechte Linie (so ruft es seg004 auf).
; Aufrufer: seg004 do_v_line.
; ---------------------------------------------------------------------------
_draw_h_spaced_dots proc	far

;dst		= dword	ptr  6
;count		= word ptr  0Ah
;color		= byte ptr  0Ch
;space		= word ptr  0Eh

		push	bp
		mov	bp, sp
		push	ds
		push	es
		push	si
		push	di

		les	di, [bp+6]
		mov	cx, [bp+0ah]
		mov	al, [bp+0ch]

@@dots_next:
		mov	es:[di], al
		add	di, [bp+0eh]
		loop	@@dots_next

		pop	di
		pop	si
		pop	es
		pop	ds
		mov	sp, bp
		pop	bp
		retf

_draw_h_spaced_dots endp


; ---------------------------------------------------------------------------
; unbenutzt: void vga_unused_get_column(uint16_t off, uint8_t *dst)
; Kopiert eine Bildschirmspalte (200 Punkte) ab A000:off nach dst.
; (Bright-Eyes: unused_func07)
; ---------------------------------------------------------------------------
vga_unused_get_column proc far

		push	bp
		mov	bp, sp
		push	ds
		push	es
		push	si
		push	di

		cld
		mov	ax, 0A000h
		mov	ds, ax
		assume ds:nothing
		mov	si, [bp+6]
		les	di, [bp+8]
		mov	cx, 200

@@get_col_pixel:
		movsb
		add	si, SCREEN_W - 1
		loop	@@get_col_pixel
		pop	di
		pop	si
		pop	es
		pop	ds
		assume ds:@DATA
		mov	sp, bp
		pop	bp
		retf

vga_unused_get_column endp


; ---------------------------------------------------------------------------
; unbenutzt: void vga_unused_put_column(uint16_t off, uint8_t *src)
; Kopiert 200 Bytes von src als Bildschirmspalte nach A000:off.
; (Bright-Eyes: unused_func08)
; ---------------------------------------------------------------------------
vga_unused_put_column proc far

		push	bp
		mov	bp, sp
		push	ds
		push	es
		push	si
		push	di

		cld
		mov	ax, 0A000h
		mov	es, ax
		assume es:nothing
		mov	di, [bp+6]
		lds	si, [bp+8]
		mov	cx, 200

@@put_col_pixel:
		movsb
		add	di, SCREEN_W - 1
		loop	@@put_col_pixel

		pop	di
		pop	si
		pop	es
		assume es:nothing
		pop	ds
		mov	sp, bp
		pop	bp
		retf

vga_unused_put_column endp


; Bildpunkt n kopieren, aber nur, wenn das Ziel keine Rahmenfarbe hat
; (_pic_copy Modus 1)
PRIO_PIXEL	MACRO	n
		LOCAL	weiter
		cmp	byte ptr es:[di+n], FRAME_COLOR
		jnb	weiter
		mov	al, [si+n]
		mov	es:[di+n], al
weiter:
		ENDM

; Bildpunkt n kopieren, aber nicht die Farbe 0 (_pic_copy Modus 2)
TRANS_PIXEL	MACRO	n
		LOCAL	weiter
		mov	al, [si+n]
		or	al, al
		jz	weiter
		mov	es:[di+n], al
weiter:
		ENDM

; ---------------------------------------------------------------------------
; void pic_copy(uint8_t *dst, int x1, int y1, int x2, int y2, int v1, int v2,
;               int v3, int v4, int width, int height, uint8_t *src, int mode)
; Kopiert ein Bild (width x height, Zeilenlaenge width) aus src an die
; Bildschirmposition (x1, y1)..(x2, y2) in dst (Zeilenlaenge 320) und
; schneidet es dabei an g_pic_copy_rect ab.
;   mode 1: nur Zielpunkte mit Farbe < 0C8h ueberschreiben (die Oberflaeche
;           bleibt stehen). Ist g_pic_copy_flag gesetzt (seg004 init_ani,
;           seg074 draw_automap_to_screen), wird in den letzten 40 Zeilen
;           dort trotzdem geschrieben, wo in der Zeile noch 76..149 Punkte
;           ausstehen (ein festes Fenster, Zaehler DX und CX).
;   mode 2: Farbe 0 der Quelle ist durchsichtig.
;   mode 3: Ausschnitt ab (v1, v2) aus einer 320 breiten Quelle. Wird
;           oben abgeschnitten, rueckt SI aber um Zeilen der Laenge width
;           statt 320 vor (wirkt nur, wenn g_pic_copy_rect oben greift).
;   sonst:  einfache Kopie.
; v3 und v4 werden nicht benutzt.
; Aufrufer: seg004 do_pic_copy (alle Bildausgaben laufen ueber g_pic_copy).
; ---------------------------------------------------------------------------
_pic_copy	proc far

src_skip	= word ptr -1Ah		; Rest einer Quellzeile (width - cur_width)
skip_bottom	= word ptr -18h		; abgeschnitten: unten
skip_top	= word ptr -16h		;                oben
skip_right	= word ptr -14h		;                rechts
skip_left	= word ptr -12h		;                links
cur_height	= word ptr -10h		; sichtbare Hoehe
cur_width	= word ptr -0Eh		; sichtbare Breite
clip_x2		= word ptr -0Ch		; Kopie von g_pic_copy_rect
clip_x1		= word ptr -0Ah
clip_y1		= word ptr -8
clip_y2		= word ptr -6
dst		= dword	ptr  6
x1		= word ptr  0Ah
y1		= word ptr  0Ch
x2		= word ptr  0Eh
y2		= word ptr  10h
v1		= word ptr  12h		; Modus 3: Quell-x
v2		= word ptr  14h		; Modus 3: Quell-y
src_width	= word ptr  1Ah
src_height	= word ptr  1Ch
src		= dword	ptr  1Eh
mode		= word ptr  22h

		push	bp
		mov	bp, sp
		sub	sp, 1Ah
		push	es
		push	di
		push	ds
		push	si

		les	di, [bp+dst]
		lds	si, [bp+src]
		mov	ax, [bp+src_width]
		mov	[bp+cur_width],	ax
		mov	ax, [bp+src_height]
		mov	[bp+cur_height], ax
		mov	[bp+skip_left], 0
		mov	[bp+skip_right], 0
		mov	[bp+skip_top], 0
		mov	[bp+skip_bottom], 0
		mov	ax, cs:[CS_PIC_COPY_RECT+0]	; g_pic_copy_rect.y1
		mov	[bp+clip_y1], ax
		mov	ax, cs:[CS_PIC_COPY_RECT+2]	; .x1
		mov	[bp+clip_x1], ax
		mov	ax, cs:[CS_PIC_COPY_RECT+4]	; .y2
		mov	[bp+clip_y2], ax
		mov	ax, cs:[CS_PIC_COPY_RECT+6]	; .x2
		mov	[bp+clip_x2], ax

		; oben abschneiden: Quelle um ganze Zeilen weiter
		mov	ax, [bp+y1]
		cmp	ax, [bp+clip_y1]
		jge	@@pc_top_ok
		mov	ax, [bp+clip_y1]
		sub	ax, [bp+y1]
		mov	[bp+skip_top], ax
		sub	[bp+cur_height], ax
		mov	cx, [bp+src_width]
		mul	cx
		add	si, ax

@@pc_top_ok:
		; unten abschneiden
		mov	ax, [bp+y2]
		cmp	ax, [bp+clip_y2]
		jle	@@pc_bottom_ok
		sub	ax, [bp+clip_y2]
		mov	[bp+skip_bottom], ax
		sub	[bp+cur_height], ax

@@pc_bottom_ok:
		; links abschneiden: Quelle um Punkte weiter
		mov	ax, [bp+x1]
		cmp	ax, [bp+clip_x1]
		jge	@@pc_left_ok
		mov	ax, [bp+clip_x1]
		sub	ax, [bp+x1]
		mov	[bp+skip_left], ax
		sub	[bp+cur_width],	ax
		add	si, ax

@@pc_left_ok:
		; rechts abschneiden
		mov	ax, [bp+x2]
		cmp	ax, [bp+clip_x2]
		jle	@@pc_right_ok
		sub	ax, [bp+clip_x2]
		mov	[bp+skip_right], ax
		sub	[bp+cur_width],	ax

@@pc_right_ok:
		; nichts mehr sichtbar?
		cmp	[bp+cur_height], 0
		jg	@@pc_height_ok
		jmp	@@pc_leave

@@pc_height_ok:
		cmp	[bp+cur_width],	0
		jg	@@pc_width_ok
		jmp	@@pc_leave

@@pc_width_ok:
		; ES:DI = dst + 320 * max(y1, clip_y1) + max(x1, clip_x1)
		mov	ax, [bp+clip_y1]
		mov	cx, SCREEN_W
		mul	cx
		add	di, ax
		add	di, [bp+clip_x1]
		mov	ax, [bp+y1]
		cmp	ax, [bp+clip_y1]
		jle	@@pc_dst_y_ok
		sub	ax, [bp+clip_y1]
		mov	cx, SCREEN_W
		mul	cx
		add	di, ax

@@pc_dst_y_ok:
		mov	ax, [bp+x1]
		cmp	ax, [bp+clip_x1]
		jle	@@pc_dst_x_ok
		sub	ax, [bp+clip_x1]
		add	di, ax

@@pc_dst_x_ok:
		mov	[bp+src_skip], 0
		mov	ax, [bp+src_width]
		sub	ax, [bp+cur_width]
		mov	[bp+src_skip], ax

		; nach Modus verzweigen
		cmp	[bp+mode], 1
		jnz	@@pc_not_mode1
		jmp	@@pc_mode1

@@pc_not_mode1:
		cmp	[bp+mode], 2
		jnz	@@pc_not_mode2
		jmp	@@pc_mode2

@@pc_not_mode2:
		cmp	[bp+mode], 3
		jz	@@pc_mode3
		jmp	@@pc_mode0

; --- Modus 1: Oberflaeche (Farben >= 0C8h) nicht ueberschreiben ------------
@@pc_mode1:
		mov	dx, [bp+cur_height]
		mov	bx, SCREEN_W
		sub	bx, [bp+cur_width]

@@pc_m1_row:
		mov	cx, [bp+cur_width]
		cmp	word ptr cs:[CS_PIC_COPY_FLAG], 0
		jz	@@pc_m1_plain

		; mit g_pic_copy_flag: Punkt fuer Punkt, Fenster unten frei
@@pc_m1_flag_pixel:
		mov	al, es:[di]
		cmp	dx, 40
		jnb	@@pc_m1_check
		cmp	cx, 75
		jle	@@pc_m1_check
		cmp	cx, 150
		jnb	@@pc_m1_check
		jmp	@@pc_m1_write

@@pc_m1_check:
		cmp	al, FRAME_COLOR
		jnb	@@pc_m1_skip

@@pc_m1_write:
		mov	al, [si]
		mov	es:[di], al

@@pc_m1_skip:
		inc	si
		inc	di
		loop	@@pc_m1_flag_pixel
		jmp	@@pc_m1_next_row

		; ohne g_pic_copy_flag: je 16 Punkte entrollt, dann der Rest
@@pc_m1_plain:
		cmp	cx, 10h
		jb	@@pc_m1_rest

@@pc_m1_16:
		IRP	n, <0,1,2,3,4,5,6,7,8,9,0Ah,0Bh,0Ch,0Dh,0Eh,0Fh>
		PRIO_PIXEL n
		ENDM
		add	si, 10h
		add	di, 10h
		sub	cx, 10h
		cmp	cx, 10h
		jnb	@@pc_m1_16

@@pc_m1_rest:
		jcxz	@@pc_m1_next_row

@@pc_m1_rest_pixel:
		PRIO_PIXEL 0
		inc	si
		inc	di
		loop	@@pc_m1_rest_pixel

@@pc_m1_next_row:
		add	di, bx
		add	si, [bp+src_skip]
		dec	dx
		jg	@@pc_m1_row
		jmp	@@pc_leave

; --- Modus 2: Farbe 0 ist durchsichtig ---------------------------------------
@@pc_mode2:
		mov	dx, [bp+cur_height]
		mov	bx, SCREEN_W
		sub	bx, [bp+cur_width]

@@pc_m2_row:
		mov	cx, [bp+cur_width]
		cmp	cx, 10h
		jb	@@pc_m2_rest

@@pc_m2_16:
		IRP	n, <0,1,2,3,4,5,6,7,8,9,0Ah,0Bh,0Ch,0Dh,0Eh,0Fh>
		TRANS_PIXEL n
		ENDM
		add	si, 10h
		add	di, 10h
		sub	cx, 10h
		cmp	cx, 10h
		jnb	@@pc_m2_16
		jcxz	@@pc_m2_next_row

@@pc_m2_rest:
		mov	al, [si]
		cmp	al, 0
		jz	@@pc_m2_rest_skip
		mov	es:[di], al

@@pc_m2_rest_skip:
		inc	si
		inc	di
		loop	@@pc_m2_rest

@@pc_m2_next_row:
		add	di, bx
		add	si, [bp+src_skip]
		dec	dx
		jg	@@pc_m2_row
		jmp	@@pc_leave

; --- Modus 3: Ausschnitt ab (v1, v2) aus einer 320 breiten Quelle -----------
@@pc_mode3:
		mov	ax, [bp+v2]
		mov	cx, SCREEN_W
		mul	cx
		add	ax, [bp+v1]
		add	si, ax
		mov	dx, [bp+cur_height]
		mov	bx, SCREEN_W
		sub	bx, [bp+cur_width]
		mov	ax, [bp+cur_width]

@@pc_m3_row:
		mov	cx, ax
		shr	cx, 1
		jnb	@@pc_m3_words
		movsb

@@pc_m3_words:
		rep movsw
		add	di, bx
		add	si, bx
		dec	dx
		jg	@@pc_m3_row
		jmp	@@pc_leave

; --- sonst: einfache Kopie ---------------------------------------------------
@@pc_mode0:
		mov	dx, [bp+cur_height]
		mov	bx, SCREEN_W
		sub	bx, [bp+cur_width]
		mov	ax, [bp+cur_width]

@@pc_m0_row:
		mov	cx, ax
		shr	cx, 1
		jnb	@@pc_m0_words
		movsb

@@pc_m0_words:
		rep movsw
		add	di, bx
		add	si, [bp+src_skip]
		dec	dx
		jg	@@pc_m0_row

@@pc_leave:
		pop	si
		pop	ds
		pop	di
		pop	es
		mov	sp, bp
		pop	bp
		retf
_pic_copy	endp


; ---------------------------------------------------------------------------
; void save_rect(uint16_t seg, uint16_t off, uint8_t *dst, int width,
;                int height)
; Kopiert ein Rechteck vom Bildschirm (seg:off, Zeilenlaenge 320) dicht
; gepackt nach dst (Zeilenlaenge width).
; Aufrufer: seg004 do_save_rect (dort heisst die Bildschirmposition "dst"
;   und der Puffer "src").
; ---------------------------------------------------------------------------
_save_rect	proc far

src_seg		= word ptr  6
src_off		= word ptr  8
dst		= dword	ptr  0Ah
r_width		= word ptr  0Eh
height		= word ptr  10h

		push	bp
		mov	bp, sp
		push	ds
		push	es
		push	si
		push	di

		mov	ax, [bp+src_seg]
		mov	ds, ax
		mov	si, [bp+src_off]
		les	di, [bp+dst]
		mov	dx, [bp+height]
		mov	bx, SCREEN_W
		sub	bx, [bp+r_width]
		mov	ax, [bp+r_width]

@@save_row:
		mov	cx, ax
		shr	cx, 1
		jnb	@@save_words
		movsb

@@save_words:
		rep movsw
		add	si, bx
		dec	dx
		jg	@@save_row

		pop	di
		pop	si
		pop	es
		pop	ds
		mov	sp, bp
		pop	bp
		retf

_save_rect	endp


; ---------------------------------------------------------------------------
; void fill_rect(uint16_t seg, uint16_t off, int color, int width,
;                int height)
; Fuellt ein Rechteck auf dem Bildschirm (seg:off, Zeilenlaenge 320) mit
; 'color'.
; Aufrufer: seg004 do_fill_rect.
; ---------------------------------------------------------------------------
_fill_rect	proc far

segm		= word ptr  6
off		= word ptr  8
color		= byte ptr  0Ah
p_width		= word ptr  0Ch
height		= word ptr  0Eh

		push	bp
		mov	bp, sp
		push	ds
		push	es
		push	si
		push	di

		mov	ax, [bp+segm]
		mov	es, ax
		mov	di, [bp+off]
		mov	dx, [bp+height]
		mov	bx, SCREEN_W
		sub	bx, [bp+p_width]
		mov	cx, [bp+p_width]
		mov	al, [bp+color]
		mov	ah, al

@@fill_row:
		push	cx
		shr	cx, 1
		jnb	@@fill_words
		stosb

@@fill_words:
		rep stosw
		pop	cx
		add	di, bx
		dec	dx
		jg	@@fill_row

		pop	di
		pop	si
		pop	es
		pop	ds
		mov	sp, bp
		pop	bp
		retf
_fill_rect	endp


; Bildpunkt kopieren, ausser er hat die Farbe 'solid' (AH ist 0).
; sign = + (copy_solid) oder - (copy_solid_permuted, Quelle rueckwaerts)
SOLID_PIXEL	MACRO	sign, n
		LOCAL	weiter
		mov	al, [si sign n]
		cmp	ax, [bp+solid]
		jz	weiter
		mov	es:[di+n], al
weiter:
		ENDM

; ---------------------------------------------------------------------------
; void copy_solid_permuted(uint8_t *dst, uint8_t *src, int width_to_copy,
;                          int height, int dst_width, int src_width,
;                          int solid)
; Wie copy_solid, aber jede Quellzeile wird rueckwaerts gelesen: das Bild
; erscheint waagerecht gespiegelt (src zeigt auf den letzten Punkt der
; ersten Zeile). Punkte der Farbe 'solid' bleiben durchsichtig.
; Aufrufer: seg075 DNG_draw_image.
; ---------------------------------------------------------------------------
_copy_solid_permuted proc far

dst		= dword	ptr  6
src		= dword	ptr  0Ah
width_to_copy	= word ptr  0Eh
height		= word ptr  10h
dst_width	= word ptr  12h
src_width	= word ptr  14h
solid		= word ptr  16h

		push	bp
		mov	bp, sp
		push	ds
		push	es
		push	si
		push	di

		les	di, [bp+dst]
		lds	si, [bp+src]
		xor	ax, ax
		xor	dx, dx			; DX = Zeile
		jmp	@@csp_row_cond

@@csp_row:
		push	si
		push	di
		mov	cx, [bp+width_to_copy]
		cmp	cx, 10h
		jb	@@csp_rest

@@csp_16:
		IRP	n, <0,1,2,3,4,5,6,7,8,9,0Ah,0Bh,0Ch,0Dh,0Eh,0Fh>
		SOLID_PIXEL -, n
		ENDM
		add	di, 10h
		sub	si, 10h
		sub	cx, 10h
		cmp	cx, 10h
		jnb	@@csp_16

@@csp_rest:
		jcxz	@@csp_next_row

@@csp_rest_pixel:
		SOLID_PIXEL -, 0
		inc	di
		dec	si
		loop	@@csp_rest_pixel

@@csp_next_row:
		pop	di
		pop	si
		add	si, [bp+src_width]
		add	di, [bp+dst_width]
		inc	dx

@@csp_row_cond:
		cmp	dx, [bp+height]
		jl	@@csp_row

		pop	di
		pop	si
		pop	es
		pop	ds
		mov	sp, bp
		pop	bp
		retf
_copy_solid_permuted endp


; ---------------------------------------------------------------------------
; void copy_solid(uint8_t *dst, uint8_t *src, int width_to_copy, int height,
;                 int dst_width, int src_width, int solid)
; Kopiert ein Rechteck von src nach dst; Punkte der Farbe 'solid' werden
; nicht kopiert (durchsichtig).
; Aufrufer: seg003 door_frame, seg066 load_town_texture,
;   seg075 DNG_draw_image.
; ---------------------------------------------------------------------------
_copy_solid	proc far

dst		= dword	ptr  6
src		= dword	ptr  0Ah
width_to_copy	= word ptr  0Eh
height		= word ptr  10h
dst_width	= word ptr  12h
src_width	= word ptr  14h
solid		= word ptr  16h

		push	bp
		mov	bp, sp
		push	ds
		push	es
		push	si
		push	di

		les	di, [bp+dst]
		lds	si, [bp+src]
		xor	ax, ax
		xor	dx, dx			; DX = Zeile
		jmp	@@cs_row_cond

@@cs_row:
		push	si
		push	di
		mov	cx, [bp+width_to_copy]
		cmp	cx, 10h
		jb	@@cs_rest

@@cs_16:
		IRP	n, <0,1,2,3,4,5,6,7,8,9,0Ah,0Bh,0Ch,0Dh,0Eh,0Fh>
		SOLID_PIXEL +, n
		ENDM
		add	si, 10h
		add	di, 10h
		sub	cx, 10h
		cmp	cx, 10h
		jnb	@@cs_16

@@cs_rest:
		jcxz	@@cs_next_row

@@cs_rest_pixel:
		SOLID_PIXEL +, 0
		inc	di
		inc	si
		loop	@@cs_rest_pixel

@@cs_next_row:
		pop	di
		pop	si
		add	si, [bp+src_width]
		add	di, [bp+dst_width]
		inc	dx

@@cs_row_cond:
		cmp	dx, [bp+height]
		jl	@@cs_row

		pop	di
		pop	si
		pop	es
		pop	ds
		mov	sp, bp
		pop	bp
		retf

_copy_solid	endp


; ---------------------------------------------------------------------------
; void decomp_rle(int width, int height, uint8_t *dst, uint8_t *src,
;                 uint8_t *tmp_buffer, int mode)
; Entpackt ein lauflaengenkodiertes NVF-Bild Zeile fuer Zeile. Im Strom
; steht 7Fh fuer eine Wiederholung (7Fh, Anzahl, Wert), jedes andere Byte
; ist ein Bildpunkt.
;   mode 2: direkt auf den Bildschirm (Zeilenlaenge 320)
;   mode 3: direkt in einen Puffer (Zeilenlaenge width)
;   mode 4: wie 2, aber waagerecht gespiegelt (Umweg ueber tmp_buffer)
;   mode 5: wie 3, aber waagerecht gespiegelt (Umweg ueber tmp_buffer)
; Aufrufer: seg002 process_nvf_extraction (tmp_buffer = g_text_output_buf).
; ---------------------------------------------------------------------------
_decomp_rle	proc far

p_width		= word ptr  6
height		= word ptr  8
dst		= dword	ptr  0Ah
src		= dword	ptr  0Eh
tmp_buffer	= dword	ptr  12h
mode		= word ptr  16h

		push	bp
		mov	bp, sp
		push	ds
		push	es
		push	si
		push	di
		mov	dx, [bp+height]

@@rle_row:
		; ES:DI = gepackte Daten, DS:SI = Ziel der Zeile
		les	di, [bp+src]
		cmp	[bp+mode], 5
		jz	@@rle_into_tmp
		cmp	[bp+mode], 4
		jnz	@@rle_into_dst

@@rle_into_tmp:
		lds	si, [bp+tmp_buffer]
		jmp	@@rle_unpack

@@rle_into_dst:
		lds	si, [bp+dst]

@@rle_unpack:
		push	dx
		mov	dx, [bp+p_width]	; DX = noch fehlende Punkte

@@rle_next:
		mov	al, es:[di]
		inc	di
		cmp	al, 7Fh
		jnz	@@rle_literal
		xor	bx, bx			; 7Fh, Anzahl, Wert
		xor	cx, cx
		mov	bl, es:[di]
		inc	di
		mov	cl, bl
		mov	al, es:[di]
		inc	di

@@rle_repeat:
		mov	[si], al
		inc	si
		loop	@@rle_repeat
		sub	dx, bx
		jmp	@@rle_line_cond

@@rle_literal:
		mov	[si], al
		inc	si
		dec	dx

@@rle_line_cond:
		cmp	dx, 0
		jg	@@rle_next
		pop	dx
		mov	word ptr [bp+src], di	; Lesezeiger merken
		mov	word ptr [bp+src+2], es
		les	di, [bp+dst]
		cmp	[bp+mode], 5
		jz	@@rle_mirror
		cmp	[bp+mode], 4
		jnz	@@rle_advance

@@rle_mirror:
		; Zeile aus tmp_buffer rueckwaerts nach ES:DI kopieren
		dec	si
		mov	cx, [bp+p_width]
		cmp	cx, 10h
		jb	@@rle_mirror_rest

@@rle_mirror_16:
		IRP	n, <0,1,2,3,4,5,6,7,8,9,0Ah,0Bh,0Ch,0Dh,0Eh,0Fh>
		mov	al, [si-n]
		mov	es:[di+n], al
		ENDM
		add	di, 10h
		sub	si, 10h
		sub	cx, 10h
		cmp	cx, 10h
		jnb	@@rle_mirror_16
		jcxz	@@rle_advance

@@rle_mirror_rest:
		mov	al, [si]
		mov	es:[di], al
		inc	di
		dec	si
		loop	@@rle_mirror_rest

@@rle_advance:
		; Zielzeiger auf die naechste Zeile
		cmp	[bp+mode], 2
		jz	@@rle_adv_screen
		cmp	[bp+mode], 4
		jz	@@rle_adv_mirror
		cmp	[bp+mode], 3
		jnz	@@rle_store_dst
		add	di, [bp+p_width]
		jmp	@@rle_store_dst

@@rle_adv_screen:
		add	di, SCREEN_W
		jmp	@@rle_store_dst

@@rle_adv_mirror:
		add	di, SCREEN_W
		sub	di, [bp+p_width]
		jmp	@@rle_store_dst

@@rle_store_dst:
		mov	word ptr [bp+dst], di
		mov	word ptr [bp+dst+2], es
		dec	dx
		jle	@@rle_leave
		jmp	@@rle_row

@@rle_leave:
		pop	di
		pop	si
		pop	es
		pop	ds
		mov	sp, bp
		pop	bp
		retf
_decomp_rle	endp

;===========================================================================
; Ungenutzter OPL2-Musiktreiber (AdLib, Port 388h/389h)
;
; Ein Abspieler fuer eine einfache Registerliste, angetrieben vom
; Timer-Interrupt 08h, den opl_unused_install_timer auf die vierfache Rate
; (PIT-Teiler 4000h, etwa 72,8 Hz) stellt. Das Spiel ruft nichts davon auf;
; die Musik laeuft ueber AIL-Treiber. Die Routinen sind far, haben aber
; keine C-Aufrufkonvention (ausser opl_unused_play).
;
; Musikdaten (opl_unused_play): am Anfang eine Tabelle von Wortabstaenden
; (relativ zum Anfang), je einer pro Abschnitt. Im Abschnitt Befehle:
;   00h            einen Takt warten
;   01h r,w ...    Register r := w (Paare), bis FFh FFh (Taktende) oder
;                  FFh FEh (naechster Abschnitt laut Tabelle, Taktende)
;   02h            anhalten
;   03h            Wiederholmarke setzen
;   04h            zur Wiederholmarke springen
;   05h t          Tempo t (Interrupts pro Takt)
;   06h n          zum Abschnitt n springen
;   sonst          anhalten
;===========================================================================

opl_delay_reg	dw 0		; Warteschleife nach Registerauswahl
opl_delay_data	dw 0		; Warteschleife nach Datenbyte
opl_old_int08	dd 0		; alter Timer-Interrupt
		dd 0		; (unbenutzt)
opl_ticks	dw 0		; Lo: Teiler fuer den alten int 08h (4),
				; Hi: Interrupts bis zum naechsten Takt
opl_song	dd 0		; Anfang der Musikdaten
opl_pos		dd 0		; Lesezeiger
opl_loop_pos	dd 0		; Wiederholmarke (Befehl 03h/04h)
opl_section	dw 0		; Abschnitt * 2 (Index in die Tabelle)
opl_tempo	db 0		; Interrupts pro Takt
opl_stopped	db 0		; 1 = steht
		db 0


; ---------------------------------------------------------------------------
; unbenutzt: opl_unused_write_reg
; Ein: AL = Register, AH = Wert. Schreibt das Register des OPL2 und wartet
; danach die geeichten Zeiten (opl_delay_reg/opl_delay_data Lesezugriffe
; auf den Statusport).
; Besonderheit: "and al, 0 / jnz" springt nie; die Wartezeit ergibt sich
; allein aus 'loop'. Die zweite Schleife beginnt auf gerader Adresse
; (align 2, daher das NOP), die erste liegt schon gerade.
; (Bright-Eyes: unused_func09)
; ---------------------------------------------------------------------------
opl_unused_write_reg proc far

		push	cx
		mov	dx, 388h
		out	dx, al
		mov	cx, cs:opl_delay_reg
		cli
		align	2

@@opl_wait_reg:
		in	al, dx
		and	al, 0
		jnz	@@opl_wait_reg
		loop	@@opl_wait_reg
		sti
		mov	al, ah
		inc	dx
		out	dx, al
		dec	dx
		mov	cx, cs:opl_delay_data
		cli
		align	2

@@opl_wait_data:
		in	al, dx
		and	al, 0
		jnz	@@opl_wait_data
		loop	@@opl_wait_data
		sti

		pop	cx
		retf
opl_unused_write_reg endp


; ---------------------------------------------------------------------------
; unbenutzt: opl_unused_calibrate
; Eicht die Warteschleifen: startet Timer 1 des OPL2 und zaehlt, wie viele
; Statusabfragen bis zu seinem Ablauf (Bit 6) vergehen. Daraus:
;   opl_delay_reg = n / 3103 + 1, opl_delay_data = n / 445 + 1.
; Laeuft der Zaehler leer (65536 Abfragen), gilt n = 7000.
; Besonderheit: Nach der Auswahl von Register 2 wird 1 hineingeschrieben,
; nicht Register 4 = 1 (Timer 1 starten). Vermutlich laeuft der Timer
; deshalb nie an und es bleibt immer bei n = 7000.
; (Bright-Eyes: unused_func10)
; ---------------------------------------------------------------------------
opl_unused_calibrate proc far
		mov	ah, 80h			; Register 4 := 80h (IRQ loeschen)
		mov	al, 4
		call	opl_unused_write_reg

		mov	ah, 80h			; Register 2 := 80h (Timer 1)
		mov	al, 2
		call	opl_unused_write_reg

		mov	dx, 388h
		mov	al, 2
		out	dx, al
		mov	cx, cs:opl_delay_reg
		cli

@@cal_wait_reg:
		in	al, dx
		and	al, 0
		jnz	@@cal_wait_reg
		loop	@@cal_wait_reg
		sti
		xor	cx, cx
		mov	al, 1
		inc	dx
		out	dx, al
		dec	dx
		cli

@@cal_poll:
		in	al, dx
		and	al, 40h
		jnz	@@cal_done
		loop	@@cal_poll
		mov	cx, 0E4A8h		; leergelaufen: -1B58h = 7000

@@cal_done:
		sti
		neg	cx
		mov	ax, cx
		xor	dx, dx
		mov	bx, 0C1Fh
		div	bx
		inc	ax
		mov	cs:opl_delay_reg, ax
		xor	dx, dx
		mov	ax, cx
		mov	bx, 1BDh
		div	bx
		inc	ax
		mov	cs:opl_delay_data, ax
		mov	ah, 80h			; Register 4 := 80h
		mov	al, 4
		call	opl_unused_write_reg
		retf

opl_unused_calibrate endp


; ---------------------------------------------------------------------------
; unbenutzt: opl_unused_reset
; Grundzustand des OPL2: Wellenformwahl an (Reg. 1), IRQ loeschen (4),
; Reg. 8 = 0, Reg. BDh = 20h (Rhythmusmodus), alle 9 Kanaele Taste aus
; (B0h..B8h = 0), alle Operatoren leise (40h..55h = 3Fh).
; Besonderheit: "mov al, al" ist ein wirkungsloser Befehl (Ueberbleibsel).
; (Bright-Eyes: unused_func11)
; ---------------------------------------------------------------------------
opl_unused_reset proc far
		mov	ah, 20h
		mov	al, 1
		call	opl_unused_write_reg

		mov	ah, 80h
		mov	al, 4
		call	opl_unused_write_reg

		mov	ah, 0
		mov	al, 8
		call	opl_unused_write_reg

		mov	ah, 20h
		mov	al, 0BDh
		call	opl_unused_write_reg

		xor	bl, bl

@@reset_key_off:
		mov	al, 0B0h
		add	al, bl
		mov	ah, 0
		mov	al, al
		call	opl_unused_write_reg

		inc	bl
		cmp	bl, 8
		jbe	@@reset_key_off
		xor	bl, bl

@@reset_level:
		mov	al, 40h
		add	al, bl
		mov	ah, 3Fh
		mov	al, al
		call	opl_unused_write_reg
		inc	bl
		cmp	bl, 15h
		jbe	@@reset_level
		retf

opl_unused_reset endp


; ---------------------------------------------------------------------------
; unbenutzt: opl_unused_timer_isr (Interrupt 08h)
; Ruft den alten Timer-Interrupt bei jedem vierten Aufruf (sonst EOI an den
; PIC) und spielt nach je opl_tempo Aufrufen einen Takt der Musikdaten
; (Befehle siehe oben).
; Besonderheit: Befehl 06h speichert opl_section zweimal.
; (Bright-Eyes: unused_func12; die Adresse 0BB0h in
; opl_unused_install_timer zeigt hierher)
; ---------------------------------------------------------------------------
opl_unused_timer_isr proc far

		push	ax
		push	bx
		mov	bx, cs:opl_ticks
		dec	bl
		jnz	@@isr_eoi
		mov	bl, 4
		pushf
		call	cs:opl_old_int08
		jmp	@@isr_tempo

@@isr_eoi:
		mov	al, 20h
		out	20h, al

@@isr_tempo:
		dec	bh
		jz	@@isr_beat
		mov	cs:opl_ticks, bx
		jmp	@@isr_leave

@@isr_beat:
		mov	bh, cs:opl_tempo
		mov	cs:opl_ticks, bx
		cmp	cs:opl_stopped, 1
		jnz	@@isr_play
		jmp	@@isr_leave

@@isr_play:
		push	cx
		push	dx
		push	si
		push	ds
		cld
		lds	si, cs:opl_pos

@@isr_cmd:
		lodsb
		cmp	al, 1
		jnz	@@isr_cmd0

		; 01h: Registerpaare bis FFh xx
@@isr_reg_pair:
		lodsw
		cmp	al, 0FFh
		jz	@@isr_reg_end
		call	opl_unused_write_reg
		jmp	@@isr_reg_pair

@@isr_reg_end:
		cmp	ah, 0FFh
		jz	@@isr_save_pos
		cmp	ah, 0FEh
		jnz	@@isr_save_pos
		; FFh FEh: naechster Abschnitt laut Tabelle
		lds	si, cs:opl_song
		mov	bx, cs:opl_section
		add	bx, 2
		mov	cs:opl_section, bx
		add	si, [bx+si]
		jmp	@@isr_save_pos

@@isr_cmd0:
		cmp	al, 0			; 00h: warten
		jnz	@@isr_cmd2
		jmp	@@isr_save_pos

@@isr_cmd2:
		cmp	al, 2			; 02h: anhalten
		jnz	@@isr_cmd3
		mov	cs:opl_stopped, 1
		jmp	@@isr_save_pos

@@isr_cmd3:
		cmp	al, 3			; 03h: Wiederholmarke setzen
		jnz	@@isr_cmd4
		mov	word ptr cs:opl_loop_pos, si
		mov	word ptr cs:opl_loop_pos+2, ds
		jmp	@@isr_save_pos

@@isr_cmd4:
		cmp	al, 4			; 04h: zur Wiederholmarke
		jnz	@@isr_cmd5
		lds	si, cs:opl_loop_pos
		jmp	@@isr_cmd

@@isr_cmd5:
		cmp	al, 5			; 05h: Tempo setzen
		jnz	@@isr_cmd6
		lodsb
		mov	cs:opl_tempo, al
		jmp	@@isr_cmd

@@isr_cmd6:
		cmp	al, 6			; 06h: zum Abschnitt n
		jnz	@@isr_cmd_other
		lodsb
		xor	ah, ah
		shl	ax, 1
		mov	cs:opl_section, ax
		mov	bx, ax
		mov	cs:opl_section, ax
		lds	si, cs:opl_song
		add	si, [bx+si]
		jmp	@@isr_cmd

@@isr_cmd_other:
		mov	cs:opl_stopped, 1	; unbekannt: anhalten

@@isr_save_pos:
		mov	word ptr cs:opl_pos, si
		mov	word ptr cs:opl_pos+2, ds
		pop	ds
		pop	si
		pop	dx
		pop	cx
@@isr_leave:
		pop	bx
		pop	ax
		iret
opl_unused_timer_isr endp


; ---------------------------------------------------------------------------
; unbenutzt: opl_unused_restore_timer
; Setzt den PIT (Kanal 0) auf die normale Rate (Teiler 0 = 65536, 18,2 Hz)
; und stellt den alten Interruptvektor 08h wieder her.
; (Bright-Eyes: unused_func13)
; ---------------------------------------------------------------------------
opl_unused_restore_timer proc far
		push	ds
		push	es
		cli
		mov	al, 36h
		out	43h, al
		xor	ax, ax
		out	40h, al
		out	40h, al
		mov	es, ax
		assume es:nothing
		lds	dx, cs:opl_old_int08
		mov	es:[INT08_VEC], dx
		mov	word ptr es:[INT08_VEC+2], ds
		sti
		pop	es
		assume es:nothing
		pop	ds
		retf

opl_unused_restore_timer endp

; ---------------------------------------------------------------------------
; unbenutzt: opl_unused_install_timer
; Haengt opl_unused_timer_isr in den Interrupt 08h (alten Vektor merken)
; und stellt den PIT (Kanal 0) auf Teiler 4000h, also die vierfache Rate.
; Besonderheit: Die Adresse des Handlers steht fest im Code (0BB0h, Lage im
; Linkbild); hier als Abstand zum Segmentanfang plus SEG008_START.
; (Bright-Eyes: unused_func14)
; ---------------------------------------------------------------------------
opl_unused_install_timer proc far
		push	es
		push	ds
		mov	dx, opl_unused_timer_isr - _swap_u16 + SEG008_START
		xor	ax, ax
		mov	ds, ax
		assume ds:nothing
		cli
		les	bx, ds:[INT08_VEC]
		mov	ds:[INT08_VEC], dx
		mov	word ptr ds:[INT08_VEC+2], cs
		mov	word ptr cs:opl_old_int08, bx
		mov	word ptr cs:opl_old_int08+2, es
		mov	al, 36h
		out	43h, al
		xor	al, al
		out	40h, al
		mov	al, 40h
		out	40h, al
		sti
		pop	ds
		assume ds:@DATA
		pop	es
		retf

opl_unused_install_timer endp


; ---------------------------------------------------------------------------
; unbenutzt: opl_unused_open
; Treiber starten: Warteschleifen vorbelegen (2000/7000), angehalten,
; OPL2 zuruecksetzen, Warteschleifen eichen, Timer einhaengen
; (opl_ticks = 0904h: Teiler 4, Tempozaehler 9). Alle Register gesichert.
; (Bright-Eyes: unused_func15)
; ---------------------------------------------------------------------------
opl_unused_open	proc far

		push	ax
		push	bx
		push	cx
		push	dx

		mov	cs:opl_delay_reg, 2000
		mov	cs:opl_delay_data, 7000
		mov	cs:opl_stopped, 1
		call	opl_unused_reset

		call	opl_unused_calibrate

		mov	ax, 2308		; = 0904h
		mov	cs:opl_ticks, ax
		call	opl_unused_install_timer

		pop	dx
		pop	cx
		pop	bx
		pop	ax
		retf

opl_unused_open	endp


; ---------------------------------------------------------------------------
; unbenutzt: opl_unused_close
; Treiber beenden: anhalten, OPL2 zuruecksetzen, Timer wiederherstellen.
; (Bright-Eyes: unused_func16)
; ---------------------------------------------------------------------------
opl_unused_close proc far

		push	ax
		push	bx
		push	cx
		push	dx

		mov	cs:opl_stopped, 1
		call	opl_unused_reset

		call	opl_unused_restore_timer

		pop	dx
		pop	cx
		pop	bx
		pop	ax
		retf

opl_unused_close endp

; ---------------------------------------------------------------------------
; unbenutzt: opl_unused_stop
; Haelt die Musik an (opl_stopped = 1).
; (Bright-Eyes: unused_func17)
; ---------------------------------------------------------------------------
opl_unused_stop	proc far

		mov	cs:opl_stopped, 1
		retf

opl_unused_stop	endp


; ---------------------------------------------------------------------------
; unbenutzt: void opl_unused_play(uint8_t *song)
; Startet die Musikdaten 'song' mit Abschnitt 0, Tempo 9.
; (Bright-Eyes: unused_func18)
; ---------------------------------------------------------------------------
opl_unused_play	proc far

song		= dword	ptr  6

		push	bp
		mov	bp, sp
		push	ds
		push	es
		push	si
		push	di
		mov	cs:opl_tempo, 9
		lds	si, [bp+song]
		mov	word ptr cs:opl_song, si
		mov	word ptr cs:opl_song+2, ds
		mov	cs:opl_section, 0
		add	si, [si]		; Abschnitt 0
		mov	word ptr cs:opl_pos, si
		mov	word ptr cs:opl_pos+2, ds
		mov	cs:opl_stopped, 0
		pop	di
		pop	si
		pop	es
		pop	ds
		mov	sp, bp
		pop	bp
		retf

opl_unused_play	endp

		end
