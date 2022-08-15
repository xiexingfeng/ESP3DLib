#ifndef _MOONTRAKER_H
#define _MOONTRAKER_H
#include "wificonfig.h"
#include "espcom.h"

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

class Moonraker
{
public:
  Moonraker();
  ~Moonraker();
  bool begin();
  void end();
  static void handle();

public:
  static void onEvent(class AsyncWebSocket *server, class AsyncWebSocketClient *client, AwsEventType type,
                      void *arg, uint8_t *data, size_t len);
  static void handleWebSocketMessage(AsyncWebSocketClient *client, void *arg, uint8_t *data, size_t len);

  static void notFound(AsyncWebServerRequest *request);
  static void sendJsonRPC(class AsyncWebServerRequest *request, String res);
  static void handle_server_info(class AsyncWebServerRequest *request);
  static void handle_database_item(class AsyncWebServerRequest *request);
  static void handle_oneshot_token(class AsyncWebServerRequest *request);

  static void build_error_message(int id, int code, String message);
  static void build_server_info(int id);
  static void build_server_config(int id);
  static void build_connection_id(int id);
  static void build_printer_info(int id);
  static void build_machine_proc_stats(int id);
  static void build_machine_system_info(int id);
  static void build_files_get_directory(int id);
  static void build_server_history_list(int id);
  static void build_server_history_totals(int id);

  static void build_printer_objects_list(int id);
  static void build_printer_objects_subscribe(int id);

  static void build_server_gcode_store(int id);
  static void build_printer_gcode_help(int id);

  static void build_server_temperature_store(int id);

  static void build_machine_device_power_devices(int id);
  static void build_machine_update_status(int id);

  static void build_printer_query_endstops_status(int id);

  static void build_server_database_list(int id);

  static void build_printer_gcode_script(int id);

  static void build_server_files_list(int id);


private:
  class AsyncWebServer *_webserver;
  class AsyncWebSocket *_ws;
};

extern Moonraker moonraker;
#endif
