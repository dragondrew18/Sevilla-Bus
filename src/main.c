#include <pebble.h>
#include "communication.h"
#include "bus_stop_detail.h"
#include "bus_stop_list.h"
#include "bus_stop_number_select.h"

static void init(void) {
	communication_init();
	stop_list_init();

	stop_list_show_favorites();

	stop_detail_init();
	win_edit_init();

//	bus_stop_detail_show("526", "");
}

static void deinit(void) {
	
	stop_detail_deinit();

	stop_list_deinit();

	win_edit_deinit();
}

int main(void) {
	
	init();
	app_event_loop();
	deinit();

	return 0;
}
