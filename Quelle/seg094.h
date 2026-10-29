/* 0x20 */
void TM_draw_track_before_crosslink(const signed int);
/* 0x2a */
void TM_do_journey_crosslink(void);
/* 0x2a */
void trv_do_journey(const signed int, const signed int);
/* 0x2f */
void prepare_map_marker(void);
/* 0x34 */
/* static */
void set_town_marker_and_textbox(const signed int);
/* 0x39 */
#if defined(__BORLANDC__)
signed int TM_unused_continue_journey(struct trv_start_point*, const signed int);
#endif
/* 0x3e */
signed int trv_journey_dest_town_viewdir(const signed int);

/* 0x43 */
signed int TM_get_track_length(struct struct_point*);
/* 0x48 */
signed int trv_find_dest_town_signpost(void);
/* 0x4d */
void TM_draw_track(const signed int, const signed int, const signed int, const signed int);
/* 0x52 */
#if defined(__BORLANDC__)
void TM_unused2(void);
#endif
