#include <pebble.h>
#include "data.h"
#include "keys.h"
#include "bus_stop_detail.h"
#include "bus_stop_list.h"
#include "ancillary_methods.h"
#include "communication.h"

/* # + # - # + # - # + # - # + # - # + # - # + # - # + #
 * # + # - # - >  Common Variables  < - # - # + #
 * # + # - # + # - # + # - # + # - # + # - # + # - # + # */

BusStopListItem bus_stop_list_favorites[BUS_STOP_LIST_MAX_ITEMS];
BusStopListItem bus_stop_list_near[BUS_STOP_LIST_MAX_ITEMS];
StopDetailItem s_stop_detail;

bool loaded_favorites = false;
bool loaded_near = false;
bool loaded_detail = false;

bool error_js_favorites = false;
bool error_js_near = false;
bool error_js_detail = false;


int list_favorites_num_of_items = 0;
int list_nearby_num_of_items = 0;
int list_details_num_of_items = 0;

enum View actual_view = Favorites;
enum Stop_List_Type stop_list_type = Stop_List_Favorites;




/* # + # - # + # - # + # - # + # - # + # - # + # - # + #
 * # + # - # - >  Declaration  < - # - # + #
 * # + # - # + # - # + # - # + # - # + # - # + # - # + # */

void add_bus_stop_to_list(char *number, char *name, char *lines, bool favorite, enum Stop_List_Type listType);
void set_error_js(bool input);
int find_index_from_stop_number(char* number, enum View list_to_find);
char* join_number_lines_from_bus_stop_details(StopDetailItem* input, char *lines);


/* # + # - # + # - # + # - # + # - # + # - # + # - # + #
 * # + # - # - >  Methods.actual_view  < - # - # + #
 * # + # - # + # - # + # - # + # - # + # - # + # - # + # */

enum View get_actual_view(void){
	return actual_view;
}

void set_actual_view(enum View view){
	APP_LOG(APP_LOG_LEVEL_INFO, "Changing to view: %d", view);
	actual_view = view;

	DictionaryIterator *iter;

	if(view == Favorites){
//	if(view == Favorites && !loaded_favorites) && get_JS_is_ready()){
		stop_list_type = Stop_List_Favorites;
		if(!loaded_favorites){
//			send_message(&iter, TUSSAM_KEY_FAVORITES,1);
			send_message(&iter, MESSAGE_KEY_favorites,1);
//			loadStopDetail("517");
//			send_message(&iter, TUSSAM_KEY_FETCH_STOP_DETAIL,"517");
//			s_stop_detail = NULL;
//			send_message(&iter, TUSSAM_KEY_NEAR,1);
		}else{
			stop_list_update_loading_feedback();
		}
	}else if (view == Near){// && get_JS_is_ready()){
		stop_list_type = Stop_List_Near;
		if(!loaded_near){
			send_message(&iter, MESSAGE_KEY_near,1);
//			send_message(&iter, TUSSAM_KEY_NEAR,1);
//			loadStopDetail("517");
//			send_message(&iter, TUSSAM_KEY_FAVORITES,1);
		}
		stop_list_update_loading_feedback();
	}else if (view == Details){
		s_stop_detail.number_of_lines = 0;
		loaded_detail = false;
		list_details_num_of_items = 0;
//		send_message(&iter, TUSSAM_KEY_NEAR,1);
//		s_stop_detail = NULL;
	}
}



/* # + # - # + # - # + # - # + # - # + # - # + # - # + #
 * # + # - # - >  Methods.stop_list  < - # - # + #
 * # + # - # + # - # + # - # + # - # + # - # + # - # + # */

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

