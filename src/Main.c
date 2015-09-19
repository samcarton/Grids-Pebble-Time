#include <pebble.h>
#include "gbitmap_color_palette_manipulator.h"
  
  
static bool is24hrFormat = true;

Window* _window;
static Layer* window_layer;

// Time
static BitmapLayer* hours_tens_layer = NULL;
static BitmapLayer* hours_ones_layer = NULL;
static GBitmap* hours_tens = NULL;
static GBitmap* hours_ones = NULL;

static BitmapLayer* minutes_tens_layer = NULL;
static BitmapLayer* minutes_ones_layer = NULL;
static GBitmap* minutes_tens = NULL;
static GBitmap* minutes_ones = NULL;

static BitmapLayer* seconds_tens_layer = NULL;
static BitmapLayer* seconds_ones_layer = NULL;
static GBitmap* seconds_tens = NULL;
static GBitmap* seconds_ones = NULL;

const int IMAGE_RESOURCE_IDS[12] = {
  RESOURCE_ID_IMAGE_0, RESOURCE_ID_IMAGE_1, RESOURCE_ID_IMAGE_2,
  RESOURCE_ID_IMAGE_3, RESOURCE_ID_IMAGE_4, RESOURCE_ID_IMAGE_5,
  RESOURCE_ID_IMAGE_6, RESOURCE_ID_IMAGE_7, RESOURCE_ID_IMAGE_8,
  RESOURCE_ID_IMAGE_9, RESOURCE_ID_IMAGE_BACKGROUND_GRID,
  RESOURCE_ID_IMAGE_BATTERY_BAR
};

// time coords
const int hours_tens_x = 49;
const int hours_tens_y = 49;

const int hours_ones_x = 74;
const int hours_ones_y = 49;

const int minutes_tens_x = 49;
const int minutes_tens_y = 75;

const int minutes_ones_x = 74;
const int minutes_ones_y = 75;

const int seconds_tens_x = 49;
const int seconds_tens_y = 101;

const int seconds_ones_x = 74;
const int seconds_ones_y = 101;

// background bitmap
static BitmapLayer* background_layer = NULL;
static GBitmap* background_grid = NULL;

// battery bitmaps
static BitmapLayer* battery_100_layer = NULL;
static GBitmap* background_100 = NULL;
static BitmapLayer* battery_80_layer = NULL;
static GBitmap* background_80 = NULL;
static BitmapLayer* battery_60_layer = NULL;
static GBitmap* background_60 = NULL;
static BitmapLayer* battery_40_layer = NULL;
static GBitmap* background_40 = NULL;
static BitmapLayer* battery_20_layer = NULL;
static GBitmap* background_20 = NULL;

// battery coords
const int battery_x = 69;
const int battery_100_y = 136;
const int battery_80_y = 139;
const int battery_60_y = 142;
const int battery_40_y = 145;
const int battery_20_y = 148;

//#define DEBUGTIME

#ifdef DEBUGTIME
static int debugHours = 1;
static int debugMinutes = 0;
static int debugSeconds = 0;
#endif
  

static void force_tick();
  
  
static void unload_bitmap(BitmapLayer** layer, GBitmap** bitmap){
  if(*layer){
    layer_remove_from_parent(bitmap_layer_get_layer(*layer));
    bitmap_layer_destroy(*layer);
    *layer = NULL;    
  }
  
  if(*bitmap){
    gbitmap_destroy(*bitmap);
    *bitmap = NULL;    
  }
}

static void load_bitmap(unsigned short image_number, BitmapLayer** layer, GBitmap** bitmap, GColor color, int x, int y, bool replaceColor)  {  
  unload_bitmap(layer, bitmap);
    
  *bitmap = gbitmap_create_with_resource(IMAGE_RESOURCE_IDS[image_number]);
  if(replaceColor){
    replace_gbitmap_color(GColorBlack, color, *bitmap, NULL);
  }
  
#ifdef PBL_PLATFORM_BASALT
  GRect bounds = gbitmap_get_bounds(*bitmap);
#else
  GRect bounds = *bitmap->bounds;
#endif
  
  *layer = bitmap_layer_create(GRect(x,y, bounds.size.w, bounds.size.h));
  
  bitmap_layer_set_bitmap(*layer, *bitmap);
  bitmap_layer_set_compositing_mode(*layer, GCompOpSet);
  
  layer_add_child(window_layer, bitmap_layer_get_layer(*layer));  
}

