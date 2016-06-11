#include <pebble.h>
#include "bus_stop_detail.h"
#include "bus_stop_number_select.h"
#include "ancillary_methods.h"
#include "keys.h"
#include "data.h"

#define VIEW_SYMBOL_PLUS_SIZE (25)
#define VIEW_SYMBOL_LENS_RADIUS (9)
#define WAIT_RESPONSE (1500)

static struct BusStopListUi {
	Window *window;
	TextLayer *feedback_text_layer;
	MenuLayer *bus_stop_menu_layer;
} ui;

static uint16_t bus_stop_row_actual;

void bus_stop_show_near(); // Declaration
void bus_stop_show_favorites(); // Declaration
static void show_loading_feedback(); // Declaration
void bus_stop_show_favorites_return();

//static int load_In_Progress = -1;

// static int bus_stop_list_num_of_items = 0;
// static int bus_stop_list_active_item = -1;

static int test_iter = 0;

ClickConfigProvider previous_ccp;

void set_list_type_to_near(void *context) {
	set_actual_view(Near);
}

static void hide_bus_stop_detail_layers(bool hide) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "hide_bus_stop_detail_layers( %d )", hide);
	layer_set_hidden(menu_layer_get_layer(ui.bus_stop_menu_layer), hide);
}

static void hide_feedback_layers(bool hide) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "hide_feedback_layers( %d )", hide);
	layer_set_hidden(text_layer_get_layer(ui.feedback_text_layer), hide);
}

//static void bus_stop_scroll_append(char *number, char *name, char *lines, int favorite) {
//
//	if (bus_stop_list_num_of_items == BUS_STOP_LIST_MAX_ITEMS) {
//		return;
//	}
//
//	if(load_In_Progress == Near){
//		strcpy(bus_stop_list_near[bus_stop_list_num_of_items].number, number);
//		strcpy(bus_stop_list_near[bus_stop_list_num_of_items].name, name);
//		strcpy(bus_stop_list_near[bus_stop_list_num_of_items].lines, lines);
//		bus_stop_list_near[bus_stop_list_num_of_items].favorite = favorite == 1;
//	}else if(load_In_Progress == Favorites){
//		strcpy(bus_stop_list_favorites[bus_stop_list_num_of_items].number, number);
//		strcpy(bus_stop_list_favorites[bus_stop_list_num_of_items].name, name);
//		strcpy(bus_stop_list_favorites[bus_stop_list_num_of_items].lines, lines);
//		bus_stop_list_favorites[bus_stop_list_num_of_items].favorite = favorite == 1;
//	}
//	bus_stop_list_num_of_items++;
//
//	if(listType == Near && bus_stop_list_num_of_items == 1){
//		vibes_short_pulse();
//	}
//
//	hide_feedback_layers(true);
//	hide_bus_stop_detail_layers(false);
//	menu_layer_reload_data(ui.bus_stop_menu_layer);
//}

//static void bus_stop_scroll_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
//	// outgoing message failed
//
//	text_layer_set_text(ui.feedback_text_layer, "Connection error.");
////	hide_bus_stop_detail_layers(true);
//	hide_bus_stop_detail_layers(false);
//	hide_feedback_layers(false);
//}

//static void nullable_load_in_progress(){
//	load_In_Progress = -1;
//}

//static void update_load_in_progress(){
//	if (timer_load_in_progress == NULL){
//		timer_load_in_progress = app_timer_register(WAIT_RESPONSE, nullable_load_in_progress, NULL);
//	}else{
//		app_timer_reschedule(timer_load_in_progress, WAIT_RESPONSE);
//	}
//}

