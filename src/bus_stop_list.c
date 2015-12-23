#include <pebble.h>
#include "bus_stop_detail.h"
#include "keys.h"

enum ListType {
	ListTypeFavorites,
	ListTypeNear,
};

#define BUS_STOP_LIST_MAX_ITEMS (20)
#define BUS_STOP_NAME_TEXT_LENGTH (60)
#define BUS_STOP_NUMBER_TEXT_LENGTH (6)
#define BUS_STOP_LINES_TEXT_LENGTH (20)

static struct BusStopListUi {
	Window *window;
//	TextLayer *text_layer;
//	MenuLayer *menu_layer;
	
	ActionBarLayer *action_bar_layer;
	
	TextLayer *feedback_text_layer;
	TextLayer *bus_stop_number_text_layer;
	TextLayer *bus_stop_name_text_layer;
	TextLayer *bus_stop_lines_text_layer;
	
	GBitmap *previous_image;
	GBitmap *next_image;
	GBitmap *times_image;
	GBitmap *bus_image;
	GBitmap *favorite_image;
	
	BitmapLayer *bus_image_layer;
	BitmapLayer *favorite_image_layer;
	
} ui;

typedef struct {
	char number[BUS_STOP_NUMBER_TEXT_LENGTH];
	char name[BUS_STOP_NAME_TEXT_LENGTH];
	char lines[BUS_STOP_LINES_TEXT_LENGTH];
	bool favorite;
} BusStopListItem;

static enum ListType listType;

static BusStopListItem bus_stop_list_items[BUS_STOP_LIST_MAX_ITEMS];
static int bus_stop_list_num_of_items = 0;
static int bus_stop_list_active_item = -1;

static BusStopListItem* get_bus_stop_list_item_at_index(int index) {
	if (index < 0 || index >= BUS_STOP_LIST_MAX_ITEMS) {
		return NULL;
	} else {
		return &bus_stop_list_items[index];
	}
}

static void update_action_bar(void) {
	
	if (bus_stop_list_active_item == 0) {
		action_bar_layer_clear_icon(ui.action_bar_layer, BUTTON_ID_UP);
	} else {
		action_bar_layer_set_icon(ui.action_bar_layer, BUTTON_ID_UP, ui.previous_image);
	}
	
	if (bus_stop_list_active_item == bus_stop_list_num_of_items - 1) {
		action_bar_layer_clear_icon(ui.action_bar_layer, BUTTON_ID_DOWN);
	} else {
		action_bar_layer_set_icon(ui.action_bar_layer, BUTTON_ID_DOWN, ui.next_image);
	}
}

static void display_bus_stop_at_index(int index) {
	
	bus_stop_list_active_item = index;
	
	BusStopListItem *bus_stop_item = get_bus_stop_list_item_at_index(index);
	
	text_layer_set_text(ui.bus_stop_number_text_layer, bus_stop_item->number);
	text_layer_set_text(ui.bus_stop_name_text_layer, bus_stop_item->name);
	text_layer_set_text(ui.bus_stop_lines_text_layer, bus_stop_item->lines);
	layer_set_hidden(bitmap_layer_get_layer(ui.favorite_image_layer), !bus_stop_item->favorite);
	
	GRect nameRect = GRect(2, 30, 120, 80);
	GSize size = graphics_text_layout_get_content_size(bus_stop_item->name, fonts_get_system_font(FONT_KEY_GOTHIC_18), nameRect, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter);
	nameRect.origin.y = nameRect.origin.y + (nameRect.size.h - size.h) / 2;
	nameRect.size.h = size.h + 2;
	layer_set_frame(text_layer_get_layer(ui.bus_stop_name_text_layer), nameRect);
	
	update_action_bar();
}

static void hide_bus_stop_detail_layers(bool hide) {
	
	layer_set_hidden(text_layer_get_layer(ui.bus_stop_number_text_layer), hide);
	layer_set_hidden(text_layer_get_layer(ui.bus_stop_name_text_layer), hide);
	layer_set_hidden(text_layer_get_layer(ui.bus_stop_lines_text_layer), hide);
	layer_set_hidden(bitmap_layer_get_layer(ui.bus_image_layer), hide);
	layer_set_hidden(bitmap_layer_get_layer(ui.favorite_image_layer), hide);
	layer_set_hidden((Layer *)ui.action_bar_layer, hide);
}

static void hide_feedback_layers(bool hide) {
	
	layer_set_hidden(text_layer_get_layer(ui.feedback_text_layer), hide);
}

static void bus_stop_list_append(char *number, char *name, char *lines, int favorite) {
	
	if (bus_stop_list_num_of_items == BUS_STOP_LIST_MAX_ITEMS) {
		return;
	}
	
	strcpy(bus_stop_list_items[bus_stop_list_num_of_items].number, number);
	strcpy(bus_stop_list_items[bus_stop_list_num_of_items].name, name);
	strcpy(bus_stop_list_items[bus_stop_list_num_of_items].lines, lines);
	bus_stop_list_items[bus_stop_list_num_of_items].favorite = favorite == 1;
	bus_stop_list_num_of_items++;
	
	if (bus_stop_list_active_item < 0) {
		hide_bus_stop_detail_layers(false);
		hide_feedback_layers(true);
		display_bus_stop_at_index(0);
	}
	
	update_action_bar();
}

