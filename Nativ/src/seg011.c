/**
 *	Rewrite of DSA1 v3.02_de functions of seg011 (AIL, miles sound system)
 *	Functions rewritten: 0/0
 */
#include "v302de.h"

#define AIL_SEGMENT (0x1042)

void AIL_startup(void)
{
	dsa_trace_enter(223, "seg011:AIL_startup");
}

void AIL_shutdown(uint8_t* signoff_msg)
{
	dsa_trace_enter(224, "seg011:AIL_shutdown");
}

int16_t AIL_register_driver(uint8_t* driver)
{
	dsa_trace_enter(225, "seg011:AIL_register_driver");
	return 0;
}

uint8_t* AIL_describe_driver(uint16_t driver)
{
	return NULL;
}

uint16_t AIL_detect_device(uint16_t driver, uint16_t IO_addr, uint16_t IRQ, uint16_t DMA, uint16_t DRQ)
{
	dsa_trace_enter(226, "seg011:AIL_detect_device");
	return 0;
}

void AIL_init_driver(uint16_t driver, uint16_t IO_addr, uint16_t IRQ, uint16_t DMA, uint16_t DRQ)
{
	dsa_trace_enter(227, "seg011:AIL_init_driver");
}

void AIL_play_VOC_file(uint16_t driver, uint8_t* VOC_file, int16_t block_marker)
{
	dsa_trace_enter(228, "seg011:AIL_play_VOC_file");
}

uint16_t AIL_format_VOC_file(uint16_t driver, uint8_t* VOC_file, int16_t block_marker)
{
	dsa_trace_enter(229, "seg011:AIL_format_VOC_file");
	return 0;
}

uint16_t AIL_VOC_playback_status(uint16_t driver)
{
	dsa_trace_enter(230, "seg011:AIL_VOC_playback_status");
	return 0;
}

void AIL_start_digital_playback(uint16_t driver)
{
	dsa_trace_enter(231, "seg011:AIL_start_digital_playback");
}

void AIL_stop_digital_playback(uint16_t driver)
{
	dsa_trace_enter(232, "seg011:AIL_stop_digital_playback");
}

void AIL_set_digital_playback_volume(uint16_t driver, uint16_t percent)
{
	dsa_trace_enter(233, "seg011:AIL_set_digital_playback_volume");
}

uint16_t AIL_state_table_size(uint16_t driver)
{
	dsa_trace_enter(234, "seg011:AIL_state_table_size");
	return 0;
}

uint16_t AIL_register_sequence(uint16_t driver, uint8_t* FORM_XMID, uint16_t sequence_num, uint8_t* state_table, uint8_t* controller_table)
{
	dsa_trace_enter(235, "seg011:AIL_register_sequence");
	return 0;
}

void AIL_release_sequence_handle(int16_t driver, int16_t sequence)
{
	dsa_trace_enter(236, "seg011:AIL_release_sequence_handle");
}

uint16_t AIL_default_timbre_cache_size(uint16_t driver)
{
	dsa_trace_enter(237, "seg011:AIL_default_timbre_cache_size");
	return 0;
}

void AIL_define_timbre_cache(uint16_t driver, uint8_t* cache_addr, uint16_t cache_size)
{
	dsa_trace_enter(238, "seg011:AIL_define_timbre_cache");
}

uint16_t AIL_timbre_request(uint16_t driver, uint16_t sequence)
{
	dsa_trace_enter(239, "seg011:AIL_timbre_request");
	return 0;
}

void AIL_install_timbre(uint16_t driver, uint16_t bank, uint16_t patch, uint8_t* src_addr)
{
	dsa_trace_enter(240, "seg011:AIL_install_timbre");
}

void AIL_start_sequence(int16_t driver, int16_t sequence)
{
	dsa_trace_enter(241, "seg011:AIL_start_sequence");
}

void AIL_stop_sequence(int16_t driver, int16_t sequence)
{
	dsa_trace_enter(242, "seg011:AIL_stop_sequence");
}

uint16_t AIL_sequence_status(uint16_t driver, uint16_t sequence)
{
	dsa_trace_enter(243, "seg011:AIL_sequence_status");
	return 0;
}

void AIL_set_relative_volume(uint16_t driver, uint16_t sequence, uint16_t percent, uint16_t ms)
{
	dsa_trace_enter(244, "seg011:AIL_set_relative_volume");
}
