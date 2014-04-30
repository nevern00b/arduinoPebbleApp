#include <pebble.h>

static Window *window;
static Window *window1;
static TextLayer *hello_layer;
static char msg[100];
void select_long_click_release_handler(ClickRecognizerRef recognizer, void *context);
void down_long_click_release_handler(ClickRecognizerRef recognizer, void *context);
void up_long_click_release_handler(ClickRecognizerRef recognizer, void *context);
void up_multi_click_handler(ClickRecognizerRef recognizer, void *context);
int polling = 0;
int firstTime = 0;
//int standbyModeToggle = 0;

 void out_sent_handler(DictionaryIterator *sent, void *context) {
   // outgoing message was delivered -- do nothing
 }


 void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
   // outgoing message failed
   text_layer_set_text(hello_layer, "Error out!");
 }


 void in_received_handler(DictionaryIterator *received, void *context) {
   // incoming message received 
     // looks for key #0 in the incoming message
   int key = 0;
   Tuple *text_tuple = dict_find(received, key);
   if (text_tuple) {
     if (text_tuple->value) {
       // put it in this global variable
       strcpy(msg, text_tuple->value->cstring);
     }
     else strcpy(msg, "no value!");
     
     text_layer_set_text(hello_layer, msg);
   }
   else {
     text_layer_set_text(hello_layer, "no message!");
   }
 }
 

 void in_dropped_handler(AppMessageResult reason, void *context) {
   // incoming message dropped
   text_layer_set_text(hello_layer, "Error in!");
 }


//new code until *****
static AppTimer *timer; // global variable to represent the timer
static void time_loop();

static void timer_callback(void *data) { 
   DictionaryIterator *iter;
   app_message_outbox_begin(&iter);
   int key = 0;
   if (polling) {
     // send the message "hello?" to the phone, using key #0
     Tuplet value = TupletCString(key, "hello?");
     dict_write_tuplet(iter, &value);
     app_message_outbox_send();
     int sleeptime = 2000;
     timer = app_timer_register(sleeptime, timer_callback, NULL);
   }
   else {
      int key = 11;
      Tuplet value = TupletCString(key, "hello?");
      dict_write_tuplet(iter, &value);
      app_message_outbox_send();
   }
}

//*********************************LONG-CLICK HANDLERS**********************************

/* This is called when the select button is clicked */
void select_long_click_release_handler(ClickRecognizerRef recognizer, void *context) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  if (!polling) {
    polling = 1;
    int key = 10;
    Tuplet value = TupletCString(key, "hello?");
    dict_write_tuplet(iter, &value);
    app_message_outbox_send();
    int sleeptime = 3000;
    timer = app_timer_register(sleeptime, timer_callback, NULL);
  }
  else {
    polling = 0;
    int sleeptime = 3000;
    timer = app_timer_register(sleeptime, timer_callback, NULL);
  }
}

void down_long_click_release_handler(ClickRecognizerRef recognizer, void *context) {
  //standbyModeToggle++;
  //if (standbyModeToggle % 2 == 0) {
  DictionaryIterator *iter;
   app_message_outbox_begin(&iter);
   int key = 2; //look for "e.payload.standby" in javascript
   // send the message "hello?" to the phone, using key #2
   Tuplet value = TupletCString(key, "hello2?");
   dict_write_tuplet(iter, &value);
   app_message_outbox_send();
  //}
  /*else {
    text_layer_set_text(hello_layer, "DownButtonLongclick\nStandby OFF");
  }*/
}

void up_long_click_release_handler(ClickRecognizerRef recognizer, void *context) {
  DictionaryIterator *iter;
   app_message_outbox_begin(&iter);
   int key = 4; //look for "e.payload.partyMode" in javascript
   // send the message "hello?" to the phone, using key #0
   Tuplet value = TupletCString(key, "hello?");
   dict_write_tuplet(iter, &value);
   app_message_outbox_send();
}
//*********************************




//************************************
static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  hello_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, bounds.size.h } });
  text_layer_set_text(hello_layer, "Welcome to Team-Awesome's Temperature App!");
  text_layer_set_text_alignment(hello_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(hello_layer));
}




static void window_load2(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  hello_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, bounds.size.h } });
  text_layer_set_text(hello_layer, "Hello world12!");
  text_layer_set_text_alignment(hello_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(hello_layer));
}

static void window_load3(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  hello_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, bounds.size.h } });
  text_layer_set_text(hello_layer, "triple-click!");
  text_layer_set_text_alignment(hello_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(hello_layer));
}

//********************************SINGLE-CLICK HANDLERS*******************************8
/* This is called when the up button is clicked */
void up_click_handler(ClickRecognizerRef recognizer, void *context) {
    DictionaryIterator *iter;
   app_message_outbox_begin(&iter);
   int key = 1; //look for "e.payload.changeUnit" in javascript
   // send the message "downButton?" to the phone, using key #0
   Tuplet value = TupletCString(key, "upButton?");
   dict_write_tuplet(iter, &value);
   app_message_outbox_send();
}

