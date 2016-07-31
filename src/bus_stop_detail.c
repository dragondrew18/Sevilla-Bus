#include <pebble.h>
#include "keys.h"
#include "data.h"
#include "ancillary_methods.h"
#include "communication.h"

/* # + # - # + # - # + # - # + # - # + # - # + # - # + #
 * # + # - # - >  Common Variables  < - # - # + #
 * # + # - # + # - # + # - # + # - # + # - # + # - # + # */

static struct BusStopDetailUi {
	Window *window;
	MenuLayer *menu_layer;
	TextLayer *feedback_text_layer;
} ui;

ClickConfigProvider previous_ccp; // ¿es necesario?
int counter = 0;

bool in_view = false;



/* # + # - # + # - # + # - # + # - # + # - # + # - # + #
 * # + # - # - >  Declaration  < - # - # + #
 * # + # - # + # - # + # - # + # - # + # - # + # - # + # */
void stop_detail_menu_set_hidden(bool hidden);
void stop_detail_feedback_set_hidden(bool hidden);
static uint16_t stop_detail_menu_num_sections(MenuLayer *me, void *data);
static uint16_t stop_detail_menu_num_rows(MenuLayer *me, uint16_t section_index, void *data);
static int16_t stop_detail_menu_cell_height(MenuLayer *me, MenuIndex* cell_index, void *data);
static int16_t stop_detail_menu_cell_height(MenuLayer *me, MenuIndex* cell_index, void *data);
static void stop_detail_menu_draw_row(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data);
static void stop_detail_row_selection_changed(struct MenuLayer *menu_layer, MenuIndex new_index, MenuIndex old_index, void *callback_context);
static void stop_detail_select_back(ClickRecognizerRef recognizer, void *context);
static void stop_detail_force_select_back(void *context);


static void stop_detail_window_load(Window *window);
void stop_detail_init(void);
void stop_detail_deinit(void);
void stop_detail_show(char *number, char *name);
void stop_detail_window_unload(Window *window);
void stop_detail_window_appear(Window *window);

void stop_detail_reload_menu(void);

void stop_detail_update_loading_feedback(void);



/* # + # - # + # - # + # - # + # - # + # - # + # - # + #
 * # + # - # - >  Methods.init  < - # - # + #
 * # + # - # + # - # + # - # + # - # + # - # + # - # + # */

void stop_detail_init(void) {
	ui.window = window_create();

	// Setup the window handlers
	window_set_window_handlers(ui.window, (WindowHandlers) {
		.load = stop_detail_window_load,
		.unload = stop_detail_window_unload,
		.appear = stop_detail_window_appear,
	});
}

void stop_detail_deinit(void) {
  window_destroy(ui.window);
}

void stop_detail_show(char *number, char *name) {
	define_stop_detail(number, name);
	set_actual_view(Details);

	window_stack_push(ui.window, true /* Animated */);

//	show_log(APP_LOG_LEVEL_WARNING, "OK -> window_stack_push");

//	layer_mark_dirty(menu_layer_get_layer(ui.menu_layer));

	menu_layer_reload_data(ui.menu_layer); // Dejar para que se marque la primera línea en la primera carga
	stop_detail_update_loading_feedback();
}



/* # + # - # + # - # + # - # + # - # + # - # + # - # + #
 * # + # - # - >  Methods.windows  < - # - # + #
 * # + # - # + # - # + # - # + # - # + # - # + # - # + # */

// This initializes the menu upon window load
static void stop_detail_window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_frame(window_layer);

	ui.menu_layer = menu_layer_create(bounds);

	menu_layer_set_callbacks(ui.menu_layer, NULL, (MenuLayerCallbacks){
		.get_num_sections = stop_detail_menu_num_sections,
		.get_cell_height = stop_detail_menu_cell_height,
		.get_num_rows = stop_detail_menu_num_rows,
		.draw_row = stop_detail_menu_draw_row,
		.selection_changed = stop_detail_row_selection_changed,
	});
	show_log(APP_LOG_LEVEL_INFO, "pre definición de colores");

#ifdef PBL_COLOR
	menu_layer_pad_bottom_enable(ui.menu_layer,false);
	menu_layer_set_highlight_colors(ui.menu_layer,GColorVividCerulean,GColorWhite);
#endif
	layer_add_child(window_layer, menu_layer_get_layer(ui.menu_layer));

	menu_layer_set_click_config_onto_window(ui.menu_layer, ui.window);

