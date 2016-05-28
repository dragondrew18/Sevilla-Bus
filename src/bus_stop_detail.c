#include <pebble.h>
#include "keys.h"
#include "data.h"
#include "ancillary_methods.h"

static struct BusStopDetailUi {
	Window *window;
	MenuLayer *menu_layer;
	TextLayer *feedback_text_layer;
} ui;

static uint16_t times_row_actual = 0;

int counter = 0;

bool in_view = false;


void set_bus_stop_list_hidden(bool hidden) {
	APP_LOG(APP_LOG_LEVEL_INFO, "Crash-01");

	layer_set_hidden(menu_layer_get_layer(ui.menu_layer), hidden);
}

void set_feedback_message_hidden(bool hidden) {
	APP_LOG(APP_LOG_LEVEL_INFO, "Crash-02");

	layer_set_hidden(text_layer_get_layer(ui.feedback_text_layer), hidden);
}


//static void line_list_append(char *name, char *bus1, char *bus2) {
//
//	if (s_stop_detail.number_of_lines == BUS_STOP_DETAIL_LINES_MAX_ITEMS) {
//		return;
//	}
//
//	strcpy(s_stop_detail.linesTimes[s_stop_detail.number_of_lines].name, name);
//	strcpy(s_stop_detail.linesTimes[s_stop_detail.number_of_lines].bus1, bus1);
//	strcpy(s_stop_detail.linesTimes[s_stop_detail.number_of_lines].bus2, bus2);
//	s_stop_detail.number_of_lines++;
//
//
//}

//void bus_stop_detail_out_sent_handler(DictionaryIterator *sent, void *context) {
//	// outgoing message was delivered
//}
//
//
//void bus_stop_detail_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
//	// outgoing message failed
//
//	text_layer_set_text(ui.feedback_text_layer, "Connection error.");
//	set_bus_stop_list_hidden(true);
//	set_feedback_message_hidden(false);
//
//}


//void bus_stop_detail_in_received_handler(DictionaryIterator *iter, void *context) {
//
//	Tuple *append_line_tuple = dict_find(iter, TUSSAM_KEY_APPEND_LINE);
//	Tuple *line_number_tuple = dict_find(iter, TUSSAM_KEY_LINE_NUMBER);
//	Tuple *line_bus1_time_tuple = dict_find(iter, TUSSAM_KEY_BUS_1_TIME);
//	Tuple *line_bus2_time_tuple = dict_find(iter, TUSSAM_KEY_BUS_2_TIME);
//
//	if (append_line_tuple) {
//
//		set_feedback_message_hidden(true);
//		if (append_line_tuple->value->uint8 == 0) {
//			s_stop_detail.number_of_lines = 0;
//		}
//
//		line_list_append(line_number_tuple->value->cstring, line_bus1_time_tuple->value->cstring, line_bus2_time_tuple->value->cstring);
//		menu_layer_reload_data(ui.menu_layer);
//	}
//}


//void bus_stop_detail_in_dropped_handler(AppMessageResult reason, void *context) {
//	// incoming message dropped
//	APP_LOG(APP_LOG_LEVEL_DEBUG, "in_dropped_handler");
//}

static uint16_t menu_get_num_sections_callback(MenuLayer *me, void *data) {
	APP_LOG(APP_LOG_LEVEL_INFO, "Crash-03");
	return 1;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *me, uint16_t section_index, void *data) {
	APP_LOG(APP_LOG_LEVEL_INFO, "Voy a mostrar el número de líneas");
	APP_LOG(APP_LOG_LEVEL_INFO, "Crash-04");
//	APP_LOG(APP_LOG_LEVEL_INFO, "Actual number of lines: %d", get_bus_stop_detail().number_of_lines);
	if(in_view){
		return get_bus_stop_detail().number_of_lines + 1;
	}else{
		return 0;
	}
//	return 1;
}

static int16_t menu_get_cell_height_callback(MenuLayer *me, MenuIndex* cell_index, void *data) {
	APP_LOG(APP_LOG_LEVEL_INFO, "Crash-05");
	return 44;
}