/* This is called when the down button is clicked */
void down_click_handler(ClickRecognizerRef recognizer, void *context) {
    DictionaryIterator *iter;
   app_message_outbox_begin(&iter);
   int key = 9; //look for "e.payload.getStats" in javascript
   // send the message "downButton?" to the phone, using key #0
   Tuplet value = TupletCString(key, "downButton?");
   dict_write_tuplet(iter, &value);
   app_message_outbox_send();
}

void select_click_handler(ClickRecognizerRef recognizer, void *context){
  DictionaryIterator *iter;
   app_message_outbox_begin(&iter);
   int key = 0; //look for "e.payload.name" in javascript
   // send the message "hello?" to the phone, using key #0
   Tuplet value = TupletCString(key, "hello?");
   dict_write_tuplet(iter, &value);
   app_message_outbox_send();
}



//************************************MULTI-CLICK HANDLERS****************************
void up_multi_click_handler(ClickRecognizerRef recognizer, void *context) {
  Window *window = (Window *)context; // This context defaults to the window, but may be changed with \ref window_set_click_context.
  const uint16_t count = click_number_of_clicks_counted(recognizer);
  if (count == 3) {
    //window_load3(window);
    DictionaryIterator *iter;
   app_message_outbox_begin(&iter);
   int key = 5; //look for "e.payload.cMode" in javascript
   // send the message "hello?" to the phone, using key #0
   Tuplet value = TupletCString(key, "hello?");
   dict_write_tuplet(iter, &value);
   app_message_outbox_send();
  }
  if (count == 2) {
      //window_load3(window);
    DictionaryIterator *iter;
   app_message_outbox_begin(&iter);
   int key = 6; //look for "e.payload.setTempUp" in javascript
   // send the message "hello?" to the phone, using key #0
   Tuplet value = TupletCString(key, "hello?");
   dict_write_tuplet(iter, &value);
   app_message_outbox_send();
  }
}

void down_multi_click_handler(ClickRecognizerRef recognizer, void *context) {
  Window *window = (Window *)context; // This context defaults to the window, but may be changed with \ref window_set_click_context.
  const uint16_t count = click_number_of_clicks_counted(recognizer);
  if (count == 3) {
    //window_load3(window);
    DictionaryIterator *iter;
   app_message_outbox_begin(&iter);
   int key = 8; //look for "e.payload.cModeExit" in javascript
   // send the message "hello?" to the phone, using key #0
   Tuplet value = TupletCString(key, "hello?");
   dict_write_tuplet(iter, &value);
   app_message_outbox_send();
  }
  if (count == 2) {
      //window_load3(window);
    DictionaryIterator *iter;
   app_message_outbox_begin(&iter);
   int key = 7; //look for "e.payload.setTempUp" in javascript
   // send the message "hello?" to the phone, using key #0
   Tuplet value = TupletCString(key, "hello?");
   dict_write_tuplet(iter, &value);
   app_message_outbox_send();
  }
}


void select_multi_click_handler(ClickRecognizerRef recognizer, void *context) {
  Window *window = (Window *)context; // This context defaults to the window, but may be changed with \ref window_set_click_context.
  const uint16_t count = click_number_of_clicks_counted(recognizer);
  if (count == 3) {
    //window_load3(window);
    DictionaryIterator *iter;
   app_message_outbox_begin(&iter);
   int key = 12; //look for "e.payload.stopServer" in javascript
   // send the message "hello?" to the phone, using key #0
   Tuplet value = TupletCString(key, "hello?");
   dict_write_tuplet(iter, &value);
   app_message_outbox_send();
  }
}




//*************************************CONFIG*******************************************
/* this registers the appropriate function to the appropriate button */
void config_provider(void *context) {
   window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
   window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
   window_long_click_subscribe(BUTTON_ID_SELECT, 700, NULL, select_long_click_release_handler);
  window_long_click_subscribe(BUTTON_ID_DOWN, 700, NULL, down_long_click_release_handler);
  window_long_click_subscribe(BUTTON_ID_UP, 700, NULL, up_long_click_release_handler);
  window_multi_click_subscribe(BUTTON_ID_UP, 2, 3, 0, true, up_multi_click_handler);
  window_multi_click_subscribe(BUTTON_ID_DOWN, 2, 3, 0, true, down_multi_click_handler);
   window_multi_click_subscribe(BUTTON_ID_SELECT, 3, 0, 0, true, select_multi_click_handler);
  
}



static void window_unload(Window *window) {
  text_layer_destroy(hello_layer);
}

static void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  // need this for adding the listener
  window_set_click_config_provider(window, config_provider);

  // for registering AppMessage handlers
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_outbox_sent(out_sent_handler);
  app_message_register_outbox_failed(out_failed_handler);
  const uint32_t inbound_size = 64;
  const uint32_t outbound_size = 64;
  app_message_open(inbound_size, outbound_size);
  
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
