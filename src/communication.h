#pragma once

bool get_JS_is_ready(void);
void set_JS_is_ready(bool input);

AppMessageResult ancillary_app_message_outbox_begin(
		DictionaryIterator **iterator);

AppMessageResult ancillary_app_message_outbox_send(void);

bool send_message(DictionaryIterator **iterator, const uint32_t key, const uint8_t value);

void response_received(void);

void test_received_handler(DictionaryIterator *iter, void *context);

void communication_init(void);

uint32_t get_load_in_progress(void);

void set_load_in_progress(int type);

void refresh_load_in_progress(void);

bool loadStopDetail(char *number);
