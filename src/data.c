#include <pebble.h>
#include "data.h"
#include "keys.h"
#include "favorites_bus_stop.h"
#include "ancillary_methods.h"

static BusStopListItem bus_stop_list_favorites[BUS_STOP_LIST_MAX_ITEMS];
static BusStopListItem bus_stop_list_near[BUS_STOP_LIST_MAX_ITEMS];

static bool loaded_favorites = false;
static bool loaded_near = false;

static bool no_bus_stop = false;

static enum View actual_view = Favorites;

static int list_favorites_num_of_items = 0;
static int list_nearby_num_of_items = 0;
static int list_details_num_of_items = 0;

static enum ListType listType = ListTypeFavorites;

BusStopListItem* get_bus_stop_list_favorites_at_index(int index) {
	if (index < 0 || index >= BUS_STOP_LIST_MAX_ITEMS) {
		return NULL;
	} else {
		return &bus_stop_list_favorites[index];
	}
}

int get_actual_view(void){
	return actual_view;
}

void set_actual_view(int view){
	APP_LOG(APP_LOG_LEVEL_INFO, "Changing to view: %d", view);
	actual_view = view;

	DictionaryIterator *iter;

	if(view == Favorites && !loaded_favorites && get_JS_is_ready())
		send_message(&iter, TUSSAM_KEY_FAVORITES,1);
	else if (view == Near && !loaded_near && get_JS_is_ready())
		send_message(&iter, TUSSAM_KEY_NEAR,1);
}

int get_bus_list_num_of_items(void){
	if(actual_view == Favorites)
		return list_favorites_num_of_items;
	else if(actual_view == Near)
		return list_nearby_num_of_items;
	else
		return list_details_num_of_items;
}

int get_list_type(void){
	return listType;
}

BusStopListItem* get_bus_stop_list_near_at_index(int index) {
	if (index < 0 || index >= BUS_STOP_LIST_MAX_ITEMS) {
		return NULL;
	} else {
		return &bus_stop_list_near[index];
	}
}

void bus_stop_scroll_append(char *number, char *name, char *lines, int favorite) {

	if (list_favorites_num_of_items == BUS_STOP_LIST_MAX_ITEMS) {
		return;
	}

	APP_LOG(APP_LOG_LEVEL_INFO, "Parada recibida: %s", number);

	if(get_load_in_progress() == ListTypeNear){
		strcpy(bus_stop_list_near[list_favorites_num_of_items].number, number);
		strcpy(bus_stop_list_near[list_favorites_num_of_items].name, name);
		strcpy(bus_stop_list_near[list_favorites_num_of_items].lines, lines);
		bus_stop_list_near[list_favorites_num_of_items].favorite = favorite == 1;
		list_nearby_num_of_items++;
	}else if(get_load_in_progress() == ListTypeFavorites){
		strcpy(bus_stop_list_favorites[list_favorites_num_of_items].number, number);
		strcpy(bus_stop_list_favorites[list_favorites_num_of_items].name, name);
		strcpy(bus_stop_list_favorites[list_favorites_num_of_items].lines, lines);
		bus_stop_list_favorites[list_favorites_num_of_items].favorite = favorite == 1;
		list_favorites_num_of_items++;
	}


	if(listType == ListTypeNear && list_favorites_num_of_items == 1){
		vibes_short_pulse();
	}

//	hide_feedback_layers(true);
//	hide_bus_stop_detail_layers(false);
//	menu_layer_reload_data(ui.bus_stop_menu_layer);
}

void received_data(DictionaryIterator *iter, void *context){


	Tuple *append_stop_tuple = dict_find(iter, TUSSAM_KEY_APPEND_STOP);
	Tuple *stop_number_tuple = dict_find(iter, TUSSAM_KEY_STOP_NUMBER);
	Tuple *stop_name_tuple = dict_find(iter, TUSSAM_KEY_STOP_NAME);
	Tuple *stop_lines_tuple = dict_find(iter, TUSSAM_KEY_STOP_LINES);
	Tuple *stop_favorite = dict_find(iter, TUSSAM_KEY_STOP_FAVORITE);
	Tuple *no_bus_stops = dict_find(iter, TUSSAM_KEY_NO_BUS_STOPS);

	if (no_bus_stops) {
		APP_LOG(APP_LOG_LEVEL_INFO, "No bus stop in the received_data");
		no_bus_stop = true;
//		hide_bus_stop_detail_layers(true);
//		hide_feedback_layers(false);
//
//		if (listType == ListTypeFavorites) {
//			text_layer_set_text(ui.feedback_text_layer,"No favorite bus stops.\n\n Search it !.");
////			text_layer_set_text(ui.feedback_text_layer,"No favorite bus stops.\n\nLong press in times button to add/remove favorites.");
//		} else {
//			text_layer_set_text(ui.feedback_text_layer,"No nearby bus stops.");
//		}
		set_load_in_progress(-1);
	} else if (append_stop_tuple) {
		// update_load_in_progress();
		bus_stop_scroll_append(stop_number_tuple->value->cstring, stop_name_tuple->value->cstring, stop_lines_tuple->value->cstring, stop_favorite->value->int8);
	}
	// menu_layer_reload_data(ui.bus_stop_menu_layer);
	reload_menu();
	update_loading_feedback_favorites(true);
}


