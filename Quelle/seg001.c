/*
 * seg001 - CD-ROM: Laufwerkssuche, CD-Pruefung und CD-Audio-Musik (MSCDEX)
 *
 * Inhalt:    Findet ueber MSCDEX (INT 2Fh, AX=1500h) das erste CD-ROM-Laufwerk,
 *            prueft per Lesezugriff auf X:\DSA\SCHICKM.EXE, ob die Spiel-CD
 *            eingelegt ist, und spielt die Musik als CD-Audio-Titel ab.
 *            Die Geraetetreiber-Anfragen (Stop, Play, Resume, IOCTL) liegen
 *            fertig in req[] (seg013.asm) und gehen per INT 2Fh, AX=1510h raus.
 *            Ein Titel wird beim Abfragen der Tastatur (CD_bioskey) wiederholt,
 *            sobald seine Spieldauer (in Timer-Ticks) abgelaufen ist.
 * Aufrufer:  main() (CD_init), set_audio_track()/sound_menu() in seg002,
 *            Tastaturabfragen in seg002/seg005/seg036 (CD_bioskey, CD_audio_repeat_check).
 * Daten:     g_cd_audio_tracklist (Musikdatei -> CD-Titel), cd_buf1 in seg013
 *            (Inhaltsverzeichnis der CD), g_cd_skipmagic (Kommandozeilen-Trick,
 *            der die CD-Pruefung abschaltet, siehe main()).
 * Besonders: Es wird nur das erste MSCDEX-Laufwerk benutzt. CD_read_qchannel_unused()
 *            wird nie aufgerufen. Kleiner Fehler beim Laenge-Uebertrag in
 *            CD_audio_track_start() (siehe dort).
 *            Ein Teil ist in Borland-Inline-Assembler geschrieben, damit die
 *            Opcodes dem Original gleichen.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Y seg001.c
 */

#include <stdio.h>
#include <string.h>

#include "v302de.h"

#include "seg001.h"
#include "seg097.h"
#include "seg120.h"

#if defined(__BORLANDC__)
#include <DOS.H>
#include "seg013.h"
#endif

char cd_ioctl_reset_code = 2; /* Steuercode 2 = "Reset Drive" fuer IOCTL-Ausgabe req[8] (seg013.asm); req[8] wird nie benutzt */
signed int g_cd_init_successful = 0; /* {0,1} */
static unsigned char cd_dummy1[4] = {0};
static signed int g_cd_audio_repeat = 0; /* {0,1} */
static unsigned char cd_dummy2[4] = {0};
static signed int g_cd_audio_paused = 0; /* {0,1} */
static signed int g_cd_audio_track_bak = 0x7fff;
static const int g_cd_audio_tracklist[15] = {
	0x7fff,	0x7fff,	0x7fff, 0x7fff,
	ARCHIVE_FILE_SUMMARY_XMI,
	ARCHIVE_FILE_THORWAL_XMI,
	ARCHIVE_FILE_TEMPLE_XMI,
	ARCHIVE_FILE_TERMS_XMI,
	ARCHIVE_FILE_SMITH_XMI,
	ARCHIVE_FILE_INN_XMI,
	ARCHIVE_FILE_HEALER_XMI,
	ARCHIVE_FILE_CAMP_XMI,
	ARCHIVE_FILE_VICTORY_XMI,
	ARCHIVE_FILE_COMBAT_XMI,
	ARCHIVE_FILE_DUNGEON_XMI
};
uint32_t g_cd_skipmagic = 0L;

static signed int g_cd_check_err_countdown = 5;
static const char g_str_insert_cd[142] = "BITTE LEGEN SIE DIE \xf2SCHICKSALSKLINGE-CD\xf0 IN LAUFWERK %c: EIN. DIESE WIRD BEN\x99TIGT, DA DATEN W\x8eHREND DES SPIELS VON CD GELADEN WERDEN M\x9aSSEN.";
static const char g_str_repeat[12] = "WIEDERHOLEN";
static const char g_str_quit[8] = "BEENDEN";
static const char g_str_cd_exepath[19] = "X:\\DSA\\SCHICKM.EXE";
static char g_str_cd_missing[204] = "DIESES PROGRAMM IST OHNE CD-ROM-LAUFWERK NICHT LAUFF\x8eHIG. BITTE INSTALLIEREN SIE DIE GER\x8eTETREIBER F\x9aR DAS CD-ROM-LAUFWERK GEM\x8eSS IHRER CD-ROM-DOKUMENTATION UND STARTEN SIE DIE 'SCHICKSALSKLINGE' ERNEUT.";
static const char g_str_cd_init[37] = "INITIALISIERE CD-ROM IN LAUFWERK %c:";
static unsigned char g_unkn_004 = 0x00;