// When receive data do that
//static void bus_stop_scroll_in_received_handler(DictionaryIterator *iter, void *context) {
//
//	APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "data received !");
//
//	Tuple *append_stop_tuple = dict_find(iter, TUSSAM_KEY_APPEND_STOP);
//	Tuple *stop_number_tuple = dict_find(iter, TUSSAM_KEY_STOP_NUMBER);
//	Tuple *stop_name_tuple = dict_find(iter, TUSSAM_KEY_STOP_NAME);
//	Tuple *stop_lines_tuple = dict_find(iter, TUSSAM_KEY_STOP_LINES);
//	Tuple *stop_favorite = dict_find(iter, TUSSAM_KEY_STOP_FAVORITE);
//	Tuple *no_bus_stops = dict_find(iter, TUSSAM_KEY_NO_BUS_STOPS);
//	Tuple *ready_tuple = dict_find(iter, AppKeyJSReady);
//
//	if (ready_tuple){
//		APP_LOG(APP_LOG_LEVEL_INFO, "Ready tuple received");
//		set_JS_is_ready(true);
//		response_received();
//		return;
//	}
//
//	if (no_bus_stops) {
//
//		// hide_bus_stop_detail_layers(true);
//		hide_feedback_layers(false);
//
//		if (get_actual_view() == Favorites) {
//			text_layer_set_text(ui.feedback_text_layer,"No favorite bus stops.\n\n Search it !.");
////			text_layer_set_text(ui.feedback_text_layer,"No favorite bus stops.\n\nLong press in times button to add/remove favorites.");
//		} else {
//			text_layer_set_text(ui.feedback_text_layer,"No nearby bus stops.");
//		}
//		load_In_Progress = -1;
//	} else if (append_stop_tuple) {
//		// update_load_in_progress();
//		bus_stop_scroll_append(stop_number_tuple->value->cstring, stop_name_tuple->value->cstring, stop_lines_tuple->value->cstring, stop_favorite->value->int8);
//	}
//	// menu_layer_reload_data(ui.bus_stop_menu_layer);
//	response_received();
//
//}

void reload_menu(void){
	layer_mark_dirty(menu_layer_get_layer(ui.bus_stop_menu_layer));
	menu_layer_reload_data(ui.bus_stop_menu_layer);

	hide_feedback_layers(true);
}

//static void loadFavoritesStops(void) {
//
//	DictionaryIterator *iter;
//
//	send_message(&iter, TUSSAM_KEY_FAVORITES,1);
////	AppMessageResult res = app_message_outbox_begin(&iter);
////
////	if (res != APP_MSG_OK) {
////		// Error establishing the connection
////		APP_LOG(APP_LOG_LEVEL_ERROR, "Error establishing the connection: %d", (int)res);
////	}
////	if (dict_write_uint8(iter, TUSSAM_KEY_FAVORITES, 1) != DICT_OK) {
////		// Error writing data petition
////		return;
////	}
////	res = app_message_outbox_send();
////	if(res != APP_MSG_OK ){
////		APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the data: %d", (int)res);
////		return;
////	}else{
////		APP_LOG(APP_LOG_LEVEL_INFO, "Message succesful sent!");
////	}
//}

//static void loadNearStops(void) {
//
//	DictionaryIterator *iter;
//
//	send_message(&iter, TUSSAM_KEY_NEAR,1);
//
////	if (app_message_outbox_begin(&iter) != APP_MSG_OK) {
////		return;
////	}
////	if (dict_write_uint8(iter, TUSSAM_KEY_NEAR, 1) != DICT_OK) {
////		return;
////	}
////	app_message_outbox_send();
//}

//static void execute_when_is_ready_true(){
//	if(get_JS_is_ready() && load_In_Progress < 0){
//		timer_execute_when_is_ready_true = NULL;
//		if(listType == Near){
//			load_In_Progress = Near;
//			loadNearStops();
//		}else{
//			load_In_Progress = Favorites;
//			loadFavoritesStops();
//		}
//	}else{
//		waiting_ready_attempts++;
//		if (timer_execute_when_is_ready_true == NULL){
//			timer_execute_when_is_ready_true = app_timer_register(550, execute_when_is_ready_true, NULL);
//		}else{
//			app_timer_reschedule(timer_execute_when_is_ready_true, 550);
//		}
//		APP_LOG(APP_LOG_LEVEL_INFO, "Waiting is_ready %dms(nº%d)...", (int) 550, waiting_ready_attempts);
//	}
//}

static void clean_menu(){

}

