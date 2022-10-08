#include <esphome.h>
#include <WireGuard-ESP32.h>

#pragma once

class WgSimpleComponent : public Component{
  public:
    WgSimpleComponent(const char * private_key, const char * public_key, IPAddress local_ip, const char * endpoint_address, const int endpoint_port) :
      private_key(private_key), public_key(public_key), local_ip(local_ip), endpoint_address(endpoint_address), endpoint_port(endpoint_port)
      {}
    void setup() override {
      wg.begin(
        local_ip,
        private_key,
        endpoint_address,
        public_key,
        endpoint_port);
    }
    void loop() override {
    }
    private:
    WireGuard wg;
        // WireGuard configuration --- UPDATE this configuration from JSON
    const char * private_key;
    const char * public_key;     // [Peer] PublicKey
    const IPAddress local_ip;            // [Interface] Address
    const char * endpoint_address;    // [Peer] Endpoint
    const int endpoint_port;              // [Peer] Endpoint
};
