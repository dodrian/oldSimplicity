#include "pebble.h"

Window *window;
TextLayer *text_date_layer;
TextLayer *text_time_layer;
Layer *line_layer;
static BitmapLayer *s_battery_layer;
static GBitmap *s_battery_bitmap_empty;
static GBitmap *s_battery_bitmap_charging;
static GBitmap *s_battery_bitmap_full;
static GBitmap *s_battery_bitmap_charge2;
static GBitmap *s_battery_bitmap_charge3;
static GBitmap *s_battery_bitmap_charge4;
static GBitmap *s_battery_bitmap_charge5;
static GBitmap *s_battery_bitmap_charge6;

static BitmapLayer *s_bluetooth_layer;
static GBitmap *s_bluetooth_bitmap_cell;
static GBitmap *s_bluetooth_bitmap_nocell;

static BitmapLayer *s_wifi_layer;
static GBitmap *s_wifi_bitmap_wificon;
static GBitmap *s_wifi_bitmap_wifioff;
static GBitmap *s_wifi_bitmap_wifisearch;

static BitmapLayer *s_data_layer;
static GBitmap *s_data_bitmap_datacon;
//static GBitmap *s_data_bitmap_dataoff;
static GBitmap *s_data_bitmap_datasearch;

static BitmapLayer *s_alarm_layer;
static GBitmap *s_alarm_bitmap;
static TextLayer *text_alarm_layer;

static AppSync sync;
static uint8_t sync_buffer[64];

static char helloworld[] = "Hello!";

static char alarmtext[] = "No alarmxxxx";

enum StatusKey {
    MESSAGE_KEY = 0,
    WIFI_KEY = 1,
    DATA_KEY = 2,
    REQUEST_KEY = 3,
};

enum ConnectionKey {
  CONNECTION_OFF = 0,
  CONNECTION_ON = 1,
  CONNECTION_SEARCHING = 2,
  
};

void line_layer_update_callback(Layer *layer, GContext* ctx) {
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

static void handle_battery(BatteryChargeState charge_state) {

  if (charge_state.is_charging) {
    bitmap_layer_set_bitmap(s_battery_layer, s_battery_bitmap_charging);
  }
  else if(charge_state.is_plugged)
    {
    
    bitmap_layer_set_bitmap(s_battery_layer, s_battery_bitmap_full);
    
  }
  else {
    if(charge_state.charge_percent > 80)
      bitmap_layer_set_bitmap(s_battery_layer, s_battery_bitmap_charge6);
    else if(charge_state.charge_percent > 60)
      bitmap_layer_set_bitmap(s_battery_layer, s_battery_bitmap_charge5);
    else if(charge_state.charge_percent > 40)
      bitmap_layer_set_bitmap(s_battery_layer, s_battery_bitmap_charge4);
    else if(charge_state.charge_percent > 20)
      bitmap_layer_set_bitmap(s_battery_layer, s_battery_bitmap_charge3);
    else if(charge_state.charge_percent > 10)
      bitmap_layer_set_bitmap(s_battery_layer, s_battery_bitmap_charge2);
    else
      bitmap_layer_set_bitmap(s_battery_layer, s_battery_bitmap_empty);
  }
}

static void handle_bluetooth(bool bluetooth_state)
  {
  if(bluetooth_state) // connected
    {
    bitmap_layer_set_bitmap(s_bluetooth_layer, s_bluetooth_bitmap_cell);
    // send a test message!

    text_layer_set_text(text_date_layer, helloworld);
  }
  else // not connected
  {
    bitmap_layer_set_bitmap(s_bluetooth_layer, s_bluetooth_bitmap_nocell) ;
    vibes_double_pulse();
  };
}

void sync_tuple_changed_callback(const uint32_t key,
        const Tuple* new_tuple, const Tuple* old_tuple, void* context)
 {
    switch(key)
      
    {
        case MESSAGE_KEY:
     
          strcpy(alarmtext, new_tuple->value->cstring);
          text_layer_set_text(text_alarm_layer, alarmtext);
        break; 
        case WIFI_KEY: 
          switch(new_tuple->value->uint32)
          {
            case CONNECTION_OFF:
              bitmap_layer_set_bitmap(s_wifi_layer, s_wifi_bitmap_wifioff);
            break;
            
            case CONNECTION_ON:
            
              bitmap_layer_set_bitmap(s_wifi_layer, s_wifi_bitmap_wificon);
            break;
            
            case CONNECTION_SEARCHING:
            
              bitmap_layer_set_bitmap(s_wifi_layer, s_wifi_bitmap_wifisearch);
            break;
           // default: 
            
          }
      //    text_layer_set_text(text_date_layer, helloworld);
      
        break;
        case DATA_KEY:
      
          switch(new_tuple->value->uint32)
          {
            case CONNECTION_OFF:
              //bitmap_layer_set_bitmap(s_data_layer, s_wifi_bitmap_dataoff);
              layer_set_hidden(bitmap_layer_get_layer(s_data_layer), true);
            break;
            
            case CONNECTION_ON:
            
              bitmap_layer_set_bitmap(s_data_layer, s_data_bitmap_datacon);
            
              layer_set_hidden(bitmap_layer_get_layer(s_data_layer), false);
            break;
            
            case CONNECTION_SEARCHING:
            
              bitmap_layer_set_bitmap(s_data_layer, s_data_bitmap_datasearch);
            
              layer_set_hidden(bitmap_layer_get_layer(s_data_layer), false);
            
            break;
           // default: 
            
          }
      
        break;
        
    }
    //uint32_t test = 8;
 //   snprintf(helloworld, sizeof helloworld, "%lu", (unsigned long)key);
   // Update your layers
   // Don't forget to call layer_mark_dirty()
 }

static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}