uint32_t g_cd_audio_duration_bak; // ds:0xbc38
uint32_t g_cd_audio_pause_tod; // ds:0xbc3c
signed int g_cd_audio_track; // ds:0xbc40
unsigned char g_unkn_071[8]; // ds:0xbc42
uint32_t g_cd_audio_start_tod; // ds:0xbc4a
uint32_t g_cd_audio_duration; // ds:0xbc4e
signed int g_cd_drive_num; // ds:0xbc52
unsigned char g_unkn_072[4]; // ds:0xbc54

/* static prototypes */
static void CD_audio_stop_hsg(void);

#if defined(__BORLANDC__)
/**
 * \brief   check if cdrom-drives are installed
 *
 *          MSCDEX-Installationspruefung (INT 2Fh, AX=1500h): BX = Anzahl der CD-Laufwerke.
 *
 * \return  {0 = no drive, 1 = at least one drive}
 */
/* Borlandified and identical */
static int16_t short CD_has_drives(void)
{
	/* al ==  0: return number of drive letters */
	asm {
		mov ax, 0x1500
		xor bx, bx
		int 0x2f
		xor ax, ax
		or  bx, bx
		jz no_drive
		inc ax
	}
no_drive: /* Sprungziel bei BX == 0, also wenn KEIN Laufwerk da ist (hiess frueher has_cd) */

	return _AX;
}

/* Borlandified and identical */
/* liefert BX von INT 2Fh, AX=1500h: Anzahl der CD-Laufwerke */
static int16_t CD_count_drives(void)
{
	asm {
		mov ax, 0x1500
		xor bx, bx
		int 0x2f
	}

	return _BX;
}

/* Borlandified and identical */
/* liefert CX von INT 2Fh, AX=1500h: Laufwerksnummer des ersten CD-Laufwerks (0 = A:) */
static int16_t CD_get_first_drive(void)
{
	asm {
		mov ax, 0x1500
		xor bx, bx
		int 0x2f
	}

	return _CX;
}

/* Borlandified and identical */
static int16_t CD_set_drive_num(void)
{
	if (!CD_has_drives()) return 0;

	if (!CD_count_drives()) return 0;

	g_cd_drive_num = CD_get_first_drive();

	return 1;
}

/* Borlandified and identical */
/* schickt eine Geraetetreiber-Anfrage an das CD-Laufwerk (MSCDEX, INT 2Fh, AX=1510h) */
static void CD_driver_request(struct driver_request far* req)
{
	asm {
		mov ax, 0x1510
		mov cx, [g_cd_drive_num]
		les bx, req
		int 0x2f
	}
}

/* Borlandified and identical */
/* TODO: check adresses of seg013 */
/* liest die "Audio Q-Channel Info" (IOCTL-Eingabe, Steuercode 0Ch, 11 Bytes ueber req[3])
 * nach cd_buf1, also die aktuelle Abspielposition. Wird nie aufgerufen.
 * (hiess CD_unused1) */
static void CD_read_qchannel_unused(void)
{
	if (!g_cd_init_successful) return;

	req[3].status = 0;
	req[3].ptr = cd_buf1;
	cd_buf1[0] = 0x0c;
	CD_driver_request(&req[3]);
}

/**
 * \brief   get time of day
 *
 * \return              clock ticks since midnight, the system time.
 *
 * \todo    produces a compiler warning and is a bit hacky
 */
/* Borlandified and identical */
static int32_t CD_get_tod(void)
{
	asm {
		mov ah, 0x0
		int 0x1a
		mov ax, dx
		mov dx, cx
		jmp near leave_tod
	}
leave_tod:
}

