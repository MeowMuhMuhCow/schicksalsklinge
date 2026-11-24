/*
 * seg026 - Texte laden, Spielstaende laden und speichern, Helden-Dateien (CHR) in TEMP
 *
 * Inhalt:    Laedt Textdateien (TEXT.LTX, ITEMNAME, MONNAMES, LTX/TLK) und zerlegt sie in Zeigerlisten.
 *            load_game_state() und save_game_state() lesen bzw. schreiben einen Spielstand (*.gam):
 *            Kopf mit Version, den Spielstatus-Block gs_datseg_status_start..end, eine Tabelle der
 *            geaenderten Dateien aus TEMP samt Inhalt und am Ende alle CHR-Dateien. Ausserhalb eines
 *            Tempels kostet Speichern jeden Helden 1 AP. Dazu CHR-Dateien in TEMP lesen/schreiben,
 *            Heldennamen fuer Tempel/Loeschen sammeln, Kopfbilder und Tempelsymbole laden.
 * Aufrufer:  main() und game_loop() (seg002), Optionsmenue (seg025), Tempel, Charaktererschaffung.
 * Daten:     g_fnames_v302de[308] (Dateinamen in SCHICK.DAT), Suffixe/Platzhalter fuer *.CHR/*.gam,
 *            g_savegame_names[5][9] (Inhalt von GAMES.NAM).
 * Besonders: Der 4-Byte-Verweis auf die CHR-Daten im Spielstand wird beim Laden nicht benutzt.
 *            save_game_state(): haengt die CHR-Dateien mit do-while an, auch wenn findfirst nichts fand.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg026.c
 */
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>

#if defined(__BORLANDC__)
#include <DIR.H>
#include <DOS.H>
#include <IO.H>
#else
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#if defined(_WIN32)
#include <io.h>
#else
#include <strings.h>
#define _stricmp strcasecmp
#endif
#endif

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg026.h"
#include "seg028.h"
#include "seg097.h"
#include "seg106.h"

extern const char g_str_temp_fmt[];
const char *g_str_temp_fmt_ptr = &g_str_temp_fmt[0]; // ds:0x4c88

const char* g_fnames_v302de[] = {
	g_fn_v302de_000, g_fn_v302de_001, g_fn_v302de_002, g_fn_v302de_003,
	g_fn_v302de_004, g_fn_v302de_005, g_fn_v302de_006, g_fn_v302de_007,
	g_fn_v302de_008, g_fn_v302de_009, g_fn_v302de_010, g_fn_v302de_011,
	g_fn_v302de_012, g_fn_v302de_013, g_fn_v302de_014, g_fn_v302de_015,
	g_fn_v302de_016, g_fn_v302de_017, g_fn_v302de_018, g_fn_v302de_019,
	g_fn_v302de_020, g_fn_v302de_021, g_fn_v302de_022, g_fn_v302de_023,
	g_fn_v302de_024, g_fn_v302de_025, g_fn_v302de_026, g_fn_v302de_027,
	g_fn_v302de_028, g_fn_v302de_029, g_fn_v302de_030, g_fn_v302de_031,
	g_fn_v302de_032, g_fn_v302de_033, g_fn_v302de_034, g_fn_v302de_035,
	g_fn_v302de_036, g_fn_v302de_037, g_fn_v302de_038, g_fn_v302de_039,
	g_fn_v302de_040, g_fn_v302de_041, g_fn_v302de_042, g_fn_v302de_043,
	g_fn_v302de_044, g_fn_v302de_045, g_fn_v302de_046, g_fn_v302de_047,
	g_fn_v302de_048, g_fn_v302de_049, g_fn_v302de_050, g_fn_v302de_051,
	g_fn_v302de_052, g_fn_v302de_053, g_fn_v302de_054, g_fn_v302de_055,
	g_fn_v302de_056, g_fn_v302de_057, g_fn_v302de_058, g_fn_v302de_059,
	g_fn_v302de_060, g_fn_v302de_061, g_fn_v302de_062, g_fn_v302de_063,
	g_fn_v302de_064, g_fn_v302de_065, g_fn_v302de_066, g_fn_v302de_067,
	g_fn_v302de_068, g_fn_v302de_069, g_fn_v302de_070, g_fn_v302de_071,
	g_fn_v302de_072, g_fn_v302de_073, g_fn_v302de_074, g_fn_v302de_075,
	g_fn_v302de_076, g_fn_v302de_077, g_fn_v302de_078, g_fn_v302de_079,
	g_fn_v302de_080, g_fn_v302de_081, g_fn_v302de_082, g_fn_v302de_083,
	g_fn_v302de_084, g_fn_v302de_085, g_fn_v302de_086, g_fn_v302de_087,
	g_fn_v302de_088, g_fn_v302de_089, g_fn_v302de_090, g_fn_v302de_091,
	g_fn_v302de_092, g_fn_v302de_093, g_fn_v302de_094, g_fn_v302de_095,
	g_fn_v302de_096, g_fn_v302de_097, g_fn_v302de_098, g_fn_v302de_099,
	g_fn_v302de_100, g_fn_v302de_101, g_fn_v302de_102, g_fn_v302de_103,
	g_fn_v302de_104, g_fn_v302de_105, g_fn_v302de_106, g_fn_v302de_107,
	g_fn_v302de_108, g_fn_v302de_109, g_fn_v302de_110, g_fn_v302de_111,
	g_fn_v302de_112, g_fn_v302de_113, g_fn_v302de_114, g_fn_v302de_115,
	g_fn_v302de_116, g_fn_v302de_117, g_fn_v302de_118, g_fn_v302de_119,
	g_fn_v302de_120, g_fn_v302de_121, g_fn_v302de_122, g_fn_v302de_123,
	g_fn_v302de_124, g_fn_v302de_125, g_fn_v302de_126, g_fn_v302de_127,
	g_fn_v302de_128, g_fn_v302de_129, g_fn_v302de_130, g_fn_v302de_131,
	g_fn_v302de_132, g_fn_v302de_133, g_fn_v302de_134, g_fn_v302de_135,
	g_fn_v302de_136, g_fn_v302de_137, g_fn_v302de_138, g_fn_v302de_139,
	g_fn_v302de_140, g_fn_v302de_141, g_fn_v302de_142, g_fn_v302de_143,
	g_fn_v302de_144, g_fn_v302de_145, g_fn_v302de_146, g_fn_v302de_147,
	g_fn_v302de_148, g_fn_v302de_149, g_fn_v302de_150, g_fn_v302de_151,
	g_fn_v302de_152, g_fn_v302de_153, g_fn_v302de_154, g_fn_v302de_155,
	g_fn_v302de_156, g_fn_v302de_157, g_fn_v302de_158, g_fn_v302de_159,
	g_fn_v302de_160, g_fn_v302de_161, g_fn_v302de_162, g_fn_v302de_163,
	g_fn_v302de_164, g_fn_v302de_165, g_fn_v302de_166, g_fn_v302de_167,
	g_fn_v302de_168, g_fn_v302de_169, g_fn_v302de_170, g_fn_v302de_171,
	g_fn_v302de_172, g_fn_v302de_173, g_fn_v302de_174, g_fn_v302de_175,
	g_fn_v302de_176, g_fn_v302de_177, g_fn_v302de_178, g_fn_v302de_179,
	g_fn_v302de_180, g_fn_v302de_181, g_fn_v302de_182, g_fn_v302de_183,
	g_fn_v302de_184, g_fn_v302de_185, g_fn_v302de_186, g_fn_v302de_187,
	g_fn_v302de_188, g_fn_v302de_189, g_fn_v302de_190, g_fn_v302de_191,
	g_fn_v302de_192, g_fn_v302de_193, g_fn_v302de_194, g_fn_v302de_195,
	g_fn_v302de_196, g_fn_v302de_197, g_fn_v302de_198, g_fn_v302de_199,
	g_fn_v302de_200, g_fn_v302de_201, g_fn_v302de_202, g_fn_v302de_203,
	g_fn_v302de_204, g_fn_v302de_205, g_fn_v302de_206, g_fn_v302de_207,
	g_fn_v302de_208, g_fn_v302de_209, g_fn_v302de_210, g_fn_v302de_211,
	g_fn_v302de_212, g_fn_v302de_213, g_fn_v302de_214, g_fn_v302de_215,
	g_fn_v302de_216, g_fn_v302de_217, g_fn_v302de_218, g_fn_v302de_219,
	g_fn_v302de_220, g_fn_v302de_221, g_fn_v302de_222, g_fn_v302de_223,
	g_fn_v302de_224, g_fn_v302de_225, g_fn_v302de_226, g_fn_v302de_227,
	g_fn_v302de_228, g_fn_v302de_229, g_fn_v302de_230, g_fn_v302de_231,
	g_fn_v302de_232, g_fn_v302de_233, g_fn_v302de_234, g_fn_v302de_235,
	g_fn_v302de_236, g_fn_v302de_237, g_fn_v302de_238, g_fn_v302de_239,
	g_fn_v302de_240, g_fn_v302de_241, g_fn_v302de_242, g_fn_v302de_243,
	g_fn_v302de_244, g_fn_v302de_245, g_fn_v302de_246, g_fn_v302de_247,
	g_fn_v302de_248, g_fn_v302de_249, g_fn_v302de_250, g_fn_v302de_251,
	g_fn_v302de_252, g_fn_v302de_253, g_fn_v302de_254, g_fn_v302de_255,
	g_fn_v302de_256, g_fn_v302de_257, g_fn_v302de_258, g_fn_v302de_259,
	g_fn_v302de_260, g_fn_v302de_261, g_fn_v302de_262, g_fn_v302de_263,
	g_fn_v302de_264, g_fn_v302de_265, g_fn_v302de_266, g_fn_v302de_267,
	g_fn_v302de_268, g_fn_v302de_269, g_fn_v302de_270, g_fn_v302de_271,
	g_fn_v302de_272, g_fn_v302de_273, g_fn_v302de_274, g_fn_v302de_275,
	g_fn_v302de_276, g_fn_v302de_277, g_fn_v302de_278, g_fn_v302de_279,
	g_fn_v302de_280, g_fn_v302de_281, g_fn_v302de_282, g_fn_v302de_283,
	g_fn_v302de_284, g_fn_v302de_285, g_fn_v302de_286, g_fn_v302de_287,
	g_fn_v302de_288, g_fn_v302de_289, g_fn_v302de_290, g_fn_v302de_291,
	g_fn_v302de_292, g_fn_v302de_293, g_fn_v302de_294, g_fn_v302de_295,
	g_fn_v302de_296, g_fn_v302de_297, g_fn_v302de_298, g_fn_v302de_299,
	g_fn_v302de_300, g_fn_v302de_301, g_fn_v302de_302, g_fn_v302de_303,
	g_fn_v302de_304, g_fn_v302de_305, g_fn_v302de_306, g_fn_v302de_307,
}; // ds:0x4c8c

