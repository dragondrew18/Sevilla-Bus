#pragma once

void graphics_draw_text_vertically_center(GContext* ctx,
		const char* text, GFont const font, const GRect box,
		const GTextOverflowMode overflow_mode, const GTextAlignment alignment);

void show_log(AppLogLevel log_level, const char* description);

bool is_empty_char(char *input);
