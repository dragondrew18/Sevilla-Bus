#include <pebble.h>
#include "keys.h"
#include "data.h"

void graphics_draw_text_vertically_center(GContext* ctx,
		const char* text, GFont const font, const GRect box,
		const GTextOverflowMode overflow_mode, const GTextAlignment alignment) {

	GSize size = graphics_text_layout_get_content_size(text, font, box,
				overflow_mode, alignment);
	GRect box_2 = box;
	box_2.origin.y += (box.size.h - size.h)/2 - 3;

	graphics_draw_text(ctx, text, font, box_2, overflow_mode, alignment, NULL);

}

void show_log(AppLogLevel log_level, const char* description){
//	APP_LOG(log_level, description);
}

bool is_empty_char(char *input) {
	return strcmp(input, "") != 0;
}