signed int g_loaded_head_id = -1; // ds:0x515c
#if defined(__BORLANDC__) || defined(_WIN32)
const char g_str_temp_fmt[] = "TEMP\\%s"; // ds:0x515e
#else
const char g_str_temp_fmt[] = "TEMP/%s"; // ds:0x515e
#endif

// ds:0x5166, names of files in SCHICK.DAT
char g_fn_v302de_XXX[1]; // ds:0x5166
char g_fn_v302de_000[9]; // ds:0x5167
char g_fn_v302de_001[11]; // ds:0x5170
char g_fn_v302de_002[11]; // ds:0x517b
char g_fn_v302de_003[11]; // ds:0x5186
char g_fn_v302de_004[11]; // ds:0x5191
char g_fn_v302de_005[10]; // ds:0x519c
char g_fn_v302de_006[7]; // ds:0x51a6
char g_fn_v302de_007[12]; // ds:0x51ad
char g_fn_v302de_008[10]; // ds:0x51b9
char g_fn_v302de_009[10]; // ds:0x51c3
char g_fn_v302de_010[11]; // ds:0x51cd
char g_fn_v302de_011[6]; // ds:0x51d8
char g_fn_v302de_012[6]; // ds:0x51de
char g_fn_v302de_013[10]; // ds:0x51e4
char g_fn_v302de_014[9]; // ds:0x51ee
char g_fn_v302de_015[6]; // ds:0x51f7
char g_fn_v302de_016[8]; // ds:0x51fd
char g_fn_v302de_017[12]; // ds:0x5205
char g_fn_v302de_018[8]; // ds:0x5211
char g_fn_v302de_019[12]; // ds:0x5219
char g_fn_v302de_020[9]; // ds:0x5225
char g_fn_v302de_021[13]; // ds:0x522e
char g_fn_v302de_022[5]; // ds:0x523b
char g_fn_v302de_023[9]; // ds:0x5240
char g_fn_v302de_024[6]; // ds:0x5249
char g_fn_v302de_025[10]; // ds:0x524f
char g_fn_v302de_026[12]; // ds:0x5259
char g_fn_v302de_027[11]; // ds:0x5265
char g_fn_v302de_028[11]; // ds:0x5270
char g_fn_v302de_029[13]; // ds:0x527b
char g_fn_v302de_030[13]; // ds:0x5288
char g_fn_v302de_031[13]; // ds:0x5295
char g_fn_v302de_032[11]; // ds:0x52a2
char g_fn_v302de_033[13]; // ds:0x52ad
char g_fn_v302de_034[11]; // ds:0x52ba
char g_fn_v302de_035[13]; // ds:0x52c5
char g_fn_v302de_036[11]; // ds:0x52d2
char g_fn_v302de_037[11]; // ds:0x52dd
char g_fn_v302de_038[16]; // ds:0x52e8
char g_fn_v302de_039[11]; // ds:0x52f8
char g_fn_v302de_040[13]; // ds:0x5303
char g_fn_v302de_041[10]; // ds:0x5310
char g_fn_v302de_042[14]; // ds:0x531a
char g_fn_v302de_043[13]; // ds:0x5328
char g_fn_v302de_044[13]; // ds:0x5335
char g_fn_v302de_045[13]; // ds:0x5342
char g_fn_v302de_046[13]; // ds:0x534f
char g_fn_v302de_047[13]; // ds:0x535c
char g_fn_v302de_048[12]; // ds:0x5369
char g_fn_v302de_049[11]; // ds:0x5375
char g_fn_v302de_050[10]; // ds:0x5380
char g_fn_v302de_051[12]; // ds:0x538a
char g_fn_v302de_052[8]; // ds:0x5396
char g_fn_v302de_053[10]; // ds:0x539e
char g_fn_v302de_054[14]; // ds:0x53a8
char g_fn_v302de_055[10]; // ds:0x53b6
char g_fn_v302de_056[13]; // ds:0x53c0
char g_fn_v302de_057[13]; // ds:0x53cd
char g_fn_v302de_058[9]; // ds:0x53da
char g_fn_v302de_059[11]; // ds:0x53e3
char g_fn_v302de_060[9]; // ds:0x53ee
char g_fn_v302de_061[13]; // ds:0x53f7
char g_fn_v302de_062[12]; // ds:0x5404
char g_fn_v302de_063[10]; // ds:0x5410
char g_fn_v302de_064[9]; // ds:0x541a
char g_fn_v302de_065[12]; // ds:0x5423
char g_fn_v302de_066[10]; // ds:0x542f
char g_fn_v302de_067[13]; // ds:0x5439
char g_fn_v302de_068[13]; // ds:0x5446
char g_fn_v302de_069[13]; // ds:0x5453
char g_fn_v302de_070[11]; // ds:0x5460
char g_fn_v302de_071[12]; // ds:0x546b
char g_fn_v302de_072[13]; // ds:0x5477
char g_fn_v302de_073[11]; // ds:0x5484
char g_fn_v302de_074[12]; // ds:0x548f
char g_fn_v302de_075[13]; // ds:0x549b
char g_fn_v302de_076[13]; // ds:0x54a8
char g_fn_v302de_077[10]; // ds:0x54b5
char g_fn_v302de_078[12]; // ds:0x54bf
char g_fn_v302de_079[11]; // ds:0x54cb
char g_fn_v302de_080[11]; // ds:0x54d6
char g_fn_v302de_081[13]; // ds:0x54e1
char g_fn_v302de_082[13]; // ds:0x54ee
char g_fn_v302de_083[13]; // ds:0x54fb
char g_fn_v302de_084[11]; // ds:0x5508
char g_fn_v302de_085[13]; // ds:0x5513
char g_fn_v302de_086[11]; // ds:0x5520
char g_fn_v302de_087[13]; // ds:0x552b
char g_fn_v302de_088[11]; // ds:0x5538
char g_fn_v302de_089[11]; // ds:0x5543
char g_fn_v302de_090[13]; // ds:0x554e
char g_fn_v302de_091[11]; // ds:0x555b
char g_fn_v302de_092[13]; // ds:0x5566
char g_fn_v302de_093[10]; // ds:0x5573
char g_fn_v302de_094[13]; // ds:0x557d
char g_fn_v302de_095[13]; // ds:0x558a
char g_fn_v302de_096[13]; // ds:0x5597
char g_fn_v302de_097[13]; // ds:0x55a4
char g_fn_v302de_098[13]; // ds:0x55b1
char g_fn_v302de_099[13]; // ds:0x55be
char g_fn_v302de_100[12]; // ds:0x55cb
char g_fn_v302de_101[11]; // ds:0x55d7
char g_fn_v302de_102[10]; // ds:0x55e2
char g_fn_v302de_103[12]; // ds:0x55ec
char g_fn_v302de_104[8]; // ds:0x55f8
char g_fn_v302de_105[10]; // ds:0x5600
char g_fn_v302de_106[13]; // ds:0x560a
char g_fn_v302de_107[10]; // ds:0x5617
char g_fn_v302de_108[13]; // ds:0x5621
char g_fn_v302de_109[13]; // ds:0x562e
char g_fn_v302de_110[9]; // ds:0x563b
char g_fn_v302de_111[11]; // ds:0x5644
char g_fn_v302de_112[9]; // ds:0x564f
char g_fn_v302de_113[13]; // ds:0x5658
char g_fn_v302de_114[12]; // ds:0x5665
char g_fn_v302de_115[10]; // ds:0x5671
char g_fn_v302de_116[9]; // ds:0x567b
char g_fn_v302de_117[12]; // ds:0x5684
char g_fn_v302de_118[10]; // ds:0x5690
char g_fn_v302de_119[13]; // ds:0x569a
char g_fn_v302de_120[13]; // ds:0x56a7
char g_fn_v302de_121[13]; // ds:0x56b4
char g_fn_v302de_122[11]; // ds:0x56c1
char g_fn_v302de_123[12]; // ds:0x56cc
char g_fn_v302de_124[13]; // ds:0x56d8
char g_fn_v302de_125[11]; // ds:0x56e5
char g_fn_v302de_126[13]; // ds:0x56f0
char g_fn_v302de_127[13]; // ds:0x56fd
char g_fn_v302de_128[13]; // ds:0x570a
char g_fn_v302de_129[12]; // ds:0x5717
char g_fn_v302de_130[11]; // ds:0x5723
char g_fn_v302de_131[11]; // ds:0x572e
char g_fn_v302de_132[12]; // ds:0x5739
char g_fn_v302de_133[10]; // ds:0x5745
char g_fn_v302de_134[9]; // ds:0x574f
char g_fn_v302de_135[12]; // ds:0x5758
char g_fn_v302de_136[13]; // ds:0x5764
char g_fn_v302de_137[10]; // ds:0x5771
char g_fn_v302de_138[11]; // ds:0x577b
char g_fn_v302de_139[11]; // ds:0x5786
char g_fn_v302de_140[11]; // ds:0x5791
char g_fn_v302de_141[12]; // ds:0x579c
char g_fn_v302de_142[12]; // ds:0x57a8
char g_fn_v302de_143[12]; // ds:0x57b4
char g_fn_v302de_144[11]; // ds:0x57c0
char g_fn_v302de_145[10]; // ds:0x57cb
char g_fn_v302de_146[10]; // ds:0x57d5
char g_fn_v302de_147[8]; // ds:0x57df
char g_fn_v302de_148[11]; // ds:0x57e7
char g_fn_v302de_149[9]; // ds:0x57f2
char g_fn_v302de_150[12]; // ds:0x57fb
char g_fn_v302de_151[10]; // ds:0x5807
char g_fn_v302de_152[6]; // ds:0x5811
char g_fn_v302de_153[10]; // ds:0x5817
char g_fn_v302de_154[1]; // ds:0x5821
char g_fn_v302de_155[1]; // ds:0x5822
char g_fn_v302de_156[9]; // ds:0x5823
char g_fn_v302de_157[12]; // ds:0x582c
char g_fn_v302de_158[1]; // ds:0x5838
char g_fn_v302de_159[13]; // ds:0x5839
char g_fn_v302de_160[13]; // ds:0x5846
char g_fn_v302de_161[13]; // ds:0x5853
char g_fn_v302de_162[10]; // ds:0x5860
char g_fn_v302de_163[10]; // ds:0x586a
char g_fn_v302de_164[10]; // ds:0x5874
char g_fn_v302de_165[10]; // ds:0x587e
char g_fn_v302de_166[10]; // ds:0x5888
char g_fn_v302de_167[10]; // ds:0x5892
char g_fn_v302de_168[10]; // ds:0x589c
char g_fn_v302de_169[12]; // ds:0x58a6
char g_fn_v302de_170[12]; // ds:0x58b2
char g_fn_v302de_171[12]; // ds:0x58be
char g_fn_v302de_172[12]; // ds:0x58ca
char g_fn_v302de_173[9]; // ds:0x58d6
char g_fn_v302de_174[11]; // ds:0x58df
char g_fn_v302de_175[13]; // ds:0x58ea
char g_fn_v302de_176[12]; // ds:0x58f7
char g_fn_v302de_177[9]; // ds:0x5903
char g_fn_v302de_178[13]; // ds:0x590c
char g_fn_v302de_179[13]; // ds:0x5919
char g_fn_v302de_180[9]; // ds:0x5926
char g_fn_v302de_181[11]; // ds:0x592f
char g_fn_v302de_182[12]; // ds:0x593a
char g_fn_v302de_183[13]; // ds:0x5946
char g_fn_v302de_184[12]; // ds:0x5953
char g_fn_v302de_185[13]; // ds:0x595f
char g_fn_v302de_186[11]; // ds:0x596c
char g_fn_v302de_187[11]; // ds:0x5977
char g_fn_v302de_188[11]; // ds:0x5982
char g_fn_v302de_189[11]; // ds:0x598d
char g_fn_v302de_190[12]; // ds:0x5998
char g_fn_v302de_191[13]; // ds:0x59a4
char g_fn_v302de_192[13]; // ds:0x59b1
char g_fn_v302de_193[13]; // ds:0x59be
char g_fn_v302de_194[13]; // ds:0x59cb
char g_fn_v302de_195[13]; // ds:0x59d8
char g_fn_v302de_196[13]; // ds:0x59e5
char g_fn_v302de_197[13]; // ds:0x59f2
char g_fn_v302de_198[13]; // ds:0x59ff
char g_fn_v302de_199[13]; // ds:0x5a0c
char g_fn_v302de_200[13]; // ds:0x5a19
char g_fn_v302de_201[12]; // ds:0x5a26
char g_fn_v302de_202[12]; // ds:0x5a32
char g_fn_v302de_203[12]; // ds:0x5a3e
char g_fn_v302de_204[12]; // ds:0x5a4a
char g_fn_v302de_205[10]; // ds:0x5a56
char g_fn_v302de_206[11]; // ds:0x5a60
char g_fn_v302de_207[10]; // ds:0x5a6b
char g_fn_v302de_208[9]; // ds:0x5a75
char g_fn_v302de_209[9]; // ds:0x5a7e
char g_fn_v302de_210[13]; // ds:0x5a87
char g_fn_v302de_211[13]; // ds:0x5a94
char g_fn_v302de_212[12]; // ds:0x5aa1
char g_fn_v302de_213[11]; // ds:0x5aad
char g_fn_v302de_214[9]; // ds:0x5ab8
char g_fn_v302de_215[9]; // ds:0x5ac1
char g_fn_v302de_216[8]; // ds:0x5aca
char g_fn_v302de_217[12]; // ds:0x5ad2
char g_fn_v302de_218[12]; // ds:0x5ade
char g_fn_v302de_219[12]; // ds:0x5aea
char g_fn_v302de_220[10]; // ds:0x5af6
char g_fn_v302de_221[13]; // ds:0x5b00
char g_fn_v302de_222[13]; // ds:0x5b0d
char g_fn_v302de_223[11]; // ds:0x5b1a
char g_fn_v302de_224[12]; // ds:0x5b25
char g_fn_v302de_225[8]; // ds:0x5b31
char g_fn_v302de_226[12]; // ds:0x5b39
char g_fn_v302de_227[11]; // ds:0x5b45
char g_fn_v302de_228[11]; // ds:0x5b50
char g_fn_v302de_229[11]; // ds:0x5b5b
char g_fn_v302de_230[12]; // ds:0x5b66
char g_fn_v302de_231[9]; // ds:0x5b72
char g_fn_v302de_232[11]; // ds:0x5b7b
char g_fn_v302de_233[10]; // ds:0x5b86
char g_fn_v302de_234[6]; // ds:0x5b90
char g_fn_v302de_235[10]; // ds:0x5b96
char g_fn_v302de_236[13]; // ds:0x5ba0
char g_fn_v302de_237[13]; // ds:0x5bad
char g_fn_v302de_238[10]; // ds:0x5bba
char g_fn_v302de_239[10]; // ds:0x5bc4
char g_fn_v302de_240[9]; // ds:0x5bce
char g_fn_v302de_241[13]; // ds:0x5bd7
char g_fn_v302de_242[9]; // ds:0x5be4
char g_fn_v302de_243[9]; // ds:0x5bed
char g_fn_v302de_244[9]; // ds:0x5bf6
char g_fn_v302de_245[9]; // ds:0x5bff
char g_fn_v302de_246[9]; // ds:0x5c08
char g_fn_v302de_247[9]; // ds:0x5c11
char g_fn_v302de_248[9]; // ds:0x5c1a
char g_fn_v302de_249[9]; // ds:0x5c23
char g_fn_v302de_250[9]; // ds:0x5c2c
char g_fn_v302de_251[9]; // ds:0x5c35
char g_fn_v302de_252[9]; // ds:0x5c3e
char g_fn_v302de_253[9]; // ds:0x5c47
char g_fn_v302de_254[9]; // ds:0x5c50
char g_fn_v302de_255[12]; // ds:0x5c59
char g_fn_v302de_256[10]; // ds:0x5c65
char g_fn_v302de_257[9]; // ds:0x5c6f
char g_fn_v302de_258[9]; // ds:0x5c78
char g_fn_v302de_259[9]; // ds:0x5c81
char g_fn_v302de_260[9]; // ds:0x5c8a
char g_fn_v302de_261[9]; // ds:0x5c93
char g_fn_v302de_262[9]; // ds:0x5c9c
char g_fn_v302de_263[9]; // ds:0x5ca5
char g_fn_v302de_264[9]; // ds:0x5cae
char g_fn_v302de_265[9]; // ds:0x5cb7
char g_fn_v302de_266[9]; // ds:0x5cc0
char g_fn_v302de_267[9]; // ds:0x5cc9
char g_fn_v302de_268[9]; // ds:0x5cd2
char g_fn_v302de_269[9]; // ds:0x5cdb
char g_fn_v302de_270[12]; // ds:0x5ce4
char g_fn_v302de_271[10]; // ds:0x5cf0
char g_fn_v302de_272[9]; // ds:0x5cfa
char g_fn_v302de_273[9]; // ds:0x5d03
char g_fn_v302de_274[9]; // ds:0x5d0c
char g_fn_v302de_275[9]; // ds:0x5d15
char g_fn_v302de_276[9]; // ds:0x5d1e
char g_fn_v302de_277[9]; // ds:0x5d27
char g_fn_v302de_278[9]; // ds:0x5d30
char g_fn_v302de_279[9]; // ds:0x5d39
char g_fn_v302de_280[9]; // ds:0x5d42
char g_fn_v302de_281[9]; // ds:0x5d4b
char g_fn_v302de_282[9]; // ds:0x5d54
char g_fn_v302de_283[9]; // ds:0x5d5d
char g_fn_v302de_284[9]; // ds:0x5d66
char g_fn_v302de_285[12]; // ds:0x5d6f
char g_fn_v302de_286[10]; // ds:0x5d7b
char g_fn_v302de_287[12]; // ds:0x5d85
char g_fn_v302de_288[10]; // ds:0x5d91
char g_fn_v302de_289[8]; // ds:0x5d9b
char g_fn_v302de_290[8]; // ds:0x5da3
char g_fn_v302de_291[8]; // ds:0x5dab
char g_fn_v302de_292[8]; // ds:0x5db3
char g_fn_v302de_293[8]; // ds:0x5dbb
char g_fn_v302de_294[8]; // ds:0x5dc3
char g_fn_v302de_295[8]; // ds:0x5dcb
char g_fn_v302de_296[8]; // ds:0x5dd3
char g_fn_v302de_297[8]; // ds:0x5ddb
char g_fn_v302de_298[9]; // ds:0x5de3
char g_fn_v302de_299[9]; // ds:0x5dec
char g_fn_v302de_300[9]; // ds:0x5df5
char g_fn_v302de_301[9]; // ds:0x5dfe
char g_fn_v302de_302[9]; // ds:0x5e07
char g_fn_v302de_303[9]; // ds:0x5e10
char g_fn_v302de_304[9]; // ds:0x5e19
char g_fn_v302de_305[9]; // ds:0x5e22
char g_fn_v302de_306[9]; // ds:0x5e2b
char g_fn_v302de_307[10]; // ds:0x5e34