static void add_remove_bus_stop_to_favorites() {
	
	uint32_t key;
	BusStopListItem *stopListItem = NULL;
	if(get_actual_view() == Favorites)
		stopListItem = get_bus_stop_list_favorites_at_index(bus_stop_row_actual - 1);
	else if (get_actual_view() == Near)
		stopListItem = get_bus_stop_list_near_at_index(bus_stop_row_actual - 1);
	
	if (stopListItem->favorite) {
		key = TUSSAM_KEY_REMOVE_FAVORITE;
	} else {
		key = TUSSAM_KEY_ADD_FAVORITE;
	}
	
	DictionaryIterator *iter;
	
	if (ancillary_app_message_outbox_begin(&iter) != APP_MSG_OK) {
		return;
	}
	if (dict_write_cstring(iter, key, stopListItem->number) != DICT_OK) {
		return;
	}
	ancillary_app_message_outbox_send();
	
	stopListItem->favorite = !stopListItem->favorite;
	// display_bus_stop_at_index(bus_stop_scroll_active_item);
	//bus_stop_scroll_show_favorites();
	hide_bus_stop_detail_layers(false);
	hide_feedback_layers(true);

}

static void select2_click_handler(struct MenuLayer *menu_layer,
        MenuIndex *cell_index,
        void *callback_context) {

	APP_LOG(APP_LOG_LEVEL_DEBUG, "select_click_handler");
	if(bus_stop_row_actual == 0){
		if(get_actual_view() == Favorites){
			bus_stop_show_near();
			//bus_stop_list_num_of_items = 0;
		}else if (get_actual_view() == Near)
			win_edit_show();
	}else{
		BusStopListItem *stopListItem = NULL;
		if(get_actual_view() == Favorites)
			stopListItem = get_bus_stop_list_favorites_at_index(bus_stop_row_actual - 1);
		else if (get_actual_view() == Near)
			stopListItem = get_bus_stop_list_near_at_index(bus_stop_row_actual - 1);
		bus_stop_detail_show(stopListItem->number, stopListItem->name);
	}
}

static void select2_long_click_handler(struct MenuLayer *menu_layer,
        MenuIndex *cell_index,
        void *callback_context) {

	APP_LOG(APP_LOG_LEVEL_DEBUG, "select_long_click_handler");
	vibes_short_pulse();
	if(bus_stop_row_actual == 0){

	}else{
		hide_feedback_layers(false);
		add_remove_bus_stop_to_favorites();
	}
	layer_mark_dirty(menu_layer_get_layer(ui.bus_stop_menu_layer));

}

static uint16_t menu2_get_num_sections_callback(MenuLayer *me, void *data) {
	return 1;
}

static uint16_t menu2_get_num_rows_callback(MenuLayer *me, uint16_t section_index, void *data) {
		return get_bus_list_num_of_items() + 1;
}

static int16_t menu2_get_cell_height_callback(MenuLayer *me, MenuIndex* cell_index, void *data) {
	return 44;
}

static void click_back_action(ClickRecognizerRef recognizer, void *context) {

	if(get_actual_view() == Near){
		window_stack_pop(true);
		// set_actual_view(Favorites);
		// show_loading_feedback();
		// clean_menu();
		// bus_stop_list_num_of_items = 0;

		// menu_layer_reload_data(ui.bus_stop_menu_layer);
		bus_stop_show_favorites_return();
	} else
		window_stack_pop_all(true);
}

static void force_back_button(void *context){
	previous_ccp(context);

	window_single_click_subscribe(BUTTON_ID_BACK, click_back_action);
}

