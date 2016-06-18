#include <pebble.h>
#include "data.h"
#include "keys.h"
#include "bus_stop_detail.h"
#include "bus_stop_list.h"
#include "ancillary_methods.h"
#include "communication.h"

BusStopListItem bus_stop_list_favorites[BUS_STOP_LIST_MAX_ITEMS];
BusStopListItem bus_stop_list_near[BUS_STOP_LIST_MAX_ITEMS];

bool loaded_favorites = false;
bool loaded_near = false;
bool loaded_detail = false;

bool no_bus_stop = false;

enum View actual_view = Favorites;

int list_favorites_num_of_items = 0;
int list_nearby_num_of_items = 0;
int list_details_num_of_items = 0;

StopDetailItem s_stop_detail;

enum ListType listType = ListTypeFavorites;


void add_bus_stop_to_list(char *number, char *name, char *lines, bool favorite, enum ListType listType);


BusStopListItem* get_bus_stop_list_favorites_at_index(int index) {
	if (index < 0 || index >= BUS_STOP_LIST_MAX_ITEMS) {
		return NULL;
	} else {
		return &bus_stop_list_favorites[index];
	}
}

BusStopListItem* get_bus_stop_list_near_at_index(int index) {
	if (index < 0 || index >= BUS_STOP_LIST_MAX_ITEMS) {
		return NULL;
	} else {
		return &bus_stop_list_near[index];
	}
}

BusStopListItem* get_bus_stop_list_at_index(int index) {
	if (index < 0 || index >= BUS_STOP_LIST_MAX_ITEMS) {
		return NULL;
	} else {
		if (get_actual_view() == Near)
			return &bus_stop_list_near[index];
		else if (get_actual_view() == Favorites)
			return &bus_stop_list_favorites[index];
		else
			return NULL;
	}
}

StopDetailItem* get_bus_stop_detail(void){
	return &s_stop_detail;
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
		listType = Favorites;
//		send_message(&iter, TUSSAM_KEY_FAVORITES,1);
		send_message(&iter, MESSAGE_KEY_favorites,1);
//		loadStopDetail("517");
//		send_message(&iter, TUSSAM_KEY_FETCH_STOP_DETAIL,"517");
//		s_stop_detail = NULL;
//		send_message(&iter, TUSSAM_KEY_NEAR,1);
	}else if (view == Near && !loaded_near){// && get_JS_is_ready()){
		listType = Near;
		send_message(&iter, MESSAGE_KEY_near,1);
//		send_message(&iter, TUSSAM_KEY_NEAR,1);
//		loadStopDetail("517");

		//send_message(&iter, TUSSAM_KEY_FAVORITES,1);
//		s_stop_detail = NULL;
	}else if (view == Details && get_JS_is_ready()){
		listType = Details;
		if(loaded_detail)
			s_stop_detail.number_of_lines = 0;
		loaded_detail = false;
		list_details_num_of_items = 0;
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
//	APP_LOG(APP_LOG_LEVEL_INFO, "Número de vista: %d", (int) actual_view);
	if(actual_view == Favorites)
		return list_favorites_num_of_items;
	else if(actual_view == Near)
		return list_nearby_num_of_items;
	else if(actual_view == Details)
		return list_details_num_of_items;
	else
		return 0;
}

bool get_bus_list_is_loaded(void){
	if(actual_view == Favorites)
		return loaded_favorites;
	else if(actual_view == Near)
		return loaded_near;
	else
		return false;
}

int get_list_type(void){
	return listType;
}



void bus_stop_scroll_append(char *number, char *name, char *lines, int favorite) {

	if (list_favorites_num_of_items == BUS_STOP_LIST_MAX_ITEMS) {
		return;
	}

	APP_LOG(APP_LOG_LEVEL_INFO, "Parada recibida: %s desde %d", number, get_load_in_progress());

	APP_LOG(APP_LOG_LEVEL_INFO, "load_in_progress actual: %d", (int) get_load_in_progress());

	if(get_load_in_progress() == TUSSAM_KEY_NEAR){
		add_bus_stop_to_list(number, name, lines, favorite == 1, ListTypeNear);
	}else if(get_load_in_progress() == TUSSAM_KEY_FAVORITES){
		add_bus_stop_to_list(number, name, lines, favorite == 1, ListTypeFavorites);
	}


	if(get_load_in_progress() == TUSSAM_KEY_NEAR && list_nearby_num_of_items == 1){
		vibes_short_pulse();
	}

//	hide_feedback_layers(true);
//	hide_bus_stop_detail_layers(false);
//	menu_layer_reload_data(ui.bus_stop_menu_layer);
}

