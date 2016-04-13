#include <pebble.h>
#include "keys.h"
#include "ancillary_methods.h"

#define BUS_STOP_DETAIL_NAME_TEXT_LENGTH (60)
#define BUS_STOP_DETAIL_NUMBER_TEXT_LENGTH (6)
#define BUS_STOP_DETAIL_LINES_MAX_ITEMS (10)
#define BUS_STOP_DETAIL_LINE_NAME_TEXT_LENGTH (10)
#define BUS_STOP_DETAIL_LINE_TIME_TEXT_LENGTH (20)

static struct BusStopDetailUi {
	Window *window;
	MenuLayer *menu_layer;
	TextLayer *feedback_text_layer;
} ui;

typedef struct {
	char name[BUS_STOP_DETAIL_LINE_NAME_TEXT_LENGTH];
	char bus1[BUS_STOP_DETAIL_LINE_TIME_TEXT_LENGTH];
	char bus2[BUS_STOP_DETAIL_LINE_TIME_TEXT_LENGTH];
} LineTimesItem;

typedef struct {
	char number[BUS_STOP_DETAIL_NUMBER_TEXT_LENGTH];
	char name[BUS_STOP_DETAIL_NAME_TEXT_LENGTH];
	LineTimesItem linesTimes[BUS_STOP_DETAIL_LINES_MAX_ITEMS];
	int number_of_lines;
} StopDetailItem;

static StopDetailItem s_stop_detail;

static uint16_t times_row_actual = 0;


void set_bus_stop_list_hidden(bool hidden) {

	layer_set_hidden(menu_layer_get_layer(ui.menu_layer), hidden);
}

void set_feedback_message_hidden(bool hidden) {

	layer_set_hidden(text_layer_get_layer(ui.feedback_text_layer), hidden);
}


static void line_list_append(char *name, char *bus1, char *bus2) {
	
	if (s_stop_detail.number_of_lines == BUS_STOP_DETAIL_LINES_MAX_ITEMS) {
		return;
	}
	
	strcpy(s_stop_detail.linesTimes[s_stop_detail.number_of_lines].name, name);
	strcpy(s_stop_detail.linesTimes[s_stop_detail.number_of_lines].bus1, bus1);
	strcpy(s_stop_detail.linesTimes[s_stop_detail.number_of_lines].bus2, bus2);
	s_stop_detail.number_of_lines++;


}

void bus_stop_detail_out_sent_handler(DictionaryIterator *sent, void *context) {
	// outgoing message was delivered
}


void bus_stop_detail_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
	// outgoing message failed
	
	text_layer_set_text(ui.feedback_text_layer, "Connection error.");
	set_bus_stop_list_hidden(true);
	set_feedback_message_hidden(false);
	
}


void bus_stop_detail_in_received_handler(DictionaryIterator *iter, void *context) {
	
	Tuple *append_line_tuple = dict_find(iter, TUSSAM_KEY_APPEND_LINE);
	Tuple *line_number_tuple = dict_find(iter, TUSSAM_KEY_LINE_NUMBER);
	Tuple *line_bus1_time_tuple = dict_find(iter, TUSSAM_KEY_BUS_1_TIME);
	Tuple *line_bus2_time_tuple = dict_find(iter, TUSSAM_KEY_BUS_2_TIME);
		
	if (append_line_tuple) {
		
		set_feedback_message_hidden(true);
		if (append_line_tuple->value->uint8 == 0) {
			s_stop_detail.number_of_lines = 0;
		}
		
		line_list_append(line_number_tuple->value->cstring, line_bus1_time_tuple->value->cstring, line_bus2_time_tuple->value->cstring);
		menu_layer_reload_data(ui.menu_layer);
	}
}


void bus_stop_detail_in_dropped_handler(AppMessageResult reason, void *context) {
	// incoming message dropped
	APP_LOG(APP_LOG_LEVEL_DEBUG, "in_dropped_handler");
}

static uint16_t menu_get_num_sections_callback(MenuLayer *me, void *data) {
	return 1;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *me, uint16_t section_index, void *data) {
	return s_stop_detail.number_of_lines + 1;
}

static int16_t menu_get_cell_height_callback(MenuLayer *me, MenuIndex* cell_index, void *data) {
	return 44;
}

