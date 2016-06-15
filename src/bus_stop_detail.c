#include <pebble.h>
#include "keys.h"
#include "data.h"
#include "ancillary_methods.h"
#include "communication.h"


// - + - + - Common Variables - + - + -
static struct BusStopDetailUi {
	Window *window;
	MenuLayer *menu_layer;
	TextLayer *feedback_text_layer;
} ui;

ClickConfigProvider previous_ccp; // ¿es necesario?
int counter = 0;

bool in_view = false;


// - + - + - Declaration - + - + -



// - + - + - Methods - + - + -
void show_log(uint8_t log_level, const char* description){
	// APP_LOG(log_level, description);
}

void set_bus_stop_list_hidden(bool hidden) {
	show_log(APP_LOG_LEVEL_INFO, "Crash-01");

	layer_set_hidden(menu_layer_get_layer(ui.menu_layer), hidden);
}

void set_feedback_message_hidden(bool hidden) {
	show_log(APP_LOG_LEVEL_INFO, "Crash-02");

	layer_set_hidden(text_layer_get_layer(ui.feedback_text_layer), hidden);
}


static uint16_t menu_get_num_sections_callback(MenuLayer *me, void *data) {
	show_log(APP_LOG_LEVEL_INFO, "Crash-03");
	return 1;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *me, uint16_t section_index, void *data) {
	show_log(APP_LOG_LEVEL_INFO, "Crash-04");
	if(in_view){
		return get_bus_stop_detail()->number_of_lines + 1;
	}else{
		return 0;
	}
//	return 1;
}

static int16_t menu_get_cell_height_callback(MenuLayer *me, MenuIndex* cell_index, void *data) {
	show_log(APP_LOG_LEVEL_INFO, "Crash-05");
	return 44;
}

static bool got_estimate(char *estimate) {
	show_log(APP_LOG_LEVEL_INFO, "Crash-06");

	return strcmp(estimate, "") != 0;
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
	counter += 1;

	show_log(APP_LOG_LEVEL_INFO, "Crash-07");
	
	show_log(APP_LOG_LEVEL_INFO, "Falla aqui ! !. Al pedir los datos nombre y número de la parada");
	show_log(APP_LOG_LEVEL_ERROR, "No se muestran todos los tiempos ya que da error");


	if(cell_index->row == 0){
		show_log(APP_LOG_LEVEL_INFO, "cell_index is 0");

		GRect detail_rect = GRect(43, 0, 99, 42);
//		GRect detail_rect = GRect(48, 0, 93, 42);
		GRect bus_stop_rect = GRect(2, 0, 45, 42);

		// Bus Stop Name
		graphics_draw_text_vertically_center(ctx, get_bus_stop_detail()->name, fonts_get_system_font(FONT_KEY_GOTHIC_14),
				detail_rect, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter);

		// Line Number
		graphics_draw_text_vertically_center(ctx, get_bus_stop_detail()->number, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD),
				bus_stop_rect, GTextOverflowModeWordWrap, GTextAlignmentCenter);


	}else{

		show_log(APP_LOG_LEVEL_INFO, "cell_index not is 0");
		//LineTimesItem lineTimeItem;

		//lineTimeItem = get_bus_stop_detail()->linesTimes[(cell_index->row) - 1];

		LineTimesItem lineTimeItem = get_bus_stop_detail()->linesTimes[(cell_index->row) - 1];

		bool got_estimate_1 = got_estimate(lineTimeItem.bus1);
		bool got_estimate_2 = got_estimate(lineTimeItem.bus2);

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
			graphics_draw_text(ctx, lineTimeItem.bus1, fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(34, 0, 108, 19), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

			// Time 2
			graphics_draw_text(ctx, lineTimeItem.bus2, fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(34, 21, 108, 19), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

		} else if (got_estimate_1) {
// SI se descomenta falla en el desplazamiento de la lista! ! !
// Probado con la parada 256
	// Posible problema, el menú no se borra completamente por lo que al desplazarse solo modificaría ciertas cosas y no todo
			show_log(APP_LOG_LEVEL_INFO, "Only estimate time 1 !");
			// Time 1
//			graphics_draw_text(ctx, lineTimeItem.bus2, fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(34, 9, 108, 19), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

		} else if (got_estimate_2) {
// SI se descomenta falla en el desplazamiento de la lista! ! !
// Probado con la parada 256
			show_log(APP_LOG_LEVEL_INFO, "Only estimate time 2 !");

			// Time 2
//			graphics_draw_text(ctx, lineTimeItem.bus2, fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(34, 9, 108, 19), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

		} else { // SI
			show_log(APP_LOG_LEVEL_INFO, "No estimates !");
		// No estimates
//			graphics_draw_text(ctx, "No estimates.", fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(34, 9, 108, 19), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

		}
	}
	show_log(APP_LOG_LEVEL_INFO, "end_Crash-07");

	
}

