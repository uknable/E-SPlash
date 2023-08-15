#include "Config.h"

void setup()
{
  Serial.begin(9600);
  setupStorage();
  readDataJson();
  configureWiFiSettings();
  scanAvailableWiFiNetworks();
  initiateWiFiConnection();
  disableAllDevices();
  printCompilationTimestamp();
  initializeRtc();
  initializeServer();
  serverHandle();
}

void loop()
{
  ws.cleanupClients();
  handleWiFiConnection();
  if (isScheduleMode)
  {
    scheduleMode();
  }
  //sleep(1);
}