void add_bus_stop_to_list(char *number, char *name, char *lines, bool favorite, enum ListType listType){
	if (listType == ListTypeNear){
		strcpy(bus_stop_list_near[list_nearby_num_of_items].number, number);
		strcpy(bus_stop_list_near[list_nearby_num_of_items].name, name);
		strcpy(bus_stop_list_near[list_nearby_num_of_items].lines, lines);
		bus_stop_list_near[list_nearby_num_of_items].favorite = favorite == 1;
		list_nearby_num_of_items++;
		loaded_near = true;
	}else if (listType == ListTypeFavorites){
		strcpy(bus_stop_list_favorites[list_favorites_num_of_items].number, number);
		strcpy(bus_stop_list_favorites[list_favorites_num_of_items].name, name);
		strcpy(bus_stop_list_favorites[list_favorites_num_of_items].lines, lines);
		bus_stop_list_favorites[list_favorites_num_of_items].favorite = favorite == 1;
		list_favorites_num_of_items++;
		loaded_favorites = true;
	}
}

static void line_list_append(char *number_stop, char *name_stop, char *name, char *bus1, char *bus2) {

	if (s_stop_detail.number_of_lines == BUS_STOP_DETAIL_LINES_MAX_ITEMS) {
		return;
	}

	strcpy(s_stop_detail.linesTimes[s_stop_detail.number_of_lines].name, name);
	strcpy(s_stop_detail.linesTimes[s_stop_detail.number_of_lines].bus1, bus1);
	strcpy(s_stop_detail.linesTimes[s_stop_detail.number_of_lines].bus2, bus2);
	strcpy(s_stop_detail.name, name_stop);
	strcpy(s_stop_detail.number, number_stop);

	s_stop_detail.number_of_lines++;
	list_details_num_of_items++;
	loaded_detail = true;
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
		if(get_load_in_progress() == TUSSAM_KEY_NEAR){
			loaded_near = true;
		}else if(get_load_in_progress() == TUSSAM_KEY_FAVORITES){
			loaded_favorites = true;
		}
		stop_list_reload_menu();
	} else if (append_stop_tuple) {
		// update_load_in_progress();
		bus_stop_scroll_append(stop_number_tuple->value->cstring, stop_name_tuple->value->cstring, stop_lines_tuple->value->cstring, stop_favorite->value->int8);
		stop_list_reload_menu();
	} else if (append_line_tuple){
		line_list_append(stop_number_tuple->value->cstring, stop_name_tuple->value->cstring, line_number_tuple->value->cstring, line_bus1_time_tuple->value->cstring, line_bus2_time_tuple->value->cstring);
		reload_details_menu();
		update_loading_feedback_details(true);
	}
	// menu_layer_reload_data(ui.bus_stop_menu_layer);
	// refresh_load_in_progress();

	APP_LOG(APP_LOG_LEVEL_INFO, "Received data procesed!");

}

void add_remove_bus_stop_to_favorites(int position_bus_stop) {
	show_log(APP_LOG_LEVEL_INFO, "crash2");

	uint32_t key;
	BusStopListItem *stopListItem = NULL;
	stopListItem = get_bus_stop_list_at_index(position_bus_stop);
	if (stopListItem->favorite) {
		key = TUSSAM_KEY_REMOVE_FAVORITE;
	} else {
		key = TUSSAM_KEY_ADD_FAVORITE;
	}

	APP_LOG(APP_LOG_LEVEL_ERROR, "OJO !! el método add_remove_bus_stop_to_favorites no espera la cola de envío");

	DictionaryIterator *iter;

	if (ancillary_app_message_outbox_begin(&iter) != APP_MSG_OK) {
		return;
	}
	if (dict_write_cstring(iter, key, stopListItem->number) != DICT_OK) {
		return;
	}
	ancillary_app_message_outbox_send();

	stopListItem->favorite = !stopListItem->favorite;
	if(stopListItem->favorite){
		add_bus_stop_to_list(stopListItem->number, stopListItem->name, stopListItem->lines, stopListItem->favorite, ListTypeFavorites);
	}

	stop_list_update_loading_feedback();

}



