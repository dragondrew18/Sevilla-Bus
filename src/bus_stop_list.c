#include <pebble.h>
#include "bus_stop_detail.h"
#include "bus_stop_number_select.h"
#include "ancillary_methods.h"
#include "communication.h"
#include "keys.h"
#include "data.h"


// - + - + - Common Variables - + - + -
#define VIEW_SYMBOL_PLUS_SIZE (25)
#define VIEW_SYMBOL_LENS_RADIUS (9)
#define WAIT_RESPONSE (1500)

static struct BusStopListUi {
	Window *window;
	TextLayer *feedback_text_layer;
	MenuLayer *bus_stop_menu_layer;
} ui;

ClickConfigProvider previous_ccp; // ¿es necesario?


// - + - + - Declaration - + - + -
void bus_stop_show_near();
void bus_stop_show_favorites();
static void show_loading_feedback();
void bus_stop_show_favorites_return();
void update_loading_feedback_favorites();



// - + - + - Methods - + - + -
void show_locked(AppLogLevel log_level, const char* description){
//	APP_LOG(log_level, description);
}

static void hide_bus_stop_detail_layers(bool hide) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "hide_bus_stop_detail_layers( %d )", hide);
	layer_set_hidden(menu_layer_get_layer(ui.bus_stop_menu_layer), hide);
}

static void hide_feedback_layers(bool hide) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "hide_feedback_layers( %d )", hide);
	layer_set_hidden(text_layer_get_layer(ui.feedback_text_layer), hide);
}


void reload_menu(void){
	show_locked(APP_LOG_LEVEL_INFO, "crash1");
	// here! ->layer_mark_dirty(menu_layer_get_layer(ui.bus_stop_menu_layer));
	menu_layer_reload_data(ui.bus_stop_menu_layer);

	// hide_feedback_layers(true);
	update_loading_feedback_favorites();
}


static void add_remove_bus_stop_to_favorites(int row_actual) {
	show_locked(APP_LOG_LEVEL_INFO, "crash2");
	
	uint32_t key;
	BusStopListItem *stopListItem = NULL;
	stopListItem = get_bus_stop_list_at_index(row_actual - 1);
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
	show_locked(APP_LOG_LEVEL_INFO, "crash3");

	APP_LOG(APP_LOG_LEVEL_INFO, "select_click_handler");
	if(cell_index->row == 0){
		if(get_actual_view() == Favorites){
			bus_stop_show_near();
		}else if (get_actual_view() == Near)
			win_edit_show();
	}else{
		BusStopListItem *stopListItem = get_bus_stop_list_at_index(
				cell_index->row - 1);

		bus_stop_detail_show(stopListItem->number, stopListItem->name);
	}
}

static void select2_long_click_handler(struct MenuLayer *menu_layer,
        MenuIndex *cell_index,
        void *callback_context) {

	APP_LOG(APP_LOG_LEVEL_INFO, "select_long_click_handler");
	vibes_short_pulse();
	if(cell_index->row == 0){

	}else{
		hide_feedback_layers(false);
		add_remove_bus_stop_to_favorites(cell_index->row);
	}
	// here!2 ->layer_mark_dirty(menu_layer_get_layer(ui.bus_stop_menu_layer));

}

static uint16_t menu2_get_num_sections_callback(MenuLayer *me, void *data) {
	show_locked(APP_LOG_LEVEL_INFO, "crash4");
	return 1;
}

static uint16_t menu2_get_num_rows_callback(MenuLayer *me, uint16_t section_index, void *data) {
//	APP_LOG(APP_LOG_LEVEL_INFO, "Número de items en la lista: %d", get_bus_list_num_of_items());
	show_locked(APP_LOG_LEVEL_INFO, "crash5");
	return get_bus_list_num_of_items() + 1;
}

static int16_t menu2_get_cell_height_callback(MenuLayer *me, MenuIndex* cell_index, void *data) {
	show_locked(APP_LOG_LEVEL_INFO, "crash6");
	return 44;
}

static void click_back_action(ClickRecognizerRef recognizer, void *context) {
	APP_LOG(APP_LOG_LEVEL_INFO, "click_back_action selected");

	if(get_actual_view() == Near){
		bus_stop_show_favorites_return();
	} else
		window_stack_pop_all(true);
}

static void force_back_button(void *context){
	show_locked(APP_LOG_LEVEL_INFO, "crash7");
	previous_ccp(context);

	window_single_click_subscribe(BUTTON_ID_BACK, click_back_action);
}