const char g_chr_file_suffix[5] = ".CHR"; // ds:0x5e3e
char g_savegame_suffix[5] = ".gam"; // ds:0x5e43
char g_all_files_wildcard[4] = "*.*"; // ds:0x5e48
char g_all_chr_wildcard[6] = "*.CHR"; // ds:0x5e4c
char g_empty_string1[1] = ""; // ds:0x5e52
char g_empty_string2[1] = ""; // ds:0x5e53
char g_savegame_suffix2[5] = ".gam"; // ds:0x5e54
char g_savegame_suffix3[5] = ".gam"; // ds:0x5e59
char g_all_chr_wildcard2[6] = "*.CHR"; // ds:0x5e5e
char g_all_chr_wildcard3[6] = "*.CHR"; // ds:0x5e64



uint32_t *g_saved_files_buf;		// ds:0xe2d2
time_t g_last_save_time;			// ds:0xe2d6
char g_savegame_names[5][9];			// ds:0xe2da

/* laedt FONT6, TEXT.LTX, ITEMNAME und MONNAMES und baut die Zeigerlisten */
void init_text(void)
{
	dsa_trace_enter(262, "seg026:init_text");
	int32_t len;
	signed int handle;

	handle = load_archive_file(ARCHIVE_FILE_FONT6);
	read_archive_file(handle, g_buf_font6, 1000);
	close(handle);

	handle = load_archive_file(ARCHIVE_FILE_TEXT_LTX);
	len = (int16_t)read_archive_file(handle, (uint8_t*)g_text_ltx_buffer, 64000);
	close(handle);

	split_textbuffer((char**)g_text_ltx_index, g_text_ltx_buffer, len);

	handle = load_archive_file(ARCHIVE_FILE_ITEMNAME);
	len = (int16_t)read_archive_file(handle, (uint8_t*)g_buffer5_ptr, 5000);
	close(handle);

	split_textbuffer((char**)g_itemsname, g_buffer5_ptr, len);

	handle = load_archive_file(ARCHIVE_FILE_MONNAMES);
	len = (int16_t)read_archive_file(handle, (uint8_t*)g_monnames_buffer, 5000);
	close(handle);

	split_textbuffer((char**)g_monnames_index, g_monnames_buffer, len);
}

