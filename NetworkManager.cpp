#include <NetworkManager.h>
#include <EthernetESP32.h>

NetworkController::NetworkController(W5500Driver& w5500, const char* hostname)
  : w5500(w5500)
  , hostname(hostname)
  , wifiManager()
  , wifiConnected(false)
  , ethernetConnected(false)
  , configPortalActive(false)
  , lastConnectionState(false)
  , lastEthernetState(false)
  , lastReconnectAttempt(0)
  , configPortalStartTime(0)
  , wifiSSID(nullptr)
  , wifiPassword(nullptr)
{
  wifiManager.setHostname(hostname);
  wifiManager.setDarkMode(true);
  wifiManager.setTitle("Robotine");
  wifiManager.setConfigPortalBlocking(false);
}

void NetworkController::start(const char* WIFISSID, const char* WIFIPASSWORD)
{
  wifiSSID = WIFISSID;
  wifiPassword = WIFIPASSWORD;
  
  Serial.println("[Network] Starting network initialization...");
  
  if (connectEthernet()) {
    Serial.println("[Network] Ethernet connected successfully");
    lastEthernetState = true;
    return;
  }
  
  lastEthernetState = false;
  Serial.println("[Network] Ethernet not available, trying WiFi...");

  Serial.println("[Network] Trying stored WiFi credentials (if any)...");
  if (connectWiFiStored()) {
    Serial.println("[Network] WiFi connected successfully using stored credentials");
    return;
  }
  
  if (wifiSSID && wifiPassword && strlen(wifiSSID) > 0) {
    if (connectWiFi(wifiSSID, wifiPassword)) {
      Serial.println("[Network] WiFi connected successfully");
      return;
    }
    Serial.println("[Network] WiFi connection failed, starting config portal...");
  } else {
    Serial.println("[Network] No WiFi credentials provided, starting config portal...");
  }
  
  startConfigPortal();
}

bool NetworkController::connectEthernet()
{
  Serial.println("[Network] Attempting Ethernet connection...");
  Ethernet.setHostname(hostname);
  Ethernet.init(w5500);
  Ethernet.begin(5000);
  
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("[Network] Ethernet hardware not found");
    return false;
  }
  
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("[Network] Ethernet cable not connected");
    return false;
  }
  
  Serial.print("[Network] Waiting for Ethernet IP");
  uint32_t startTime = millis();
  while (Ethernet.localIP() == IPAddress(0, 0, 0, 0) && (millis() - startTime) < 15000) {
    delay(500);
    Serial.print(".");
    Ethernet.maintain();
  }
  Serial.println();
  
  IPAddress ip = Ethernet.localIP();
  ethernetConnected = (Ethernet.connected() && ip != IPAddress(0, 0, 0, 0));
  
  if (ethernetConnected) {
    Serial.print("[Network] Ethernet IP: ");
    Serial.println(ip);
  } else {
    Serial.println("[Network] Ethernet connection failed - no valid IP received");
  }
  return ethernetConnected;
}

bool NetworkController::connectWiFi(const char* ssid, const char* password)
{
  Serial.print("[Network] Attempting WiFi connection to: ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  
  WiFi.setHostname(hostname);
  WiFi.begin(ssid, password);
  
  Serial.print("[Network] Waiting for WiFi connection");
  uint32_t startTime = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < WIFI_TIMEOUT) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  
  wifiConnected = (WiFi.status() == WL_CONNECTED);
  if (wifiConnected) {
    Serial.print("[Network] WiFi connected! IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("[Network] WiFi connection timeout");
  }
  return wifiConnected;
}

bool NetworkController::connectWiFiStored()
{
  Serial.println("[Network] Attempting WiFi connection with stored credentials");
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  
  WiFi.setHostname(hostname);
  WiFi.begin();
  
  Serial.print("[Network] Waiting for WiFi connection (stored)");
  uint32_t startTime = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < WIFI_TIMEOUT) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  
  wifiConnected = (WiFi.status() == WL_CONNECTED);
  if (wifiConnected) {
    Serial.print("[Network] WiFi connected with stored credentials! IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("[Network] WiFi connection timeout (stored credentials)");
  }
  return wifiConnected;
}

void NetworkController::startConfigPortal()
{
  if (!configPortalActive) {
    Serial.println("[Network] Starting WiFi configuration portal...");
    WiFi.mode(WIFI_AP_STA);
    wifiManager.setConfigPortalTimeout(0);
    wifiManager.setAPStaticIPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));
    wifiManager.startConfigPortal("Robotine_Config", "");
    configPortalActive = true;
    configPortalStartTime = millis(); 
    Serial.println("[Network] Config portal active: Robotine_Config");
    Serial.println("[Network] AP IP: 192.168.4.1");

    for (int i = 0; i < 10; i++) {
      wifiManager.process();
      delay(100);
    }
  } else {
    configPortalStartTime = millis();
  }
}

void NetworkController::stopConfigPortal()
{
  if (configPortalActive) {
    Serial.println("[Network] Stopping config portal...");
    wifiManager.stopConfigPortal();
    configPortalActive = false;
    WiFi.mode(WIFI_STA);
    Serial.println("[Network] Config portal stopped");
  }
}