int get_actual_view_list_size(void){
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

int get_actual_message_bus_list_size(void){
//	APP_LOG(APP_LOG_LEVEL_INFO, "Número de vista: %d", (int) actual_view);
	if(get_load_in_progress() == MESSAGE_KEY_favorites)
		return list_favorites_num_of_items;
	else if(get_load_in_progress() == MESSAGE_KEY_near)
		return list_nearby_num_of_items;
	else if(get_load_in_progress() == MESSAGE_KEY_fetchStopDetail)
		return list_details_num_of_items;
	else
		return 0;
}

bool get_is_loaded(void){
	if(actual_view == Favorites)
		return loaded_favorites;
	else if(actual_view == Near)
		return loaded_near;
	else if (actual_view == Details)
		return loaded_detail;
	else
		return false;
}

int get_stop_list_type(void){
	return stop_list_type;
}

void received_add_bus_stop_to_list(char *number, char *name, char *lines, int favorite) {

	if (get_actual_message_bus_list_size() == BUS_STOP_LIST_MAX_ITEMS) {
		return;
	}

//	APP_LOG(APP_LOG_LEVEL_INFO, "Parada recibida: %s desde %lu", number, get_load_in_progress());

//	APP_LOG(APP_LOG_LEVEL_INFO, "load_in_progress actual: %d", (int) get_load_in_progress());

	if(get_load_in_progress() == TUSSAM_KEY_NEAR){
		add_bus_stop_to_list(number, name, lines, favorite == 1, Stop_List_Near);
	}else if(get_load_in_progress() == TUSSAM_KEY_FAVORITES){
		add_bus_stop_to_list(number, name, lines, favorite == 1, Stop_List_Favorites);
	}


	if(get_load_in_progress() == TUSSAM_KEY_NEAR && list_nearby_num_of_items == 1){
		vibes_short_pulse();
	}

//	hide_feedback_layers(true);
//	hide_bus_stop_detail_layers(false);
//	menu_layer_reload_data(ui.bus_stop_menu_layer);
}

void add_bus_stop_to_list(char *number, char *name, char *lines, bool favorite, enum Stop_List_Type listType){
	if (listType == Stop_List_Near){
		strcpy(bus_stop_list_near[list_nearby_num_of_items].number, number);
		strcpy(bus_stop_list_near[list_nearby_num_of_items].name, name);
		strcpy(bus_stop_list_near[list_nearby_num_of_items].lines, lines);
		bus_stop_list_near[list_nearby_num_of_items].favorite = favorite == 1;
		list_nearby_num_of_items++;
		loaded_near = true;
	}else if (listType == Stop_List_Favorites){
		strcpy(bus_stop_list_favorites[list_favorites_num_of_items].number, number);
		strcpy(bus_stop_list_favorites[list_favorites_num_of_items].name, name);
		strcpy(bus_stop_list_favorites[list_favorites_num_of_items].lines, lines);
		bus_stop_list_favorites[list_favorites_num_of_items].favorite = favorite == 1;
		list_favorites_num_of_items++;
		loaded_favorites = true;
	}
}

static void line_list_append(char *number_stop, char *name_stop, bool favorite, char *name, char *bus1, char *bus2) {

	if (s_stop_detail.number_of_lines == BUS_STOP_DETAIL_LINES_MAX_ITEMS) {
		return;
	}

	strcpy(s_stop_detail.linesTimes[s_stop_detail.number_of_lines].name, name);
	strcpy(s_stop_detail.linesTimes[s_stop_detail.number_of_lines].bus1, bus1);
	strcpy(s_stop_detail.linesTimes[s_stop_detail.number_of_lines].bus2, bus2);
	strcpy(s_stop_detail.name, name_stop);
	strcpy(s_stop_detail.number, number_stop);
	s_stop_detail.favorite = favorite == 1;

	s_stop_detail.number_of_lines++;
	list_details_num_of_items++;
	loaded_detail = true;
}



/* # + # - # + # - # + # - # + # - # + # - # + # - # + #
 * # + # - # - >  Methods.stop_detail  < - # - # + #
 * # + # - # + # - # + # - # + # - # + # - # + # - # + # */

StopDetailItem* get_bus_stop_detail(void){
	return &s_stop_detail;
}

void define_stop_detail(char *number, char *name){
	strcpy(s_stop_detail.name, name);
	strcpy(s_stop_detail.number, number);
	s_stop_detail.favorite = 0;
	APP_LOG(APP_LOG_LEVEL_INFO, "stop_detail definido: %s (%s)", s_stop_detail.name, s_stop_detail.number);
	loadStopDetail(s_stop_detail.number);
	show_log(APP_LOG_LEVEL_INFO, "solicitados los datos de la parada");

}

void received_data(DictionaryIterator *iter, void *context){
	APP_LOG(APP_LOG_LEVEL_ERROR, "El método data.received_data debe moverse a communication.c");

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
	Tuple *error_tuple = dict_find(iter, MESSAGE_KEY_fail);

	set_error_js(false);

	if (no_bus_stops) {
		APP_LOG(APP_LOG_LEVEL_INFO, "No bus stop in the received_data");

		if(get_load_in_progress() == TUSSAM_KEY_NEAR){
			loaded_near = true;
		}else if(get_load_in_progress() == TUSSAM_KEY_FAVORITES){
			loaded_favorites = true;
		}else if(get_load_in_progress() == TUSSAM_KEY_FETCH_STOP_DETAIL){
			loaded_detail = true;
		}
		stop_list_reload_menu();
	} else if (append_stop_tuple) {
		// update_load_in_progress();
		received_add_bus_stop_to_list(stop_number_tuple->value->cstring, stop_name_tuple->value->cstring, stop_lines_tuple->value->cstring, stop_favorite->value->int8);
		stop_list_reload_menu();
	} else if (append_line_tuple){
		line_list_append(stop_number_tuple->value->cstring, stop_name_tuple->value->cstring, stop_favorite->value->int8, line_number_tuple->value->cstring, line_bus1_time_tuple->value->cstring, line_bus2_time_tuple->value->cstring);
		stop_detail_update_loading_feedback();
		stop_detail_reload_menu();
	} else if (error_tuple){
		set_error_js(true);
		APP_LOG(APP_LOG_LEVEL_INFO, "error loading %d", (int) get_load_in_progress());
		if(get_load_in_progress() == TUSSAM_KEY_NEAR || get_load_in_progress() == TUSSAM_KEY_FAVORITES){
			stop_list_update_loading_feedback();
		}else if(get_load_in_progress() == TUSSAM_KEY_FETCH_STOP_DETAIL){
			stop_detail_update_loading_feedback();
		}
	}
	// menu_layer_reload_data(ui.bus_stop_menu_layer);
	// refresh_load_in_progress();

	show_log(APP_LOG_LEVEL_INFO, "Received data procesed!");
}



/* # + # - # + # - # + # - # + # - # + # - # + # - # + #
 * # + # - # - >  Methods.others  < - # - # + #
 * # + # - # + # - # + # - # + # - # + # - # + # - # + # */

void set_error_js(bool input){
	if(get_load_in_progress() == MESSAGE_KEY_favorites){
		error_js_favorites = input;
	} else if (get_load_in_progress() == MESSAGE_KEY_near){
		error_js_near = input;
	} else if (get_load_in_progress() == MESSAGE_KEY_fetchStopDetail){
		error_js_detail = input;
	}
}

bool get_has_error_js(void){
	if(get_actual_view() == Favorites){
		return error_js_favorites;
	} else if (get_actual_view() == Near){
		return error_js_near;
	} else if (get_actual_view() == Details){
		return error_js_detail;
	} else {
		return true;
	}
}

void add_remove_bus_stop_to_favorites(int input, char *number) {
	APP_LOG(APP_LOG_LEVEL_ERROR, "El método add_remove_bus_stop_to_favorites está inacabado");
	//Only one of both

	show_log(APP_LOG_LEVEL_INFO, "crash2");

	uint32_t key;
	bool favState = false;
	char *name;
	char lines[BUS_STOP_LINES_TEXT_LENGTH];
	BusStopListItem *stopListItem;
	int pos_fav, pos_nearby;

	if(input >= 0){
		stopListItem = get_bus_stop_list_at_index(input);
		favState = stopListItem->favorite;
		number = stopListItem->number;
		name = stopListItem->name;
	}else{
		if(strcmp(number, get_bus_stop_detail()->number) == 0)
			favState = get_bus_stop_detail()->favorite;

		name = get_bus_stop_detail()->name;
	}

	pos_fav = find_index_from_stop_number(number, Favorites);
	pos_nearby = find_index_from_stop_number(number, Near);

	if (favState) {
		key = TUSSAM_KEY_REMOVE_FAVORITE;
	} else {
		key = TUSSAM_KEY_ADD_FAVORITE;
		if(input < 0){
			strcpy(lines, join_number_lines_from_bus_stop_details(get_bus_stop_detail(), lines));
		}else{
			strcpy(lines, stopListItem->lines);
		}
	}

	APP_LOG(APP_LOG_LEVEL_ERROR, "OJO !! el método add_remove_bus_stop_to_favorites NO espera la cola de envío");

	DictionaryIterator *iter;

	if (ancillary_app_message_outbox_begin(&iter) != APP_MSG_OK) {
		return;
	}
	if (dict_write_cstring(iter, key, number) != DICT_OK) {
		return;
	}
	ancillary_app_message_outbox_send();

	if(get_actual_view() == Details){
		s_stop_detail.favorite = !s_stop_detail.favorite;
		stop_detail_reload_menu();
	}


	if(pos_fav < 0){
		add_bus_stop_to_list(number, name, lines, !favState, Stop_List_Favorites);
	}else{
		stopListItem = get_bus_stop_list_favorites_at_index(pos_fav);
		stopListItem->favorite = !stopListItem->favorite;
	}

	if(pos_nearby >= 0){
		stopListItem = get_bus_stop_list_near_at_index(pos_nearby);
		stopListItem->favorite = !stopListItem->favorite;
	}

	if(pos_fav >= 0 || pos_nearby >= 0)
		stop_list_update_loading_feedback();


}

char* join_number_lines_from_bus_stop_details(StopDetailItem* input, char* result){
	for(int a = 0; a < input->number_of_lines; a++){
		char *line = input->linesTimes[a].name;
		if (a == 0){
			strcpy(result, line);
		}else{
			strcat(result, ", ");
			strcat(result, line);
		}

	}
	APP_LOG(APP_LOG_LEVEL_INFO, "Lineas resultantes: '%s'", result);
	return result;
}

int find_index_from_stop_number(char* number, enum View list_to_find){
	int size;
	int result = -1;
	BusStopListItem *bus_stop_list;

	if(list_to_find == Favorites){
		bus_stop_list = bus_stop_list_favorites;
		size = list_favorites_num_of_items;
	}else if (list_to_find == Near) {
		bus_stop_list = bus_stop_list_near;
		size = list_nearby_num_of_items;
	}
	else{
		bus_stop_list = NULL;
		size = 0;
	}
	for(int i = 0; i < size; i++){
		if(strcmp(number, bus_stop_list[i].number) == 0)
			return i;
	}

	return result;
}


