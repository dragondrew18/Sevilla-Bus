#include <pebble.h>
#include "main_menu.h"
#include "bus_stop_list.h"
#include "bus_stop_detail.h"

static void init(void) {

	main_menu_init();
	bus_stop_list_init();
	bus_stop_detail_init();
	
	app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit(void) {
	
	main_menu_deinit();
	bus_stop_list_deinit();
	bus_stop_detail_deinit();
}

int main(void) {
	
  init();
  app_event_loop();
  deinit();

  return 0;
}