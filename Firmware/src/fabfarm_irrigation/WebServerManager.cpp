#include "WebServerManager.h"
#include "Config.h"

// HTTP status codes
const int HTTP_OK = 200;
const int HTTP_INTERNAL_SERVER_ERROR = 500;
const int HTTP_METHOD_NOT_ALLOWED = 405;

int getPinIndex(const char* pin) {
    const char* pinIndices[] = {
        RELAY_PIN_FRUIT,  // For RELAY_PIN_FRUIT
        RELAY_PIN_VEGETABLES,  // For RELAY_PIN_VEGETABLES
        RELAY_PIN_WATER   // For RELAY_PIN_WATER
    };

    for (int i = 0; i < sizeof(pinIndices) / sizeof(pinIndices[0]); i++) {
        if (String(pin).equals(pinIndices[i])) {
            return i;
        }
    }

    // return -1 when the pin is not recognised
    // TODO: write a handler for this possiblity
    return -1;
}

void relayDeleteSchedule(const char* pin, const char* scheduleId) {
    int index = getPinIndex(pin); 

    // hacky way of converting a string into an int but just for testing purposes
    const char* scheduleIndices[] = {
        "0",  
        "1",  
        "2",
        "3",
        "4"
    };

    int scheduleIndex = -1;

    for (int i = 0; i < sizeof(scheduleIndices) / sizeof(scheduleIndices[0]); i++) {
        if (String(scheduleId).equals(scheduleIndices[i])) {
           scheduleIndex = i;
        }
    }

    doc["relays"][index]["schedules"].remove(scheduleIndex);
}

void relayAddSchedule(const char* pin, const char* startTime, const char* duration) {
    int index = getPinIndex(pin); 

    Serial.print("Adding relay startTime: ");
    Serial.println(startTime);
    Serial.print("duration: ");
    Serial.println(duration);

    // Create a new JSON object to add to the "schedules" array
    JsonObject newSchedule = doc["relays"][index]["schedules"].createNestedObject();

    // Populate the new schedule object with key-value pairs
    newSchedule["startTime"] = startTime;
    newSchedule["duration"] = duration;
}

void relayScheduleModeChange(const char* pin) {
    int index = getPinIndex(pin); 

    bool scheduleState = doc["relays"][index]["isScheduleMode"];
    doc["relays"][index]["isScheduleMode"].set(!scheduleState);

    // turn off relay when going into schedule mode
    if (scheduleState) {
        doc["relays"][index]["isEnabled"].set(false);
    }
}

void relayEnable(const char* pin) {
    int index = getPinIndex(pin); 

    bool scheduleState = doc["relays"][index]["isScheduleMode"];

    // First check if pin is NOT in schedule mode
    if (scheduleState == false) {

        bool state = doc["relays"][index]["isEnabled"];
        doc["relays"][index]["isEnabled"].set(!state);

    } else { // This else statement shouldn't be possible as user doesn't have option to enable while in schedule mode
        Serial.println("Pin is in Schedule mode, how did you do that?");
    }
}


// Websocket code from https://chat.openai.com/share/5a88f9ca-4172-4c3e-8ae7-a0a75ff5e305
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        // Handle new WebSocket connection
        Serial.println("WebSocket client connected");
        client->text("ESP32 confirming WebSocket connection");
    } else if (type == WS_EVT_DISCONNECT) {
        // Handle WebSocket disconnection
        Serial.println("WebSocket client disconnected");
    } else if (type == WS_EVT_DATA) {
        // Handle WebSocket data received
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {

            Serial.println("Received WebSocket message: " + String((char*)data));

            // Parse the received JSON data
            const size_t bufferSize = JSON_OBJECT_SIZE(100);
            DynamicJsonDocument jsonDoc(bufferSize);

            DeserializationError error = deserializeJson(jsonDoc, (char*)data);

            if (!error) {
                const char* pin = jsonDoc["relayPin"];

                Serial.print("Parsed value: ");
                Serial.println(pin);
                
                if (String(pin).equals(RELAY_PIN_FRUIT) || String(pin).equals(RELAY_PIN_VEGETABLES) || String(pin).equals(RELAY_PIN_WATER)) {

                    const char* action = jsonDoc["action"]; 

                    // Enabling relay pin
                    if (strcmp(action, "enable") == 0) {

                        Serial.print("Enabling pin ");
                        Serial.println(pin);

                        relayEnable(pin);

                    } else if (strcmp(action, "scheduleMode") == 0) {

                        // Handle "schedule" action
                        Serial.print("Changing ScheduleMode for pin ");
                        Serial.println(pin);

                        relayScheduleModeChange(pin);

                    } else if (strcmp(action, "addSchedule") == 0) {

                        // Handle "add" action
                        Serial.print("Adding a schedule for pin ");
                        Serial.println(pin);
                        
                        const char* startTime = jsonDoc["startTime"]; 
                        const char* duration = jsonDoc["duration"]; 

                        relayAddSchedule(pin, startTime, duration);

                    } else if (strcmp(action, "deleteSchedule") == 0) {
                        const char* scheduleId = jsonDoc["scheduleId"]; 

                        // Handle "delete" action
                        Serial.print("Deleting schedule for pin ");
                        Serial.print(pin);
                        Serial.print(" at schedule id  ");
                        Serial.println(scheduleId);

                        relayDeleteSchedule(pin, scheduleId);

                    } else {
                        // Handle other cases
                    }

                    // Send response to React
                    DynamicJsonDocument jsonResponse(256); // Adjust the size as needed

                    if (writeDataJson()) {
                        jsonResponse["status"] = "Success";
                        // jsonResponse["message"] = "JSON ";
                    } else {
                        jsonResponse["status"] = "Failure";
                    }
                    
                    // Prepare JSON response
                    String jsonString;
                    serializeJson(jsonResponse, jsonString); // Serialize the JSON object to a string
                    
                    client->text(jsonString); // Send the JSON response back to the client
                } else {
                    Serial.println("Pin not recognised");
                }

            } else {
                Serial.print(F("Failed to parse JSON: "));
                Serial.println(error.c_str());
            }
            
        }
    }
}

