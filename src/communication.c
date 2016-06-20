#include <pebble.h>
#include "keys.h"
#include "data.h"

/* # + # - # + # - # + # - # + # - # + # - # + # - # + #
 * # + # - # - >  Common Variables  < - # - # + #
 * # + # - # + # - # + # - # + # - # + # - # + # - # + # */

#define WAIT_RESPONSE (1500)
#define WAIT_EMPTY_QUEUE (500)
#define MESSAGE_QUEUE_LENGTH (20)

static bool is_ready = false;

enum Keys load_In_Progress = -1;

typedef struct {
	DictionaryIterator **iterator;
	uint32_t key;
	uint8_t value;
	char* valueChar;
} MessageQueue;

static int message_in_progess = (int) AppKeyJSReady;
AppTimer *timer_load_in_progress;
bool timer_load_in_progress_status = false;
AppTimer *timer_response;
bool timer_response_status = false;

static MessageQueue message_queue[MESSAGE_QUEUE_LENGTH];
static int message_queue_position = 0;
static int message_queue_position_lower = 0;




/* # + # - # + # - # + # - # + # - # + # - # + # - # + #
 * # + # - # - >  Declaration  < - # - # + #
 * # + # - # + # - # + # - # + # - # + # - # + # - # + # */
void wait_message_queue();
bool loadStopDetail(char *number);
AppMessageResult ancillary_app_message_outbox_begin(DictionaryIterator **iterator);
AppMessageResult ancillary_app_message_outbox_send(void);
void communication_init(void);
static void test_dropped_handler(AppMessageResult reason, void *context);
static void test_out_sent_handler(DictionaryIterator *sent, void *context);
static void test_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context);
void update_message_queue_position(bool increase);
bool send_message(DictionaryIterator **iterator, const uint32_t key, const uint8_t value);
void test_received_handler(DictionaryIterator *iter, void *context);




/* # + # - # + # - # + # - # + # - # + # - # + # - # + #
 * # + # - # - >  Methods.init  < - # - # + #
 * # + # - # + # - # + # - # + # - # + # - # + # - # + # */

void communication_init(void){
//	app_message_register_inbox_received(test_received_handler);
//	DictionaryIterator *iter;

	app_message_register_inbox_received(test_received_handler);
	app_message_register_inbox_dropped(test_dropped_handler);
	app_message_register_outbox_sent(test_out_sent_handler);
	app_message_register_outbox_failed(test_failed_handler);

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


}

AppMessageResult ancillary_app_message_outbox_begin(DictionaryIterator **iterator){
	AppMessageResult res = app_message_outbox_begin(iterator);
	if (res != APP_MSG_OK) {
		// Error establishing the connection
		APP_LOG(APP_LOG_LEVEL_ERROR, "Error establishing the connection: %d", (int)res);
	}
	return res;
}

AppMessageResult ancillary_app_message_outbox_send(void){
	AppMessageResult res = app_message_outbox_send();

	if(res != APP_MSG_OK ){
		APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the data: %d", (int)res);
	}

	return res;
}

static void test_dropped_handler(AppMessageResult reason, void *context) {
	// incoming message dropped
	APP_LOG(APP_LOG_LEVEL_ERROR, "Error catching data in test.");
	APP_LOG(APP_LOG_LEVEL_ERROR, "Reason: %d", (int) reason);
}

static void test_out_sent_handler(DictionaryIterator *sent, void *context) {
	// outgoing message was delivered
	APP_LOG(APP_LOG_LEVEL_INFO, "mensaje recibido en js satisfactoriamente");
}

static void test_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
	// outgoing message failed

	APP_LOG(APP_LOG_LEVEL_ERROR, "fallo en el envío del mensaje");

//	text_layer_set_text(ui.feedback_text_layer, "Connection error.");
////	hide_bus_stop_detail_layers(true);
//	hide_bus_stop_detail_layers(false);
//	hide_feedback_layers(false);
}




/* # + # - # + # - # + # - # + # - # + # - # + # - # + #
 * # + # - # - >  Methods.status  < - # - # + #
 * # + # - # + # - # + # - # + # - # + # - # + # - # + # */

bool get_JS_is_ready() {
	return is_ready;
}

void set_JS_is_ready(bool input){
	is_ready = input;
}

uint32_t get_load_in_progress(){ // Debería contener la key enviada en el mensaje
	return load_In_Progress;
}

void set_load_in_progress(int type){ // Debería contener la key enviada en el mensaje
	load_In_Progress = type;
}

void refresh_load_in_progress(){
//	load_In_Progress = type;
}

bool is_message_in_progess(){
	if (message_in_progess < 0 && get_JS_is_ready())
		return false;
	else
		return true;
}




/* # + # - # + # - # + # - # + # - # + # - # + # - # + #
 * # + # - # - >  Methods.queue  < - # - # + #
 * # + # - # + # - # + # - # + # - # + # - # + # - # + # */

void update_message_queue_position(bool increase){
	if(increase){
		if(message_queue_position == MESSAGE_QUEUE_LENGTH){
			message_queue_position = 0;
		}else{
			message_queue_position++;
		}
	}else{
		// quiero quitar un objeto de la cola
		if(message_queue_position_lower == MESSAGE_QUEUE_LENGTH)
			message_queue_position_lower = 0;
		else
			message_queue_position_lower++;
	}
}