static bool got_estimate(char *estimate) {
	
	return strcmp(estimate, "") != 0;
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
	
	
	if(cell_index->row == 0){
		GRect detail_rect = GRect(43, 0, 99, 42);
		GRect bus_stop_rect = GRect(2, 0, 45, 42);

		// Bus Stop Name
		graphics_draw_text_vertically_center(ctx, s_stop_detail.name, fonts_get_system_font(FONT_KEY_GOTHIC_14),
				detail_rect, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter);

		// Line Number
		graphics_draw_text_vertically_center(ctx, s_stop_detail.number, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD),
				bus_stop_rect, GTextOverflowModeWordWrap, GTextAlignmentCenter);
	}else{
		LineTimesItem lineTimeItem = s_stop_detail.linesTimes[(cell_index->row) - 1];
		
		bool got_estimate_1 = got_estimate(lineTimeItem.bus1);
		bool got_estimate_2 = got_estimate(lineTimeItem.bus2);
		
		graphics_context_set_text_color(ctx, GColorBlack);
		
		// Line Number
		graphics_draw_text(ctx, lineTimeItem.name, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(2, 5, 30, 24), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);

		if (got_estimate_1 && got_estimate_2) {

			// Time 1
			graphics_draw_text(ctx, lineTimeItem.bus1, fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(34, 0, 108, 19), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

			// Time 2
			graphics_draw_text(ctx, lineTimeItem.bus2, fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(34, 21, 108, 19), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

		} else if (got_estimate_1) {

			// Time 1
			graphics_draw_text(ctx, lineTimeItem.bus1, fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(34, 9, 108, 19), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

		} else if (got_estimate_2) {

			// Time 1
			graphics_draw_text(ctx, lineTimeItem.bus2, fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(34, 9, 108, 19), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

		} else {

			// No estimates
			graphics_draw_text(ctx, "No estimates.", fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(34, 9, 108, 19), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

		}
	}
	
}

static void time_row_selection_changed(struct MenuLayer *menu_layer,
        MenuIndex new_index,
        MenuIndex old_index,
        void *callback_context){

	layer_mark_dirty(menu_layer_get_layer(menu_layer));

	times_row_actual = new_index.row;
}

static void loadStopDetail(char *number) {
	
	DictionaryIterator *iter;
	
	if (app_message_outbox_begin(&iter) != APP_MSG_OK) {
		return;
	}
	if (dict_write_cstring(iter, TUSSAM_KEY_FETCH_STOP_DETAIL, number) != DICT_OK) {
		return;
	}
	app_message_outbox_send();
}

// This initializes the menu upon window load
static void bus_stop_detail_window_load(Window *window) {
	
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_frame(window_layer);
	
	ui.menu_layer = menu_layer_create(bounds);
	menu_layer_set_callbacks(ui.menu_layer, NULL, (MenuLayerCallbacks){
		.get_num_sections = menu_get_num_sections_callback,
		.get_cell_height = menu_get_cell_height_callback,
		.get_num_rows = menu_get_num_rows_callback,
		.draw_row = menu_draw_row_callback,
		.selection_changed = time_row_selection_changed,
	});
#ifdef PBL_COLOR
	menu_layer_pad_bottom_enable(ui.menu_layer,false);
	menu_layer_set_highlight_colors(ui.menu_layer,GColorVividCerulean,GColorWhite);
#endif
	menu_layer_set_click_config_onto_window(ui.menu_layer, ui.window);
	layer_add_child(window_layer, menu_layer_get_layer(ui.menu_layer));
	
	// Feedback Text Layer
	ui.feedback_text_layer = text_layer_create(bounds);
	text_layer_set_text_color(ui.feedback_text_layer, GColorBlack);
	text_layer_set_font(ui.feedback_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	text_layer_set_text_alignment(ui.feedback_text_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(ui.feedback_text_layer));
	

	text_layer_set_text(ui.feedback_text_layer, "Loading times...");
	loadStopDetail(s_stop_detail.number);
}

// Deinitialize resources on window unload that were initialized on window load
void bus_stop_detail_window_unload(Window *window) {
	menu_layer_destroy(ui.menu_layer);
	text_layer_destroy(ui.feedback_text_layer); // Originally not exist. After put that the aplication close going back
	s_stop_detail.number_of_lines = 0;
}

void bus_stop_detail_window_appear(Window *window) {
	
	app_message_register_inbox_received(bus_stop_detail_in_received_handler);
	app_message_register_inbox_dropped(bus_stop_detail_in_dropped_handler);
	app_message_register_outbox_sent(bus_stop_detail_out_sent_handler);
	app_message_register_outbox_failed(bus_stop_detail_out_failed_handler);
	
}


void bus_stop_detail_show(char *number, char *name) {
	
	strcpy(s_stop_detail.name, name);
	strcpy(s_stop_detail.number, number);
	
	window_stack_push(ui.window, true /* Animated */);
}

void bus_stop_detail_init(void) {
	ui.window = window_create();
	
	// Setup the window handlers
	window_set_window_handlers(ui.window, (WindowHandlers) {
		.load = bus_stop_detail_window_load,
		.unload = bus_stop_detail_window_unload,
		.appear = bus_stop_detail_window_appear,
	});
}

void bus_stop_detail_deinit(void) {
  window_destroy(ui.window);
}
