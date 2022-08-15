#include "moonraker.h"

#include <WiFi.h>

#include <ArduinoJson.h>
#include "espcom.h"

Moonraker moonraker;
static char g_jsondata[2 * 1024];

Moonraker::Moonraker() : _webserver(NULL), _ws(NULL)
{
}

Moonraker::~Moonraker()
{
}

bool Moonraker::begin()
{
  _webserver = new AsyncWebServer(7125);
  _ws = new AsyncWebSocket("/websocket");
  _ws->onEvent(onEvent);
  _webserver->addHandler(_ws);

  _webserver->onNotFound(notFound);
  _webserver->on("/server/info", HTTP_GET, handle_server_info);
  _webserver->on("/server/database/item", HTTP_GET, handle_database_item);
  _webserver->on("/access/oneshot_token", HTTP_GET, handle_oneshot_token);

  _webserver->begin();
  return true;
}

void Moonraker::end()
{
  if (_webserver)
  {
    _webserver->end();
    delete _webserver;
    _webserver = NULL;
  }
  if (_ws)
  {
    delete _ws;
    _ws = NULL;
  }
}

void Moonraker::handle()
{
}

void Moonraker::onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
                        void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
  case WS_EVT_CONNECT:
    Esp3DCom::echo("WebSocket client  connected.");
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    Esp3DCom::echo("WebSocket client  disconnected.");
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(client, arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}

void Moonraker::build_error_message(int id, int code, String message)
{
  DynamicJsonDocument doc(2 * 1024);

  doc["jsonrpc"] = "2.0";
  doc["id"] = id;

  JsonObject error = doc.createNestedObject("error");
  error["code"] = code;
  error["message"] = message;

  serializeJson(doc, g_jsondata);
}

void Moonraker::build_server_info(int id)
{
  DynamicJsonDocument doc(2 * 1024);

  doc["jsonrpc"] = "2.0";
  doc["id"] = id;

  JsonObject result = doc.createNestedObject("result");
  result["klippy_connected"] = true;
  result["klippy_state"] = "ready";
  result["components"] = serialized("[\"database\",\"file_manager\",\"klippy_apis\",\"machine\",\"data_store\",\"shell_command\",\"history\",\"octoprint_compat\",\"update_manager\",\"power\"]");
  result["failed_components"] = serialized("[]");
  result["registered_directories"] = serialized("[\"config\", \"gcodes\", \"config_examples\", \"docs\"]");
  result["warnings"] = serialized("[]");
  result["websocket_count"] = 1;
  result["moonraker_version"] = "v0.7.1-105-ge4f103c";
  result["missing_klippy_requirements"] = serialized("[]");
  result["api_version"] = serialized("[1,0,5]");
  result["api_version_string"] = "1.0.5";

  serializeJson(doc, g_jsondata);
}

