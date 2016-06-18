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
void stop_list_show_near();
void stop_list_show_favorites();
void stop_list_show_favorites_return();
void stop_list_update_loading_feedback();
void stop_list_hide_detail_layers(bool hide);
void stop_list_hide_feedback_layers(bool hide);
void stop_list_reload_menu();
static void stop_list_select_simple(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void stop_list_select_long(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void stop_list_select_back(ClickRecognizerRef recognizer, void *context);
static void stop_list_force_select_back(void *context);

static uint16_t stop_list_menu_num_sections(MenuLayer *me, void *data);
static uint16_t stop_list_menu_num_rows(MenuLayer *me, uint16_t section_index, void *data);
static int16_t stop_list_menu_cell_height(MenuLayer *me, MenuIndex* cell_index, void *data);
static void stop_list_menu_draw_row(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data);
static void stop_list_menu_row_selection_changed(struct MenuLayer *menu_layer, MenuIndex new_index,
		MenuIndex old_index, void *callback_context);

static void stop_list_window_load(Window *window);

// FALTAN ! más métodos


// - + - + - Methods - + - + -
static void stop_list_select_simple(struct MenuLayer *menu_layer,
		MenuIndex *cell_index, void *callback_context) {

	APP_LOG(APP_LOG_LEVEL_INFO, "select_click_handler");
	if(cell_index->row == 0){
		if(get_actual_view() == Favorites){
			stop_list_show_near();
		}else if (get_actual_view() == Near)
			win_edit_show();
	}else{
		BusStopListItem *stopListItem = get_bus_stop_list_at_index(
				cell_index->row - 1);

		bus_stop_detail_show(stopListItem->number, stopListItem->name);
	}
}

static void stop_list_select_long(struct MenuLayer *menu_layer,
		MenuIndex *cell_index, void *callback_context) {

	APP_LOG(APP_LOG_LEVEL_INFO, "select_long_click_handler");
	vibes_short_pulse();
	if(cell_index->row == 0){

	}else{
		add_remove_bus_stop_to_favorites(cell_index->row - 1);
	}
	// here!2 ->layer_mark_dirty(menu_layer_get_layer(ui.bus_stop_menu_layer));

}

static void stop_list_select_back(ClickRecognizerRef recognizer, void *context) {
	APP_LOG(APP_LOG_LEVEL_INFO, "stop_list_select_back selected");

	if(get_actual_view() == Near){
		stop_list_show_favorites_return();
	} else
		window_stack_pop_all(true);
}

static void stop_list_force_select_back(void *context){
	show_log(APP_LOG_LEVEL_INFO, "crash7");
	previous_ccp(context);

	window_single_click_subscribe(BUTTON_ID_BACK, stop_list_select_back);
}

static uint16_t stop_list_menu_num_sections(MenuLayer *me, void *data) {
	return 1;
}

static uint16_t stop_list_menu_num_rows(MenuLayer *me, uint16_t section_index, void *data) {
	return get_bus_list_num_of_items() + 1;
}

static int16_t stop_list_menu_cell_height(MenuLayer *me, MenuIndex* cell_index, void *data) {
	return 44;
}

static void stop_list_menu_draw_row(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
	show_log(APP_LOG_LEVEL_INFO, "crash8");

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

void stop_list_update_loading_feedback(void){
	APP_LOG(APP_LOG_LEVEL_WARNING, "El método update_loading_feedback_favorites necesita mejora");
	bool loaded = get_bus_list_is_loaded();

	stop_list_hide_feedback_layers(false);

	if(loaded == true && get_bus_list_num_of_items() > 0){
		stop_list_hide_feedback_layers(true);
	}else if(loaded == true && get_bus_list_num_of_items() < 1){
		if (get_actual_view() == Favorites) {
			text_layer_set_text(ui.feedback_text_layer,"No favorite bus stops.\n Search it and long press to add to favorites.");
		} else {
			text_layer_set_text(ui.feedback_text_layer,"No nearby bus stops.");
		}
	}else if(!connection_service_peek_pebble_app_connection()){
		text_layer_set_text(ui.feedback_text_layer,"No phone connected");
	}else{
		if (get_actual_view() == Favorites) {
			text_layer_set_text(ui.feedback_text_layer,"Loading favorite bus stops...");
		} else {
			text_layer_set_text(ui.feedback_text_layer,"Loading nearby bus stops...");
		}
	}
}

static void stop_list_menu_row_selection_changed(struct MenuLayer *menu_layer, MenuIndex new_index,
		MenuIndex old_index, void *callback_context){
	show_log(APP_LOG_LEVEL_INFO, "crash10");

	// here!2 ->layer_mark_dirty(menu_layer_get_layer(menu_layer));
}

// This initializes the menu upon window load
static void stop_list_window_load(Window *window) {
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "bus_stop_window_load");
	
	ui.window = window;
	
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_frame(window_layer);

	APP_LOG(APP_LOG_LEVEL_INFO, "Tamaño ventana (origen): %d, %d", bounds.origin.x, bounds.origin.y);
	APP_LOG(APP_LOG_LEVEL_INFO, "Tamaño ventana (size): %d, %d", bounds.size.h, bounds.size.w);

	//MenuLayer

	ui.bus_stop_menu_layer = menu_layer_create(bounds);

	menu_layer_set_callbacks(ui.bus_stop_menu_layer, NULL, (MenuLayerCallbacks){
		.get_num_sections = stop_list_menu_num_sections,
		.get_cell_height = stop_list_menu_cell_height,
		.get_num_rows = stop_list_menu_num_rows,
		.draw_row = stop_list_menu_draw_row,
		.selection_changed = stop_list_menu_row_selection_changed,
		.select_click = stop_list_select_simple,
		.select_long_click= stop_list_select_long,
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
	stop_list_update_loading_feedback();

	//stop_list_force_select_back(ui.window, ui.bus_stop_menu_layer);
	previous_ccp = window_get_click_config_provider(ui.window);
	window_set_click_config_provider_with_context(ui.window, stop_list_force_select_back, ui.bus_stop_menu_layer);

}

// Deinitialize resources on window unload that were initialized on window load
static void stop_list_window_unload(Window *window) {
	show_log(APP_LOG_LEVEL_INFO, "crash11");
	
	menu_layer_destroy(ui.bus_stop_menu_layer);

	text_layer_destroy(ui.feedback_text_layer);

}

static void stop_list_window_appear(Window *window) {
	show_log(APP_LOG_LEVEL_INFO, "crash12");
	APP_LOG(APP_LOG_LEVEL_DEBUG, "bus_stop_window_appear");

	//bus_stop_list_num_of_items = 0;

	// here! ->layer_mark_dirty(menu_layer_get_layer(ui.bus_stop_menu_layer));
	menu_layer_reload_data(ui.bus_stop_menu_layer);
}

static void stop_list_window_disappear(Window *window) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "bus_stop_window_disappear");
}

static void stop_list_load_stops_type(int _listType) {
	show_log(APP_LOG_LEVEL_INFO, "crash13");
	
	set_actual_view(_listType);

	window_stack_push(ui.window, true /* Animated */);
}

void stop_list_show_favorites(void) { // Usado para la primera carga
	show_log(APP_LOG_LEVEL_INFO, "crash14");
	
	stop_list_load_stops_type(Favorites);
}

void stop_list_show_favorites_return(void) {
	show_log(APP_LOG_LEVEL_INFO, "crash15");

	set_actual_view(Favorites);
	stop_list_reload_menu();

	// here! ->layer_mark_dirty(menu_layer_get_layer(ui.bus_stop_menu_layer));
	menu_layer_reload_data(ui.bus_stop_menu_layer);
//	window_stack_push(ui.window, false /* Animated */);
}

void stop_list_init(void) {
	
	ui.window = window_create();
	
	// Setup the window handlers
	window_set_window_handlers(ui.window, (WindowHandlers) {
		.load = stop_list_window_load,
		.unload = stop_list_window_unload,
		.appear = stop_list_window_appear,
		.disappear = stop_list_window_disappear
	});
//	bus_stop_show_favorites();
	// window_stack_push(ui.window, true /* Animated */);
}

void stop_list_deinit(void) {
	window_destroy(ui.window);
}

void stop_list_show_near(void) {
	show_log(APP_LOG_LEVEL_INFO, "crash16");

	stop_list_load_stops_type(Near);

	if(get_bus_list_num_of_items() < 1){
		stop_list_update_loading_feedback();
	}

// here! ->layer_mark_dirty(menu_layer_get_layer(ui.bus_stop_menu_layer));
	menu_layer_reload_data(ui.bus_stop_menu_layer);
//	window_stack_pop(false);
//	window_stack_push(ui.window, true /* Animated */);
}

void stop_list_hide_detail_layers(bool hide) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "hide_bus_stop_detail_layers( %d )", hide);
	layer_set_hidden(menu_layer_get_layer(ui.bus_stop_menu_layer), hide);
}

void stop_list_hide_feedback_layers(bool hide) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "hide_feedback_layers( %d )", hide);
	layer_set_hidden(text_layer_get_layer(ui.feedback_text_layer), hide);
}


void stop_list_reload_menu(void){
	show_log(APP_LOG_LEVEL_INFO, "crash1");
	// here! ->layer_mark_dirty(menu_layer_get_layer(ui.bus_stop_menu_layer));
	menu_layer_reload_data(ui.bus_stop_menu_layer);

	// hide_feedback_layers(true);
	stop_list_update_loading_feedback();
}


