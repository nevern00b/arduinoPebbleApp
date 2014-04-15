#include <pebble.h>

static Window *window;
static TextLayer *hello_layer;
static char msg[100];

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

/* This is called when the select button is clicked */
void select_click_handler(ClickRecognizerRef recognizer, void *context) {
   //text_layer_set_text(hello_layer, "Selected!");
    DictionaryIterator *iter;
   app_message_outbox_begin(&iter);
   int key = 0;
   // send the message "hello?" to the phone, using key #0
   Tuplet value = TupletCString(key, "hello?");
   dict_write_tuplet(iter, &value);
   app_message_outbox_send();
}

/* this registers the appropriate function to the appropriate button */
void config_provider(void *context) {
   window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  hello_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(hello_layer, "Hello world!");
  text_layer_set_text_alignment(hello_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(hello_layer));
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