static void init_dictionary()
  {
  
  const int inbound_size = 64;
  const int outbound_size = 64;
  app_message_open(inbound_size, outbound_size);
  
   Tuplet initial_values[] = {
     TupletCString(MESSAGE_KEY, "Hello"),
     TupletInteger(WIFI_KEY, CONNECTION_OFF),
     TupletInteger(DATA_KEY, CONNECTION_OFF),
     TupletInteger(REQUEST_KEY, true),
   };
  
   app_sync_init(&sync, sync_buffer, sizeof(sync_buffer),
               initial_values, ARRAY_LENGTH(initial_values),
               sync_tuple_changed_callback, sync_error_callback, NULL);
  
  app_sync_set(&sync, initial_values, ARRAY_LENGTH(initial_values));
  
  
}

static void deinit_dictionary()
  {
  app_sync_deinit(&sync);
  
}

static void init_alarm()
  {
    s_alarm_layer = bitmap_layer_create(GRect(1, 18, 16, 16));
    s_alarm_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ALARM);
    bitmap_layer_set_bitmap(s_alarm_layer, s_alarm_bitmap);
    layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_alarm_layer));
  
    text_alarm_layer = text_layer_create(GRect(18, 18, 144-18, 16));
    text_layer_set_text_color(text_alarm_layer, GColorWhite);
    text_layer_set_background_color(text_alarm_layer, GColorClear);
    text_layer_set_font(text_alarm_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_alarm_layer));
}

static void deinit_alarm()
  {
    gbitmap_destroy(s_alarm_bitmap);
    bitmap_layer_destroy(s_alarm_layer);
    text_layer_destroy(text_alarm_layer);
  
  
}

static void init_data_icons()
  {
  s_data_layer = bitmap_layer_create(GRect(18,1,16,16));  //GRect(144 - 16, 168 - 16, 144, 168));
  s_data_bitmap_datacon = gbitmap_create_with_resource(RESOURCE_ID_DATA_CONN);
 // s_data_bitmap_dataoff = gbitmap_create_with_resource(RESOURCE_ID_WIFI_OFF);
  s_data_bitmap_datasearch = gbitmap_create_with_resource(RESOURCE_ID_QUESTION_MARK);
  // have to hear from app to set, maybe requestupdate?
  
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_data_layer));
}

static void deinit_data_icons()
  {
  gbitmap_destroy(s_data_bitmap_datacon);
  gbitmap_destroy(s_data_bitmap_datasearch);
  bitmap_layer_destroy(s_data_layer);
}

static void init_wifi_icons()
  {
    
  s_wifi_layer = bitmap_layer_create(GRect(1,1,16,16));  //GRect(144 - 16, 168 - 16, 144, 168));
  s_wifi_bitmap_wificon = gbitmap_create_with_resource(RESOURCE_ID_WIFI_CON);
  s_wifi_bitmap_wifioff = gbitmap_create_with_resource(RESOURCE_ID_WIFI_OFF);
  s_wifi_bitmap_wifisearch = gbitmap_create_with_resource(RESOURCE_ID_WIFI_SEARCH);
  // have to hear from app to set, maybe requestupdate?
  
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_wifi_layer));
}

static void deinit_wifi_icons()
  {
  gbitmap_destroy(s_wifi_bitmap_wificon);
  gbitmap_destroy(s_wifi_bitmap_wifioff);
  gbitmap_destroy(s_wifi_bitmap_wifisearch);
  bitmap_layer_destroy(s_wifi_layer);
  
}

static void init_bluetooth_icons()
  {
    
  s_bluetooth_layer = bitmap_layer_create(GRect(144-34,1,16,16));  //GRect(144 - 16, 168 - 16, 144, 168));
  s_bluetooth_bitmap_cell = gbitmap_create_with_resource(RESOURCE_ID_CELL2);
  s_bluetooth_bitmap_nocell = gbitmap_create_with_resource(RESOURCE_ID_NOCELL2);
  
  bluetooth_connection_service_peek() ?
    bitmap_layer_set_bitmap(s_bluetooth_layer, s_bluetooth_bitmap_cell) : 
    bitmap_layer_set_bitmap(s_bluetooth_layer, s_bluetooth_bitmap_nocell);
  
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bluetooth_layer));
  bluetooth_connection_service_subscribe(&handle_bluetooth);
}

