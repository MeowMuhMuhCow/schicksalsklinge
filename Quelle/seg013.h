#if defined (__BORLANDC__)
/* Anfrageblock (Request Header) fuer den CD-Treiber, int 2Fh AX=1510h,
 * 28 Bytes, Inhalt siehe seg013.asm (GEN_REQ) */
struct driver_request {
	char dummy1;		/* +00h Laenge des Blocks */
	char subunit;		/* +01h Untereinheit */
	char function;		/* +02h Kommando: 03h IOCTL INPUT, 0Ch IOCTL OUTPUT,
				   83h SEEK, 84h PLAY, 85h STOP, 88h RESUME */
	unsigned short status;	/* +03h Status, vom Treiber gesetzt */
	char dummy2[8];		/* +05h reserviert */
	char redbook;		/* +0Dh Adressierung 0 = HSG, 1 = Red Book (bei IOCTL: Medium) */
	void *ptr;		/* +0Eh IOCTL/SEEK: Puffer, PLAY: Startadresse */
	unsigned short count_lo;		/* +12h IOCTL: Anzahl Bytes (0,1,7,b), PLAY: Sektoren (lo) */
	unsigned short count_hi;		/* +14h PLAY: Sektoren (hi), SEEK: Startsektor (0 or 2) */
	char dummy7[6];		/* +16h */
};

#if defined(__cplusplus)
extern "C" {
#endif
extern struct driver_request far req[9];
extern unsigned char far cd_buf1[804];
extern unsigned char far cd_buf2[20];	/* liegt direkt hinter cd_buf1; seg001 benutzt ihn als cd_buf1[0x324..] */
#if defined(__cplusplus)
}
#endif

#endif
