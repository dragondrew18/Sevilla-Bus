#include <pebble.h>
#include "bus_stop_detail.h"
#include "favorites_bus_stop.h"
#include "bus_stop_number_select.h"

static void init(void) {

	favorites_bus_stop_init();

	bus_stop_detail_init();
	win_edit_init();

	app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit(void) {
	
	bus_stop_detail_deinit();

	favorites_bus_stop_deinit();

}

int main(void) {
	
  init();
  app_event_loop();
  deinit();

  return 0;
}