/* laedt eine Textdatei in g_buffer7_ptr, Zeiger nach g_tx_index (get_tx()) */
void load_tx(const signed int index)
{
	dsa_trace_enter(263, "seg026:load_tx");
	int32_t archive_file_len;
	signed int archive_file_handle;

	if (index == -1)
		return;

	archive_file_handle = load_archive_file(index);

	archive_file_len = (int16_t)read_archive_file(archive_file_handle, (uint8_t*)g_buffer7_ptr, 64000);

	close(archive_file_handle);

	split_textbuffer((char**)g_tx_index, g_buffer7_ptr, archive_file_len);

	g_tx_file_index = index;
}

/* laedt eine Textdatei in g_buffer8_ptr, Zeiger nach g_tx2_index (get_tx2()) */
void load_tx2(const signed int index)
{
	dsa_trace_enter(264, "seg026:load_tx2");
	int32_t len;
	register signed int handle;

	if (index == -1)
		return;

	g_text_file_index = index;
	handle = load_archive_file(index);
	len = (int16_t)read_archive_file(handle, g_buffer8_ptr, 12000);
	close(handle);

	split_textbuffer((char**)g_tx2_index, (char*)g_buffer8_ptr, len);
}

/* REMARK: 1000 byte for pointer, means 250 strings on 16/32-bit machines and 125 strings on 64-bit machines.
 * WAFFINFO.LTX seems to have 19 + 40 + 67 = 126 strings, which will lead to problems */
void load_ltx(const signed int index)
{
	dsa_trace_enter(265, "seg026:load_ltx");
	int32_t len;
	signed int handle = load_archive_file(index);

	g_area_prepared = AREA_TYPE_NONE;

#if defined(__BORLANDC__)
	len = (int16_t)read_archive_file(handle, ((uint8_t*)g_buffer9_ptr3) + 1000, 64000);

	close(handle);

	split_textbuffer((char**)g_buffer9_ptr3, (char*)(g_buffer9_ptr3 + 1000L), len);
#else
	len = (int16_t)read_archive_file(handle, ((uint8_t*)g_buffer9_ptr3) + 250 * sizeof(char*), 64000);

	close(handle);

	split_textbuffer((char**)g_buffer9_ptr3, (char*)(g_buffer9_ptr3 + 250 * sizeof(char*)), len);
#endif
}

/* zerlegt len Byte nullterminierter Texte in eine Liste von Zeigern auf die einzelnen Texte */
void split_textbuffer(char **dst, char *src, uint32_t len)
{
	dsa_trace_enter(266, "seg026:split_textbuffer");
	uint32_t i = 0;

	*dst = src;
	dst++;

	for (; i != len; src++, i++) {

		/* continue if not the end of the string */
		if (!(*src)) {

			/* write the adress of the next string */
			*dst = src + 1;
			dst++;
		}
	}
}

void load_ggsts_nvf(void)
{
	dsa_trace_enter(267, "seg026:load_ggsts_nvf");
	signed int handle;

	/* seek to GGSTS.NVF */
	handle = load_archive_file(ARCHIVE_FILE_GGSTS_NVF);

	/* read it */
	read_archive_file(handle, g_buffer10_ptr, 16771);

	/* close it */
	close(handle);

	g_area_prepared = AREA_TYPE_NONE;
}

/* Dateiname fuer einen Helden: hoechstens 8 Zeichen, Sonderzeichen werden zu '_', dazu ".CHR" */
void prepare_chr_name(char *dst, char *src)
{
	dsa_trace_enter(268, "seg026:prepare_chr_name");
	char tmp_str[40];
	signed int i;

	strcpy(tmp_str, src);

	for (i = 0; i < 8; i++) {

		if (!tmp_str[i])
			break;

		if (!isalnum(tmp_str[i])) {
			tmp_str[i] = '_';
		}
	}

	strncpy(dst, tmp_str, 8);
	dst[8] = '\0';
	strcat(dst, g_chr_file_suffix);
}

/* Dateiname (ohne Endung) fuer einen Spielstand: genau 8 Zeichen, mit '_' aufgefuellt */
void prepare_sg_name(char *dst, char *src)
{
	dsa_trace_enter(269, "seg026:prepare_sg_name");
	char tmp_str[40];
	signed int i;

	strcpy(tmp_str, src);

	i = 0;
	while (i < 8) {

		if (!tmp_str[i]) {

			while (i < 8) {
				/* fill up with underscores */
				tmp_str[i] = '_';
				i++;
			}
			break;
		}

		if (!isalnum(tmp_str[i])) {
			tmp_str[i] = '_';
		}
		i++;
	}

	strncpy(dst, tmp_str, 8);
	dst[8] = '\0';
}

/**
 * \brief   laedt einen Spielstand
 *
 * \return  1 = geladen, 0 = abgebrochen ("KEINEN"/ESC), -1 = Datei fehlt (der Aufrufer fragt dann erneut)
 */