static void menu2_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {

	BusStopListItem *act_bus_stop;

	if(get_actual_view() == Near){
		act_bus_stop = get_bus_stop_list_near_at_index(cell_index->row);
	}else{
		act_bus_stop = get_bus_stop_list_favorites_at_index(cell_index->row);
	}

	graphics_context_set_text_color(ctx, GColorBlack);
#ifdef PBL_RECT
	GRect detail_rect = GRect(48, 0, 93, 42);
	GRect bus_stop_rect = GRect(2, 0, 45, 42);
#else
	GRect detail_rect = GRect(48, 0, 128, 42);
	GRect bus_stop_rect = GRect(2, 0, 45, 42);
#endif
	// Layer *window_layer = window_get_root_layer(ui.window);
	// GRect bounds = layer_get_frame(window_layer);


	if(cell_index->row == 0){
		if(menu_cell_layer_is_highlighted(cell_layer))
			graphics_context_set_stroke_color(ctx,PBL_IF_COLOR_ELSE(GColorBlack, GColorWhite));
		else
			graphics_context_set_stroke_color(ctx,PBL_IF_COLOR_ELSE(GColorBlue, GColorBlack));
		graphics_context_set_stroke_width(ctx,2);

		// GColorVividCerulean

		int center_x = (detail_rect.size.w + detail_rect.origin.x) / 2;
		int center_y = (detail_rect.size.h + detail_rect.origin.y) / 2;

		if(get_actual_view() == Near){
			graphics_draw_line(ctx,
					GPoint(center_x, center_y - VIEW_SYMBOL_PLUS_SIZE/2),
					GPoint(center_x, center_y + VIEW_SYMBOL_PLUS_SIZE/2));
			graphics_draw_line(ctx,
					GPoint(center_x - VIEW_SYMBOL_PLUS_SIZE/2, center_y),
					GPoint(center_x + VIEW_SYMBOL_PLUS_SIZE/2, center_y));
		}else{
			int dif_radius = VIEW_SYMBOL_LENS_RADIUS/2 + (VIEW_SYMBOL_LENS_RADIUS/2)/2;
			center_x = center_x - (VIEW_SYMBOL_LENS_RADIUS)/2;
			center_y = center_y - (VIEW_SYMBOL_LENS_RADIUS)/2;
			graphics_draw_circle(ctx, GPoint(center_x, center_y),
					VIEW_SYMBOL_LENS_RADIUS);
			graphics_draw_line(ctx, GPoint(center_x + dif_radius
					, center_y + dif_radius),
					GPoint(center_x + dif_radius + VIEW_SYMBOL_LENS_RADIUS,
							center_y + dif_radius + VIEW_SYMBOL_LENS_RADIUS));
		}
	}else{


		if(get_actual_view() == Near)
			act_bus_stop = get_bus_stop_list_near_at_index(cell_index->row -1);
		else
			act_bus_stop = get_bus_stop_list_favorites_at_index(cell_index->row -1);

		if(// cell_index->row == bus_stop_row_actual
				menu_cell_layer_is_highlighted(cell_layer)){
			// Bus Stop Lines
			graphics_context_set_text_color(ctx, PBL_IF_COLOR_ELSE(GColorBlack, GColorWhite));
//			text_layer_set_text_color(ctx, GColorWhite);
//			graphics_context_set_text_color();

			graphics_draw_text_vertically_center(ctx, act_bus_stop->lines, fonts_get_system_font(FONT_KEY_GOTHIC_18),
					detail_rect, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter);
		}else{
//			GSize size = graphics_text_layout_get_content_size(text, font, box,
//						overflow_mode, alignment);
//			GRect box_2 = box;
//			box_2.origin.y = (box.size.h - size.h)/2 + box.origin.y - 2;
//
//			graphics_draw_text(ctx, text, font, box_2, overflow_mode, alignment, NULL);
			// Bus Stop Name
			graphics_draw_text_vertically_center(ctx, act_bus_stop->name, fonts_get_system_font(FONT_KEY_GOTHIC_14),
					detail_rect, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter);
		}

		// Line Number

		GRect rect2 = bus_stop_rect;
		if(act_bus_stop->favorite == true){
			rect2.origin.y += rect2.size.h/8;

			rect2.size.h -= rect2.size.h/4;
			if(menu_cell_layer_is_highlighted(cell_layer)){
				graphics_context_set_text_color(ctx, PBL_IF_COLOR_ELSE(GColorRed, GColorBlack));
				graphics_context_set_fill_color(ctx, GColorWhite);
			}else{
				graphics_context_set_text_color(ctx, GColorWhite);
				graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorFolly, GColorBlack));
			}
			graphics_fill_rect(ctx, rect2, 4, GCornersAll);
		}

		graphics_draw_text_vertically_center(ctx, act_bus_stop->number, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD),
				bus_stop_rect, GTextOverflowModeWordWrap, GTextAlignmentCenter);

		//draw_layout_border(ctx, bus_stop_rect, 1, 2, GColorFashionMagenta);
	}

}