void wait_message_queue(){
	if (get_JS_is_ready()
			&& message_in_progess < 0) {
		// Ready to send
		// && message_queue_position == message_queue_position_lower
		MessageQueue toSend = message_queue[message_queue_position_lower];
		update_message_queue_position(false);
		if(toSend.key != TUSSAM_KEY_FETCH_STOP_DETAIL)
			send_message(toSend.iterator, toSend.key, toSend.value);
		else
			loadStopDetail(toSend.valueChar);
	}else{
		APP_LOG(APP_LOG_LEVEL_INFO, "message_in_progess: %d ...", message_in_progess);
		APP_LOG(APP_LOG_LEVEL_INFO, "is_ready: %d ...", is_ready);
		APP_LOG(APP_LOG_LEVEL_INFO, "Pointer timer_load_in_progress= %p...", timer_load_in_progress);
		// Aquí es donde dice que no está inicializado timer_load_in_progress
//		app_timer_cancel(timer_load_in_progress);
		timer_load_in_progress = app_timer_register(WAIT_EMPTY_QUEUE, wait_message_queue,NULL);
		APP_LOG(APP_LOG_LEVEL_INFO, "Waiting is_ready %dms...", WAIT_EMPTY_QUEUE);

	}
}




/* # + # - # + # - # + # - # + # - # + # - # + # - # + #
 * # + # - # - >  Methods.send  < - # - # + #
 * # + # - # + # - # + # - # + # - # + # - # + # - # + # */

bool send_message(DictionaryIterator **iterator, const uint32_t key, const uint8_t value){

	if (is_message_in_progess()){
		message_queue[message_queue_position].iterator = iterator;
		message_queue[message_queue_position].key = key;
		message_queue[message_queue_position].value = value;
		update_message_queue_position(true);
		timer_load_in_progress = app_timer_register(WAIT_EMPTY_QUEUE, wait_message_queue,NULL);
		APP_LOG(APP_LOG_LEVEL_INFO, "Reshedule message: %dms", WAIT_EMPTY_QUEUE);
		return true;
	}else {
		DictionaryIterator *iter;

		message_in_progess = key;
		set_load_in_progress(key);
		AppMessageResult res = app_message_outbox_begin(&iter);
		bool result = true;

		if (res != APP_MSG_OK) {
			// Error establishing the connection
			APP_LOG(APP_LOG_LEVEL_ERROR, "Error establishing the connection: %d", (int)res);
			result = false;
		}
//		if (dict_write_cstring(*iterator, key, value) != DICT_OK && result) {
		if (dict_write_uint8(iter, key, value) != DICT_OK && result) {
			// Error writing data petition
			//return;
			result = false;
		}
		if (result) {
			res = app_message_outbox_send();
		}
		if(res == APP_MSG_OK) {
			APP_LOG(APP_LOG_LEVEL_INFO, "Message succesful sent! (code:%lu)", (unsigned long) key);
			result = true;
		} else {
			APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the data: %d", (int)res);
			//return;
			result = false;
		}
	return result;
	}
}

bool loadStopDetail(char *number) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "loadStopDetail");

	DictionaryIterator *iter;

	if (is_message_in_progess()) {
		message_queue[message_queue_position].iterator = &iter;
		message_queue[message_queue_position].key =
				TUSSAM_KEY_FETCH_STOP_DETAIL;
		message_queue[message_queue_position].valueChar = number;
		update_message_queue_position(true);
		timer_load_in_progress = app_timer_register(WAIT_EMPTY_QUEUE,
				wait_message_queue, NULL);
		APP_LOG(APP_LOG_LEVEL_INFO, "Reshedule message: %dms",
				WAIT_EMPTY_QUEUE);
		return true;
	} else {
		message_in_progess = MESSAGE_KEY_fetchStopDetail;
		set_load_in_progress(message_in_progess);
		AppMessageResult res = app_message_outbox_begin(&iter);
		bool result = true;

		if (res != APP_MSG_OK) {
			// Error establishing the connection
			APP_LOG(APP_LOG_LEVEL_ERROR, "Error establishing the connection: %d", (int )res);
			result = false;
		}
		if (dict_write_cstring(iter, MESSAGE_KEY_fetchStopDetail, number) != DICT_OK && result) {
			// Error writing data petition
			//return;
			result = false;
		}
		if (result) {
			res = app_message_outbox_send();
		}
		if (res != APP_MSG_OK) {
			APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the data: %d", (int )res);
			//return;
			result = false;
		} else {
			APP_LOG(APP_LOG_LEVEL_INFO, "Message succesful sent! (code:%lu)", (unsigned long) MESSAGE_KEY_fetchStopDetail);
			result = true;
		}
		return result;
	}
}

void no_message_in_progress(){
	APP_LOG(APP_LOG_LEVEL_INFO, "no_message_in_progress");
	message_in_progess = -1;
	load_In_Progress = -1;
	timer_load_in_progress_status = false;
}




/* # + # - # + # - # + # - # + # - # + # - # + # - # + #
 * # + # - # - >  Methods.receive  < - # - # + #
 * # + # - # + # - # + # - # + # - # + # - # + # - # + # */

void response_received(){
	if (!timer_load_in_progress_status){
		//app_timer_cancel(timer_response);
		timer_response = app_timer_register(WAIT_RESPONSE, no_message_in_progress, NULL);
		//message_in_progess = -1;
		timer_load_in_progress_status = true;
	}else
		app_timer_reschedule(timer_response, WAIT_RESPONSE);
}

void test_received_handler(DictionaryIterator *iter, void *context) {

	APP_LOG(APP_LOG_LEVEL_WARNING, "data received! (test)");

	Tuple *ready_tuple = dict_find(iter, MESSAGE_KEY_AppKeyJSReady);

	if (ready_tuple){
		APP_LOG(APP_LOG_LEVEL_INFO, "Ready tuple received");
		set_JS_is_ready(true);
		response_received();
		return;
	} else {
		received_data(iter, context);

		response_received();
	}
	APP_LOG(APP_LOG_LEVEL_WARNING, "data procesed!");
}