static void menu2_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
	show_locked(APP_LOG_LEVEL_INFO, "crash8");

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

		BusStopListItem *act_bus_stop;

		act_bus_stop = get_bus_stop_list_at_index(cell_index->row -1);

		if(menu_cell_layer_is_highlighted(cell_layer)){
			// Bus Stop Lines

			graphics_context_set_text_color(ctx, PBL_IF_COLOR_ELSE(GColorBlack, GColorWhite));

			graphics_draw_text_vertically_center(ctx, act_bus_stop->lines, fonts_get_system_font(FONT_KEY_GOTHIC_18),
					detail_rect, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter);
		}else{

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
	}

}

void update_loading_feedback_favorites(void){ // MEJORAR ! ! ! ! !! !
	APP_LOG(APP_LOG_LEVEL_WARNING, "El método update_loading_feedback_favorites necesita mejora");
	bool loaded = get_bus_list_is_loaded();
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


static void show_loading_feedback() { // Juntar con el método anterior !
	show_locked(APP_LOG_LEVEL_INFO, "crash9");
	
	if (get_actual_view() == Favorites) {
		text_layer_set_text(ui.feedback_text_layer,"Loading favorite bus stops...");
	} else {
		text_layer_set_text(ui.feedback_text_layer,"Loading nearby bus stops...");
	}

	// hide_bus_stop_detail_layers(true);
	hide_feedback_layers(false);
}

static void menu2_row_selection_changed(struct MenuLayer *menu_layer,
        MenuIndex new_index,
        MenuIndex old_index,
        void *callback_context){
	show_locked(APP_LOG_LEVEL_INFO, "crash10");

	// here!2 ->layer_mark_dirty(menu_layer_get_layer(menu_layer));
}

// This initializes the menu upon window load
static void bus_stop_window_load(Window *window) {
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "bus_stop_window_load");
	
	ui.window = window;
	
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_frame(window_layer);

	APP_LOG(APP_LOG_LEVEL_INFO, "Tamaño ventana (origen): %d, %d", bounds.origin.x, bounds.origin.y);
	APP_LOG(APP_LOG_LEVEL_INFO, "Tamaño ventana (size): %d, %d", bounds.size.h, bounds.size.w);

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
	show_locked(APP_LOG_LEVEL_INFO, "crash11");
	
	menu_layer_destroy(ui.bus_stop_menu_layer);

	text_layer_destroy(ui.feedback_text_layer);

}

static void bus_stop_window_appear(Window *window) {
	show_locked(APP_LOG_LEVEL_INFO, "crash12");
	APP_LOG(APP_LOG_LEVEL_DEBUG, "bus_stop_window_appear");

	//bus_stop_list_num_of_items = 0;

	// here! ->layer_mark_dirty(menu_layer_get_layer(ui.bus_stop_menu_layer));
	menu_layer_reload_data(ui.bus_stop_menu_layer);
}

static void bus_stop_window_disappear(Window *window) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "bus_stop_window_disappear");
}

static void load_view_for_bus_stops_type(int _listType) {
	show_locked(APP_LOG_LEVEL_INFO, "crash13");
	
	set_actual_view(_listType);

	window_stack_push(ui.window, true /* Animated */);
}

void bus_stop_show_favorites(void) { // Usado para la primera carga
	show_locked(APP_LOG_LEVEL_INFO, "crash14");
	
	load_view_for_bus_stops_type(Favorites);
}

void bus_stop_show_favorites_return(void) {
	show_locked(APP_LOG_LEVEL_INFO, "crash15");

	set_actual_view(Favorites);
	reload_menu();

	// here! ->layer_mark_dirty(menu_layer_get_layer(ui.bus_stop_menu_layer));
	menu_layer_reload_data(ui.bus_stop_menu_layer);
//	window_stack_push(ui.window, false /* Animated */);
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
//	bus_stop_show_favorites();
	// window_stack_push(ui.window, true /* Animated */);
}

void favorites_bus_stop_deinit(void) {
	window_destroy(ui.window);
}

void bus_stop_show_near(void) {
	show_locked(APP_LOG_LEVEL_INFO, "crash16");

	load_view_for_bus_stops_type(Near);

	if(get_bus_list_num_of_items() < 1){
		show_loading_feedback();
	}

// here! ->layer_mark_dirty(menu_layer_get_layer(ui.bus_stop_menu_layer));
	menu_layer_reload_data(ui.bus_stop_menu_layer);
//	window_stack_pop(false);
//	window_stack_push(ui.window, true /* Animated */);
}


