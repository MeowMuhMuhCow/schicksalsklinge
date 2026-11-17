#if !defined(__BORLANDC__)
void decomp_pp20(uint8_t *packed, uint8_t *dst, uint8_t *eff_table, uint32_t packed_len);
#else
#if defined(__cplusplus)
extern "C" {
#endif
/* entpackt PP20 rueckwaerts: packed = gepackte Datei, dst = Ziel, eff_off:eff_seg = Effizienztabelle
 * (packed + 4), packed_len = Laenge der gepackten Datei. Bright-Eyes hatte packed und dst vertauscht benannt. */
void decomp_pp20(uint8_t *packed, uint8_t *dst, uint16_t eff_off, uint16_t eff_seg, uint32_t packed_len);
#if defined(__cplusplus)
}
#endif
#endif