void update_loading_feedback_favorites(bool loaded){
	if(loaded == true && get_bus_list_num_of_items() > 0){
		hide_feedback_layers(true);
	}else if(loaded == true && get_bus_list_num_of_items() < 1){
		hide_feedback_layers(false);
		if (get_actual_view() == Favorites) {
			text_layer_set_text(ui.feedback_text_layer,"No favorite bus stops.\n\n Search it !.");
		} else {
			text_layer_set_text(ui.feedback_text_layer,"No nearby bus stops.");
		}
	}else{
		show_loading_feedback();
	}
}


static void show_loading_feedback() {
	
	if (get_actual_view() == Favorites) {
		text_layer_set_text(ui.feedback_text_layer,"Loading favorite bus stops...");
	} else {
		text_layer_set_text(ui.feedback_text_layer,"Loading nearby bus stops...");
	}

	// hide_bus_stop_detail_layers(true);
	hide_feedback_layers(false);
}

static void menu2_in_row_bus_stop(uint16_t old_index, uint16_t new_index){
	bus_stop_row_actual = new_index;
}

static void menu2_row_selection_changed(struct MenuLayer *menu_layer,
        MenuIndex new_index,
        MenuIndex old_index,
        void *callback_context){

	layer_mark_dirty(menu_layer_get_layer(menu_layer));

	menu2_in_row_bus_stop(old_index.row, new_index.row);
}

// This initializes the menu upon window load
static void bus_stop_window_load(Window *window) {
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "bus_stop_window_load");
	
	ui.window = window;
	
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_frame(window_layer);

	// Windows are 152(x) x 144(y)

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Tamaño ventana (origen): %d, %d", bounds.origin.x, bounds.origin.y);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Tamaño ventana (size): %d, %d", bounds.size.h, bounds.size.w);

	//MenuLayer

	ui.bus_stop_menu_layer = menu_layer_create(bounds);

	menu_layer_set_callbacks(ui.bus_stop_menu_layer, NULL, (MenuLayerCallbacks){
		.get_num_sections = menu2_get_num_sections_callback,
		.get_cell_height = menu2_get_cell_height_callback,
		.get_num_rows = menu2_get_num_rows_callback,
		.draw_row = menu2_draw_row_callback,
		.selection_changed = menu2_row_selection_changed,
		.select_click = select2_click_handler,
		.select_long_click= select2_long_click_handler,
//		.select_click = select2_long_click_handler,
//		.select_long_click= select2_click_handler,
	});
	#ifdef PBL_COLOR
		menu_layer_pad_bottom_enable(ui.bus_stop_menu_layer,false);
		menu_layer_set_highlight_colors(ui.bus_stop_menu_layer,GColorVividCerulean,GColorWhite);
	#endif
	layer_add_child(window_layer, menu_layer_get_layer(ui.bus_stop_menu_layer));

	menu_layer_set_click_config_onto_window(ui.bus_stop_menu_layer, ui.window);

	// Feedback Text Layer

	GRect feedback_grect = bounds;
	feedback_grect.origin.y = bounds.size.h / 4 + 5;

	ui.feedback_text_layer = text_layer_create(feedback_grect);
	text_layer_set_text_color(ui.feedback_text_layer, GColorBlack);
	text_layer_set_font(ui.feedback_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	text_layer_set_text_alignment(ui.feedback_text_layer, GTextAlignmentCenter);

	layer_add_child(window_layer, text_layer_get_layer(ui.feedback_text_layer));
	show_loading_feedback();

	//force_back_button(ui.window, ui.bus_stop_menu_layer);
	previous_ccp = window_get_click_config_provider(ui.window);
	window_set_click_config_provider_with_context(ui.window, force_back_button, ui.bus_stop_menu_layer);

}


// Deinitialize resources on window unload that were initialized on window load
static void bus_stop_window_unload(Window *window) {
	
	menu_layer_destroy(ui.bus_stop_menu_layer);

	text_layer_destroy(ui.feedback_text_layer);

	// bus_stop_list_num_of_items = 0;
	// bus_stop_list_active_item = -1;
}