#if !defined(__BORLANDC__)
/* ---- Spielstaende nativ (Windows & Linux) ----
 * Setzt den 5952-Byte-Block ds:0x2d34..0x4474 ueber datseg_status.h zusammen,
 * sodass DOS- und Port-Spielstaende (.GAM) 100 % kompatibel sind. */

#define STATUS_VAR(off, len, var) STATIC_ASSERT(sizeof(var) == (len), size_mismatch_##var);
#define STATUS_DUMMY STATUS_VAR
#define STATUS_PTR(off, len, var) STATIC_ASSERT((len) == 4, ptr_size_mismatch_##var);
#define STATUS_SKIP(off, len, var)
#include "datseg_status.h"
#undef STATUS_VAR
#undef STATUS_DUMMY
#undef STATUS_PTR
#undef STATUS_SKIP

static void status_to_dos(uint8_t *buf)
{
	uint32_t hero_off;
	memset(buf, 0, STATUS_DOS_LENGTH);

#define STATUS_VAR(off, len, var) memcpy(buf + (off), &(var), (len));
#define STATUS_DUMMY STATUS_VAR
#define STATUS_PTR(off, len, var) \
	hero_off = ((var) != NULL) ? (uint32_t)(((struct struct_hero*)(var) - g_heroes) * sizeof(struct struct_hero)) : 0; \
	memcpy(buf + (off), &hero_off, 4);
#define STATUS_SKIP(off, len, var)
#include "datseg_status.h"
#undef STATUS_VAR
#undef STATUS_DUMMY
#undef STATUS_PTR
#undef STATUS_SKIP
}

static void status_from_dos(const uint8_t *buf)
{
	uint32_t hero_off;

#define STATUS_VAR(off, len, var) memcpy(&(var), buf + (off), (len));
#define STATUS_DUMMY STATUS_VAR
#define STATUS_PTR(off, len, var) \
	memcpy(&hero_off, buf + (off), 4); \
	(var) = (typeof(var))((hero_off && (hero_off / sizeof(struct struct_hero) <= 6)) \
		? (g_heroes + (hero_off / sizeof(struct struct_hero))) : NULL);
#define STATUS_SKIP(off, len, var)
#include "datseg_status.h"
#undef STATUS_VAR
#undef STATUS_DUMMY
#undef STATUS_PTR
#undef STATUS_SKIP
}

static int has_suffix_ci(const char *name, const char *suffix)
{
	const size_t nlen = strlen(name);
	const size_t slen = strlen(suffix);
	return (nlen >= slen) && (_stricmp(name + nlen - slen, suffix) == 0);
}

static signed int open_file_ci(const char *fname, int flags)
{
	signed int h = _open(fname, flags);
	DIR *dir;
	struct dirent *dp;

	if (h != -1) return h;

	dir = opendir(".");
	if (!dir) return -1;
	while ((dp = readdir(dir)) != NULL) {
		if (_stricmp(dp->d_name, fname) == 0) {
			h = _open(dp->d_name, flags);
			break;
		}
	}
	closedir(dir);
	return h;
}
#endif

signed int load_game_state(void)
{
	dsa_trace_enter(270, "seg026:load_game_state");
#if 1
	register signed int handle_sg;
	signed int i;
	signed int handle;
	signed int answer;
	signed int sl_bak;
	HugePt p_status_start;
	HugePt p_status_end;
	signed int status_length;
	signed int done;
	signed int length_hero;
	signed int retval;
	signed int ani_bak;
	struct struct_hero* hero_i;
	signed char version[4];
#if defined(__BORLANDC__)
	struct ffblk blk;
#endif
	char name[20];

	retval = 0;

	/* select a game state */
	answer = GUI_radio(get_ttx(0), 6, g_savegame_names[0], /* "WELCHEN ALTEN SPIELSTAND WOLLEN SIE LADEN ?" */
			g_savegame_names[1], g_savegame_names[2],
			g_savegame_names[3], g_savegame_names[4],
			get_ttx(737)) - 1; /* "KEINEN" */

	/* sanity check if answer is in range */
	if (answer != -2 && answer != 5) {

		prepare_sg_name(g_text_output_buf, g_savegame_names[answer]);
		/* concat with ".gam" */
		strcat(g_text_output_buf, g_savegame_suffix);

		/* open the game state file */
#if defined(__BORLANDC__)
		if ((handle_sg = open(g_text_output_buf, O_BINARY | O_RDONLY)) == -1)
#else
		handle_sg = open_file_ci(g_text_output_buf, O_BINARY | O_RDONLY);
		if (handle_sg == -1) {
			char gam_resolved[512];
			handle_sg = _open(dsa_platform_resolve_path(g_text_output_buf, gam_resolved, sizeof(gam_resolved)), O_BINARY | O_RDONLY);
		}
		if (handle_sg == -1)
#endif
		{
			GUI_output(get_ttx(635)); /* "SPIELSTAND NICHT VERFÜGBAR!" */
			retval = -1;
			return retval;
		}

		call_mouse_bg();

		/* something ani related */
		/* (Statuszeile und Animation fuer die Dauer des Ladens abschalten) */
		sl_bak = g_update_statusline;
		g_update_statusline = 0;

		ani_bak = g_ani_enabled;
		g_ani_enabled = 0;

		/* delete every file TEMP\\*.* */
		sprintf(g_text_output_buf, g_str_temp_fmt_ptr, g_all_files_wildcard);

#if defined(__BORLANDC__)
		done = findfirst(g_text_output_buf, &blk, 0);

		if (done == 0) {

			do {
				/* (&blk + 30 = blk.ff_name) */
				sprintf(g_text_output_buf, g_str_temp_fmt_ptr, ((char*)(&blk))+ 30);
				unlink(g_text_output_buf);

				done = findnext(&blk);

			} while (done == 0);
		}
#else
		{
			DIR *dir = opendir("TEMP");
			struct dirent *dp;
			if (dir != NULL) {
				while ((dp = readdir(dir)) != NULL) {
					if (dp->d_name[0] == '.') continue;
					sprintf(g_text_output_buf, g_str_temp_fmt_ptr, dp->d_name);
					unlink(g_text_output_buf);
				}
				closedir(dir);
			}
		}
#endif

		/* init */
		g_saved_files_buf = (uint32_t*)g_dtp2;
		memset(g_saved_files_buf, 0, 286 * sizeof(uint32_t));

		/* read version info */
		_read(handle_sg, (uint8_t*)g_text_output_buf, 12);
		_read(handle_sg, (uint8_t*)&version[3], 1);
		_read(handle_sg, (uint8_t*)&version[2], 1);
		_read(handle_sg, (uint8_t*)&version[0], 1);
		_read(handle_sg, (uint8_t*)&version[1], 1);

#if defined(__BORLANDC__)
		/* liest den 4-Byte-Verweis auf die CHR-Daten; er wird gleich darauf ueberschrieben, also nicht benutzt */
		_read(handle_sg, &gs_datseg_status_start, 4);

		/* read game status */
		/* TODO: check pointer arithmetics work with other pointers */
		p_status_start = (HugePt)&gs_datseg_status_start;
		p_status_end = (HugePt)&gs_datseg_status_end;
		status_length = p_status_end - p_status_start;	/* REMARK: pointer arithmetic */

		_read(handle_sg, &gs_datseg_status_start, status_length);
#else
		{
			uint32_t dummy_pos;
			uint8_t *status_buf = (uint8_t*)calloc(STATUS_DOS_LENGTH, 1);
			_read(handle_sg, (uint8_t*)&dummy_pos, 4);
			status_length = STATUS_DOS_LENGTH;
			if (status_buf != NULL) {
				_read(handle_sg, status_buf, status_length);
				status_from_dos(status_buf);
				free(status_buf);
			}
		}
#endif

		g_special_screen = 1;

		/* read file table */
		_read(handle_sg, g_saved_files_buf, 286 * sizeof(uint32_t));

		/* create for each saved file in gam a file in TEMP */
		for (i = 0; i < 286; i++) {

			if (g_saved_files_buf[i]) {

				/* write file content to TEMP */
				sprintf(g_text_output_buf, g_str_temp_fmt_ptr, g_fnames_v302de[i]);

				/* TODO: should be O_BINARY | O_WRONLY */
				handle = _creat(g_text_output_buf, 0);

				_read(handle_sg, g_renderbuf_ptr, (uint16_t)g_saved_files_buf[i]);
				write(handle, g_renderbuf_ptr, (uint16_t)g_saved_files_buf[i]);
				close(handle);
			}
		}

		/* clear the heroes */
		hero_i = get_hero(0);
		for (i = 0; i <= 6; i++, hero_i++) {
			memset(hero_i, 0, sizeof(struct struct_hero));
		}

		hero_i = (struct struct_hero*)g_renderbuf_ptr;

		do {
			length_hero = _read(handle_sg, (uint8_t*)hero_i, sizeof(struct struct_hero));

			if (length_hero != 0) {

				prepare_chr_name(name, (char*)hero_i);

				/* write file content to TEMP */
				sprintf(g_text_output_buf, g_str_temp_fmt_ptr, name);

				/* TODO: should be O_BINARY | O_WRONLY */
				handle = _creat(g_text_output_buf, 0);

				write(handle, (uint8_t*)hero_i, sizeof(struct struct_hero));
				close(handle);

				if (hero_i->slot_pos != 0) {

					prepare_chr_name(name, (char*)hero_i);

					read_chr_temp(name, hero_i->slot_pos - 1, hero_i->group_id);
				}
			}

		} while (length_hero != 0);

		close(handle_sg);

#if defined(__BORLANDC__)
		/* search for "*.CHR" */
		done = findfirst(g_all_chr_wildcard, &blk, 0);

		while (done == 0) {

			sprintf(g_text_output_buf, g_str_temp_fmt_ptr, ((char*)(&blk)) + 30);

			if ((handle_sg = open(g_text_output_buf, O_BINARY | O_RDWR)) == -1) {
				handle = open(blk.ff_name, O_BINARY | O_RDWR);
				_read(handle, g_renderbuf_ptr, sizeof(struct struct_hero));
				close(handle);

				/* TODO: should be O_BINARY | O_WRONLY */
				handle_sg = _creat(g_text_output_buf, 0);
				write(handle_sg, g_renderbuf_ptr, sizeof(struct struct_hero));
			} else {
				/* Yes, indeed! */
			}

			close(handle_sg);

			done = findnext(&blk);
		}
#else
		{
			DIR *dir = opendir(".");
			struct dirent *dp;
			if (dir != NULL) {
				while ((dp = readdir(dir)) != NULL) {
					if (!has_suffix_ci(dp->d_name, g_chr_file_suffix)) continue;
					sprintf(g_text_output_buf, g_str_temp_fmt_ptr, dp->d_name);
					if ((handle_sg = _open(g_text_output_buf, O_BINARY | O_RDWR)) == -1) {
						handle = _open(dp->d_name, O_BINARY | O_RDWR);
						if (handle != -1) {
							long flen = lseek(handle, 0, SEEK_END);
							lseek(handle, 0, SEEK_SET);
							if ((flen == (long)sizeof(struct struct_hero)) || (flen == (long)sizeof(struct struct_hero) - 16)) {
								handle_sg = _creat(g_text_output_buf, 0);
								if (flen == (long)sizeof(struct struct_hero)) {
									_read(handle, g_renderbuf_ptr, sizeof(struct struct_hero));
								} else {
									_read(handle, g_renderbuf_ptr + 16, sizeof(struct struct_hero) - 16);
									memcpy(g_renderbuf_ptr, g_renderbuf_ptr + 16, 16);
								}
								write(handle_sg, g_renderbuf_ptr, sizeof(struct struct_hero));
								close(handle_sg);
							}
							close(handle);
						}
					} else {
						close(handle_sg);
					}
				}
				closedir(dir);
			}
		}
#endif

		/* den NSC suchen, der in der Gruppe ist (slot_pos == 7) */
		for (i = ARCHIVE_FILE_NPCS; i <= (ARCHIVE_FILE_NPCS+5); i++) {

			load_npc(i);

			if (get_hero(6)->slot_pos != 7) {

				memset(get_hero(6), 0, sizeof(struct struct_hero));
			} else {
				break;
			}
		}

		g_area_prepared = AREA_TYPE_NONE;
		g_request_refresh = retval = 1;
#if FIX_0442
		/* Fix 442: das Symbol "Gruppen vereinen" nach dem Laden neu bestimmen. g_can_merge_group steht nicht im
		 * Spielstand und wurde sonst erst beim naechsten Schritt neu berechnet (gemeldet von Andrean). */
		g_can_merge_group = (signed char)can_merge_group();
#endif
		g_check_disease = 0;
		g_check_poison = 0;
		g_fading_state = 3;

		if (gs_town_loc_type != LOCTYPE_TEMPLE) {
			g_new_menu_icons[6] = g_new_menu_icons[7] = g_new_menu_icons[8] = MENU_ICON_NONE;
		}

		load_area_description(2);

		g_update_statusline = sl_bak;
		g_ani_enabled = ani_bak;

		call_mouse();
	}

	return retval;
#endif
}

