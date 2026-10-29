;===========================================================================
; SEG009.ASM - PowerPacker-Entpacker (PP20)
; Die Schicksalsklinge (DSA 1), Attic 1992 - Rekonstruktion, bytegleich mit
; SCHICKM.EXE 3.02 (Bright-Eyes, kommentiert und in Attics Schreibweise)
;
; Inhalt:    decomp_pp20 entpackt eine mit PowerPacker 2.0 ("PP20", Amiga)
;            gepackte Datei. Fast alle Bilder des Spiels (NVF, Hintergruende,
;            Karte, Spielerbilder) liegen so gepackt im SCHICK.DAT.
; Aufrufer:  seg002 process_nvf_extraction (NVF-Bilder), seg027 load_pp20
;            (gepufferte Vollbilder).
; Verfahren: PowerPacker entpackt RUECKWAERTS: Lesen vom Ende der gepackten
;            Daten, Schreiben vom Ende des Ziels nach vorn. Deshalb darf das
;            Ziel die gepackten Daten ueberlappen (seg027 liest notfalls 8 Bytes
;            vor das Ziel und entpackt an Ort und Stelle).
;            Die gepackte Datei: "PP20", 4 Bytes Effizienztabelle (Bitbreiten
;            der Abstaende fuer die Laengen 2, 3, 4, >=5), Bitstrom, am Ende
;            3 Bytes entpackte Laenge (big endian) und 1 Byte "zu
;            ueberspringende Bits" fuer das erste Langwort.
;            Der Bitstrom besteht aus zwei Arten von Bloecken:
;              Literale:  Anzahl (2-Bit-Gruppen, 3 = weiter) und dann je 8 Bit
;              Kopie:     2 Bit Laengenindex, Abstand mit Bitbreite aus der
;                         Effizienztabelle, Bytes aus dem schon Entpackten
;            Nach einem Literalblock folgt immer eine Kopie (ohne Merkerbit).
; Aufruf:    far, C-Konvention.
;            void decomp_pp20(uint8_t *packed, uint8_t *dst,
;                             uint16_t eff_off, uint16_t eff_seg,
;                             uint32_t packed_len)
;            eff_off:eff_seg zeigt auf die Effizienztabelle (packed + 4).
; Register:  DS, SI, DI gesichert. Waehrend der Arbeit:
;            DS:SI = Lesezeiger in die gepackten Daten (von hinten)
;            ES:DI = Schreibzeiger ins Ziel (von hinten)
;            DX    = Bitpuffer: wird nach rechts geschoben, ein gesetztes
;                    Waechterbit zeigt an, wann 16 neue Bits zu laden sind
; Attics Schreibweise: Das Original wurde mit TASM und JUMPS uebersetzt.
;            Vorwaerts-Spruenge bekommen dabei Platz fuer einen langen
;            Sprung; reicht ein kurzer, fuellt TASM mit drei NOPs auf. Zu
;            weite bedingte Spruenge setzt TASM selbst in Gegensprung plus
;            jmp um. Bright-Eyes hatte die NOPs und Umwege von Hand
;            ausgeschrieben (NOP3); hier stehen wieder die einfachen Befehle.
;===========================================================================
.186
.model large
JUMPS
.code
	public _decomp_pp20

	assume cs:@code

; Segmentuebertrag: Ist beim Herunterzaehlen (sub ... ) ein Uebertrag
; entstanden, lag der Zeiger im vorigen 64-KB-Block: Segment - 1000h.
ADJUST_SEG	MACRO	SEG
		LOCAL	ok
		jnb	ok
		push	ax
		mov	ax, SEG
		sub	ax, 1000h
		mov	SEG, ax
		pop	ax
ok:
		ENDM

; Naechstes Bit des Bitstroms ins Carry holen. Ist der Puffer leer (nur noch
; das Waechterbit war drin), die naechsten 16 Bit von DS:SI-2 lesen (die
; Bytes stehen in Motorola-Reihenfolge), Waechterbit oben einschieben.
NEXTBIT		MACRO
		LOCAL	hat
		shr	dx, 1
		jnz	hat
		sub	si, 2
		ADJUST_SEG ds
		mov	dx, [si]
		xchg	dh, dl
		stc
		rcr	dx, 1
hat:
		ENDM

_decomp_pp20	proc far

packed_off	= word ptr  6		; gepackte Datei (Anfang)
packed_seg	= word ptr  8
dst_off		= word ptr  0Ah		; Ziel (Anfang)
dst_seg		= word ptr  0Ch
eff_table	= dword	ptr 0Eh		; Effizienztabelle (4 Bytes)
len_lo		= word ptr  12h		; Laenge der gepackten Datei
len_hi		= word ptr  14h

		enter	0, 0
		push	ds
		push	si
		push	di

		; DS:SI = Ende der gepackten Daten (packed + packed_len)
		mov	ax, [bp+packed_off]
		mov	dx, [bp+packed_seg]
		add	ax, [bp+len_lo]
		jnb	@@kein_uebertrag1
		add	dx, 1000h
@@kein_uebertrag1:
		mov	bx, [bp+len_hi]
		shl	bx, 0Ch
		add	dx, bx
		mov	si, ax
		mov	ds, dx
		xor	dx, dx

		; letztes Langwort: [si] [si+1] [si+2] = entpackte Laenge (24 Bit,
		; big endian), [si+3] = Bits, die im ersten Wort zu ueberspringen sind
		sub	si, 4
		ADJUST_SEG ds

		mov	al, [si+3]
		mov	cl, [si]
		mov	bh, [si+1]
		mov	bl, [si+2]
		and	al, al
		jz	@@uebersprungen
