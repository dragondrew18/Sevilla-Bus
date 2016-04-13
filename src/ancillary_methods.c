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
		const GTextOverflowMode overflow_mode, const GTextAlignment alignment) {

	GSize size = graphics_text_layout_get_content_size(text, font, box,
				overflow_mode, alignment);
	GRect box_2 = box;
	box_2.origin.y += (box.size.h - size.h)/2 - 3;

	graphics_draw_text(ctx, text, font, box_2, overflow_mode, alignment, NULL);

}

void draw_layout_border (GContext *ctx, GRect zoneToBorder, int strokeWidth, int difference, GColor color){
	graphics_context_set_stroke_color(ctx, color);

//	a-----b
//	|     |
//	c-----d

	GPoint a = GPoint(zoneToBorder.origin.x + difference, zoneToBorder.origin.y + difference);
	GPoint b = GPoint(zoneToBorder.origin.x + zoneToBorder.size.w - difference, zoneToBorder.origin.y + difference);
	GPoint c = GPoint(zoneToBorder.origin.x + difference, zoneToBorder.origin.y + zoneToBorder.size.h - difference);
	GPoint d = GPoint(zoneToBorder.origin.x + zoneToBorder.size.w - difference, zoneToBorder.origin.y + zoneToBorder.size.h - difference);

	graphics_draw_line(ctx, a, b);
	graphics_draw_line(ctx, c, d);
	graphics_draw_line(ctx, a, c);
	graphics_draw_line(ctx, b, d);

}

AppMessageResult ancillary_app_message_outbox_begin(DictionaryIterator **iterator){
	AppMessageResult res = app_message_outbox_begin(iterator);
	if (res != APP_MSG_OK) {
		// Error establishing the connection
		APP_LOG(APP_LOG_LEVEL_ERROR, "Error establishing the connection: %d", (int)res);
	}
	return res;
}
AppMessageResult ancillary_app_message_outbox_send(void){
	AppMessageResult res = app_message_outbox_send();

	if(res != APP_MSG_OK ){
		APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the data: %d", (int)res);
	}

	return res;
}
