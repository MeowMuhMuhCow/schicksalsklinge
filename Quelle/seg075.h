/* 0x20 */
void DNG_update_pos(void);
/* 0x25 */
void DNG_inc_level(void);
/* 0x2a */
void DNG_dec_level(void);
/* 0x2f */
void DNG_render_view(void);
/* 0x34 */
void DNG_draw_squares(void);
/* 0x39 */
void DNG_draw_image(signed int, signed int, signed int);
/* 0x3e */
void DNG_show_view(void);
/* 0x43 */
void DNG_timestep(const signed int);
/* 0x48 */
void DNG_draw_doorframe(void);
/* 0x4d */
void DNG_floor_ceil(void);
/* 0x52 */
void DNG_set_squares_tex(void);
/* 0x57 */
void DNG_open_door(void);
/* 0x5c */
void DNG_close_door(void);
/* 0x61 */
void DNG_lights(void);
/* 0x66 */
signed int DNG_pitfall(const signed int);
/* 0x6b */
signed int is_staff_lvl2_in_group(void);
/* 0x70 */
void DNG01_pitfall(void);
/* 0x75 */
signed int DNG_check_climb_tools(void);
/* 0x7a */
void DNG_enter_dungeon(const signed int);