void Moonraker::build_server_config(int id)
{
  DynamicJsonDocument doc(2 * 1024);

  doc["jsonrpc"] = "2.0";
  doc["id"] = id;

  JsonObject result = doc.createNestedObject("result");
  JsonObject config = result.createNestedObject("config");
  // JsonObject server = config.createNestedObject("server");
  result["config"]["server"]["host"] = "0.0.0.0";
  result["config"]["server"]["port"] = 7125;
  result["config"]["server"]["ssl_port"] = 7130;
  result["config"]["server"]["enable_debug_logging"] = true;
  result["config"]["server"]["enable_asyncio_debug"] = false;
  result["config"]["server"]["klippy_uds_address"] = "/tmp/klippy_uds";
  result["config"]["server"]["max_upload_size"] = 1024;
  result["config"]["server"]["ssl_certificate_path"] = NULL;
  result["config"]["server"]["ssl_key_path"] = NULL;
  result["config"]["server"]["config_path"] = "~/printer_config";

  result["config"]["dbus_manager"] = serialized("{}");
  result["config"]["database"]["database_path"] = "~/.moonraker_database";
  result["config"]["database"]["enable_database_debug"] = false;

  result["config"]["file_manager"]["enable_object_processing"] = false;
  result["config"]["file_manager"]["queue_gcode_uploads"] = false;
  result["config"]["file_manager"]["log_path"] = "";

  result["config"]["klippy_apis"] = serialized("{}");

  result["config"]["machine"]["provider"] = "systemd_dbus";

  result["config"]["shell_command"] = serialized("{}");

  result["config"]["data_store"]["temperature_store_size"] = 1200;
  result["config"]["data_store"]["gcode_store_size"] = 1000;

  result["config"]["proc_stats"] = serialized("{}");
  result["config"]["job_state"] = serialized("{}");

  result["config"]["job_queue"]["load_on_startup"] = false;
  result["config"]["job_queue"]["automatic_transition"] = false;
  result["config"]["job_queue"]["job_transition_delay"] = 0.01;
  result["config"]["job_queue"]["job_transition_gcode"] = "";

  result["config"]["http_client"] = serialized("{}");

  result["config"]["announcements"]["dev_mode"] = false;
  result["config"]["announcements"]["subscriptions"] = serialized("[]");

  result["config"]["extensions"] = serialized("[]");

  result["config"]["authorization"]["login_timeout"] = 90;
  result["config"]["authorization"]["force_logins"] = false;
  result["config"]["authorization"]["default_source"] = "moonraker";
  result["config"]["authorization"]["cors_domains"] = serialized("[\"http://192.168.200.200\"]");
  result["config"]["authorization"]["trusted_clients"] = serialized("[\"192.168.200.0/24\"]");

  result["config"]["octoprint_compat"]["enable_ufp"] = true;
  result["config"]["octoprint_compat"]["flip_h"] = false;
  result["config"]["octoprint_compat"]["flip_v"] = false;
  result["config"]["octoprint_compat"]["rotate_90"] = false;
  result["config"]["octoprint_compat"]["stream_url"] = "/webcam/?action=stream";
  result["config"]["octoprint_compat"]["webcam_enabled"] = true;

  result["orig"]["DEFAULT"] = serialized("[]");

  result["orig"]["server"]["enable_debug_logging"] = true;
  result["orig"]["server"]["config_path"] = "~/printer_config";

  result["orig"]["authorization"]["enabled"] = "false";
  result["orig"]["authorization"]["trusted_clients"] = "\n192.168.200.0/24";
  result["orig"]["authorization"]["cors_domains"] = "\nhttp://192.168.200.200";

  result["orig"]["octoprint_compat"] = serialized("{}");

  result["files"]["filename"] = "moonraker.conf";
  result["files"]["sections"] = serialized("[\"server\", \"authorization\", \"octoprint_compat\"]");

  serializeJson(doc, g_jsondata);
}

void Moonraker::build_connection_id(int id)
{
  DynamicJsonDocument doc(2 * 1024);

  doc["jsonrpc"] = "2.0";
  doc["id"] = id;

  JsonObject result = doc.createNestedObject("result");
  result["connection_id"] = 139802077171920;

  serializeJson(doc, g_jsondata);
}

void Moonraker::build_printer_info(int id)
{
  DynamicJsonDocument doc(2 * 1024);

  doc["jsonrpc"] = "2.0";
  doc["id"] = id;

  JsonObject result = doc.createNestedObject("result");
  result["state"] = "ready";
  result["state_message"] = "Printer is ready";
  result["hostname"] = "simon printer";
  result["software_version"] = "v0.9.1-302-g900c7396";
  result["cpu_info"] = "4 core ARMv7 Processor rev 4 (v7l)";
  result["klipper_path"] = "/home/pi/klipper";
  result["python_path"] = "/home/pi/klipper";
  result["log_file"] = "/tmp/klippy.log";
  result["config_file"] = "/home/pi/printer.cfg";

  serializeJson(doc, g_jsondata);
}

