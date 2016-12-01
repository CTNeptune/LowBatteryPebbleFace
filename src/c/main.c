#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_batteryLevel_layer;
static Layer *s_battery_layer;
static int s_battery_level;
static GBitmap *s_battery_icon;

void battery_callback(BatteryChargeState state) {
  // Record the new battery level
  s_battery_level = state.charge_percent;
  // Update meter
  layer_mark_dirty(s_battery_layer);
  // Write to buffer and display
  static char s_battery_buffer[32];
  snprintf(s_battery_buffer, sizeof(s_battery_buffer), "%d%%", state.charge_percent);
  text_layer_set_text(s_batteryLevel_layer, s_battery_buffer);
  layer_mark_dirty(text_layer_get_layer(s_batteryLevel_layer));
}
static void battery_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Find the width of the bar
  int width = (int)(float)(((float)s_battery_level / 100.0F) * 10.0F);

  // Draw the background
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Draw the bar
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 0, GCornerNone);
}
static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";
  // Write the current hour
  strftime(buffer, sizeof(buffer), "%l:%M", tick_time);
  
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
}
static void main_window_load(Window *window) {
  // Create TextLayer
  s_time_layer = text_layer_create(GRect(0, 64, 144, 32));
  s_batteryLevel_layer = text_layer_create(GRect(40,48,32,16));
  
  text_layer_set_background_color(s_batteryLevel_layer,GColorClear);
  text_layer_set_text_color(s_batteryLevel_layer,GColorBlack);
  
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  
  // Improve the layout to be more like a watchface
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  text_layer_set_font(s_batteryLevel_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_batteryLevel_layer, GTextAlignmentRight);
  
  s_battery_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY);
  
  static BitmapLayer *s_bitmap_layer;
  s_bitmap_layer = bitmap_layer_create(GRect(76,52,14,12));
  bitmap_layer_set_bitmap(s_bitmap_layer,s_battery_icon);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bitmap_layer));
  // Create battery meter Layer
  s_battery_layer = layer_create(GRect(77, 54, 10, 8));
  layer_set_update_proc(s_battery_layer, battery_update_proc);
  // Add to Window
  layer_add_child(window_get_root_layer(window), s_battery_layer);
  // Add text layer as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_batteryLevel_layer));
  layer_mark_dirty(s_battery_layer);
  
}
static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
}
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}
static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Make sure the time is displayed from the start
  update_time();
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  // Register for battery level updates
  battery_state_service_subscribe(battery_callback);
  // Ensure battery level is displayed from the start
  battery_callback(battery_state_service_peek());
  
  window_stack_push(s_main_window, true);
}
static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
  layer_destroy(s_battery_layer);
}
int main(void) {
  init();
  app_event_loop();
  deinit();
}