static void time_row_selection_changed(struct MenuLayer *menu_layer,
        MenuIndex new_index,
        MenuIndex old_index,
        void *callback_context){
	show_log(APP_LOG_LEVEL_INFO, "Crash-08");

	// here! ->layer_mark_dirty(menu_layer_get_layer(menu_layer));
	//menu_layer_reload_data(ui.menu_layer);

	// times_row_actual = new_index.row;
}


static void click2_back_action(ClickRecognizerRef recognizer, void *context) {
	show_log(APP_LOG_LEVEL_WARNING, "corregir metodo click_back_action");

	set_actual_view(Favorites);

	window_stack_pop(true);
}

static void force2_back_button(void *context){
	show_log(APP_LOG_LEVEL_INFO, "Crash-AAAAA-01");

	previous_ccp(context);

	window_single_click_subscribe(BUTTON_ID_BACK, click2_back_action);
}


// This initializes the menu upon window load
static void bus_stop_detail_window_load(Window *window) {
	show_log(APP_LOG_LEVEL_INFO, "Crash-09");
	show_log(APP_LOG_LEVEL_INFO, "Empezando a cargar la vista...");
	
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
	show_log(APP_LOG_LEVEL_INFO, "pre definición de colores");

#ifdef PBL_COLOR
	menu_layer_pad_bottom_enable(ui.menu_layer,false);
	menu_layer_set_highlight_colors(ui.menu_layer,GColorVividCerulean,GColorWhite);
#endif
	layer_add_child(window_layer, menu_layer_get_layer(ui.menu_layer));

	menu_layer_set_click_config_onto_window(ui.menu_layer, ui.window);

	
	show_log(APP_LOG_LEVEL_INFO, "Cargado el menu");


	// Feedback Text Layer
	GRect feedback_grect = bounds;
	feedback_grect.origin.y = bounds.size.h / 4 + 5;
	ui.feedback_text_layer = text_layer_create(feedback_grect);
	text_layer_set_text_color(ui.feedback_text_layer, GColorBlack);
	text_layer_set_font(ui.feedback_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	text_layer_set_text_alignment(ui.feedback_text_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(ui.feedback_text_layer));
	
	show_log(APP_LOG_LEVEL_INFO, "Cargado el feedback");

	text_layer_set_text(ui.feedback_text_layer, "Loading times...");

	show_log(APP_LOG_LEVEL_INFO, "fin carga de la vista");

	//Force back button
	previous_ccp = window_get_click_config_provider(ui.window);
	window_set_click_config_provider_with_context(ui.window, force2_back_button, ui.menu_layer);

}

// Deinitialize resources on window unload that were initialized on window load
void bus_stop_detail_window_unload(Window *window) {
	show_log(APP_LOG_LEVEL_INFO, "Crash-10");
	menu_layer_destroy(ui.menu_layer);
	text_layer_destroy(ui.feedback_text_layer); // Originally not exist. After put that the aplication close going back
	// get_bus_stop_detail.number_of_lines = 0;
}


void reload_details_menu(void){
	show_log(APP_LOG_LEVEL_INFO, "Crash-11");
//	layer_mark_dirty(menu_layer_get_layer(ui.menu_layer));
	menu_layer_reload_data(ui.menu_layer);

//	hide_feedback_layers(true);
//	set_bus_stop_list_hidden(false);
//	set_feedback_message_hidden(true);
}

void bus_stop_detail_window_appear(Window *window) {
	show_log(APP_LOG_LEVEL_WARNING, "In bus stop detail view");
	in_view = true;
	reload_details_menu();
}


void bus_stop_detail_show(char *number, char *name) {
	show_log(APP_LOG_LEVEL_WARNING, "bus_stop_detail_show");
	
	define_stop_detail(number, name);
	set_actual_view(Details);
	
	show_log(APP_LOG_LEVEL_WARNING, "definido el numero y nombre");

	window_stack_push(ui.window, true /* Animated */);

	show_log(APP_LOG_LEVEL_WARNING, "OK -> window_stack_push");

	layer_mark_dirty(menu_layer_get_layer(ui.menu_layer));

	show_log(APP_LOG_LEVEL_WARNING, "OK -> layer_mark_dirty(menu_layer_get_layer");

	menu_layer_reload_data(ui.menu_layer);
	show_log(APP_LOG_LEVEL_WARNING, "Recargada la vista Ok");
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
	show_log(APP_LOG_LEVEL_INFO, "Crash-12");

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