void Moonraker::build_machine_proc_stats(int id)
{
  DynamicJsonDocument doc(2 * 1024);

  doc["jsonrpc"] = "2.0";
  doc["id"] = id;

  JsonObject result = doc.createNestedObject("result");
  JsonArray moonraker_stats = doc.createNestedArray("moonraker_stats");
  JsonObject obj = moonraker_stats.createNestedObject();

  obj["time"] = 1660271368.4862978;
  obj["cpu_usage"] = 0.3;
  obj["memory"] = 37648;
  obj["mem_units"] = "kB";

  result["flags"] = serialized("[]");

  result["cpu_temp"] = 45.622;

  serializeJson(doc, g_jsondata);
}

void Moonraker::build_machine_system_info(int id)
{
  DynamicJsonDocument doc(2 * 1024);

  doc["jsonrpc"] = "2.0";
  doc["id"] = id;

  JsonObject result = doc.createNestedObject("result");
  JsonObject system_info = result.createNestedObject("system_info");

  system_info["python"]["version"] = serialized("[3,8,10,\"final\",0]");
  system_info["python"]["version_string"] = "3.8.10 (default, Jun 22 2022, 20:18:18)  [GCC 9.4.0]";

  system_info["cpu_info"]["cpu_count"] = 4;
  system_info["cpu_info"]["bits"] = "32bit";
  system_info["cpu_info"]["processor"] = "armv7l";
  system_info["cpu_info"]["cpu_desc"] = "ARMv7 Processor rev 4 (v7l)";
  system_info["cpu_info"]["serial_number"] = "b898bdb4";
  system_info["cpu_info"]["hardware_desc"] = "BCM2835";
  system_info["cpu_info"]["model"] = "Raspberry Pi 3 Model B Rev 1.2";
  system_info["cpu_info"]["total_memory"] = 945364;
  system_info["cpu_info"]["memory_units"] = "kB";

  system_info["sd_info"]["manufacturer_id"] = "03";
  system_info["sd_info"]["manufacturer"] = "Sandisk";
  system_info["sd_info"]["oem_id"] = "5344";
  system_info["sd_info"]["product_name"] = "SU32G";
  system_info["sd_info"]["product_revision"] = "8.0";
  system_info["sd_info"]["serial_number"] = "46ba46";
  system_info["sd_info"]["manufacturer_date"] = "4/2018";
  system_info["sd_info"]["capacity"] = "29.7 GiB";
  system_info["sd_info"]["total_bytes"] = 31914983424;

  system_info["distribution"]["name"] = "Raspbian GNU/Linux 10 (buster)";
  system_info["distribution"]["id"] = "raspbian";
  system_info["distribution"]["version"] = "10";
  system_info["distribution"]["version_parts"]["major"] = "10";
  system_info["distribution"]["version_parts"]["minor"] = "";
  system_info["distribution"]["version_parts"]["build_number"] = "";
  system_info["distribution"]["like"] = "debian";
  system_info["distribution"]["codename"] = "buster)";

  system_info["virtualization"]["virt_type"] = "none";
  system_info["virtualization"]["virt_identifier"] = "none";

  system_info["network"]["wlan0"]["mac_address"] = "<redacted_mac>";
  system_info["network"]["wlan0"]["ip_addresses"] = serialized("[]");

  system_info["available_services"] = serialized("[\"klipper\",\"klipper_mcu\",\"moonraker\"]");

  system_info["service_state"]["klipper"]["active_state"] = "active";
  system_info["service_state"]["klipper"]["sub_state"] = "running";
  system_info["service_state"]["klipper_mcu"]["active_state"] = "active";
  system_info["service_state"]["klipper_mcu"]["sub_state"] = "running";
  system_info["service_state"]["moonraker"]["active_state"] = "active";
  system_info["service_state"]["moonraker"]["sub_state"] = "running";

  serializeJson(doc, g_jsondata);
}

