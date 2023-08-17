#include "WebServerManager.h"
#include "Config.h"

// HTTP status codes
const int HTTP_OK = 200;
const int HTTP_INTERNAL_SERVER_ERROR = 500;
const int HTTP_METHOD_NOT_ALLOWED = 405;


// void relayActionSwitch(char action) {
//     switch(action) {
//         case "enable":
//             break;
//         case "schedule":
//             break;
//         case "add":
//             break;
//         case "delete":
//             break;
//         default:
//             break;
//     }
// }

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
            
            // const size_t bufferSize = JSON_OBJECT_SIZE(100); // Adjust as needed
            // StaticJsonDocument<bufferSize> jsonDoc;
            // DeserializationError error = deserializeJson(jsonDoc, data, len);

            // String jsonStr((char *)data);
            // StaticJsonDocument<200> jsonDoc; // Adjust the buffer size as needed
            // DeserializationError error = deserializeJson(jsonDoc, jsonStr);

            Serial.println("Received WebSocket message: " + String((char*)data));

            // Parse the received JSON data
            const size_t bufferSize = JSON_OBJECT_SIZE(100);
            DynamicJsonDocument jsonDoc(bufferSize);

            DeserializationError error = deserializeJson(jsonDoc, (char*)data);

            if (!error) {
                // const char* pin = jsonDoc["relayId"];
                const char* pin = jsonDoc["relayPin"];
                // const char* action = jsonDoc["action"]; 

                Serial.print("Parsed value: ");
                Serial.println(pin);
                
                if (String(pin).equals("25") || String(pin).equals("26") || String(pin).equals("27")) {
                    Serial.println("Pin recognised");
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
    doc["relays"][relayIndex]["times"][timeIndex]["startTime"].set(json["startTime"]);
    doc["relays"][relayIndex]["times"][timeIndex]["duration"].set(json["duration"]);

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
    JsonObject nested = doc["relays"][json["relayIndex"]]["times"].as<JsonArray>().createNestedObject();
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

    doc["relays"][relayIndex]["times"].as<JsonArray>().remove(timeIndex);

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
    nested["times"] = array;

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