// Helper function to serialize JSON and send response
void sendJsonResponse(AsyncWebServerRequest *request, DynamicJsonDocument &data)
{
    char jsonReply[1800];
    serializeJson(data, jsonReply);
    request->send(HTTP_OK, "application/json", jsonReply);
}

// Handler for GET /data.json
void handleGetDataJsonRequest(AsyncWebServerRequest *request)
{
    Serial.println("/data.json");
    DynamicJsonDocument data = doc;

    // Figured out format from: https://cplusplus.com/reference/ctime/strftime/
    data["global"]["time"] = rtc.getTime("%Y-%m-%dT%H:%M");
    data["global"]["temperature"] = readDHTTemperature();
    data["global"]["humidity"] = readDHTHumidity();
    data["global"]["batLevel"] = getBatteryLevel();

    sendJsonResponse(request, data);

}

// Handler for POST /updateData
void handleUpdateDataRequest(AsyncWebServerRequest *request, JsonVariant &json)
{
    doc = json.as<JsonObject>();

    if (writeDataJson())
    {
        request->send(HTTP_OK);
    }
    else
    {
        request->send(HTTP_INTERNAL_SERVER_ERROR);
    }
}

// Handler for GET /mode/(manual|scheduled)
void handleModeChangeRequest(AsyncWebServerRequest *request)
{
    isScheduleMode = request->pathArg(0).equals("schedule-mode");
    
    doc["data"]["isScheduleMode"].set(isScheduleMode);

    if (writeDataJson())
    {
        if (isScheduleMode)
        {
            scheduleMode();
        }
        else
        {
            manualMode();
        }

        request->send(HTTP_OK);
    }
    else
    {
        request->send(HTTP_INTERNAL_SERVER_ERROR);
    }
}

// Handler for GET /relay/(number)/(on|off)
void handleRelayRequest(AsyncWebServerRequest *request)
{
    if (isScheduleMode)
    {
        request->send(HTTP_METHOD_NOT_ALLOWED);
    }

    int relayIndex = request->pathArg(0).toInt();
    bool state = request->pathArg(1).equals("on");

    Serial.printf("Relay %d: %d\n\r", relayIndex, state);
    doc["relays"][relayIndex]["isEnabled"].set(state);

    if (writeDataJson())
    {
        manualMode();
        request->send(HTTP_OK);
    }
    else
    {
        request->send(HTTP_INTERNAL_SERVER_ERROR);
    }
}

// Handler for POST /relay/update-time
void handleUpdateRelayTimeRequest(AsyncWebServerRequest *request, JsonVariant &json)
{
    int relayIndex = json["relayIndex"];
    int timeIndex = json["timeIndex"];
    doc["relays"][relayIndex]["schedules"][timeIndex]["startTime"].set(json["startTime"]);
    doc["relays"][relayIndex]["schedules"][timeIndex]["duration"].set(json["duration"]);

    if (writeDataJson())
    {
        request->send(HTTP_OK);
    }
    else
    {
        request->send(HTTP_INTERNAL_SERVER_ERROR);
    }
}