static void bus_stop_window_appear(Window *window) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "bus_stop_window_appear");
	test_iter++;
	//bus_stop_list_num_of_items = 0;
	APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "This windows appear... %d times", test_iter);
	layer_mark_dirty(menu_layer_get_layer(ui.bus_stop_menu_layer));
	menu_layer_reload_data(ui.bus_stop_menu_layer);
//	if(get_actual_view() == Near){
//		show_loading_feedback();
////		execute_when_is_ready_true();
////				if(get_actual_view() == Near){
////					set_actual_view(Near);
////					// loadNearStops();
////				}else{
////					set_actual_view(Favorites);
////					// loadFavoritesStops();
////				}
//	}
}

static void bus_stop_window_disappear(Window *window) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "bus_stop_window_disappear");
	test_iter++;
	APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "This windows disappear... %d times", test_iter);

//	listType = Favorites;
	layer_mark_dirty(menu_layer_get_layer(ui.bus_stop_menu_layer));
	menu_layer_reload_data(ui.bus_stop_menu_layer);
}

static void load_view_for_bus_stops_type(int _listType) {
	
	// listType = _listType;
	set_actual_view(_listType);

	// register_app_message_callbacks();

	window_stack_push(ui.window, true /* Animated */);
}

void bus_stop_show_favorites(void) { // Usado para la primera carga
	
	load_view_for_bus_stops_type(Favorites);
}

void bus_stop_show_favorites_return(void) {

	//load_view_for_bus_stops_type(Favorites);

	set_actual_view(Favorites);
//	register_app_message_callbacks();
//	show_loading_feedback();
//	execute_when_is_ready_true();
//			if(get_actual_view() == Near){
//				set_actual_view(Near);
//				// loadNearStops();
//			}else{
//				set_actual_view(Favorites);
//				// loadFavoritesStops();
//			}

	clean_menu();
	layer_mark_dirty(menu_layer_get_layer(ui.bus_stop_menu_layer));
	menu_layer_reload_data(ui.bus_stop_menu_layer);
	window_stack_push(ui.window, false /* Animated */);
}

void favorites_bus_stop_init(void) {
	
	ui.window = window_create();
	
	// Setup the window handlers
	window_set_window_handlers(ui.window, (WindowHandlers) {
		.load = bus_stop_window_load,
		.unload = bus_stop_window_unload,
		.appear = bus_stop_window_appear,
		.disappear = bus_stop_window_disappear
	});
	bus_stop_show_favorites();
	// window_stack_push(ui.window, true /* Animated */);
}

void favorites_bus_stop_deinit(void) {
	window_destroy(ui.window);
}

void bus_stop_show_near_seg(void) {
//	set_actual_view(Near);
	// execute_when_is_ready_true();
//			if(get_actual_view() == Near){
//				set_actual_view(Near);
//				// loadNearStops();
//			}else{
//				set_actual_view(Favorites);
//				// loadFavoritesStops();
//			}
	clean_menu();
	load_view_for_bus_stops_type(Near);
	show_loading_feedback();

	layer_mark_dirty(menu_layer_get_layer(ui.bus_stop_menu_layer));
	menu_layer_reload_data(ui.bus_stop_menu_layer);
//	window_stack_pop(false);
//	window_stack_push(ui.window, true /* Animated */);
}

void bus_stop_show_near(void) {
//	//bus_stop_window_unload(ui.window);
//	//favorites_bus_stop_deinit();
//	listType = Near;
//	//favorites_bus_stop_init();
//	loadNearStops();
//	layer_mark_dirty(menu_layer_get_layer(ui.bus_stop_menu_layer));
//	menu_layer_reload_data(ui.bus_stop_menu_layer);
//	load_view_for_bus_stops_type(Near);
//	menu_layer_reload_data(ui.bus_stop_menu_layer);
//	//loadNearStops();
//	layer_mark_dirty(menu_layer_get_layer(ui.bus_stop_menu_layer));

	bus_stop_show_near_seg();
}


