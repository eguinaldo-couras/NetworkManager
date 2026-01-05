#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <EthernetESP32.h>
#include <WiFiManager.h>

class W5500Driver;

class NetworkController {
public:
  NetworkController(W5500Driver& w5500, const char* hostname, const char* portalAPName = nullptr);
  
  void start(const char* WIFISSID, const char* WIFIPASSWORD);
  
  bool isWiFiConnected() const;
  bool isEthernetConnected() const;
  bool isAnyConnected() const;
  
  IPAddress getWiFiIP() const;
  IPAddress getEthernetIP() const;
  
  void update();

private:
  W5500Driver& w5500;
  const char* hostname;
  WiFiManager wifiManager;

  bool wifiConnected;
  bool ethernetConnected;
  bool configPortalActive;
  bool lastConnectionState;
  bool lastEthernetState;

  uint32_t lastReconnectAttempt;
  uint32_t configPortalStartTime;

  String wifiSSID;
  String wifiPassword;

  String portalAPName;
  IPAddress portalAPIP;
  IPAddress portalAPGateway;
  IPAddress portalAPNetmask;

  uint32_t wifiTimeout;
  uint32_t configPortalTimeout;

  bool connectEthernet();
  bool connectWiFi(const char* ssid, const char* password);
  bool connectWiFiStored();
  void startConfigPortal();
  void stopConfigPortal();

  static const uint32_t RECONNECT_INTERVAL = 5000;
  static const uint32_t DEFAULT_WIFI_TIMEOUT = 15000;
  static const uint32_t DEFAULT_CONFIG_PORTAL_TIMEOUT = 300000;
};

#endif 