void Moonraker::build_files_get_directory(int id)
{
  DynamicJsonDocument doc(2 * 1024);

  doc["jsonrpc"] = "2.0";
  doc["id"] = id;

  JsonObject result = doc.createNestedObject("result");

  result["dirs"] = serialized("[]");
  result["files"] = serialized("[]");

  result["disk_usage"]["total"] = 7522213888;
  result["disk_usage"]["used"] = 4280369152;
  result["disk_usage"]["free"] = 2903625728;

  result["root_info"]["name"] = "gcodes";
  result["root_info"]["permissions"] = "rw";

  serializeJson(doc, g_jsondata);
}

void Moonraker::build_server_history_list(int id)
{
  DynamicJsonDocument doc(2 * 1024);

  doc["jsonrpc"] = "2.0";
  doc["id"] = id;

  JsonObject result = doc.createNestedObject("result");

  result["count"] = 0;
  result["jobs"] = serialized("[]");

  serializeJson(doc, g_jsondata);
}

void Moonraker::build_server_history_totals(int id)
{
  DynamicJsonDocument doc(2 * 1024);

  doc["jsonrpc"] = "2.0";
  doc["id"] = id;

  JsonObject result = doc.createNestedObject("result");

  result["job_totals"]["total_jobs"] = 3;
  result["job_totals"]["total_time"] = 11748.077333278954;
  result["job_totals"]["total_print_time"] = 11348.794790096988;
  result["job_totals"]["total_filament_used"] = 11615.718840001999;
  result["job_totals"]["longest_job"] = 11665.191012736992;
  result["job_totals"]["longest_print"] = 11348.794790096988;

  serializeJson(doc, g_jsondata);
}

void Moonraker::build_printer_objects_list(int id)
{
  DynamicJsonDocument doc(2 * 1024);

  doc["jsonrpc"] = "2.0";
  doc["id"] = id;

  JsonObject result = doc.createNestedObject("result");

  result["objects"] = serialized("[\"gcode\", \"toolhead\", \"bed_mesh\", \"configfile\"]");

  serializeJson(doc, g_jsondata);
}

void Moonraker::build_printer_objects_subscribe(int id)
{
  DynamicJsonDocument doc(2 * 1024);

  doc["jsonrpc"] = "2.0";
  doc["id"] = id;

  JsonObject result = doc.createNestedObject("result");
  JsonObject status = doc.createNestedObject("status");

  result["eventtime"] = 578243.57824499;

  status["gcode_move"]["absolute_coordinates"] = true;
  status["gcode_move"]["absolute_extrude"] = true;
  status["gcode_move"]["extrude_factor"] = 1;
  status["gcode_move"]["gcode_position"] = serialized("[0,0,0,0]");
  status["gcode_move"]["homing_origin"] = serialized("[0,0,0,0]");
  status["gcode_move"]["position"] = serialized("[0,0,0,0]");
  status["gcode_move"]["speed"] = 1500;
  status["gcode_move"]["speed_factor"] = 1;

  status["toolhead"]["position"] = serialized("[0,0,0,0]");
  status["toolhead"]["status"] = "Ready";

  serializeJson(doc, g_jsondata);
}

void Moonraker::build_server_gcode_store(int id)
{
  DynamicJsonDocument doc(2 * 1024);

  doc["jsonrpc"] = "2.0";
  doc["id"] = id;

  JsonObject result = doc.createNestedObject("result");
  // JsonObject gcode_store = doc.createNestedObject("gcode_store");

  result["gcode_store"] = serialized("[]");
  serializeJson(doc, g_jsondata);
}

void Moonraker::build_printer_gcode_help(int id)
{
  DynamicJsonDocument doc(2 * 1024);

  doc["jsonrpc"] = "2.0";
  doc["id"] = id;

  JsonObject result = doc.createNestedObject("result");
  // JsonObject gcode_store = doc.createNestedObject("gcode_store");

  result["gcode_store"] = serialized("[]");
  serializeJson(doc, g_jsondata);
}