/* Borlandified and nearly identical */
/**
 * \brief   startet einen CD-Audio-Titel (PLAY AUDIO, req[5], Adressierung Red Book)
 *
 *          Die Titeldaten stammen aus CD_read_toc(): je Titel 8 Bytes ab cd_buf1[0x0c + 8 * n],
 *          Startadresse als Frame/Sekunde/Minute in den Bytes 2..4.
 *          Merkt sich Startzeit und Spieldauer in Timer-Ticks, damit CD_audio_repeat_check()
 *          den Titel nach Ablauf neu starten kann.
 *
 * \param   track_num   Nummer des CD-Titels
 */
static void CD_audio_track_start(signed int track_num)
{
	int32_t track_start;
	int32_t track_end;

	if (g_cd_init_successful) {

		req[5].status = 0;

		/* Startadresse im Red-Book-Format (Minute << 16 | Sekunde << 8 | Frame) */
		req[5].ptr = MK_FP(cd_buf1[0x10 + track_num * 8],
				(cd_buf1[0x0f + track_num * 8] << 8) +
				(cd_buf1[0x0e + track_num * 8]));

		/* calculate track_start */
		track_start = (60L * (uint32_t)cd_buf1[0x10 + track_num * 8]
			+ cd_buf1[0x0f + track_num * 8]) * 75L
			+ cd_buf1[0x0e + track_num * 8];

		/* calculate track_end */
		if (cd_buf1[0x326] == track_num) {
			/* letzter Titel: Ende = Beginn der Auslaufrille (lead-out) aus der Audio-Disk-Info */

			track_end = (((uint32_t)cd_buf1[0x329] * 60
					+ cd_buf1[0x328]) * 75
					+ cd_buf1[0x327]);
		} else {
			track_end = (((uint32_t)cd_buf1[0x18 + track_num * 8] * 60
					+ cd_buf1[0x17 + track_num * 8]) * 75
					+ cd_buf1[0x16 + track_num * 8]);

		}

		/* ab hier enthaelt track_start die Laenge des Titels in Frames (1/75 s) */
		track_start = track_end - track_start;

		/* Anzahl der abzuspielenden Sektoren = Laenge - 150 Frames (2 s, vermutlich die Pause vor dem naechsten Titel).
		 * Original-Bug (harmlos): der Uebertrag der Subtraktion geht nicht ins High-Word (count_hi) ueber.
		 * Spielt nur eine Rolle, wenn das Low-Word der Laenge < 150 ist. */
#if FIX_0275
		/* Fix 275: 32 Bit rechnen, der Uebertrag geht ins High-Word */
		track_start -= 150;
		req[5].count_lo = (uint16_t)track_start;
		req[5].count_hi = (int32_t)(track_start >> 16);

		CD_driver_request(&req[5]);

		g_cd_audio_duration = ((uint32_t)track_start * 0x1234e) / 0x4b000;
#else
		req[5].count_lo = ((uint16_t)track_start) - 150;
		req[5].count_hi = (int32_t)(track_start >> 16);

		CD_driver_request(&req[5]);

		/* Spieldauer in Timer-Ticks: Frames * 18.2 / 75 (0x1234e / 0x4b000 = 0.24275) */
		g_cd_audio_duration = (((uint32_t)track_start - 150) * 0x1234e) / 0x4b000;
#endif
		g_cd_audio_start_tod = CD_get_tod();
	}
}
#endif

/* Borlandified and identical */
/**
 * \brief   startet den laufenden CD-Titel neu, wenn seine Spieldauer abgelaufen ist
 *
 *          Wird bei jeder Tastaturabfrage aufgerufen. Der Name ist irrefuehrend:
 *          die Funktion schaltet keine Wiederholung ein, sie fuehrt sie aus.
 */
void CD_audio_repeat_check(void)
{
#if defined(__BORLANDC__)
	if (!g_cd_init_successful) return;

	if (CD_get_tod() - (int32_t)g_cd_audio_start_tod < (int32_t)g_cd_audio_duration)
		return;

	if (g_cd_audio_repeat == 1)
	{
		CD_audio_stop_hsg();
		CD_audio_stop_hsg();
		CD_audio_track_start(g_cd_audio_track);
		g_cd_audio_repeat = 1;
	}
#endif
}

