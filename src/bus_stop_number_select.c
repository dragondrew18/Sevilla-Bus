#include <pebble.h>
#include "keys.h"
#include "bus_stop_detail.h"
#include "favorites_bus_stop.h"
#include "ancillary_methods.h"


static void time_window_load(Window* window);
static void time_window_unload(Window* window);
static void click_config_provider(void *context);

#define NUMBER_BUS_STOP_MAX_LENGTH 4 // Maximum number length
#define NUMBER_HEIGHT 32 // height of each number
#define NUMBER_WIDTH 26 // width of each number
#define CLICK_TIME 70
// #define CLICK_TIME 2
// #define SPACE_BETWEEN_NUMBERS (3)


static struct BusStopNumberSelectUi {
	Window *s_time_window;
	Layer *s_canvas_layer;
	TextLayer *s_input_layers[NUMBER_BUS_STOP_MAX_LENGTH]; //Every number
} ui;

static int stop_numbers[NUMBER_BUS_STOP_MAX_LENGTH];

static char s_value_buffers[NUMBER_BUS_STOP_MAX_LENGTH][NUMBER_BUS_STOP_MAX_LENGTH];
static int s_selection; // Actual position

//#ifdef PBL_RECT
	#define OFFSET_LEFT 3
	#define OFFSET_TOP 0
	#define ITEM_HEIGHT 28
	#define OFFSET_ITEM_TOP 0
/*#else
	#define OFFSET_LEFT 18
	#define OFFSET_TOP 11
	#define ITEM_HEIGHT 40
	#define OFFSET_ITEM_TOP 6
#endif*/

#define SPACING_BETWEEN_NUMBERS (16)

static const GPathInfo PATH_INFO = {
		.num_points = 3,
		.points = (GPoint []) {{0, -5}, {5,5}, {-5, 5}}
};
static GPath *s_my_path_ptr;

static int space_number_width;

// Menu stuff
#define MENU_SECTION_WEEKDAYS 1
#define MENU_SECTION_OK 0


#ifndef PBL_PLATFORM_APLITE
/*
static DictationSession *s_dictation_session;
static void dictation_session_callback(DictationSession *session, DictationSessionStatus status,
						char *transcription, void *context) {

	if (status == DictationSessionStatusSuccess) {
		strncpy(temp_alarm.description, transcription, sizeof(temp_alarm.description));
	}
	else { // fallback to old solution
		tertiary_text_show(temp_alarm.description);
	}
}*/
#endif

void win_edit_init(void){
	ui.s_time_window = window_create();

	window_set_window_handlers(ui.s_time_window, (WindowHandlers) {
		.load = time_window_load,
		.unload = time_window_unload,
	});

	#ifndef PBL_PLATFORM_APLITE
//	s_dictation_session = dictation_session_create(sizeof(temp_alarm.description),
//			dictation_session_callback, NULL);
	#endif
//	check_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_CHECK_INV);
//	check_icon_inv = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_CHECK);

	s_my_path_ptr= gpath_create(&PATH_INFO);
}

void win_edit_show(void){
	s_selection = 0;

	window_stack_push(ui.s_time_window,true);
}

static void update_ui(Layer *layer, GContext *ctx) {

	for(int i = 0; i < NUMBER_BUS_STOP_MAX_LENGTH; i++) {
//		text_layer_set_background_color(ui.s_input_layers[i],
//				(i == s_selection) ?
//						PBL_IF_COLOR_ELSE(GColorBlue, GColorBlack) :
//						PBL_IF_COLOR_ELSE(GColorDarkGray, GColorLightGray));
		text_layer_set_background_color(ui.s_input_layers[i],PBL_IF_COLOR_ELSE(GColorDarkGray, GColorLightGray));
		text_layer_set_text_color(ui.s_input_layers[i],GColorWhite);
		if(i==s_selection){
			text_layer_set_background_color(ui.s_input_layers[i],PBL_IF_COLOR_ELSE(GColorBlue, GColorBlack));
			GPoint selection_center = {
					.x = (int16_t) i * (NUMBER_WIDTH + space_number_width) + OFFSET_LEFT + space_number_width/2 + NUMBER_WIDTH/2,
					.y = (int16_t) 60 + OFFSET_TOP - 2 * space_number_width,
			};
			gpath_rotate_to(s_my_path_ptr, 0);
			gpath_move_to(s_my_path_ptr, selection_center);
//			graphics_context_set_fill_color(ctx,
//					PBL_IF_COLOR_ELSE(GColorBlue, GColorBlack));
			graphics_context_set_fill_color(ctx,GColorBlack);
			gpath_draw_filled(ctx, s_my_path_ptr);
			gpath_rotate_to(s_my_path_ptr, TRIG_MAX_ANGLE/2);
			selection_center.y += NUMBER_HEIGHT + 4 * space_number_width;
			gpath_move_to(s_my_path_ptr, selection_center);
			gpath_draw_filled(ctx, s_my_path_ptr);
		}

		snprintf(s_value_buffers[i], sizeof("9"), "%1d", stop_numbers[i]);
		text_layer_set_text(ui.s_input_layers[i], s_value_buffers[i]);
	}

/*	#ifdef PBL_ROUND
	// draw graphical representations as rings around the border
	GRect bounds = layer_get_bounds(layer);
	int hour_angle = (s_digits[0] * 360) / 12;
	graphics_context_set_fill_color(ctx, s_selection==0?GColorBlue:GColorDarkGray);
	graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, 10, 0, DEG_TO_TRIGANGLE(hour_angle));
	int minute_angle = (s_digits[1] * 360) / 60;
	bounds = grect_inset(bounds,GEdgeInsets(9));
	graphics_context_set_fill_color(ctx, s_selection==1?GColorBlue:GColorDarkGray);
	graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, 9, 0, DEG_TO_TRIGANGLE(minute_angle));

	#endif*/
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
	// Next column
	if(s_selection == NUMBER_BUS_STOP_MAX_LENGTH - 1) {
		char char_array[]="0000";
		char *puntero_char_array;
		for(int i = 0; i < NUMBER_BUS_STOP_MAX_LENGTH; i++) {
			char_array[i] = '0' + stop_numbers[i];
		}
		puntero_char_array = char_array;

		bus_stop_detail_show(puntero_char_array, "");
	}else
		s_selection++;
		layer_mark_dirty(ui.s_canvas_layer);
}