void Moonraker::build_server_temperature_store(int id)
{
  DynamicJsonDocument doc(2 * 1024);

  doc["jsonrpc"] = "2.0";
  doc["id"] = id;

  JsonObject result = doc.createNestedObject("result");
  // JsonObject gcode_store = doc.createNestedObject("gcode_store");

  result["extruder"]["temperatures"] = serialized("[21.05, 21.12, 21.1, 21.1, 21.1]");
  result["extruder"]["targets"] = serialized("[0, 0, 0, 0, 0]");
  result["extruder"]["powers"] = serialized("[0, 0, 0, 0, 0]");

  result["temperature_fan my_fan"]["temperatures"] = serialized("[21.05, 21.12, 21.1, 21.1, 21.1]");
  result["temperature_fan my_fan"]["targets"] = serialized("[0, 0, 0, 0, 0]");
  result["temperature_fan my_fan"]["powers"] = serialized("[0, 0, 0, 0, 0]");

  result["temperature_sensor my_sensor"]["temperatures"] = serialized("[21.05, 21.12, 21.1, 21.1, 21.1]");

  serializeJson(doc, g_jsondata);
}

void Moonraker::build_machine_device_power_devices(int id)
{
  DynamicJsonDocument doc(2 * 1024);

  doc["jsonrpc"] = "2.0";
  doc["id"] = id;

  JsonObject result = doc.createNestedObject("result");
  // JsonObject gcode_store = doc.createNestedObject("gcode_store");

  result["devices"] = serialized("[]");

  serializeJson(doc, g_jsondata);
}

void Moonraker::build_machine_update_status(int id)
{
  DynamicJsonDocument doc(2 * 1024);

  doc["jsonrpc"] = "2.0";
  doc["id"] = id;

  JsonObject result = doc.createNestedObject("result");
  // JsonObject gcode_store = doc.createNestedObject("gcode_store");

  result["version_info"] = false;
  result["github_rate_limit"] = 60;
  result["github_requests_remaining"] = 57;
  result["github_limit_reset_time"] = 1615836932;

  result["version_info"] = serialized("{}");

  serializeJson(doc, g_jsondata);
}

void Moonraker::build_printer_query_endstops_status(int id)
{
  DynamicJsonDocument doc(2 * 1024);

  doc["jsonrpc"] = "2.0";
  doc["id"] = id;

  JsonObject result = doc.createNestedObject("result");
  // JsonObject gcode_store = doc.createNestedObject("gcode_store");

  result["x"] = "TRIGGERED";
  result["y"] = "open";
  result["z"] = "open";

  serializeJson(doc, g_jsondata);
}

void Moonraker::build_server_database_list(int id)
{
  DynamicJsonDocument doc(2 * 1024);

  doc["jsonrpc"] = "2.0";
  doc["id"] = id;

  JsonObject result = doc.createNestedObject("result");
  // JsonObject gcode_store = doc.createNestedObject("gcode_store");

  result["namespaces"] = serialized("[\"gcode_metadata\",\"history\",\"moonraker\",\"test_namespace\"]");

  serializeJson(doc, g_jsondata);
}

void Moonraker::build_printer_gcode_script(int id)
{
  DynamicJsonDocument doc(2 * 1024);

  doc["jsonrpc"] = "2.0";
  doc["id"] = id;
  doc["result"] = "ok";
  serializeJson(doc, g_jsondata);
}

void Moonraker::build_server_files_list(int id)
{
  DynamicJsonDocument doc(2 * 1024);

  doc["jsonrpc"] = "2.0";
  doc["id"] = id;
  doc["result"] = serialized("[]");
  serializeJson(doc, g_jsondata);
}