static void bus_stop_list_out_sent_handler(DictionaryIterator *sent, void *context) {
	// outgoing message was delivered
}


static void bus_stop_list_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
	// outgoing message failed
	
	text_layer_set_text(ui.feedback_text_layer, "Connection error.");
	hide_bus_stop_detail_layers(true);
	hide_feedback_layers(false);
}

static void bus_stop_list_in_received_handler(DictionaryIterator *iter, void *context) {
	
	Tuple *append_stop_tuple = dict_find(iter, TUSSAM_KEY_APPEND_STOP);
	Tuple *stop_number_tuple = dict_find(iter, TUSSAM_KEY_STOP_NUMBER);
	Tuple *stop_name_tuple = dict_find(iter, TUSSAM_KEY_STOP_NAME);
	Tuple *stop_lines_tuple = dict_find(iter, TUSSAM_KEY_STOP_LINES);
	Tuple *stop_favorite = dict_find(iter, TUSSAM_KEY_STOP_FAVORITE);
	Tuple *no_bus_stops = dict_find(iter, TUSSAM_KEY_NO_BUS_STOPS);
	
	if (no_bus_stops) {
				
		hide_bus_stop_detail_layers(true);
		hide_feedback_layers(false);

		if (listType == ListTypeFavorites) {
			text_layer_set_text(ui.feedback_text_layer,"No favorite bus stops.\n\nLong press in times button to add/remove favorites.");
		} else {
			text_layer_set_text(ui.feedback_text_layer,"No nearby bus stops.");
		}
		
	} else if (append_stop_tuple) {
		
		bus_stop_list_append(stop_number_tuple->value->cstring, stop_name_tuple->value->cstring, stop_lines_tuple->value->cstring, stop_favorite->value->int8);
	}
}


static void bus_stop_list_in_dropped_handler(AppMessageResult reason, void *context) {
	// incoming message dropped
	APP_LOG(APP_LOG_LEVEL_DEBUG, "in_dropped_handler");
}

static void loadFavoritesStops(void) {
	
	DictionaryIterator *iter;
	
	if (app_message_outbox_begin(&iter) != APP_MSG_OK) {
		return;
	}
	if (dict_write_uint8(iter, TUSSAM_KEY_FAVORITES, 1) != DICT_OK) {
		return;
	}
	app_message_outbox_send();
	
}

