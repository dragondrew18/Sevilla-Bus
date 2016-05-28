#include <pebble.h>
#include "data.h"
#include "keys.h"
#include "favorites_bus_stop.h"
#include "bus_stop_detail.h"
#include "ancillary_methods.h"

static BusStopListItem bus_stop_list_favorites[BUS_STOP_LIST_MAX_ITEMS];
static BusStopListItem bus_stop_list_near[BUS_STOP_LIST_MAX_ITEMS];

static bool loaded_favorites = false;
static bool loaded_near = false;
static bool loaded_detail = false;

static bool no_bus_stop = false;

static enum View actual_view = Favorites;

static int list_favorites_num_of_items = 0;
static int list_nearby_num_of_items = 0;
static int list_details_num_of_items = 0;

static StopDetailItem s_stop_detail;

static enum ListType listType = ListTypeFavorites;

BusStopListItem* get_bus_stop_list_favorites_at_index(int index) {
	if (index < 0 || index >= BUS_STOP_LIST_MAX_ITEMS) {
		return NULL;
	} else {
		return &bus_stop_list_favorites[index];
	}
}

StopDetailItem get_bus_stop_detail(void){
	return s_stop_detail;
}

int get_actual_view(void){
	return actual_view;
}

void set_actual_view(int view){
	APP_LOG(APP_LOG_LEVEL_INFO, "Changing to view: %d", view);
	actual_view = view;

	DictionaryIterator *iter;

	if(view == Favorites && !loaded_favorites){// && get_JS_is_ready()){
//	if(view == Favorites && !loaded_favorites) && get_JS_is_ready()){
		send_message(&iter, TUSSAM_KEY_FAVORITES,1);
//		loadStopDetail("517");
//		send_message(&iter, TUSSAM_KEY_FETCH_STOP_DETAIL,"517");

//		s_stop_detail = NULL;
//		send_message(&iter, TUSSAM_KEY_NEAR,1);
	}else if (view == Near && !loaded_near){// && get_JS_is_ready()){
		send_message(&iter, TUSSAM_KEY_NEAR,1);
		//send_message(&iter, TUSSAM_KEY_FAVORITES,1);
//		s_stop_detail = NULL;
	}else if (view == Details && get_JS_is_ready()){
//		send_message(&iter, TUSSAM_KEY_NEAR,1);
//		s_stop_detail = NULL;
	}
}

void define_stop_detail(char *number, char *name){
	strcpy(s_stop_detail.name, name);
	strcpy(s_stop_detail.number, number);
	APP_LOG(APP_LOG_LEVEL_INFO, "stop_detail definido: %s (%s)", s_stop_detail.name, s_stop_detail.number);
	loadStopDetail(s_stop_detail.number);
	APP_LOG(APP_LOG_LEVEL_INFO, "solicitados los datos de la parada");

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

	APP_LOG(APP_LOG_LEVEL_INFO, "Parada recibida: %s desde %d", number, get_load_in_progress());

	if(get_load_in_progress() == ListTypeNear){
		strcpy(bus_stop_list_near[list_favorites_num_of_items].number, number);
		strcpy(bus_stop_list_near[list_favorites_num_of_items].name, name);
		strcpy(bus_stop_list_near[list_favorites_num_of_items].lines, lines);
		bus_stop_list_near[list_favorites_num_of_items].favorite = favorite == 1;
		list_nearby_num_of_items++;
		loaded_near = true;
	}else if(get_load_in_progress() == ListTypeFavorites){
		strcpy(bus_stop_list_favorites[list_favorites_num_of_items].number, number);
		strcpy(bus_stop_list_favorites[list_favorites_num_of_items].name, name);
		strcpy(bus_stop_list_favorites[list_favorites_num_of_items].lines, lines);
		bus_stop_list_favorites[list_favorites_num_of_items].favorite = favorite == 1;
		list_favorites_num_of_items++;
		loaded_favorites = true;
	}


	if(get_load_in_progress() == ListTypeNear && list_nearby_num_of_items == 1){
		vibes_short_pulse();
	}

//	hide_feedback_layers(true);
//	hide_bus_stop_detail_layers(false);
//	menu_layer_reload_data(ui.bus_stop_menu_layer);
}

static void line_list_append(char *name, char *bus1, char *bus2) {

	if (s_stop_detail.number_of_lines == BUS_STOP_DETAIL_LINES_MAX_ITEMS) {
		return;
	}

	strcpy(s_stop_detail.linesTimes[s_stop_detail.number_of_lines].name, name);
	strcpy(s_stop_detail.linesTimes[s_stop_detail.number_of_lines].bus1, bus1);
	strcpy(s_stop_detail.linesTimes[s_stop_detail.number_of_lines].bus2, bus2);
	s_stop_detail.number_of_lines++;
	list_details_num_of_items++;



}

void received_data(DictionaryIterator *iter, void *context){


	Tuple *append_stop_tuple = dict_find(iter, TUSSAM_KEY_APPEND_STOP);
	Tuple *stop_number_tuple = dict_find(iter, TUSSAM_KEY_STOP_NUMBER);
	Tuple *stop_name_tuple = dict_find(iter, TUSSAM_KEY_STOP_NAME);
	Tuple *stop_lines_tuple = dict_find(iter, TUSSAM_KEY_STOP_LINES);
	Tuple *stop_favorite = dict_find(iter, TUSSAM_KEY_STOP_FAVORITE);
	Tuple *no_bus_stops = dict_find(iter, TUSSAM_KEY_NO_BUS_STOPS);
	Tuple *append_line_tuple = dict_find(iter, TUSSAM_KEY_APPEND_LINE);
	Tuple *line_number_tuple = dict_find(iter, TUSSAM_KEY_LINE_NUMBER);
	Tuple *line_bus1_time_tuple = dict_find(iter, TUSSAM_KEY_BUS_1_TIME);
	Tuple *line_bus2_time_tuple = dict_find(iter, TUSSAM_KEY_BUS_2_TIME);


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
	} else if (append_stop_tuple) {
		// update_load_in_progress();
		bus_stop_scroll_append(stop_number_tuple->value->cstring, stop_name_tuple->value->cstring, stop_lines_tuple->value->cstring, stop_favorite->value->int8);
		reload_menu();
		update_loading_feedback_favorites(true);
	} else if (append_line_tuple){
		line_list_append(line_number_tuple->value->cstring, line_bus1_time_tuple->value->cstring, line_bus2_time_tuple->value->cstring);
		reload_details_menu();
		update_loading_feedback_details(true);
	}
	// menu_layer_reload_data(ui.bus_stop_menu_layer);
	// refresh_load_in_progress();

	APP_LOG(APP_LOG_LEVEL_INFO, "Received data procesed!");

}


