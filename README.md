# NetworkManager

Biblioteca de gerenciamento de rede para ESP32 com suporte a WiFi e Ethernet.

## Características

- Gerenciamento automático de conexões WiFi e Ethernet
- Portal de configuração WiFi automático quando necessário
- Priorização de Ethernet sobre WiFi
- Reconexão automática
- Suporte a credenciais WiFi salvas
- Configuração de hostname para identificação na rede

## Dependências

- `EthernetESP32` - Gerenciamento de Ethernet
- `WiFiManager` - Portal de configuração WiFi

## Uso

```cpp
#include <NetworkManager.h>

W5500Driver w5500(ETH_CS);
NetworkController networkController(w5500, "MeuDispositivo");

void setup() {
  networkController.start("SSID", "PASSWORD");
}

void loop() {
  networkController.update();
  
  if (networkController.isAnyConnected()) {
    // Dispositivo conectado
  }
}
```

## Métodos Principais

- `start(ssid, password)` - Inicia o gerenciamento de rede
- `update()` - Deve ser chamado no loop principal
- `isWiFiConnected()` - Verifica se WiFi está conectado
- `isEthernetConnected()` - Verifica se Ethernet está conectado
- `isAnyConnected()` - Verifica se há alguma conexão ativa
- `getWiFiIP()` - Retorna o IP do WiFi
- `getEthernetIP()` - Retorna o IP do Ethernet