//	#ifdef PBL_ROUND
//		menu_layer_set_center_focused(ui.menu_layer, false);
//	#endif


	show_log(APP_LOG_LEVEL_INFO, "Cargado el menu");


	// Feedback Text Layer
	GRect feedback_grect = bounds;
	#ifdef PBL_RECT
		feedback_grect.origin.y = bounds.size.h / 4 + 5;
	#else
		feedback_grect.origin.y = bounds.size.h / 2 + bounds.size.h / 5;
	#endif

	ui.feedback_text_layer = text_layer_create(feedback_grect);
	text_layer_set_text_color(ui.feedback_text_layer, GColorBlack);
	text_layer_set_font(ui.feedback_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	text_layer_set_text_alignment(ui.feedback_text_layer, GTextAlignmentCenter);

	layer_add_child(window_layer, text_layer_get_layer(ui.feedback_text_layer));

	show_log(APP_LOG_LEVEL_INFO, "fin carga de la vista");

	//Force back button
	previous_ccp = window_get_click_config_provider(ui.window);
	window_set_click_config_provider_with_context(ui.window, stop_detail_force_select_back, ui.menu_layer);

}

// Deinitialize resources on window unload that were initialized on window load
void stop_detail_window_unload(Window *window) {
	menu_layer_destroy(ui.menu_layer);
	text_layer_destroy(ui.feedback_text_layer); // Originally not exist. After put that the aplication close going back
	// get_bus_stop_detail.number_of_lines = 0;
}

void stop_detail_window_appear(Window *window) {
	in_view = true;
//	stop_detail_reload_menu();
}



/* # + # - # + # - # + # - # + # - # + # - # + # - # + #
 * # + # - # - >  Methods.menu  < - # - # + #
 * # + # - # + # - # + # - # + # - # + # - # + # - # + # */

static uint16_t stop_detail_menu_num_sections(MenuLayer *me, void *data) {
	return 1;
}

static uint16_t stop_detail_menu_num_rows(MenuLayer *me, uint16_t section_index, void *data) {
	if(in_view) {
		if (!get_is_loaded())
			return 1;
		else{
			return get_bus_stop_detail()->number_of_lines + 1;
		}
	} else {
		return 0;
	}
//	return 1;
}

static int16_t stop_detail_menu_cell_height(MenuLayer *me, MenuIndex* cell_index, void *data) {
	return 44;
}

