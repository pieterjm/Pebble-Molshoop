
#include "pebble.h"

static Window *window;

static GBitmap *background_image;
static BitmapLayer *background_layer;
static TextLayer *time_layer;
static TextLayer *molshoop_layer;
static TextLayer *countdown_layer;


const int BACKGROUND_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_BEZEMMOL,
  RESOURCE_ID_IMAGE_BLOEMENMOL,
  RESOURCE_ID_IMAGE_BLOEMMOL,
  RESOURCE_ID_IMAGE_LAMPIONMOL,
  RESOURCE_ID_IMAGE_LEESMOL,
  RESOURCE_ID_IMAGE_MOLENBIJ,
  RESOURCE_ID_IMAGE_PADDESTOEL,
  RESOURCE_ID_IMAGE_PETMOL,
  RESOURCE_ID_IMAGE_REGENMOL,
  RESOURCE_ID_IMAGE_SCHEPMOL,
  RESOURCE_ID_IMAGE_SNEEUWMOL,
  RESOURCE_ID_IMAGE_SNEEUWVLOK,
  RESOURCE_ID_IMAGE_SPINNENMOL,
  RESOURCE_ID_IMAGE_SPORTMOL,
  RESOURCE_ID_IMAGE_TULPENMOL,
  RESOURCE_ID_IMAGE_UFOMOL,
  RESOURCE_ID_IMAGE_VLINDERMOL,
  RESOURCE_ID_IMAGE_WINTERSTERREN
};

#define BACKGROUND_SIZE 18


static void set_container_image(GBitmap **bmp_image, BitmapLayer *bmp_layer, const int resource_id, GPoint origin) {
  GBitmap *old_image = *bmp_image;

  *bmp_image = gbitmap_create_with_resource(resource_id);
  GRect frame = (GRect) {
    .origin = origin,
    .size = (*bmp_image)->bounds.size
  };
  bitmap_layer_set_bitmap(bmp_layer, *bmp_image);
  layer_set_frame(bitmap_layer_get_layer(bmp_layer), frame);

  if (old_image != NULL) {
  	gbitmap_destroy(old_image);
  }
}

static void update_display(struct tm *tick_time) {
  // TODO: Only update changed values?

  
    // Need to be static because they're used by the system later.
  static char time_text[] = "00:00";
  static char date_text[] = "Xxxxxxxxx 00";

  char *time_format;


  // TODO: Only update the date when it's changed.
  strftime(date_text, sizeof(date_text), "%B %e", tick_time);
  //  text_layer_set_text(text_date_layer, date_text);


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

  text_layer_set_text(time_layer, time_text);

  static char countdown_text[20];
  long ts = time(NULL);

#define MOLSHOOP_EINDE (2 * 3600 + 1403863200)
#define SWSNOORDHORN_BEGIN (2 * 3600 + 1404369000)
  if ( ts < MOLSHOOP_EINDE ) {
    long delay = MOLSHOOP_EINDE - ts;
    int d,h,m = 0;
    d = delay / 86400;
    h = ( delay - 86400 * d ) / 3600;
    m = ( delay - 86400 * d - 3600 * h ) / 60;
    snprintf(countdown_text,20,"%dd, %dh, %dm",d,h,m);
    text_layer_set_text(countdown_layer, countdown_text);  

    set_container_image(&background_image, background_layer, BACKGROUND_IMAGE_RESOURCE_IDS[ rand() % BACKGROUND_SIZE], GPoint(0, 22));
  } else if ( ts < SWSNOORDHORN_BEGIN ) {
    text_layer_set_text(molshoop_layer, "");  
    text_layer_set_text(countdown_layer, "");    
    set_container_image(&background_image, background_layer, RESOURCE_ID_IMAGE_GEENMOL, GPoint(0, 22));
  } else {
    text_layer_set_text(molshoop_layer, "SWS");  
    text_layer_set_text(countdown_layer, "Noordhorn");  
    set_container_image(&background_image, background_layer, RESOURCE_ID_IMAGE_GEENMOL, GPoint(0, 22));
  }
}


static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  update_display(tick_time);
}


static void init(void) {

  window = window_create();
  if (window == NULL) {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "OOM: couldn't allocate window");
      return;
  }
  window_stack_push(window, true /* Animated */);
  Layer *window_layer = window_get_root_layer(window);
  
  // Create time and date layers
  GRect dummy_frame = { {0, 0}, {0, 0} };
  background_layer = bitmap_layer_create(dummy_frame);
  layer_add_child(window_layer, bitmap_layer_get_layer(background_layer));


  //  background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLOEMENMOL);
  // background_layer = bitmap_layer_create((GRect){.origin = {0,10}, .size = {144,144}});
  //background_layer = bitmap_layer_create((GRect){.origin = {.x = 0, .y = 0},.size = {.}layer_get_frame(window_layer));
  //bitmap_layer_set_bitmap(background_layer, background_image);
  //layer_add_child(window_layer, bitmap_layer_get_layer(background_layer));

  time_layer = text_layer_create((GRect) { .origin = { 3, 124 }, .size = { 141, 45 } });
  text_layer_set_text(time_layer, "11:30");
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  text_layer_set_text_color(time_layer,GColorBlack);
  text_layer_set_background_color(time_layer,GColorClear);
  text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_MEDIUM_NUMBERS));
  layer_add_child(window_layer, text_layer_get_layer(time_layer));

  countdown_layer = text_layer_create((GRect) { .origin = { 3, 18 }, .size = { 141, 30 } });
  text_layer_set_text(countdown_layer, "30d 23h 12m");
  text_layer_set_text_alignment(countdown_layer, GTextAlignmentCenter);
  text_layer_set_text_color(countdown_layer,GColorBlack);
  text_layer_set_background_color(countdown_layer,GColorClear);
  text_layer_set_font(countdown_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  layer_add_child(window_layer, text_layer_get_layer(countdown_layer));

  molshoop_layer = text_layer_create((GRect) { .origin = { 3, -5 }, .size = { 141, 30 } });
  text_layer_set_text(molshoop_layer, "OBS Molshoop");
  text_layer_set_text_alignment(molshoop_layer, GTextAlignmentCenter);
  text_layer_set_text_color(molshoop_layer,GColorBlack);
  text_layer_set_background_color(molshoop_layer,GColorClear);
  text_layer_set_font(molshoop_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(molshoop_layer));

  // Avoids a blank screen on watch start.
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);

  update_display(tick_time);

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}


static void deinit(void) {
  layer_remove_from_parent(bitmap_layer_get_layer(background_layer));
  bitmap_layer_destroy(background_layer);
  gbitmap_destroy(background_image);

  layer_remove_from_parent(text_layer_get_layer(time_layer));
  text_layer_destroy(time_layer);

  layer_remove_from_parent(text_layer_get_layer(countdown_layer));
  text_layer_destroy(countdown_layer);

  layer_remove_from_parent(text_layer_get_layer(molshoop_layer));
  text_layer_destroy(molshoop_layer);
  
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
