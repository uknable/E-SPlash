#ifndef WEB_SERVER_MANAGER_H
#define WEB_SERVER_MANAGER_H

#include "Config.h"

void serverHandle();
void handleUpdateDataRequest(AsyncWebServerRequest *request, JsonVariant &json);
void handleRelayRequest(AsyncWebServerRequest *request);
void handleUpdateRelayTimeRequest(AsyncWebServerRequest *request, JsonVariant &json);
void handleAddRelayTimeRequest(AsyncWebServerRequest *request, JsonVariant &json);
void handleDeleteRelayTimeRequest(AsyncWebServerRequest *request);
void handleAddRelayRequest(AsyncWebServerRequest *request, JsonVariant &json);
void handleDeleteRelayRequest(AsyncWebServerRequest *request);
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

#endif // WEB_SERVER_MANAGER_H