@@ueberspringen:
		NEXTBIT
		dec	al
		jnz	@@ueberspringen

@@uebersprungen:
		; ES:DI = Ende des Ziels (dst + entpackte Laenge, CL:BX)
		mov	ch, 0
		push	dx
		mov	ax, [bp+dst_off]
		mov	dx, [bp+dst_seg]
		add	ax, bx
		jnb	@@kein_uebertrag2
		add	dx, 1000h
@@kein_uebertrag2:
		shl	cx, 0Ch
		add	dx, cx
		mov	di, ax
		mov	es, dx

		; Anfang des Ziels als lineare Adresse (fuer die Endepruefung)
		mov	ax, [bp+dst_off]
		mov	cx, [bp+dst_seg]
		mov	bl, ch
		shr	bl, 4
		mov	bh, 0
		shl	cx, 4
		add	ax, cx
		mov	cx, 0
		adc	cx, bx
		mov	[bp+dst_off], ax
		mov	[bp+dst_seg], cx
		pop	dx

; --- Hauptschleife: Merkerbit 0 = erst Literale, 1 = gleich eine Kopie -----
@@block:
		NEXTBIT
		jb	@@kopie

		; Zahl der Literale: 1 + Summe von 2-Bit-Gruppen, solange 3
		mov	cx, 1
@@literal_anzahl:
		xor	ax, ax
		NEXTBIT
		rcl	al, 1
		NEXTBIT
		rcl	al, 1
		add	cx, ax
		cmp	al, 3
		jz	@@literal_anzahl

		; CX Literale zu je 8 Bit uebernehmen
@@literal:
		mov	al, 0
		NEXTBIT
		rcl	al, 1
		NEXTBIT
		rcl	al, 1
		NEXTBIT
		rcl	al, 1
		NEXTBIT
		rcl	al, 1
		NEXTBIT
		rcl	al, 1
		NEXTBIT
		rcl	al, 1
		NEXTBIT
		rcl	al, 1
		NEXTBIT
		rcl	al, 1
		sub	di, 1
		ADJUST_SEG es

		mov	es:[di], al
		loop	@@literal

		; fertig, wenn ES:DI den Anfang des Ziels erreicht hat
		mov	ax, di
		mov	cx, es
		mov	bl, ch
		shr	bl, 4
		mov	bh, 0
		shl	cx, 4
		add	ax, cx
		mov	cx, 0
		adc	cx, bx
		cmp	cx, [bp+dst_seg]
		ja	@@kopie
		jb	@@fertig
		cmp	ax, [bp+dst_off]
		ja	@@kopie
		jmp	@@fertig

; --- Kopie aus dem schon Entpackten -----------------------------------------
@@kopie:
		; 2 Bit Laengenindex 0..3 (Laenge 2..5), CL = Abstandsbreite aus der
		; Effizienztabelle
		mov	bl, 0
		NEXTBIT
		rcl	bl, 1
		NEXTBIT
		rcl	bl, 1
		mov	bh, 0
		push	es
		push	di
		les	di, [bp+eff_table]
		mov	cl, es:[bx+di]
		pop	di
		pop	es
		cmp	bl, 3
		jnz	@@kurze_kopie

		; Laengenindex 3: ein Bit waehlt die Abstandsbreite, 0 = 7 Bit
		NEXTBIT
		jb	@@breite_ok
		mov	cl, 7
@@breite_ok:
		xor	ax, ax
		mov	ch, 0
@@abstand_lang:
		NEXTBIT
		rcl	ax, 1
		loop	@@abstand_lang
		push	ax

		; Laenge verlaengern: 3-Bit-Gruppen addieren, solange 7
@@laenge_lang:
		xor	cx, cx
		NEXTBIT
		rcl	cl, 1
		NEXTBIT
		rcl	cl, 1
		NEXTBIT
		rcl	cl, 1
		add	bx, cx
		cmp	cl, 7
		jz	@@laenge_lang
		jmp	@@kopieren

@@kurze_kopie:
		; Laengenindex 0..2: Abstand mit CL Bit
		mov	ch, 0
		xor	ax, ax
@@abstand:
		NEXTBIT
		rcl	ax, 1
		loop	@@abstand
		push	ax

@@kopieren:
		; Laenge = Index + 2, Quelle = ES:DI + Abstand (vorwaerts, denn die
		; Ausgabe waechst nach hinten)
		add	bx, 2
		mov	cx, bx
		pop	bx
@@kopiere_byte:
		mov	al, es:[bx+di]
		sub	di, 1
		ADJUST_SEG es
		mov	es:[di], al
		loop	@@kopiere_byte

		; weiter, solange ES:DI den Anfang des Ziels nicht erreicht hat
		mov	ax, di
		mov	cx, es
		mov	bl, ch
		shr	bl, 4
		mov	bh, 0
		shl	cx, 4
		add	ax, cx
		mov	cx, 0
		adc	cx, bx
		cmp	cx, [bp+dst_seg]
		ja	@@block
		jb	@@fertig
		cmp	ax, [bp+dst_off]
		ja	@@block

@@fertig:
		pop	di
		pop	si
		pop	ds
		leave
		retf

_decomp_pp20	endp

		end
