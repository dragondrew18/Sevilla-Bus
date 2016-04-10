#pragma once

bool get_JS_is_ready(void);
void set_JS_is_ready(bool input);
void graphics_draw_text_vertically_center(GContext* ctx,
		const char* text, GFont const font, const GRect box,
		const GTextOverflowMode overflow_mode, const GTextAlignment alignment,
		const GTextLayoutCacheRef layout);
