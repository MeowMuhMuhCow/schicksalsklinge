void CD_audio_repeat_check(void);
#if defined(__BORLANDC__)
signed int CD_bioskey(const signed int);
#endif
void CD_audio_stop(void);
void CD_audio_pause(void);
void CD_audio_resume(void);
void CD_set_track(const signed int);
void CD_check(void);
signed int CD_init(void);