static bool got_estimate(char *estimate) {
	APP_LOG(APP_LOG_LEVEL_INFO, "Crash-06");

	return strcmp(estimate, "") != 0;
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
	counter += 1;

	int a;

	APP_LOG(APP_LOG_LEVEL_INFO, "Crash-07");
//	APP_LOG(APP_LOG_LEVEL_INFO, "Counter: %d",counter);
	
	a = cell_index->row;
	APP_LOG(APP_LOG_LEVEL_INFO, "Falla aqui ! !. Al pedir los datos nombre y número de la parada");


//	APP_LOG(APP_LOG_LEVEL_INFO, "cell_index->row: %d", a);
//	APP_LOG(APP_LOG_LEVEL_INFO, "cell_index->row == 0: %d", a == 0);



	if(a == 0){
		APP_LOG(APP_LOG_LEVEL_INFO, "cell_index is 0");

//		GRect detail_rect = GRect(43, 0, 99, 42);
		GRect detail_rect = GRect(48, 0, 93, 42);
		GRect bus_stop_rect = GRect(2, 0, 45, 42);


//		graphics_draw_text(ctx, get_bus_stop_detail().name, fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(34, 0, 108, 19), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
//		graphics_draw_text(ctx, details.number, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), GRect(2, 5, 30, 24), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);

//
////		// Bus Stop Name
//		graphics_draw_text_vertically_center(ctx, get_bus_stop_detail().name, fonts_get_system_font(FONT_KEY_GOTHIC_14),
//				detail_rect, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter);
//
////		// Line Number
//		graphics_draw_text_vertically_center(ctx, get_bus_stop_detail().number, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD),
//				bus_stop_rect, GTextOverflowModeWordWrap, GTextAlignmentCenter);
	}else{
		APP_LOG(APP_LOG_LEVEL_INFO, "cell_index not is 0");
		LineTimesItem lineTimeItem = get_bus_stop_detail().linesTimes[(cell_index->row) - 1];

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
	APP_LOG(APP_LOG_LEVEL_INFO, "end_Crash-07");

	
}

static void time_row_selection_changed(struct MenuLayer *menu_layer,
        MenuIndex new_index,
        MenuIndex old_index,
        void *callback_context){
	APP_LOG(APP_LOG_LEVEL_INFO, "Crash-08");

	layer_mark_dirty(menu_layer_get_layer(menu_layer));

	times_row_actual = new_index.row;
}

//static void loadStopDetail(char *number) {
//
//	DictionaryIterator *iter;
//
//	if (app_message_outbox_begin(&iter) != APP_MSG_OK) {
//		return;
//	}
//	if (dict_write_cstring(iter, TUSSAM_KEY_FETCH_STOP_DETAIL, number) != DICT_OK) {
//		return;
//	}
//	app_message_outbox_send();
//}

// This initializes the menu upon window load
static void bus_stop_detail_window_load(Window *window) {
	APP_LOG(APP_LOG_LEVEL_INFO, "Crash-09");
	APP_LOG(APP_LOG_LEVEL_INFO, "Empezando a cargar la vista...");
	
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
	APP_LOG(APP_LOG_LEVEL_INFO, "pre definición de colores");

#ifdef PBL_COLOR
	menu_layer_pad_bottom_enable(ui.menu_layer,false);
	menu_layer_set_highlight_colors(ui.menu_layer,GColorVividCerulean,GColorWhite);
#endif
	menu_layer_set_click_config_onto_window(ui.menu_layer, ui.window);
	layer_add_child(window_layer, menu_layer_get_layer(ui.menu_layer));
	
	APP_LOG(APP_LOG_LEVEL_INFO, "Cargado el menu");


	// Feedback Text Layer
	GRect feedback_grect = bounds;
	feedback_grect.origin.y = bounds.size.h / 4 + 5;
	ui.feedback_text_layer = text_layer_create(feedback_grect);
	text_layer_set_text_color(ui.feedback_text_layer, GColorBlack);
	text_layer_set_font(ui.feedback_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	text_layer_set_text_alignment(ui.feedback_text_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(ui.feedback_text_layer));
	
	APP_LOG(APP_LOG_LEVEL_INFO, "Cargado el feedback");

	text_layer_set_text(ui.feedback_text_layer, "Loading times...");

	APP_LOG(APP_LOG_LEVEL_INFO, "fin carga de la vista");
}

// Deinitialize resources on window unload that were initialized on window load
void bus_stop_detail_window_unload(Window *window) {
	APP_LOG(APP_LOG_LEVEL_INFO, "Crash-10");
	menu_layer_destroy(ui.menu_layer);
	text_layer_destroy(ui.feedback_text_layer); // Originally not exist. After put that the aplication close going back
	// get_bus_stop_detail.number_of_lines = 0;
}


void reload_details_menu(void){
	layer_mark_dirty(menu_layer_get_layer(ui.menu_layer));
	menu_layer_reload_data(ui.menu_layer);

//	hide_feedback_layers(true);
//	set_bus_stop_list_hidden(false);
//	set_feedback_message_hidden(true);
}

void bus_stop_detail_window_appear(Window *window) {
	APP_LOG(APP_LOG_LEVEL_WARNING, "In bus stop detail view");
	in_view = true;
	reload_details_menu();
//	app_message_register_inbox_received(bus_stop_detail_in_received_handler);
//	app_message_register_inbox_dropped(bus_stop_detail_in_dropped_handler);
//	app_message_register_outbox_sent(bus_stop_detail_out_sent_handler);
//	app_message_register_outbox_failed(bus_stop_detail_out_failed_handler);
	
}


void bus_stop_detail_show(char *number, char *name) {
	APP_LOG(APP_LOG_LEVEL_WARNING, "bus_stop_detail_show");
	
	define_stop_detail(number, name);
	set_actual_view(Details);
	
	APP_LOG(APP_LOG_LEVEL_WARNING, "definido el numero y nombre");

	window_stack_push(ui.window, true /* Animated */);

	APP_LOG(APP_LOG_LEVEL_WARNING, "OK -> window_stack_push");

	layer_mark_dirty(menu_layer_get_layer(ui.menu_layer));

	APP_LOG(APP_LOG_LEVEL_WARNING, "OK -> layer_mark_dirty(menu_layer_get_layer");

	menu_layer_reload_data(ui.menu_layer);
	APP_LOG(APP_LOG_LEVEL_WARNING, "Recargada la vista Ok");
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


void update_loading_feedback_details(bool loaded){
	if(loaded == true && get_bus_list_num_of_items() > 0){
		set_bus_stop_list_hidden(false);
		set_feedback_message_hidden(true);
	}else if(loaded == true && get_bus_list_num_of_items() < 1){
		// hide_feedback_layers(false);
		set_bus_stop_list_hidden(true);
		set_feedback_message_hidden(false);
		text_layer_set_text(ui.feedback_text_layer,"No favorite bus stops.\n\n Search it !.");
	}else{
		set_bus_stop_list_hidden(true);
		set_feedback_message_hidden(false);
		// show_loading_feedback();
	}
}