#if FIX_SPEICHERN
/* Zusatzfunktion FIX_SPEICHERN (Opt-in): liegt APFREI.AN im Spielordner, kostet Speichern ausserhalb eines Tempels
 * keine AP mehr, und der Hinweis darauf entfaellt (wie der optionale Extra-Patch NOAPLOSS von NRS, 2016).
 * Die Startdateien legen die Datei an (SPEICHERN_OHNE_AP_AN.bat) oder entfernen sie. Dateiname Zeichen fuer Zeichen. */
static signed int speichern_ohne_ap(void)
{
	dsa_trace_enter(271, "seg026:speichern_ohne_ap");
	char name[12];
	signed int fd;

	name[0] = 'A'; name[1] = 'P'; name[2] = 'F'; name[3] = 'R'; name[4] = 'E'; name[5] = 'I';
	name[6] = '.'; name[7] = 'A'; name[8] = 'N'; name[9] = '\0';
	if ((fd = open(name, O_BINARY | O_RDONLY)) != -1) {
		close(fd);
		return 1;
	}
	return 0;
}
#endif

/**
 * \brief   writes a game state file
 *
 * \return              1 = OK, 0 = error
 */
signed int save_game_state(void)
{
	dsa_trace_enter(272, "seg026:save_game_state");
#if 1
	signed int handle_sg;
	HugePt p_status_start;
	HugePt p_status_end;
	unsigned int status_length;
	signed int handle;
	signed int tw_bak;
	signed int done;
	signed int slot;
	uint32_t filepos;
	uint32_t filepos2;
	signed int flag;
	uint32_t len;
#if defined(__BORLANDC__)
	struct ffblk blk;
#endif
#if FIX_SPEICHERN
	signed int ohne_ap = speichern_ohne_ap();
#endif

	tw_bak = g_textbox_width;
	g_textbox_width = 5;

	/* prepare the header for the radio box */
	if (g_game_state == GAME_STATE_VICTORY) {

		/* game won. creating savegame for import in DSA2 */
		strcpy(g_text_output_buf, get_ttx(810)); /* "Welcher Spielstand soll fuer die Fortsetzung abgespeichert werden?" */

	} else {

#ifndef M302de_FEATURE_MOD
		/* Feature mod 4: In the original game, when creating a savegame while not being in a temple, the AP of all heroes is decreased by 1. This feature mod stops the AP decrease.
		 * Here, the warning message "Dabei verliert jeder Held in der Gruppe einen Abenteuerpunkt" is displayed. */
#if FIX_SPEICHERN
		if ((gs_town_loc_type != LOCTYPE_TEMPLE) && (g_game_state != GAME_STATE_VICTORY) && !ohne_ap) {
#else
		if ((gs_town_loc_type != LOCTYPE_TEMPLE) && (g_game_state != GAME_STATE_VICTORY)) {
#endif

			/* create savegame not in a temple */

			sprintf(g_dtp2,
				get_ttx(813), /* "Dabei verliert jeder Held in der Gruppe einen Abenteuerpunkt" */
				1, get_ttx(392), g_empty_string1); /* "PUNKT" */

			sprintf(g_text_output_buf,
				get_ttx(1), /* "Welchen Spielstand wollen Sie abspeichern ?" */
				g_dtp2);
		} else {
#endif

			/* create savegame inside a temple */
			sprintf(g_text_output_buf,
				get_ttx(1), /* "Welchen Spielstand wollen Sie abspeichern ?" */
				g_empty_string2);
#ifndef M302de_FEATURE_MOD
		/* Feature mod 4: In the original game, when creating a savegame while not being in a temple, the AP of all heroes is decreased by 1. This mod stops the AP decrease. */
		}
#endif
	}

	/* get the slot number */
	slot = GUI_radio(g_text_output_buf, 6, g_savegame_names[0], g_savegame_names[1],
					g_savegame_names[2], g_savegame_names[3],
					g_savegame_names[4], get_ttx(737)) - 1; /* "KEINEN" */

	g_textbox_width = tw_bak;

	g_saved_files_buf = (uint32_t*)g_dtp2;
	memset(g_saved_files_buf, 0, 286 * sizeof(uint32_t));

	if (slot != -2 && slot != 5) {

		do {
			/* ask for filename */
			g_gui_entering_savegame = 1;
			strcpy(g_text_input_buf, g_savegame_names[slot]);
			GUI_input(get_ttx(634), 8); /* "WIE SOLL DIESER SPIELSTAND HEISSEN?" */
			g_gui_entering_savegame = 0;

			if (*g_text_input_buf == '\0') {
				return 0;
			}

			flag = 0;

			prepare_sg_name(g_text_output_buf, g_text_input_buf);

			for (tw_bak = 0; tw_bak < 5; tw_bak++) {

				prepare_sg_name(g_text_output_buf + 50, g_savegame_names[tw_bak]);

				if ((slot != tw_bak) && !strcmp(g_text_output_buf, g_text_output_buf + 50)) {

					GUI_output(get_ttx(806)); /* "SPIELSTAND MIT SELBEM NAMEN EXISTIERT BEREITS UNTER ANDEREM EINTRAG!" */
					flag = 1;
				}
			}
		} while (flag != 0);

		/* delete the previous file of that slot */
		prepare_sg_name(g_text_output_buf, g_savegame_names[slot]);
		strcat(g_text_output_buf, g_savegame_suffix2);
		unlink(g_text_output_buf);
		strcpy(g_savegame_names[slot], g_text_input_buf);

		/* create a CHR-file for each hero in TEMP */
		for (tw_bak = 0; tw_bak < 6; tw_bak++) {

			if (get_hero(tw_bak)->typus != HERO_TYPE_NONE) {

				/* save position on the playmask */
				get_hero(tw_bak)->slot_pos = tw_bak + 1;

#ifndef M302de_FEATURE_MOD
				/* Feature mod 4: In the original game, when creating a savegame while not being in a temple, the AP of all heroes is decrease by 1. This feature mod stops the AP decrease.
				 * Here, the actual decrease is executed */
#if FIX_SPEICHERN
				if ((g_game_state != GAME_STATE_VICTORY) && (gs_town_loc_type != LOCTYPE_TEMPLE) && (get_hero(tw_bak)->ap > 0) && !ohne_ap)
#else
				if ((g_game_state != GAME_STATE_VICTORY) && (gs_town_loc_type != LOCTYPE_TEMPLE) && (get_hero(tw_bak)->ap > 0))
#endif
				{
					add_hero_ap(get_hero(tw_bak), -1L);
				}
#endif

				write_chr_temp(tw_bak);
			}
		}

		/* save the current NPC in TEMP */
		if (get_hero(6)->typus != HERO_TYPE_NONE) {

			get_hero(6)->slot_pos = 7;
			save_npc(get_hero(6)->npc_id + 225);
		}

		load_area_description(1);

#if defined(__BORLANDC__)
		/* TODO: check if pointer arithmetics work with other pointers */
		p_status_start = (HugePt)&gs_datseg_status_start;
		p_status_end = (HugePt)&gs_datseg_status_end;
		status_length = p_status_end - p_status_start;
#else
		p_status_start = NULL;
		p_status_end = NULL;
		status_length = STATUS_DOS_LENGTH;
#endif

		prepare_sg_name(g_text_output_buf, g_savegame_names[slot]);
		strcat(g_text_output_buf, g_savegame_suffix3);

		/* TODO: should be O_BINARY | O_RWONLY */
		while ((handle_sg = _creat(g_text_output_buf, 0)) == -1) {
			GUI_output(get_ttx(348)); /* "SPEICHERFEHLER!@EVENTUELL DISKETTE VOLL?@FUNKTION WIRD ABGEBROCHEN!" */
			return 0;
		}

		time(&g_last_save_time);

		filepos = 0;

		/* write version identifier 16 bytes */
		filepos += write(handle_sg, &g_dsa_version_string, 12);
		filepos += write(handle_sg, &g_version_token4, 1);
		filepos += write(handle_sg, &g_version_token3, 1);
		filepos += write(handle_sg, &g_version_token1, 1);
		filepos += write(handle_sg, &g_version_token2, 1);

		/* write fileposition 4 bytes */
		/* this will be updated later to find the data of the CHR files */
		filepos += write(handle_sg, &filepos, 4);

		/* save the status section 5952 bytes */
#if defined(__BORLANDC__)
		filepos += write(handle_sg, p_status_start, status_length);
#else
		{
			uint8_t *status_buf = (uint8_t*)calloc(STATUS_DOS_LENGTH, 1);
			if (status_buf != NULL) {
				status_to_dos(status_buf);
				filepos += write(handle_sg, status_buf, status_length);
				free(status_buf);
			}
		}
#endif

		/* check if enough bytes were written */
		if (status_length + 16 + 4L != filepos) {

			GUI_output(get_ttx(348)); /* "SPEICHERFEHLER!@EVENTUELL DISKETTE VOLL?@FUNKTION WIRD ABGEBROCHEN!" */
			close(handle_sg);
			return 0;
		}

		filepos2 = filepos;
		len = (uint16_t)write(handle_sg, g_saved_files_buf, 286 * sizeof(uint32_t));
		filepos += len;

		if (len != 4 * 286) {

			GUI_output(get_ttx(348)); /* "SPEICHERFEHLER!@EVENTUELL DISKETTE VOLL?@FUNKTION WIRD ABGEBROCHEN!" */
			close(handle_sg);
			return 0;
		}

		/* save all changed files from SCHICK.DAT */
		for (tw_bak = 0; tw_bak < 286; tw_bak++) {

			sprintf(g_text_output_buf, g_str_temp_fmt_ptr, g_fnames_v302de[tw_bak]);

#if defined(__BORLANDC__)
			done = findfirst(g_text_output_buf, &blk, 0);
#else
			{
				struct stat st;
				done = (stat(g_text_output_buf, &st) == 0 && S_ISREG(st.st_mode)) ? 0 : -1;
			}
#endif


			if (done == 0) {

				handle = load_archive_file(tw_bak + 0x8000);
				g_saved_files_buf[tw_bak] = get_archive_file_length(handle);
				_read(handle, g_renderbuf_ptr, (uint16_t)g_saved_files_buf[tw_bak]);
				close(handle);

				len = (uint16_t)write(handle_sg, g_renderbuf_ptr, (uint16_t)g_saved_files_buf[tw_bak]);
				filepos += len;

				if ((uint16_t)g_saved_files_buf[tw_bak] != len) {
					GUI_output(get_ttx(348)); /* "SPEICHERFEHLER!@EVENTUELL DISKETTE VOLL?@FUNKTION WIRD ABGEBROCHEN!" */
					close(handle_sg);
					return 0;
				}
			}
		}

		/* skip back to the start of the offset of the CHR data */
		lseek(handle_sg, 16, 0);
		write(handle_sg, &filepos, 4);

		/* write the file table */
		lseek(handle_sg, filepos2, 0);
		write(handle_sg, g_saved_files_buf, 286 * sizeof(uint32_t));

		/* append all CHR files */
		lseek(handle_sg, filepos, 0);
#if defined(__BORLANDC__)
		sprintf(g_text_output_buf, g_str_temp_fmt_ptr, g_all_chr_wildcard2);

		done = findfirst(g_text_output_buf, &blk, 0);
		/* Original-Bug (latent): do-while ohne Pruefung von done; findet findfirst nichts,
		 * wird einmal mit ungueltigem blk.ff_name gelesen. Praktisch gibt es immer CHR-Dateien. */
#if FIX_0289
		/* Fix 289: nur anhaengen, wenn findfirst() etwas gefunden hat */
		if (done == 0)
#endif
		do {
			/* create the CHR filename */
			sprintf(g_text_output_buf, g_str_temp_fmt_ptr, blk.ff_name);

			/* read the CHR file from temp */
			handle = open(g_text_output_buf, O_BINARY | O_RDWR);
			_read(handle, g_renderbuf_ptr, sizeof(struct struct_hero));
			close(handle);

			/* append it */
			len = write(handle_sg, g_renderbuf_ptr, sizeof(struct struct_hero));

			if (len != sizeof(struct struct_hero)) {

				GUI_output(get_ttx(348)); /* "SPEICHERFEHLER!@EVENTUELL DISKETTE VOLL?@FUNKTION WIRD ABGEBROCHEN!" */
				close(handle_sg);
				return 0;
			}

			done = findnext(&blk);

		} while (done == 0);
#elif defined(_WIN32)
		{
			DIR *dir = opendir("TEMP");
			struct dirent *dp;
			if (dir != NULL) {
				while ((dp = readdir(dir)) != NULL) {
					if (!has_suffix_ci(dp->d_name, g_chr_file_suffix)) continue;
					sprintf(g_text_output_buf, g_str_temp_fmt_ptr, dp->d_name);
					handle = _open(g_text_output_buf, O_BINARY | O_RDWR);
					if (handle != -1) {
						_read(handle, g_renderbuf_ptr, sizeof(struct struct_hero));
						close(handle);
						len = write(handle_sg, g_renderbuf_ptr, sizeof(struct struct_hero));
						if (len != sizeof(struct struct_hero)) {
							GUI_output(get_ttx(348));
							closedir(dir);
							close(handle_sg);
							return 0;
						}
					}
				}
				closedir(dir);
			}
		}
#else
		{
			DIR *dir = opendir("TEMP");
			struct dirent *dp;
			char names[128][32];
			int n_names = 0;
			if (dir != NULL) {
				while ((dp = readdir(dir)) != NULL && n_names < 128) {
					if (!has_suffix_ci(dp->d_name, g_chr_file_suffix)) continue;
					strncpy(names[n_names], dp->d_name, 31);
					names[n_names][31] = ' ';
					n_names++;
				}
				closedir(dir);
				for (int a = 0; a < n_names - 1; ++a) {
					for (int b = a + 1; b < n_names; ++b) {
						if (_stricmp(names[a], names[b]) > 0) {
							char tmp[32];
							memcpy(tmp, names[a], 32);
							memcpy(names[a], names[b], 32);
							memcpy(names[b], tmp, 32);
						}
					}
				}
				for (int a = 0; a < n_names; ++a) {
					sprintf(g_text_output_buf, g_str_temp_fmt_ptr, names[a]);
					handle = _open(g_text_output_buf, O_BINARY | O_RDWR);
					if (handle != -1) {
						_read(handle, g_renderbuf_ptr, sizeof(struct struct_hero));
						close(handle);
						len = write(handle_sg, g_renderbuf_ptr, sizeof(struct struct_hero));
						if (len != sizeof(struct struct_hero)) {
							GUI_output(get_ttx(348));
							close(handle_sg);
							return 0;
						}
					}
				}
			}
		}
#endif

		close(handle_sg);

		/* rewrite GAMES.NAM */
		handle_sg = _creat(g_fnames_v302de[207], 0);
		write(handle_sg, &g_savegame_names[0][0], 45);
		close(handle_sg);
#if defined(__EMSCRIPTEN__)
		dsa_platform_sync_savegames();
#endif

		return 1;
	}
#endif

	return 0;
}

/**
 * \brief   reads a CHR file from TEMP-dir
 *
 * \param   fname       filename
 * \param   hero_pos    position of the hero
 * \param   group_id    id of the group
 * \return              1 = OK, 0 = Error
 */
signed int read_chr_temp(char *fname, const signed int hero_pos, const signed int group_id)
{
	dsa_trace_enter(273, "seg026:read_chr_temp");
#if 1
	signed int handle;
	signed int hero_size = sizeof(struct struct_hero);
	struct struct_hero *hero;

	sprintf(g_text_output_buf, g_str_temp_fmt_ptr, fname);

#if defined(__BORLANDC__)
	if ((handle = open(g_text_output_buf, O_BINARY | O_RDWR)) == -1) {
		copy_file_to_temp(fname, g_text_output_buf);
		handle = open(g_text_output_buf, O_BINARY | O_RDWR);
	}
#else
	if ((handle = _open(g_text_output_buf, O_BINARY | O_RDWR)) == -1) {
		copy_file_to_temp(fname, g_text_output_buf);
		handle = _open(g_text_output_buf, O_BINARY | O_RDWR);
	}
#endif

	if (handle != -1) {

		hero = get_hero(hero_pos);
		_read(handle, hero, hero_size);
		close(handle);

		hero->group_id = group_id;

		if (hero->sex == 1) {

			hero->actor_sprite_id = hero->typus + 11;

			if (hero->actor_sprite_id > 21) {
				hero->actor_sprite_id = 21;
			}
		} else {
			hero->actor_sprite_id = hero->typus;

			if (hero->actor_sprite_id > 10) {
				hero->actor_sprite_id = 10;
			}
		}

		/* In the following line it would be more consistent to check only bit 0.
		 * bit 1 is a flag which is set if the hero got the IN attribute bonus at the black eye at the Monolith (Einsiedlersee <-> Einsiedlersee, tevent135).
		 * However, this should still be ok, as it should never happen that bit 0 is unset and bit 1 is set. */

		if (!hero->start_gear__monolith_reward) {

			startup_equipment(hero);

			get_hero(hero_pos)->start_gear__monolith_reward = 1; /* it would be more consistent to set only bit 0 and leave the others untouched, see above. */

			write_chr_temp(hero_pos);
		}

	} else {
		GUI_output(get_ttx(4)); /* "ICH KANN DIESEN HELDEN LEIDER NICHT FINDEN!" */
		return 0;
	}
#endif
	return 1;
}

/**
 * \brief   writes a CHR file to temp
 *
 * \param   hero_pos    position of the hero
 */
void write_chr_temp(const signed int hero_pos)
{
	dsa_trace_enter(274, "seg026:write_chr_temp");
	char fname[20];
	signed int handle;

	prepare_chr_name(fname, get_hero(hero_pos)->name);

	sprintf(g_text_output_buf, g_str_temp_fmt_ptr, fname);

	/* TODO: should be O_BINARY | O_WRONLY */
	handle = _creat(g_text_output_buf, 0);

	write(handle, get_hero(hero_pos), sizeof(struct struct_hero));

	close(handle);
}

/**
 * \brief   copy the names from CHR file
 *
 * \param   ptr         pointer
 *                      (Zielpuffer: je Held 32 Byte, Name ab +0, Rufname ab +16)
 * \param   temple_id   > 0 the id of the temple, -1 on delete mode
 * \return              # of CHR-files in TEMP-dir
 *                      (genauer: Anzahl der passenden Helden, die nicht in der Gruppe sind)
 */
signed int copy_chr_names(char *ptr, const signed int temple_id)
{
	dsa_trace_enter(275, "seg026:copy_chr_names");
#if defined(_WIN32)
	signed int count = 0;
	DIR *dir = opendir("TEMP");
	struct dirent *dp;
	char path[260];
	struct struct_hero *hero = (struct struct_hero*)g_renderbuf_ptr;

	if (dir != NULL) {
		while ((dp = readdir(dir)) != NULL) {
			if (!has_suffix_ci(dp->d_name, g_chr_file_suffix)) continue;
			snprintf(path, sizeof(path), "TEMP/%s", dp->d_name);
			{
				signed int handle = _open(path, O_BINARY | O_RDONLY);
				if (handle != -1) {
					_read(handle, hero, sizeof(struct struct_hero));
					close(handle);
					if (((hero->temple_id == temple_id) && !hero->slot_pos) ||
						(!hero->slot_pos && (temple_id == -1)))
					{
						strcpy((char*)ptr + 32 * count, hero->name);
						strcpy((char*)ptr + 32 * count + 16, hero->alias);
						count++;
					}
				}
			}
		}
		closedir(dir);
	}
	return count;
#elif !defined(__BORLANDC__)
	signed int count = 0;
	DIR *dir = opendir("TEMP");
	struct dirent *dp;
	char path[260];
	char names[128][32];
	int n_names = 0;
	struct struct_hero *hero = (struct struct_hero*)g_renderbuf_ptr;

	if (dir != NULL) {
		while ((dp = readdir(dir)) != NULL && n_names < 128) {
			if (!has_suffix_ci(dp->d_name, g_chr_file_suffix)) continue;
			strncpy(names[n_names], dp->d_name, 31);
			names[n_names][31] = '\0';
			n_names++;
		}
		closedir(dir);
		for (int a = 0; a < n_names - 1; ++a) {
			for (int b = a + 1; b < n_names; ++b) {
				if (_stricmp(names[a], names[b]) > 0) {
					char tmp[32];
					memcpy(tmp, names[a], 32);
					memcpy(names[a], names[b], 32);
					memcpy(names[b], tmp, 32);
				}
			}
		}
		for (int a = 0; a < n_names; ++a) {
			snprintf(path, sizeof(path), "TEMP/%s", names[a]);
			{
				signed int handle = _open(path, O_BINARY | O_RDONLY);
				if (handle != -1) {
					_read(handle, hero, sizeof(struct struct_hero));
					close(handle);
					if (((hero->temple_id == temple_id) && !hero->slot_pos) ||
						(!hero->slot_pos && (temple_id == -1)))
					{
						strcpy((char*)ptr + 32 * count, hero->name);
						strcpy((char*)ptr + 32 * count + 16, hero->alias);
						count++;
					}
				}
			}
		}
	}
	return count;
#elif defined(__BORLANDC__)
	signed int count = 0;
	signed int done;
	signed int handle;
	struct struct_hero *hero;
	struct ffblk blk;

	hero = (struct struct_hero*)(g_renderbuf_ptr + 60000);

	sprintf(g_text_output_buf, g_str_temp_fmt_ptr, g_all_chr_wildcard3);

	done = findfirst(g_text_output_buf, &blk, 0);

	if (!done) {

		do {
			/* create the CHR filename */
			sprintf(g_text_output_buf, g_str_temp_fmt_ptr, blk.ff_name);

			/* read the CHR file from temp */
			handle = open(g_text_output_buf, O_BINARY | O_RDWR);
			_read(handle, hero, sizeof(struct struct_hero));
			close(handle);

			if (((hero->temple_id == temple_id) && !hero->slot_pos) ||
				(!hero->slot_pos && (temple_id == -1)))
			{
				strcpy(ptr + 32 * count, hero->name);
				strcpy(ptr + 32 * count + 16, hero->alias);
				count++;
			}

			done = findnext(&blk);

		} while (!done);

		return count;
	} else {
		return 0;
	}
#else
	return 0;
#endif
}

/**
 * \brief   loads a head icon from IN_HEADS.NVF
 *
 * \param   head        index of the desired head
 */
void load_in_head(const signed int head)
{
	dsa_trace_enter(276, "seg026:load_in_head");
	signed int handle;

	if (head >= 0) {

		handle = load_archive_file(ARCHIVE_FILE_IN_HEADS_NVF);

		seek_archive_file(handle, 1024L * head, 0);

		read_archive_file(handle, (unsigned char*)g_dtp2, 1024);

		close(handle);

		g_loaded_head_id = head;
	}
}

/**
 * \brief   load temple logo
 *
 * \param   god_id          ID of the god
 */
void load_temple_logo(signed int god_id)
{
	dsa_trace_enter(277, "seg026:load_temple_logo");
	struct nvf_extract_desc nvf;
	signed int handle; /* REMARK: reused differently */

	// there is no dedicated logo for Ifirn, use the one of Firun.
	if (god_id == GOD_ID_IFIRN) {
		god_id = GOD_ID_FIRUN;
	}

	/* load TEMPICON */
	handle = load_archive_file(ARCHIVE_FILE_TEMPICON);
	read_archive_file(handle, g_buffer8_ptr, 7000);
	close(handle);

	nvf.dst = g_buffer8_ptr + 7000;
	nvf.src = g_buffer8_ptr;
	nvf.image_num = god_id;
	nvf.compression_type = 0;
	nvf.width = &handle;
	nvf.height = &handle;

	process_nvf_extraction(&nvf);
}

#if defined(__EMSCRIPTEN__)
void dsa_web_reload_savegame_names(void)
{
	signed int handle = load_regular_file(ARCHIVE_FILE_GAMES_NAM);
	if (handle != -1) {
		memset(g_savegame_names, 0, 45);
		_read(handle, g_savegame_names, 45);
		close(handle);
	}
}
#endif
