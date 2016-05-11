#pragma once

#define BUS_STOP_LIST_MAX_ITEMS (20)
#define BUS_STOP_NAME_TEXT_LENGTH (60)
#define BUS_STOP_NUMBER_TEXT_LENGTH (6)
#define BUS_STOP_LINES_TEXT_LENGTH (20)

typedef struct {
	char number[BUS_STOP_NUMBER_TEXT_LENGTH];
	char name[BUS_STOP_NAME_TEXT_LENGTH];
	char lines[BUS_STOP_LINES_TEXT_LENGTH];
	bool favorite;
} BusStopListItem;

enum ListType {
	ListTypeFavorites,
	ListTypeNear,
};

enum View {
	Favorites,
	Near,
	NumberSelect,
	Details,
};

BusStopListItem* get_bus_stop_list_favorites_at_index(int index);

BusStopListItem* get_bus_stop_list_near_at_index(int index);

void bus_stop_scroll_append(char *number, char *name, char *lines, int favorite);

void received_data(DictionaryIterator *iter, void *context);

int get_bus_list_num_of_items(void);

int get_list_type(void);

int get_actual_view(void);

void set_actual_view(int view);
