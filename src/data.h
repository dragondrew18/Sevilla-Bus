#pragma once

#define BUS_STOP_LIST_MAX_ITEMS (40)
#define BUS_STOP_NAME_TEXT_LENGTH (60)
#define BUS_STOP_NUMBER_TEXT_LENGTH (6)
#define BUS_STOP_LINES_TEXT_LENGTH (20)
#define BUS_STOP_DETAIL_NAME_TEXT_LENGTH (60)
#define BUS_STOP_DETAIL_NUMBER_TEXT_LENGTH (6)
#define BUS_STOP_DETAIL_LINES_MAX_ITEMS (10)
#define BUS_STOP_DETAIL_LINE_NAME_TEXT_LENGTH (10)
#define BUS_STOP_DETAIL_LINE_TIME_TEXT_LENGTH (20)

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

typedef struct {
	char name[BUS_STOP_DETAIL_LINE_NAME_TEXT_LENGTH];
	char bus1[BUS_STOP_DETAIL_LINE_TIME_TEXT_LENGTH];
	char bus2[BUS_STOP_DETAIL_LINE_TIME_TEXT_LENGTH];
} LineTimesItem;

typedef struct {
	char number[BUS_STOP_DETAIL_NUMBER_TEXT_LENGTH];
	char name[BUS_STOP_DETAIL_NAME_TEXT_LENGTH];
	LineTimesItem linesTimes[BUS_STOP_DETAIL_LINES_MAX_ITEMS];
	int number_of_lines;
} StopDetailItem;

BusStopListItem* get_bus_stop_list_favorites_at_index(int index);

BusStopListItem* get_bus_stop_list_at_index(int index);

BusStopListItem* get_bus_stop_list_near_at_index(int index);

void bus_stop_scroll_append(char *number, char *name, char *lines, int favorite);

void received_data(DictionaryIterator *iter, void *context);

int get_bus_list_num_of_items(void);

bool get_bus_list_is_loaded(void);

int get_list_type(void);

int get_actual_view(void);

void set_actual_view(int view);

StopDetailItem* get_bus_stop_detail(void);

void define_stop_detail(char *number, char *name);

void add_remove_bus_stop_to_favorites(int position_bus_stop);