// Handler for POST /relay/add-time
void handleAddRelayTimeRequest(AsyncWebServerRequest *request, JsonVariant &json)
{
    JsonObject nested = doc["relays"][json["relayIndex"]]["schedules"].as<JsonArray>().createNestedObject();
    nested["startTime"] = "10:00";
    nested["duration"] = 30;

    if (writeDataJson())
    {
        request->send(HTTP_OK);
    }
    else
    {
        request->send(HTTP_INTERNAL_SERVER_ERROR);
    }
}

// Handler for DELETE /relay/(number)/time/(number)
void handleDeleteRelayTimeRequest(AsyncWebServerRequest *request)
{
    int relayIndex = request->pathArg(0).toInt();
    int timeIndex = request->pathArg(1).toInt();

    doc["relays"][relayIndex]["schedules"].as<JsonArray>().remove(timeIndex);

    if (writeDataJson())
    {
        request->send(HTTP_OK);
    }
    else
    {
        request->send(HTTP_INTERNAL_SERVER_ERROR);
    }
}

// Handler for POST /relay/add
void handleAddRelayRequest(AsyncWebServerRequest *request, JsonVariant &json)
{
    JsonObject nested = doc["relays"].as<JsonArray>().createNestedObject();

    const size_t CAPACITY = JSON_ARRAY_SIZE(3);

    StaticJsonDocument<CAPACITY> newArray;

    JsonArray array = newArray.to<JsonArray>();
    JsonObject newTime = array.createNestedObject();
    newTime["startTime"] = "10:00";
    newTime["duration"] = 30;
    nested["schedules"] = array;

    nested["name"] = json["name"];
    nested["pin"] = json["pin"];
    nested["isEnabled"] = false;

    if (writeDataJson())
    {
        request->send(HTTP_OK);
    }
    else
    {
        request->send(HTTP_INTERNAL_SERVER_ERROR);
    }
}

// Handler for DELETE /relay/(number)
void handleDeleteRelayRequest(AsyncWebServerRequest *request)
{
    int relayIndex = request->pathArg(0).toInt();
    Serial.printf("Removing relay %d\n\r", relayIndex);
    doc["relays"].as<JsonArray>().remove(relayIndex);

    if (writeDataJson())
    {
        request->send(HTTP_OK);
    }
    else
    {
        request->send(HTTP_INTERNAL_SERVER_ERROR);
    }
}

void serverHandle()
{
    server.on("/data.json", HTTP_GET, handleGetDataJsonRequest);
    AsyncCallbackJsonWebHandler *updateData = new AsyncCallbackJsonWebHandler("/updateData", handleUpdateDataRequest);

    // server.on("^/relays/27/schedule-mode", HTTP_GET, handleModeChangeRequest);
    server.on("^\\/relays\\/([0-9]+)\\/((manual)|(schedule-mode))$", HTTP_GET, handleModeChangeRequest);

    server.on("^\\/relays\\/([0-9]+)\\/((on)|(off))$", HTTP_GET, handleRelayRequest);
    AsyncCallbackJsonWebHandler *updateRelayTime = new AsyncCallbackJsonWebHandler("/relay/update-time", handleUpdateRelayTimeRequest);
    AsyncCallbackJsonWebHandler *addRelayTime = new AsyncCallbackJsonWebHandler("/relay/add-time", handleAddRelayTimeRequest);

    server.on("^\\/relays\\/([0-9]+)\\/time\\/([0-9]+)$", HTTP_DELETE, handleDeleteRelayTimeRequest);
    AsyncCallbackJsonWebHandler *addRelay = new AsyncCallbackJsonWebHandler("/relay/add", handleAddRelayRequest);

    server.on("^\\/relays\\/([0-9]+)$", HTTP_DELETE, handleDeleteRelayRequest);

    server.addHandler(updateData);
    server.addHandler(updateRelayTime);
    server.addHandler(addRelayTime);
    server.addHandler(addRelay);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/index.html", String(), false); });

    // Websocket code from https://chat.openai.com/share/5a88f9ca-4172-4c3e-8ae7-a0a75ff5e305
    // Placed before following "server.on()" because otherwise it will try to serve a file at "/ws"
    ws.onEvent(onWebSocketEvent); // Attach the event handler
    server.addHandler(&ws); // Add WebSocket handler to server

    server.on("^(\\/[a-zA-Z0-9_.-]*)$", HTTP_GET, [](AsyncWebServerRequest *request) {
        String file = request->pathArg(0);
        Serial.printf("Serving file %s\n\r", file.c_str());
        request->send(LittleFS, file, String(), false); 
    });

    isScheduleMode = doc["data"]["isScheduleMode"];

    if (!isScheduleMode)
    {
        manualMode();
    }


}

