#include <pebble.h>
 
Window* window;
static TextLayer *time_layer;
static TextLayer *date_layer;
static TextLayer *sec_layer;
static TextLayer *battery_layer;
static BitmapLayer *baticon_layer;
static GBitmap *battery_icon;

static char date_buffer[] = "Mon 01 Jan";
static char time_buffer[] = "12:00";
static char sec_buffer[] = "00";
char *time_format;

static bool status_showing = false;
static bool battery_hide = false;
static AppTimer *display_timer;

// Battery
static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100%";

	if (charge_state.charge_percent <= 80) battery_icon = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_ICON_75);
	if (charge_state.charge_percent <= 50) battery_icon = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_ICON_50);
	if (charge_state.charge_percent <= 30) battery_icon = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_ICON_25);
	if (charge_state.charge_percent <= 10) battery_icon = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_ICON_0);
	
	bitmap_layer_set_bitmap(baticon_layer, battery_icon);
	
  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "chrging");
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
  }
  text_layer_set_text(battery_layer, battery_text);
	if (charge_state.is_plugged) battery_hide = false;
	else if (charge_state.charge_percent <= 30) battery_hide = false;
	else if (status_showing) battery_hide = false;
	else battery_hide = true;
	layer_set_hidden(text_layer_get_layer(battery_layer), battery_hide);
	layer_set_hidden(bitmap_layer_get_layer(baticon_layer), battery_hide);
	
}

//UPDATE TIME
void time_handler(struct tm *tick_time, TimeUnits units_changed)
{
if(clock_is_24h_style()) {
time_format = "%R";
} else {
time_format = "%I:%M";
}
//Format the buffer string using tick_time as the time source
strftime(date_buffer, sizeof(date_buffer), "%a %d %b", tick_time);
strftime(sec_buffer, sizeof(sec_buffer), "%S", tick_time);
strftime(time_buffer, sizeof(time_buffer), time_format, tick_time);
	
//Change the Sec Layer text to show the new time!
text_layer_set_text(sec_layer, sec_buffer);
	
//Manually update Time Layer
if(time_buffer != text_layer_get_text(time_layer))
	text_layer_set_text(time_layer, time_buffer);

//Manually update Date Layer
if(date_buffer != text_layer_get_text(date_layer))
	text_layer_set_text(date_layer, date_buffer);
}

// Hides battery
void hide_status() {
	status_showing = false;
	layer_set_hidden(text_layer_get_layer(battery_layer), true);
	layer_set_hidden(bitmap_layer_get_layer(baticon_layer), true);	
}
// Shows battery
void show_status() {
	status_showing = true;
	//Poll for battery
	handle_battery(battery_state_service_peek());
	// 4 Sec timer then call "hide_status". Cancels previous timer if another show_status is called within the 4000ms
	app_timer_cancel(display_timer);
	display_timer = app_timer_register(4000, hide_status, NULL);
}

// Shake/Tap Handler. On shake/tap... call "show_status"
void tap_handler(AccelAxisType axis, int32_t direction) {
	show_status();	
}

void window_load(Window *window)
{
//Load fonts
ResHandle date_font_handle = resource_get_handle(RESOURCE_ID_CICLE_GORDITA_30);
ResHandle time_font_handle = resource_get_handle(RESOURCE_ID_CICLE_GORDITA_55);
ResHandle sec_font_handle = resource_get_handle(RESOURCE_ID_CICLE_FINA_50);
ResHandle bat_font_handle = resource_get_handle(RESOURCE_ID_CICLE_FINA_18);

//Date layer
date_layer = text_layer_create(GRect(0, 0, 144, 168));
text_layer_set_background_color(date_layer, GColorClear);
#ifdef PBL_COLOR
	text_layer_set_text_color(date_layer, GColorMelon);
#else
	text_layer_set_text_color(date_layer, GColorWhite);
#endif
text_layer_set_text_alignment(date_layer, GTextAlignmentLeft);
text_layer_set_font(date_layer, fonts_load_custom_font(date_font_handle));

layer_add_child(window_get_root_layer(window), (Layer*) date_layer);	
	
//Time layer
time_layer = text_layer_create(GRect(0, 50, 144, 168));
text_layer_set_background_color(time_layer, GColorClear);
text_layer_set_text_color(time_layer, GColorWhite);
text_layer_set_text_alignment(time_layer, GTextAlignmentRight);
text_layer_set_font(time_layer, fonts_load_custom_font(time_font_handle));

layer_add_child(window_get_root_layer(window), (Layer*) time_layer);
	
//Sec layer
sec_layer = text_layer_create(GRect(0, 100, 144, 168));
text_layer_set_background_color(sec_layer, GColorClear);
#ifdef PBL_COLOR
	text_layer_set_text_color(sec_layer, GColorMelon);
#else
	text_layer_set_text_color(sec_layer, GColorWhite);
#endif
text_layer_set_text_alignment(sec_layer, GTextAlignmentRight);
text_layer_set_font(sec_layer, fonts_load_custom_font(sec_font_handle));

layer_add_child(window_get_root_layer(window), (Layer*) sec_layer);
	
//Battery layer
battery_layer = text_layer_create(GRect(14, 130, 144, 168));
text_layer_set_background_color(battery_layer, GColorClear);
text_layer_set_text_color(battery_layer, GColorWhite);
text_layer_set_text_alignment(battery_layer, GTextAlignmentLeft);
//text_layer_set_font(battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
text_layer_set_font(battery_layer, fonts_load_custom_font(bat_font_handle));

layer_add_child(window_get_root_layer(window), (Layer*) battery_layer);

//ICON
battery_icon = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_ICON);
baticon_layer = bitmap_layer_create(GRect(0,132,12,18));
bitmap_layer_set_background_color(baticon_layer, GColorClear);

layer_add_child(window_get_root_layer(window), (Layer*) baticon_layer);	

//Get a time structure so that the face doesn't start blank
struct tm *t;
time_t temp;	
temp = time(NULL);	
t = localtime(&temp);	

//Manually call the tick handler when the window is loading
time_handler(t, SECOND_UNIT);
}
 
void window_unload(Window *window)
{
//Destroy TextLayer
text_layer_destroy(time_layer);
text_layer_destroy(date_layer);
text_layer_destroy(sec_layer);
text_layer_destroy(battery_layer);
bitmap_layer_destroy(baticon_layer);
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

tick_timer_service_subscribe(SECOND_UNIT, (TickHandler) time_handler);
battery_state_service_subscribe(&handle_battery);
accel_tap_service_subscribe(tap_handler);
	

window_stack_push(window, true);

	//comment this out if you don't want battery level on start
show_status();

}
 
void deinit()
{
//De-initialize elements here to save memory!
tick_timer_service_unsubscribe();
battery_state_service_unsubscribe();
accel_tap_service_unsubscribe();

window_destroy(window);
}
 
int main(void)
{
init();
app_event_loop();
deinit();
}