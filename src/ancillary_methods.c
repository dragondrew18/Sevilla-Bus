#include <pebble.h>
#include "keys.h"
#include "data.h"

static bool is_ready = false;

#define WAIT_RESPONSE (1500)
#define WAIT_EMPTY_QUEUE (500)
#define MESSAGE_QUEUE_LENGTH (20)

static int load_In_Progress = -1;

void wait_message_queue(); // Declaration
bool loadStopDetail(char *number);


typedef struct {
	DictionaryIterator **iterator;
	uint32_t key;
	uint8_t value;
} MessageQueue;

static int message_in_progess = (int) AppKeyJSReady;
AppTimer *timer_load_in_progress;
bool timer_load_in_progress_status = false;
AppTimer *timer_response;
bool timer_response_status = false;

static MessageQueue message_queue[MESSAGE_QUEUE_LENGTH];
static int message_queue_position = 0;
static int message_queue_position_lower = 0;


/**
 * PROBLEMA ! ! ! ! ! No recibe mensajes (no se sabe si envía pero primero debe recibir el OK y no lo hace)
 */
bool get_JS_is_ready() {
	return is_ready;
}

int get_load_in_progress(){ // Debería contener la key enviada en el mensaje
	return load_In_Progress;
}

void set_load_in_progress(int type){ // Debería contener la key enviada en el mensaje
	load_In_Progress = type;
}

void refresh_load_in_progress(){
//	load_In_Progress = type;
}

void set_JS_is_ready(bool input){
	APP_LOG(APP_LOG_LEVEL_INFO, "is_ready_previus: %d, is_ready_finish: %d", is_ready, input);

	is_ready = input;
}

AppMessageResult ancillary_app_message_outbox_begin(DictionaryIterator **iterator);

AppMessageResult ancillary_app_message_outbox_send(void);

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

//static void loadFavoritesStops(void) {
//
//	DictionaryIterator *iter;
//
//	send_message(&iter, TUSSAM_KEY_FAVORITES,1);
//}
//
//static void loadNearStops(void) {
//
//	DictionaryIterator *iter;
//
//	send_message(&iter, TUSSAM_KEY_NEAR,1);
//}

bool is_message_in_progess(){
	if (message_in_progess < 0 && get_JS_is_ready())
		return false;
	else
		return true;
}

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
		message_in_progess = key;
		set_load_in_progress(get_list_type());
		AppMessageResult res = app_message_outbox_begin(iterator);
		bool result = true;

		if (res != APP_MSG_OK) {
			// Error establishing the connection
			APP_LOG(APP_LOG_LEVEL_ERROR, "Error establishing the connection: %d", (int)res);
			result = false;
		}
//		if (dict_write_cstring(*iterator, key, value) != DICT_OK && result) {
		if (dict_write_uint8(*iterator, key, value) != DICT_OK && result) {
			// Error writing data petition
			//return;
			result = false;
		}
		if (result) {
			res = app_message_outbox_send();
		}
		if(res != APP_MSG_OK) {
			APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the data: %d", (int)res);
			//return;
			result = true;
		} else {
			APP_LOG(APP_LOG_LEVEL_INFO, "Message succesful sent! (code:%lu)", (unsigned long) key);
			result = false;
		}
	return result;
	}
}

void no_message_in_progress(){
	APP_LOG(APP_LOG_LEVEL_INFO, "no_message_in_progress");
	message_in_progess = -1;
	timer_load_in_progress_status = false;
}

void response_received(){
	if (!timer_load_in_progress_status){
		//app_timer_cancel(timer_response);
		timer_response = app_timer_register(WAIT_RESPONSE, no_message_in_progress, NULL);
		//message_in_progess = -1;
		timer_load_in_progress_status = true;
	}else
		app_timer_reschedule(timer_response, WAIT_RESPONSE);
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
			loadStopDetail("517");
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


void test_received_handler(DictionaryIterator *iter, void *context) {

	APP_LOG(APP_LOG_LEVEL_WARNING, "data received! (test)");
	APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "data received! (test)");

	Tuple *ready_tuple = dict_find(iter, AppKeyJSReady);

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


void graphics_draw_text_vertically_center(GContext* ctx,
		const char* text, GFont const font, const GRect box,
		const GTextOverflowMode overflow_mode, const GTextAlignment alignment) {

	GSize size = graphics_text_layout_get_content_size(text, font, box,
				overflow_mode, alignment);
	GRect box_2 = box;
	box_2.origin.y += (box.size.h - size.h)/2 - 3;

	graphics_draw_text(ctx, text, font, box_2, overflow_mode, alignment, NULL);

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

void ancillary_init(){
//	app_message_register_inbox_received(test_received_handler);
	DictionaryIterator *iter;

}

static void test_dropped_handler(AppMessageResult reason, void *context) {
	// incoming message dropped
	APP_LOG(APP_LOG_LEVEL_ERROR, "Error catching data in test. Reason: %d", (int) reason);
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

void ancillary_message_context(){
	APP_LOG(APP_LOG_LEVEL_INFO, "ancillary_message_context loaded");

	app_message_register_inbox_received(test_received_handler);
	app_message_register_inbox_dropped(test_dropped_handler);
	app_message_register_outbox_sent(test_out_sent_handler);
	app_message_register_outbox_failed(test_failed_handler);

}

bool loadStopDetail(char *number) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "loadStopDetail");

	DictionaryIterator *iter;

	if (is_message_in_progess()) {
		message_queue[message_queue_position].iterator = &iter;
		message_queue[message_queue_position].key =
				TUSSAM_KEY_FETCH_STOP_DETAIL;
		message_queue[message_queue_position].value = 0;
		update_message_queue_position(true);
		timer_load_in_progress = app_timer_register(WAIT_EMPTY_QUEUE,
				wait_message_queue, NULL);
		APP_LOG(APP_LOG_LEVEL_INFO, "Reshedule message: %dms",
				WAIT_EMPTY_QUEUE);
		return true;
	} else {
		message_in_progess = TUSSAM_KEY_FETCH_STOP_DETAIL;
		set_load_in_progress(get_list_type());
		AppMessageResult res = app_message_outbox_begin(&iter);
		static bool result = true;

		if (res != APP_MSG_OK) {
			// Error establishing the connection
			APP_LOG(APP_LOG_LEVEL_ERROR, "Error establishing the connection: %d", (int )res);
			result = false;
		}
		if (dict_write_cstring(iter, TUSSAM_KEY_FETCH_STOP_DETAIL, number) != DICT_OK && result) {
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
			result = true;
		} else {
			APP_LOG(APP_LOG_LEVEL_INFO, "Message succesful sent! (code:%lu)", (unsigned long ) TUSSAM_KEY_FETCH_STOP_DETAIL);
			result = false;
		}
		return result;
	}
}