static void deinit_bluetooth_icons()
  {
  gbitmap_destroy(s_bluetooth_bitmap_cell);
  gbitmap_destroy(s_bluetooth_bitmap_nocell);
  bitmap_layer_destroy(s_bluetooth_layer);
  bluetooth_connection_service_unsubscribe();
  
}

static void init_battery_icons()
  {
  
  s_battery_layer = bitmap_layer_create(GRect(144-17,1,16,16));  //GRect(144 - 16, 168 - 16, 144, 168));
  s_battery_bitmap_full = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_FULL);
  s_battery_bitmap_empty = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_EMPTY);
  s_battery_bitmap_charging = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_CHARGING);
  s_battery_bitmap_charge2 = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_CHARGE_2);
  s_battery_bitmap_charge3 = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_CHARGE_3);
  s_battery_bitmap_charge4 = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_CHARGE_4);
  s_battery_bitmap_charge5 = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_CHARGE_5);
  s_battery_bitmap_charge6 = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_CHARGE_6);
  bitmap_layer_set_bitmap(s_battery_layer, s_battery_bitmap_full);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_battery_layer));
  
  
  battery_state_service_subscribe(&handle_battery);
  handle_battery(battery_state_service_peek	());
  
}

static void deinit_battery_icons()
  {
  
  gbitmap_destroy(s_battery_bitmap_full);
  gbitmap_destroy(s_battery_bitmap_empty);
  gbitmap_destroy(s_battery_bitmap_charging);
  gbitmap_destroy(s_battery_bitmap_charge2);
  gbitmap_destroy(s_battery_bitmap_charge3);
  gbitmap_destroy(s_battery_bitmap_charge4);
  gbitmap_destroy(s_battery_bitmap_charge5);
  gbitmap_destroy(s_battery_bitmap_charge6);
  bitmap_layer_destroy(s_battery_layer);
  battery_state_service_unsubscribe();
}

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  // Need to be static because they're used by the system later.
  static char time_text[] = "00:00";
  static char date_text[] = "Xxxxxxxxx 00";

  char *time_format;

  if (!tick_time) {
    time_t now = time(NULL);
    tick_time = localtime(&now);
  }

  // TODO: Only update the date when it's changed.
  strftime(date_text, sizeof(date_text), "%B %e", tick_time);
  text_layer_set_text(text_date_layer, date_text);


  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }

  strftime(time_text, sizeof(time_text), time_format, tick_time);

  // Kludge to handle lack of non-padded hour format string
  // for twelve hour clock.
  if (!clock_is_24h_style() && (time_text[0] == '0')) {
    memmove(time_text, &time_text[1], sizeof(time_text) - 1);
  }
  /* 
    Tuplet new_tuples[] = {
     //TupletCString(1, time_text),
      ((const Tuplet) { .type = TUPLE_CSTRING, .key = 1, .cstring = { .data = time_text, .length = strlen(time_text) + 1 }})
    };
    app_sync_set(&sync, new_tuples, 1);
    */
  text_layer_set_text(text_time_layer, time_text);
}

void handle_deinit(void) {
  tick_timer_service_unsubscribe();
  deinit_battery_icons();
  deinit_bluetooth_icons();
  deinit_wifi_icons();
  deinit_data_icons();
  deinit_alarm();
  deinit_dictionary();
}

void handle_init(void) {
  window = window_create();
  window_stack_push(window, true /* Animated */);
  window_set_background_color(window, GColorBlack);

  Layer *window_layer = window_get_root_layer(window);

  text_date_layer = text_layer_create(GRect(8, 68, 144-8, 168-68));
  text_layer_set_text_color(text_date_layer, GColorWhite);
  text_layer_set_background_color(text_date_layer, GColorClear);
  text_layer_set_font(text_date_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  layer_add_child(window_layer, text_layer_get_layer(text_date_layer));

  text_time_layer = text_layer_create(GRect(7, 92, 144-7, 168-92));
  text_layer_set_text_color(text_time_layer, GColorWhite);
  text_layer_set_background_color(text_time_layer, GColorClear);
  text_layer_set_font(text_time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  layer_add_child(window_layer, text_layer_get_layer(text_time_layer));

  GRect line_frame = GRect(8, 97, 139, 2);
  line_layer = layer_create(line_frame);
  layer_set_update_proc(line_layer, line_layer_update_callback);
  layer_add_child(window_layer, line_layer);
  
  init_battery_icons();
  init_bluetooth_icons();
  init_wifi_icons();
  init_data_icons();
  init_alarm();
  init_dictionary();

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  handle_minute_tick(NULL, MINUTE_UNIT);
}


int main(void) {
  handle_init();

  app_event_loop();
  
  handle_deinit();
}