void Moonraker::handleWebSocketMessage(AsyncWebSocketClient *client, void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    data[len] = 0;
    Esp3DCom::echo((const char *)data);
    DynamicJsonDocument JSON_Buffer(2 * 1024);
    DeserializationError error = deserializeJson(JSON_Buffer, data); /* 将JSON_Value字符串反序列化为JSON报文的格式,存入JSON_Buffer中 */
    if (error)                                                       /* 若反序列化返回错误,则打印错误信息,然后中止程序 */
    {
      Serial.println("deserializeJson JSON_Buffer is ERROR!!!");
      return;
    }
    else
    {
      String method = JSON_Buffer["method"];
      int id = JSON_Buffer["id"];
      if (method == "server.info")
      {
        build_server_info(id);
        client->text(g_jsondata);
      }
      else if (method == "server.connection.identify")
      {
        build_connection_id(id);
        client->text(g_jsondata);
      }
      else if (method == "printer.info")
      {
        build_printer_info(id);
        client->text(g_jsondata);
      }
      else if (method == "server.config")
      {
        build_server_config(id);
        client->text(g_jsondata);
      }
      else if (method == "machine.proc_stats")
      {
        build_machine_proc_stats(id);
        client->text(g_jsondata);
      }
      else if (method == "machine.system_info")
      {
        build_machine_system_info(id);
        client->text(g_jsondata);
      }
      else if (method == "server.device_power.devices")
      {
        build_error_message(id, -32601, "Method not found");
        client->text(g_jsondata);
      }
      else if (method == "server.update.status")
      {
        build_error_message(id, -32601, "Method not found");
        client->text(g_jsondata);
      }
      else if (method == "server.history.list")
      {
        build_server_history_list(id);
        client->text(g_jsondata);
      }
      else if (method == "server.history.totals")
      {
        build_server_history_totals(id);
        client->text(g_jsondata);
      }
      else if (method == "server.files.get_directory")
      {
        build_files_get_directory(id);
        client->text(g_jsondata);
      }
      else if (method == "printer.objects.list")
      {
        build_printer_objects_list(id);
        client->text(g_jsondata);
      }
      else if (method == "printer.objects.subscribe")
      {
        build_printer_objects_subscribe(id);
        client->text(g_jsondata);
      }
      else if (method == "server.gcode_store")
      {
        build_server_gcode_store(id);
        client->text(g_jsondata);
      }
      else if (method == "server.temperature_store")
      {
        build_server_temperature_store(id);
        client->text(g_jsondata);
      }
      else if (method == "printer.gcode.help")
      {
        build_printer_gcode_help(id);
        client->text(g_jsondata);
      }
      else if (method == "machine.device_power.devices")
      {
        build_machine_device_power_devices(id);
        client->text(g_jsondata);
      }
      else if (method == "machine.update.status")
      {
        build_machine_update_status(id);
        client->text(g_jsondata);
      }
      else if (method == "printer.query_endstops.status")
      {
        build_printer_query_endstops_status(id);
        client->text(g_jsondata);
      }
      else if (method == "server.database.list")
      {
        build_server_database_list(id);
        client->text(g_jsondata);
      }
      else if (method == "printer.gcode.script")
      {
        build_printer_gcode_script(id);
        client->text(g_jsondata);
      }
      else if (method == "server.files.list")
      {
        build_server_files_list(id);
        client->text(g_jsondata);
      }
      else
      {
        Esp3DCom::echo(method.c_str());
        build_error_message(id, -32601, "Method not found");
        client->text(g_jsondata);
      }
    }
  }
}

void Moonraker::notFound(AsyncWebServerRequest *request)
{
  Esp3DCom::echo("-------------------------notFound-------------------------");
  Esp3DCom::echo(request->url().c_str());

  AsyncWebServerResponse *response = request->beginResponse(404, "text/plain", "Not found");
  response->addHeader("Access-Control-Allow-Origin", "*");
  request->send(response);
}

