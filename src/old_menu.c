#include <pebble.h>
#include "favorites_bus_stop.h"
#include "old_bus_stop_list.h"
#include "bus_stop_number_select.h"


#define MAIN_MENU_SECTIONS 1
#define MAIN_MENU_ITEMS 5

static struct MainMenuUi {
	Window *window;
	SimpleMenuLayer *simple_menu_layer;
	SimpleMenuSection main_menu_sections[MAIN_MENU_SECTIONS];
	SimpleMenuItem main_menu_first_section_items[MAIN_MENU_ITEMS];
} ui;

static void menu_select_push_to_near_stops_callback(int index, void *ctx) {
	
	bus_stop_list_show_near();
}

static void menu_select_push_favorites_stops_callback(int index, void *ctx) {
	
	bus_stop_list_show_favorites();
}

static void menu_select_new_favorites_stops_callback(int index, void *ctx) {
	
	bus_stop_show_favorites();
}

static void menu_select_new_near_stops_callback(int index, void *ctx) {

	bus_stop_show_near();
}

static void menu_select_new_select_number_stops_callback(int index, void *ctx) {

	win_edit_show();
}

// This initializes the menu upon window load
static void window_load(Window *window) {
	
	// This is an example of how you'd set a simple menu item
	ui.main_menu_first_section_items[0] = (SimpleMenuItem){
		// You should give each menu item a title and callback
		.title = "Nearby Stops",
		.callback = menu_select_push_to_near_stops_callback,
	};
	// The menu items appear in the order saved in the menu items array
	ui.main_menu_first_section_items[1] = (SimpleMenuItem){
		.title = "Favorite Stops",
		.callback = menu_select_push_favorites_stops_callback,
	};
	ui.main_menu_first_section_items[2] = (SimpleMenuItem){
		.title = "Favorite Stops_2",
		.callback = menu_select_new_favorites_stops_callback,
	};
	ui.main_menu_first_section_items[3] = (SimpleMenuItem){
		.title = "Nearby Stops_2",
		.callback = menu_select_new_near_stops_callback,
	};
	ui.main_menu_first_section_items[3] = (SimpleMenuItem){
		.title = "Select by Number",
		.callback = menu_select_new_select_number_stops_callback,
	};
	
	// Bind the menu items to the corresponding menu sections
	ui.main_menu_sections[0] = (SimpleMenuSection){
		.num_items = MAIN_MENU_ITEMS,
		.items = ui.main_menu_first_section_items,
	};
	
	// Now we prepare to initialize the simple menu layer
	// We need the bounds to specify the simple menu layer's viewport size
	// In this case, it'll be the same as the window's
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_frame(window_layer);
	
	// Initialize the simple menu layer
	ui.simple_menu_layer = simple_menu_layer_create(bounds, window, ui.main_menu_sections, MAIN_MENU_SECTIONS, NULL);
	
	// Add it to the window for display
	layer_add_child(window_layer, simple_menu_layer_get_layer(ui.simple_menu_layer));
}

// Deinitialize resources on window unload that were initialized on window load
void window_unload(Window *window) {
	simple_menu_layer_destroy(ui.simple_menu_layer);
}

static void window_appear(Window *window) {
}

void main_menu_init(void) {
	ui.window = window_create();
	
	// Setup the window handlers
	window_set_window_handlers(ui.window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
		.appear = window_appear
	});
	
	// window_stack_push(ui.window, true /* Animated */);
}

void main_menu_load(void) {
	window_stack_push(ui.window, true /* Animated */);
}

void main_menu_deinit(void) {
  window_destroy(ui.window);
}