static void loadNearStops(void) {
	
	DictionaryIterator *iter;
	
	if (app_message_outbox_begin(&iter) != APP_MSG_OK) {
		return;
	}
	if (dict_write_uint8(iter, TUSSAM_KEY_NEAR, 1) != DICT_OK) {
		return;
	}
	app_message_outbox_send();
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
	
	if (bus_stop_list_active_item > 0) {
		display_bus_stop_at_index(bus_stop_list_active_item - 1);
	}
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {

	BusStopListItem *stopListItem = get_bus_stop_list_item_at_index(bus_stop_list_active_item);
	bus_stop_detail_show(stopListItem->number, stopListItem->name);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {

	if (bus_stop_list_active_item + 1 < bus_stop_list_num_of_items) {
		display_bus_stop_at_index(bus_stop_list_active_item + 1);
	}
}

static void add_remove_bus_stop_to_favorites() {
	
	uint32_t key;
	BusStopListItem *stopListItem = get_bus_stop_list_item_at_index(bus_stop_list_active_item);
	
	if (stopListItem->favorite) {
		key = TUSSAM_KEY_REMOVE_FAVORITE;
	} else {
		key = TUSSAM_KEY_ADD_FAVORITE;
	}
	
	DictionaryIterator *iter;
	
	if (app_message_outbox_begin(&iter) != APP_MSG_OK) {
		return;
	}
	if (dict_write_cstring(iter, key, stopListItem->number) != DICT_OK) {
		return;
	}
	app_message_outbox_send();
	
	stopListItem->favorite = !stopListItem->favorite;
	display_bus_stop_at_index(bus_stop_list_active_item);
	
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {

	APP_LOG(APP_LOG_LEVEL_DEBUG, "select_long_click_handler");
	add_remove_bus_stop_to_favorites();
}

static void click_config_provider(void *ctx) {
	window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
	window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
	window_long_click_subscribe(BUTTON_ID_SELECT, 0, select_long_click_handler, NULL);
}

static void show_loading_feedback() {
	
	if (listType == ListTypeFavorites) {
		text_layer_set_text(ui.feedback_text_layer,"Loading favorite bus stops...");
	} else {
		text_layer_set_text(ui.feedback_text_layer,"Loading nearby bus stops...");
	}
	hide_bus_stop_detail_layers(true);
	hide_feedback_layers(false);
}

// This initializes the menu upon window load
static void bus_stop_window_load(Window *window) {
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "bus_stop_window_load");
	
	ui.window = window;
	
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_frame(window_layer);
	
	// Number Text Layer
	ui.bus_stop_number_text_layer = text_layer_create(GRect(4, 0, 40, 24));
	text_layer_set_text_color(ui.bus_stop_number_text_layer, GColorBlack);
	text_layer_set_font(ui.bus_stop_number_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_text_alignment(ui.bus_stop_number_text_layer, GTextAlignmentLeft);
	layer_add_child(window_layer, text_layer_get_layer(ui.bus_stop_number_text_layer));
	
	// Name Text Label
	ui.bus_stop_name_text_layer = text_layer_create(GRect(2, 30, 120, 80));
	text_layer_set_text_color(ui.bus_stop_name_text_layer, GColorBlack);
	text_layer_set_font(ui.bus_stop_name_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
	text_layer_set_text_alignment(ui.bus_stop_name_text_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(ui.bus_stop_name_text_layer));
	
	// Lines Text Layer
	ui.bus_stop_lines_text_layer = text_layer_create(GRect(32, 112, 90, 28));
	text_layer_set_text_color(ui.bus_stop_lines_text_layer, GColorBlack);
	text_layer_set_font(ui.bus_stop_lines_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	text_layer_set_text_alignment(ui.bus_stop_lines_text_layer, GTextAlignmentLeft);
	layer_add_child(window_layer, text_layer_get_layer(ui.bus_stop_lines_text_layer));
	
	// Feedback Text Layer
	ui.feedback_text_layer = text_layer_create(bounds);
	text_layer_set_text_color(ui.feedback_text_layer, GColorBlack);
	text_layer_set_font(ui.feedback_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	text_layer_set_text_alignment(ui.feedback_text_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(ui.feedback_text_layer));
	
	// Images
	ui.previous_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_BAR_UP);
	ui.next_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_BAR_DOWN);
	ui.times_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_BAR_CLOCK);
	ui.bus_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MENU_ICON);
	ui.bus_image_layer = bitmap_layer_create(GRect(2, 112, 30, 28));
	bitmap_layer_set_bitmap(ui.bus_image_layer, ui.bus_image);
	layer_add_child(window_layer, bitmap_layer_get_layer(ui.bus_image_layer));
	ui.favorite_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_FAVORITE);
	ui.favorite_image_layer = bitmap_layer_create(GRect(100, 10, 14, 14));
	bitmap_layer_set_bitmap(ui.favorite_image_layer, ui.favorite_image);
	layer_add_child(window_layer, bitmap_layer_get_layer(ui.favorite_image_layer));
	
	
	// Action Bar Layer
	ui.action_bar_layer = action_bar_layer_create();
    action_bar_layer_set_click_config_provider(ui.action_bar_layer, click_config_provider);
//	action_bar_layer_set_background_color(ui.action_bar_layer, GColorWhite);
    action_bar_layer_set_icon(ui.action_bar_layer, BUTTON_ID_UP, ui.previous_image);
    action_bar_layer_set_icon(ui.action_bar_layer, BUTTON_ID_SELECT, ui.times_image);
    action_bar_layer_set_icon(ui.action_bar_layer, BUTTON_ID_DOWN, ui.next_image);
	action_bar_layer_add_to_window(ui.action_bar_layer, window);
	
	show_loading_feedback();
	
	if (listType == ListTypeNear) {
		loadNearStops();
	} else {
		loadFavoritesStops();
	}
}

// Deinitialize resources on window unload that were initialized on window load
static void bus_stop_window_unload(Window *window) {
	
	action_bar_layer_destroy(ui.action_bar_layer);
	
	text_layer_destroy(ui.feedback_text_layer);
	text_layer_destroy(ui.bus_stop_number_text_layer);
	text_layer_destroy(ui.bus_stop_name_text_layer);
	text_layer_destroy(ui.bus_stop_lines_text_layer);
	
	bitmap_layer_destroy(ui.bus_image_layer);
	bitmap_layer_destroy(ui.favorite_image_layer);
	
	bus_stop_list_num_of_items = 0;
	bus_stop_list_active_item = -1;
}

static void bus_stop_window_appear(Window *window) {
	
}

static void register_app_message_callbacks() {
	
	app_message_register_inbox_received(bus_stop_list_in_received_handler);
	app_message_register_inbox_dropped(bus_stop_list_in_dropped_handler);
	app_message_register_outbox_sent(bus_stop_list_out_sent_handler);
	app_message_register_outbox_failed(bus_stop_list_out_failed_handler);
}

static void load_view_for_bus_stops_type(int _listType) {
	
	listType = _listType;
	register_app_message_callbacks();
	window_stack_push(ui.window, true /* Animated */);
}

void bus_stop_list_show_favorites(void) {
	
	load_view_for_bus_stops_type(ListTypeFavorites);
}

void bus_stop_list_show_near(void) {
	
	load_view_for_bus_stops_type(ListTypeNear);
}

void bus_stop_list_init(void) {
	
	ui.window = window_create();
	
	// Setup the window handlers
	window_set_window_handlers(ui.window, (WindowHandlers) {
		.load = bus_stop_window_load,
		.unload = bus_stop_window_unload,
		.appear = bus_stop_window_appear
	});
}

void bus_stop_list_deinit(void) {
	window_destroy(ui.window);

}