void Moonraker::sendJsonRPC(class AsyncWebServerRequest *request, String res)
{
  AsyncWebServerResponse *response = request->beginResponse(200, "application/json", res);
  response->addHeader("Access-Control-Allow-Origin", "*");
  request->send(response);
}
void Moonraker::handle_server_info(class AsyncWebServerRequest *request)
{
  sendJsonRPC(request, "{\"klippy_connected\":true,\"klippy_state\":\"ready\",\"components\":[\"database\",\"file_manager\",\"klippy_apis\",\"machine\",\"data_store\",\"shell_command\",\"history\",\"octoprint_compat\",\"update_manager\",\"power\"],\"failed_components\":[],\"registered_directories\":[\"config\",\"gcodes\",\"config_examples\",\"docs\"],\"warnings\":[\"Invalid config option 'api_key_path' detected in section [authorization]. Remove the option to resolve this issue. In the future this will result in a startup error.\",\"Unparsed config section [fake_section] detected.  This may be the result of a component that failed to load.  In the future this will result in a startup error.\"],\"websocket_count\":2,\"moonraker_version\":\"v0.7.1-105-ge4f103c\",\"api_version\":[1,0,0],\"api_version_string\":\"1.0.0\"}");
}

void Moonraker::handle_database_item(class AsyncWebServerRequest *request)
{
  if (request->hasArg("namespace") && request->hasArg("key"))
  {
    String name_space = request->arg("namespace");
    String key = request->arg("key");
    if (name_space == "fluidd")
    {
      if (key == "uiSettings")
        sendJsonRPC(request, "{\"result\":{\"namespace\":\"fluidd\",\"key\":\"uiSettings\",\"value\":{\"theme\":{\"isDark\":true,\"logo\":{\"src\":\"/logo_eva.svg\",\"dark\":\"#232323\",\"light\":\"#ffffff\",\"dynamic\":false},\"currentTheme\":{\"primary\":\"#76FB00\"}},\"general\":{\"locale\":\"en\"}}}}");
      else if (key == "macros")
        sendJsonRPC(request, "{\"result\":{\"namespace\":\"fluidd\",\"key\":\"macros\",\"value\":{}}}");
      else if (key == "console")
        sendJsonRPC(request, "{\"result\":{\"namespace\":\"fluidd\",\"key\":\"console\",\"value\":{}}}");
      else if (key == "charts")
        sendJsonRPC(request, "{\"result\":{\"namespace\":\"fluidd\",\"key\":\"charts\",\"value\":{}}}");
      else if (key == "cameras")
        sendJsonRPC(request, "{\"result\":{\"namespace\":\"fluidd\",\"key\":\"cameras\",\"value\":{}}}");
      else if (key == "layout")
        sendJsonRPC(request, "{\"result\":{\"namespace\":\"fluidd\",\"key\":\"layout\",\"value\":{\"layouts\":{\"dashboard\":{\"container1\":[{\"id\":\"printer-status-card\",\"enabled\":true,\"collapsed\":false},{\"id\":\"camera-card\",\"enabled\":true,\"collapsed\":false},{\"id\":\"toolhead-card\",\"enabled\":true,\"collapsed\":false},{\"id\":\"macros-card\",\"enabled\":true,\"collapsed\":false},{\"id\":\"outputs-card\",\"enabled\":true,\"collapsed\":false},{\"id\":\"printer-limits-card\",\"enabled\":true,\"collapsed\":false},{\"id\":\"retract-card\",\"enabled\":true,\"collapsed\":false}],\"container2\":[{\"id\":\"temperature-card\",\"enabled\":true,\"collapsed\":false},{\"id\":\"console-card\",\"enabled\":true,\"collapsed\":false},{\"id\":\"jobs-card\",\"enabled\":true,\"collapsed\":false},{\"id\":\"gcode-preview-card\",\"enabled\":true,\"collapsed\":false}]}}}}}");
      else
      {
        // request->sendHeader("Access-Control-Allow-Origin", "*");
        request->send(404, "text/plain", "");
      }
    }
  }
  else
  {
    // request->sendHeader("Access-Control-Allow-Origin", "*");
    request->send(404, "text/plain", "");
  }
}

void Moonraker::handle_oneshot_token(class AsyncWebServerRequest *request)
{
  sendJsonRPC(request, "{\"result\":\"HBBABDWQAHSWG542SVVY7SE6VNJPGCXF\"}");
}
