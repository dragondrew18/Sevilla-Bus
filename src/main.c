#include <pebble.h>
#include "ancillary_methods.h"
#include "bus_stop_detail.h"
#include "favorites_bus_stop.h"
#include "bus_stop_number_select.h"

static void init(void) {
	ancillary_message_context();

	app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);

	uint32_t inbox_size = heap_bytes_free() / 4;
	uint32_t outbox_size = heap_bytes_free() / 4;
	if (inbox_size > app_message_inbox_size_maximum()){
		inbox_size = app_message_inbox_size_maximum();
	}

	if (outbox_size > app_message_inbox_size_maximum()){
		outbox_size = app_message_inbox_size_maximum();
	}
	APP_LOG(APP_LOG_LEVEL_INFO, "Maxium message: %lu, %lu", (unsigned long) inbox_size, (unsigned long) outbox_size);

	app_message_open(inbox_size, outbox_size); // It's necessary by aplite heap. Maximum cause no communication

	ancillary_init();

	favorites_bus_stop_init();
	bus_stop_detail_init();
	win_edit_init();
}

static void deinit(void) {
	
	bus_stop_detail_deinit();

	favorites_bus_stop_deinit();
	win_edit_deinit();
}

int main(void) {
	
  init();
  app_event_loop();
  deinit();

  return 0;
}