/* Borlandified and identical */
signed int CD_bioskey(const signed int cmd)
{
#if defined(__BORLANDC__)
	CD_audio_repeat_check();

	return bioskey(cmd);
	/*
	 * bioskey invokes the BIOS keyboard service INT 16h, with function AH value given by 'cmd'
	 * see https://en.wikipedia.org/wiki/INT_16H
	 * cmd == 0 -> wait until a key is pressed,
	 *             return BIOS scan code (hardware key identifier) as low byte and ASCII code as high byte
	 * cmd == 1 -> get the state of the keyboard buffer
	 *             return 0 if no key is waiting, otherwise return keycode as for cmd == 0
	 */
#else
	return 0;
#endif
}

#if defined(__BORLANDC__)
/* CD_audio_stop_hsg() - stop audio playback in HSG format */
/* STOP AUDIO ueber req[0]. Das "HSG" bezieht sich nur auf das Adressierungs-Byte der Anfrage,
 * das STOP gar nicht auswertet. Laut MSCDEX haelt ein erstes STOP an (Pause),
 * ein zweites STOP setzt die Wiedergabe ganz zurueck; deshalb wird es oft doppelt aufgerufen. */
/* Borlandified and identical */
static void CD_audio_stop_hsg(void)
{
	if (!g_cd_init_successful) return;

	req[0].status = 0;
	CD_driver_request(&req[0]);

	g_cd_audio_repeat = 0;
}

/**
 *\brief	stop audio playback in HSG and redbook format
 *
 *	Schickt zweimal STOP AUDIO (req[0], req[1]): das erste haelt an, das zweite setzt zurueck.
 *	Der Unterschied HSG/Red Book ist bei STOP bedeutungslos.
 */
/* Borlandified and identical */
void CD_audio_stop(void)
{
	if (!g_cd_init_successful) return;

	CD_audio_stop_hsg();
	req[1].status = 0;
	CD_driver_request(&req[1]);
}
#endif

/* Borlandified and identical */
/* haelt die CD-Musik an (STOP AUDIO ueber req[6]) */
void CD_audio_pause(void)
{
#if defined(__BORLANDC__)
	/* Is CD initialized ? */
	if (!g_cd_init_successful) return;

	/* Is CD already paused ? */
	if (g_cd_audio_paused) return;

	/* set CD pause */
	g_cd_audio_paused = 1;
	g_cd_audio_pause_tod = CD_get_tod();
	/* save current position */
	/* (g_cd_audio_duration ist die Spieldauer des Titels in Ticks, keine Position) */
	g_cd_audio_duration_bak = g_cd_audio_duration;
	/* set current position to maximum signed int */
	/* Spieldauer auf Maximum, damit CD_audio_repeat_check() waehrend der Pause nicht neu startet */
	g_cd_audio_duration = 0x7fffffff;

	req[6].status = 0;
	CD_driver_request(&req[6]);
#endif
}

/* Borlandified and identical */
/* setzt die angehaltene CD-Musik fort (RESUME AUDIO ueber req[7]); startet nichts Neues */
void CD_audio_resume(void)
{
#if defined(__BORLANDC__)
	/* Is CD initialized ? */
	if (!g_cd_init_successful) return;

	/* Is CD paused ? */
	if (!g_cd_audio_paused)	return;

	CD_check();

	/* reset CD pause */
	g_cd_audio_paused = 0;
	g_cd_audio_duration = g_cd_audio_duration_bak;
	/* Startzeit um die Pausendauer verschieben */
	g_cd_audio_start_tod += (CD_get_tod() - g_cd_audio_pause_tod);

	req[7].status = 0;
	CD_driver_request(&req[7]);
#endif
}

#if defined(__BORLANDC__)
/* Borlandified and identical */
/* liest das Inhaltsverzeichnis der CD nach cd_buf1 (hiess CD_0432):
 * Audio-Disk-Info (IOCTL 0Ah) nach cd_buf1[0x324..]: erster Titel [0x325], letzter Titel [0x326],
 * Beginn der Auslaufrille [0x327..0x329] (Frame, Sekunde, Minute);
 * danach fuer jeden Titel die Audio-Track-Info (IOCTL 0Bh) nach cd_buf1[0x0c + 8 * Titel]. */
static void CD_read_toc(void)
{
	signed int track_num;

	if (!g_cd_init_successful) return;

	req[2].status = 0;
	req[2].ptr = &cd_buf1[0x324];
	cd_buf1[0x324] = 0x0a;
	CD_driver_request(&req[2]);

	track_num = (uint16_t)cd_buf1[0x325];

	while (cd_buf1[0x326] >= track_num)
	{
		req[2].status = 0;
		req[2].ptr = &cd_buf1[0x0c + 8 * track_num];
		cd_buf1[0x0c + 8 * track_num] = 0x0b;
		cd_buf1[0x0d + 8 * track_num] = track_num;
		CD_driver_request(&req[2]);

		track_num++;
	}
}
#endif