static void update_hours(unsigned short hours){    
  if(hours < 10 || hours == 24) // && show leading zero
  {
    load_bitmap(0,&hours_tens_layer, &hours_tens, GColorFolly, hours_tens_x, hours_tens_y,true); //0 
  }
  else
  {    
    int hourTens = hours/10;   
    load_bitmap(hourTens,&hours_tens_layer, &hours_tens, GColorFolly, hours_tens_x, hours_tens_y,true);
  }
  
  // hours ones
  if(hours == 24) // or 12 and 12hourtime
  {
    load_bitmap(0,&hours_ones_layer, &hours_ones, GColorFolly, hours_ones_x, hours_ones_y,true);   
  }
  else
  {    
    int hourOnes = hours%10;    
    load_bitmap(hourOnes,&hours_ones_layer, &hours_ones, GColorFolly, hours_ones_x, hours_ones_y,true);
  }    
}

static void update_minutes(unsigned short minutes){  
  //tens
  if(minutes<10){    
    load_bitmap(0,&minutes_tens_layer, &minutes_tens, GColorTiffanyBlue, minutes_tens_x, minutes_tens_y,true) ;
  }
  else{
    unsigned short minuteTens = minutes/10;    
    load_bitmap(minuteTens,&minutes_tens_layer, &minutes_tens, GColorTiffanyBlue,  minutes_tens_x, minutes_tens_y,true);
  }
  
  //ones
  unsigned short minuteOnes = minutes%10;
  load_bitmap(minuteOnes,&minutes_ones_layer, &minutes_ones, GColorTiffanyBlue, minutes_ones_x, minutes_ones_y,true) ;
  
}

static void update_seconds(unsigned short seconds)  {
  return;
}

static unsigned short get_display_hour(unsigned short hour) {
  if (is24hrFormat) {
    return hour;
  }

  unsigned short display_hour = hour % 12;

  // Converts "0" to "12"
  return display_hour ? display_hour : 12;

}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed){  
#ifdef DEBUGTIME
  int hours = get_display_hour(debugHours);
#else
  int hours = get_display_hour(tick_time->tm_hour);
#endif

#ifdef DEBUGTIME
  int minutes = debugMinutes;
#else
  int minutes = tick_time->tm_min;
#endif
  
#ifdef DEBUGTIME
  int seconds = debugSeconds;
#else
  int seconds = tick_time->tm_sec;
#endif
  
  update_hours(hours);
  update_minutes(minutes);
  update_seconds(seconds);

#ifdef DEBUGTIME
  ++debugHours;
  if(debugHours > 24){
    debugHours = 1;
  }
  ++debugMinutes;
  if(debugMinutes > 59){
    debugMinutes = 0;
  }    
  ++debugSeconds;
  if(debugSeconds > 59){
    debugSeconds = 0;
  }
#endif
  
}



static void force_tick(){
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);
  handle_tick(tick_time, MINUTE_UNIT);
}

static void window_load(Window *window) {
  load_bitmap(10, &background_layer, &background_grid, GColorBlack, 0,0,false);
  window_layer = window_get_root_layer(window);  
  force_tick();
}


static void window_unload(Window *window) {  
  unload_bitmap(&hours_tens_layer, &hours_tens);
  unload_bitmap(&hours_ones_layer, &hours_ones);
  unload_bitmap(&minutes_tens_layer, &minutes_tens);
  unload_bitmap(&minutes_ones_layer, &minutes_ones);
  unload_bitmap(&seconds_tens_layer, &seconds_tens);
  unload_bitmap(&seconds_ones_layer, &seconds_ones);
  unload_bitmap(&background_layer, &background_grid);
}


void handle_init(void) {  
  is24hrFormat = clock_is_24h_style();
  
  _window = window_create();
  window_set_background_color(_window, GColorBlack);
  window_set_window_handlers(_window, (WindowHandlers) {.load = window_load, .unload = window_unload});
    
  window_stack_push(_window, true);  
  
  tick_timer_service_subscribe(MONTH_UNIT | DAY_UNIT | HOUR_UNIT | MINUTE_UNIT | SECOND_UNIT, 
                               handle_tick); 
  
}

void handle_deinit(void) {  
  window_destroy(_window);
}

int main(void) {  
  handle_init();
  app_event_loop();
  handle_deinit();
}