static void stop_detail_menu_draw_row(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
//	counter += 1;

//	show_log(APP_LOG_LEVEL_INFO, "Crash-07");
	
//	show_log(APP_LOG_LEVEL_INFO, "Falla aqui ! !. Al pedir los datos nombre y número de la parada");
//	show_log(APP_LOG_LEVEL_ERROR, "No se muestran todos los tiempos ya que da error");

//	APP_LOG(APP_LOG_LEVEL_INFO, "Trying to write the cell... %d", (int) cell_index->row);
	GRect bounds = layer_get_frame(cell_layer);


	if(cell_index->row == 0){
	#ifdef PBL_RECT
		GRect detail_rect = GRect(43, 0, 99, 42);
//		GRect detail_rect = GRect(48, 0, 93, 42);
		GRect bus_stop_rect = GRect(2, 0, 45, 42);

		// Bus Stop Name
		graphics_draw_text_vertically_center(ctx, get_bus_stop_detail()->name, fonts_get_system_font(FONT_KEY_GOTHIC_14),
				detail_rect, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter);

		// Line Number
		graphics_draw_text_vertically_center(ctx, get_bus_stop_detail()->number, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD),
				bus_stop_rect, GTextOverflowModeWordWrap, GTextAlignmentCenter);
	#else
		GRect detail_rect;
		GRect bus_stop_rect;

		if(menu_cell_layer_is_highlighted(cell_layer)){
			detail_rect = GRect(43, 0, bounds.size.w - 45, 42);
			bus_stop_rect = GRect(2, 0, 45, 42);

			// Bus Stop Name
			graphics_draw_text_vertically_center(ctx, get_bus_stop_detail()->name, fonts_get_system_font(FONT_KEY_GOTHIC_14),
					detail_rect, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter);

		}else{
			bus_stop_rect = bounds;
		}

		// Line Number
		graphics_draw_text_vertically_center(ctx, get_bus_stop_detail()->number, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD),
				bus_stop_rect, GTextOverflowModeWordWrap, GTextAlignmentCenter);

	#endif
	}else{
		LineTimesItem lineTimeItem = get_bus_stop_detail()->linesTimes[(cell_index->row) - 1];

		bool got_estimate_1 = is_empty_char(lineTimeItem.bus1);
		bool got_estimate_2 = is_empty_char(lineTimeItem.bus2);

		if(menu_cell_layer_is_highlighted(cell_layer)){
			// Bus Stop Lines
			graphics_context_set_text_color(ctx, PBL_IF_COLOR_ELSE(GColorBlack, GColorWhite));
		}else{
			graphics_context_set_text_color(ctx, GColorBlack);
		}
		// Line Number
		graphics_draw_text(ctx, lineTimeItem.name, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(2, 5, 30, 24), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);

		if (got_estimate_1 && got_estimate_2) {

			// Time 1
			graphics_draw_text(ctx, lineTimeItem.bus1, fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(34, 0, bounds.size.w - 32, 19), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

			// Time 2
			graphics_draw_text(ctx, lineTimeItem.bus2, fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(34, 21, bounds.size.w - 32, 19), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

		}
		else if (got_estimate_1) {
// SI se descomenta falla en el desplazamiento de la lista! ! !
// Probado con la parada 256
	// Posible problema, el menú no se borra completamente por lo que al desplazarse solo modificaría ciertas cosas y no todo
			show_log(APP_LOG_LEVEL_INFO, "Only estimate time 1 !");
			// Time 1
			graphics_draw_text(ctx, lineTimeItem.bus1, fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(34, 9, bounds.size.w - 32, 19), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
//			graphics_draw_text(ctx, lineTimeItem.bus1, fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(34, 0, bounds.size.w - 32, 19), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

		} else if (got_estimate_2) {
// SI se descomenta falla en el desplazamiento de la lista! ! !
// Probado con la parada 256
			show_log(APP_LOG_LEVEL_INFO, "Only estimate time 2 !");

			// Time 2
			graphics_draw_text(ctx, lineTimeItem.bus2, fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(34, 9, bounds.size.w - 32, 19), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

		} else { // SI
			show_log(APP_LOG_LEVEL_INFO, "No estimates !");
		// No estimates
			graphics_draw_text(ctx, "No estimates.", fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(34, 9, bounds.size.w - 32, 19), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

		}
	}
}

static void stop_detail_row_selection_changed(struct MenuLayer *menu_layer, MenuIndex new_index, MenuIndex old_index, void *callback_context){

	// here! -> layer_mark_dirty(menu_layer_get_layer(menu_layer));
	// menu_layer_reload_data(ui.menu_layer);

	// times_row_actual = new_index.row;
}

static void stop_detail_select_back(ClickRecognizerRef recognizer, void *context) {
	show_log(APP_LOG_LEVEL_WARNING, "corregir metodo click_back_action");

	set_actual_view(get_stop_list_type());

	window_stack_pop(true);
}

static void stop_detail_force_select_back(void *context){

	previous_ccp(context);

	window_single_click_subscribe(BUTTON_ID_BACK, stop_detail_select_back);
}



/* # + # - # + # - # + # - # + # - # + # - # + # - # + #
 * # + # - # - >  Methods.others  < - # - # + #
 * # + # - # + # - # + # - # + # - # + # - # + # - # + # */

void stop_detail_reload_menu(void){
//	layer_mark_dirty(menu_layer_get_layer(ui.menu_layer));
	menu_layer_reload_data(ui.menu_layer);

//	hide_feedback_layers(true);
//	set_bus_stop_list_hidden(false);
//	set_feedback_message_hidden(true);
}

void stop_detail_menu_set_hidden(bool hidden) {

	layer_set_hidden(menu_layer_get_layer(ui.menu_layer), hidden);
}

void stop_detail_feedback_set_hidden(bool hidden) {

	layer_set_hidden(text_layer_get_layer(ui.feedback_text_layer), hidden);
}

void stop_detail_update_loading_feedback(){
	APP_LOG(APP_LOG_LEVEL_WARNING, "El método stop_detail_update_loading_feedback necesita mejora");
	APP_LOG(APP_LOG_LEVEL_WARNING, "El método stop_detail_update_loading_feedback no contempla errores en la comunicación");


	if(get_has_error_js()){
		stop_detail_feedback_set_hidden(false);
		text_layer_set_text(ui.feedback_text_layer,"Unknown bus number");
	} else if(get_is_loaded() == true && get_actual_view_list_size() > 0){
		stop_detail_menu_set_hidden(false);
		stop_detail_feedback_set_hidden(true);
	}else if(get_is_loaded() == true && get_actual_view_list_size() < 1){
		// hide_feedback_layers(false);
		stop_detail_feedback_set_hidden(false);
		text_layer_set_text(ui.feedback_text_layer,"No favorite bus stops.\n\n Search it !.");
	}else{
		stop_detail_feedback_set_hidden(false);
		text_layer_set_text(ui.feedback_text_layer,"Loading...");
		// show_loading_feedback();
	}
}