struct dummy15 {
	int16_t a[15];
};

/* Borlandified and identical */
/**
 * \brief   spielt die Musik zu einer Musikdatei als CD-Titel
 *
 * \param   index       Index der Musikdatei in SCHICK.DAT (ARCHIVE_FILE_*_XMI);
 *                      CD-Titel = Position in g_cd_audio_tracklist + 1 (Titel 1 ist die Datenspur)
 */
void CD_set_track(const signed int index)
{
#if defined(__BORLANDC__)
	signed int i;
	struct dummy15 tracks = *(struct dummy15*)g_cd_audio_tracklist;

	for (i = 0; i < 15; i++)
	{
		if (tracks.a[i] == index) break;
	}

	g_cd_audio_track = i + 1;

	if (g_cd_audio_track_bak == g_cd_audio_track)
	{
	} else {
		g_cd_audio_track_bak = g_cd_audio_track;
		CD_audio_stop_hsg();
		CD_audio_stop_hsg();

		CD_audio_track_start(g_cd_audio_track);

		g_cd_audio_repeat = 1;

		if (g_cd_audio_paused)
		{
			g_cd_audio_paused = 0;
			CD_audio_pause();
		}
	}
#endif
}

#if defined(__BORLANDC__)
/* Borlandified and identical */
/**
 * \brief   prueft, ob die Spiel-CD lesbar ist: liest je ein Byte am Anfang und an Position 2000 der Datei
 *
 * \return  1 = gelesen (oder Pruefung per g_cd_skipmagic abgeschaltet), 0 = Datei zu kurz, -1 = Fehler
 */
static int16_t CD_read_exe(char *path)
{
	signed int handle;
	signed int buffer;
	uint16_t nread;

	/* skip read check */
	if (g_cd_skipmagic == 0x682772e4L) return 1;

	if (_dos_open(path, 1, (int*)&handle)) return -1;

	if (_dos_read(handle, &buffer, 1, (unsigned int*)&nread)) return -1;

	lseek(handle, 2000L, 0);

	if (_dos_read(handle, &buffer, 1, (unsigned int*)&nread)) return -1;

	_dos_close(handle);

	return nread;
}

/* Borlandified and identical */
static void CD_insert_msg(void)
{
	signed int answer;
	char str[160];

	sprintf(str, g_str_insert_cd, g_cd_drive_num + 'A');

	/* TODO: do {} while; would be better */
	answer = -2;
	while (answer == -2)
	{
		answer = GUI_radio(str, 2, g_str_repeat, g_str_quit);
	}

	if (answer == 2)
	{
		cleanup_game();
		exit(0);
	}
}

/* Borlandified and identical */
/* DOS-Handler fuer kritische Fehler (harderr): bei jedem 5. Fehler die Aufforderung, die CD einzulegen */
static signed int CD_harderr_handler(void)
{
	if (g_cd_check_err_countdown == 0)
	{
		CD_insert_msg();
		g_cd_check_err_countdown = 5;
	}

	g_cd_check_err_countdown--;

#if defined(__BORLANDC__)
	hardresume(1);
#endif

	return 1;
}
#endif

/* Borlandified and identical */
void CD_check(void)
{
#if defined(__BORLANDC__)
	char text[80];

	harderr((int(*)(int, int, int, int))CD_harderr_handler);

	strcpy(text, g_str_cd_exepath);

	text[0] = g_cd_drive_num + 'A';

	while (CD_read_exe(text) <= 0)
	{
		CD_insert_msg();
	}
#endif
}

/* Borlandified and identical */
signed int CD_init(void)
{
#if defined(__BORLANDC__)
	char str[80];

	if (!CD_set_drive_num())
	{
		GUI_output((char*)g_str_cd_missing);

		return 0;
	}

	sprintf(str, g_str_cd_init, g_cd_drive_num + 'A');

	GUI_output(str);

	g_cd_init_successful = 1;

	CD_audio_stop();

	CD_check();

	CD_read_toc();

	return 1;
#else
	return 1;
#endif
}