static void back_click_handler(ClickRecognizerRef recognizer, void *context) {
	// Previous column
	s_selection--;

	if(s_selection == -1) {
		set_list_type_to_near();
		window_stack_pop(true);
	}
	else
		layer_mark_dirty(ui.s_canvas_layer);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
	if(stop_numbers[s_selection] < 9)
		stop_numbers[s_selection] += 1;
	else
		stop_numbers[s_selection] = 0;

	layer_mark_dirty(ui.s_canvas_layer);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
	if(stop_numbers[s_selection] > 0)
		stop_numbers[s_selection] -= 1;
	else
		stop_numbers[s_selection] = 9;

	layer_mark_dirty(ui.s_canvas_layer);
}

static void click_config_provider(void *context) {
	window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
	window_single_repeating_click_subscribe(BUTTON_ID_UP, CLICK_TIME, up_click_handler);
	window_single_repeating_click_subscribe(BUTTON_ID_DOWN, CLICK_TIME, down_click_handler);
	window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
}

static void calculateWidth(GRect bounds){
	space_number_width = (bounds.size.w
			- NUMBER_WIDTH * NUMBER_BUS_STOP_MAX_LENGTH - 2 * OFFSET_LEFT)
			/ NUMBER_BUS_STOP_MAX_LENGTH;
}

static void time_window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	calculateWidth(bounds);
	// init hands
	ui.s_canvas_layer = layer_create(bounds);
	layer_set_update_proc(ui.s_canvas_layer, update_ui);
	layer_add_child(window_layer, ui.s_canvas_layer);

	for(int i = 0; i < NUMBER_BUS_STOP_MAX_LENGTH; i++) {
		ui.s_input_layers[i] =
			text_layer_create(
//				GRect(
//						(s_withampm?3:30) + i * (SPACING_BETWEEN_NUMBERS + SPACING_BETWEEN_NUMBERS) + OFFSET_LEFT,
//						60 + OFFSET_TOP, 40, 40));
					GRect(
							OFFSET_LEFT + space_number_width / 2 + i * (space_number_width + NUMBER_WIDTH),
							60 + OFFSET_TOP, NUMBER_WIDTH, NUMBER_HEIGHT));
/*		#ifdef PBL_COLOR
			text_layer_set_text_color(s_input_layers[i], GColorWhite);
			text_layer_set_background_color(s_input_layers[i], GColorDarkGray);
		#else*/
			text_layer_set_text_color(ui.s_input_layers[i], GColorBlack);
			text_layer_set_background_color(ui.s_input_layers[i], GColorWhite);
		// #endif
		stop_numbers[i] = 0;
		text_layer_set_text(ui.s_input_layers[i], "0");
		text_layer_set_font(ui.s_input_layers[i], fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
		text_layer_set_text_alignment(ui.s_input_layers[i], GTextAlignmentCenter);
		layer_add_child(window_layer, text_layer_get_layer(ui.s_input_layers[i]));
	}
	window_set_click_config_provider(window, click_config_provider);
	layer_mark_dirty(ui.s_canvas_layer);
}

static void time_window_unload(Window *window) {
	for(int i = 0; i < NUMBER_BUS_STOP_MAX_LENGTH; i++) {
		text_layer_destroy(ui.s_input_layers[i]);
	}
	layer_destroy(ui.s_canvas_layer);
	//window_destroy(window);
}
void win_edit_deinit(void){
	window_destroy(ui.s_time_window);
}