void NetworkController::update()
{
  bool currentWifiConnected = WiFi.isConnected();
  IPAddress ethernetIP = Ethernet.localIP();
  bool currentEthernetConnected = Ethernet.connected() && (ethernetIP != IPAddress(0, 0, 0, 0));
  
  if (!lastEthernetState && currentEthernetConnected) {
    Serial.println("[Network] Ethernet cable connected!");
    Serial.print("[Network] Ethernet IP: ");
    Serial.println(ethernetIP);
    
    if (currentWifiConnected) {
      Serial.println("[Network] Disconnecting WiFi due to Ethernet connection");
      WiFi.disconnect();
      currentWifiConnected = false;
    }
    
    if (configPortalActive) {
      stopConfigPortal();
    }
  }
  
  if (!lastEthernetState && Ethernet.linkStatus() == LinkON && ethernetIP == IPAddress(0, 0, 0, 0)) {
    Serial.println("[Network] Ethernet cable detected, waiting for IP...");
  }
  
  if (lastEthernetState && !currentEthernetConnected) {
    Serial.println("[Network] Ethernet cable disconnected!");
    
    if (wifiSSID && wifiPassword && strlen(wifiSSID) > 0) {
      Serial.println("[Network] Attempting to reconnect WiFi...");
      if (connectWiFi(wifiSSID, wifiPassword)) {
        Serial.println("[Network] WiFi reconnected successfully");
        currentWifiConnected = true;
      } else {
        Serial.println("[Network] WiFi reconnection with fixed credentials failed, trying stored credentials...");
        if (connectWiFiStored()) {
          Serial.println("[Network] WiFi reconnected successfully using stored credentials");
          currentWifiConnected = true;
        } else {
          Serial.println("[Network] WiFi reconnection failed, starting config portal...");
          startConfigPortal();
        }
      }
    } else {
      Serial.println("[Network] No fixed WiFi credentials, trying stored credentials...");
      if (connectWiFiStored()) {
        Serial.println("[Network] WiFi reconnected successfully using stored credentials");
        currentWifiConnected = true;
      } else {
        Serial.println("[Network] No WiFi credentials, starting config portal...");
        startConfigPortal();
      }
    }
  }
  
  wifiConnected = currentWifiConnected;
  ethernetConnected = currentEthernetConnected;
  
  bool currentConnectionState = isAnyConnected();
  
  if (configPortalActive) {
    wifiManager.process();
    
    if (WiFi.getMode() != WIFI_AP_STA && WiFi.getMode() != WIFI_AP) {
      Serial.println("[Network] Portal mode changed, restoring AP_STA mode");
      WiFi.mode(WIFI_AP_STA);
    }
    
    if (wifiConnected) {
      Serial.println("[Network] WiFi connected via portal, closing portal");
      wifiSSID = nullptr;
      wifiPassword = nullptr;
      stopConfigPortal();
      WiFi.mode(WIFI_STA);
    }
    
    uint32_t now = millis();
    uint32_t elapsed = (now >= configPortalStartTime) 
                      ? (now - configPortalStartTime) 
                      : (UINT32_MAX - configPortalStartTime + now);
    
    if (!currentConnectionState && elapsed > CONFIG_PORTAL_TIMEOUT) {
      Serial.println("[Network] Config portal timeout reached");
      stopConfigPortal();
      configPortalStartTime = 0;
    }
  }
  
  if (lastConnectionState && !currentConnectionState) {
    Serial.println("[Network] Connection lost!");
    if (!configPortalActive && !ethernetConnected) {
      Serial.println("[Network] Opening config portal due to connection loss");
      startConfigPortal();
    }
  }
  
  if (!lastConnectionState && currentConnectionState) {
    Serial.println("[Network] Connection restored!");
    if (configPortalActive) {
      stopConfigPortal();
    }
  }
  
  if (!wifiConnected && !configPortalActive && !ethernetConnected) {
    uint32_t now = millis();
    uint32_t elapsed = (now >= lastReconnectAttempt)
                      ? (now - lastReconnectAttempt)
                      : (UINT32_MAX - lastReconnectAttempt + now);
    
    if (elapsed >= RECONNECT_INTERVAL) {
      Serial.println("[Network] Attempting WiFi reconnect...");
      if (WiFi.getMode() != WIFI_STA) {
        WiFi.mode(WIFI_STA);
        delay(100);
      }
      WiFi.reconnect();
      lastReconnectAttempt = now;
    }
  }
  
  lastEthernetState = ethernetConnected;
  lastConnectionState = currentConnectionState;
}

bool NetworkController::isWiFiConnected() const
{
  return wifiConnected && WiFi.isConnected();
}

bool NetworkController::isEthernetConnected() const
{
  IPAddress ip = Ethernet.localIP();
  return ethernetConnected && Ethernet.connected() && (ip != IPAddress(0, 0, 0, 0));
}

bool NetworkController::isAnyConnected() const
{
  return isEthernetConnected() || isWiFiConnected();
}

IPAddress NetworkController::getWiFiIP() const
{
  return WiFi.localIP();
}

IPAddress NetworkController::getEthernetIP() const
{
  return Ethernet.localIP();
}
