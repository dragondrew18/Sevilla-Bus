#include <pebble.h>

static bool is_ready = false;

bool get_JS_is_ready() {
	return is_ready;
}

void set_JS_is_ready(bool input){
	is_ready = input;
}

void graphics_draw_text_vertically_center(GContext* ctx,
		const char* text, GFont const font, const GRect box,
		const GTextOverflowMode overflow_mode, const GTextAlignment alignment,
		const GTextLayoutCacheRef layout) {

	GSize size = graphics_text_layout_get_content_size(text, font, box,
				overflow_mode, alignment);
	GRect box_2 = box;
	box_2.origin.y = (box.size.h - size.h)/2 + box.origin.y - 2;

	graphics_draw_text(ctx, text, font, box_2, overflow_mode, alignment, layout);

}


