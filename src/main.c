#include <pebble.h>
 
Window* window;
TextLayer *time_layer;
TextLayer *date_layer;
TextLayer *sec_layer;

char date_buffer[] = "Mon 01 Jan";
char time_buffer[] = "00:00";
char sec_buffer[] = "00";

//UPDATE MINUTES/TIME
void time_handler(struct tm *tick_time, TimeUnits units_changed)
{
//Format the buffer string using tick_time as the time source
strftime(time_buffer, sizeof("00:00"), "%H:%M", tick_time);

//Change the Time Layer text to show the new time!
text_layer_set_text(time_layer, time_buffer);
}

//UPDATE SECONDS
void sec_handler(struct tm *tick_time, TimeUnits units_changed)
{
//Format the buffer string using tick_time as the time source
strftime(date_buffer, sizeof(date_buffer), "%a %d %b", tick_time);
strftime(sec_buffer, sizeof(sec_buffer), "%S", tick_time);
strftime(time_buffer, sizeof(time_buffer), "%H:%M", tick_time);

//Change the Sec Layer text to show the new time!
text_layer_set_text(sec_layer, sec_buffer);
	
//Manually update Time Layer
if(time_buffer != text_layer_get_text(time_layer))
	text_layer_set_text(time_layer, time_buffer);

//Manually update Date Layer
if(date_buffer != text_layer_get_text(date_layer))
	text_layer_set_text(date_layer, date_buffer);
}

void window_load(Window *window)
{
//Load fonts
ResHandle date_font_handle = resource_get_handle(RESOURCE_ID_CICLE_FINA_28);
ResHandle time_font_handle = resource_get_handle(RESOURCE_ID_CICLE_GORDITA_62);
ResHandle sec_font_handle = resource_get_handle(RESOURCE_ID_CICLE_FINA_54);

//Date layer
date_layer = text_layer_create(GRect(0, 0, 144, 168));
text_layer_set_background_color(date_layer, GColorClear);
text_layer_set_text_color(date_layer, GColorWhite);
text_layer_set_text_alignment(date_layer, GTextAlignmentLeft);
text_layer_set_font(date_layer, fonts_load_custom_font(date_font_handle));

layer_add_child(window_get_root_layer(window), (Layer*) date_layer);	
	
//Time layer
time_layer = text_layer_create(GRect(0, 45, 144, 168));
text_layer_set_background_color(time_layer, GColorClear);
text_layer_set_text_color(time_layer, GColorWhite);
text_layer_set_text_alignment(time_layer, GTextAlignmentRight);
text_layer_set_font(time_layer, fonts_load_custom_font(time_font_handle));

layer_add_child(window_get_root_layer(window), (Layer*) time_layer);
	
//Sec layer
sec_layer = text_layer_create(GRect(0, 100, 144, 168));
text_layer_set_background_color(sec_layer, GColorClear);
text_layer_set_text_color(sec_layer, GColorWhite);
text_layer_set_text_alignment(sec_layer, GTextAlignmentRight);
text_layer_set_font(sec_layer, fonts_load_custom_font(sec_font_handle));

layer_add_child(window_get_root_layer(window), (Layer*) sec_layer);	

//Get a time structure so that the face doesn't start blank
struct tm *t;
time_t temp;	
temp = time(NULL);	
t = localtime(&temp);	

//Manually call the tick handler when the window is loading
sec_handler(t, SECOND_UNIT);
}
 
void window_unload(Window *window)
{
//Destroy TextLayer
text_layer_destroy(time_layer);
text_layer_destroy(date_layer);
text_layer_destroy(sec_layer);

}

void init()
{
//Initialize the app elements here!
window = window_create();
window_set_background_color(window, GColorBlack);
window_set_window_handlers(window, (WindowHandlers) {
.load = window_load,
.unload = window_unload,
});

tick_timer_service_subscribe(SECOND_UNIT, (TickHandler) sec_handler);

window_stack_push(window, true);
}
 
void deinit()
{
//De-initialize elements here to save memory!
tick_timer_service_unsubscribe();

window_destroy(window);
}
 
int main(void)
{
init();
app_event_loop();
deinit();
}