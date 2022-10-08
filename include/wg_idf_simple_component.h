#include <esphome.h>
#include <esp_wireguard.h>


#pragma once

class WgSimpleComponent : public Component{
  public:
    WgSimpleComponent(const char * private_key, const char * public_key, const char * local_ip, const char * endpoint_address, const int endpoint_port) :
      private_key(private_key), public_key(public_key), local_ip(local_ip), endpoint_address(endpoint_address), endpoint_port(endpoint_port), err(ESP_FAIL), ctx(0) 
      {}
    void setup() override {
      this->wg_config = ESP_WIREGUARD_CONFIG_DEFAULT();

      this->wg_config.private_key = private_key;
      this->wg_config.listen_port = endpoint_port;
      this->wg_config.public_key = public_key;
      this->wg_config.allowed_ip = local_ip;
      this->wg_config.allowed_ip_mask = "255.255.255.0";
      this->wg_config.endpoint = endpoint_address;
      this->wg_config.port = endpoint_port;
      this->wg_config.persistent_keepalive = 10;

      /* If the device is behind NAT or stateful firewall, set persistent_keepalive.
        persistent_keepalive is disabled by default */
      // wg_config.persistent_keepalive = 10;

      err = esp_wireguard_init(&this->wg_config, &this->ctx);

      /* start establishing the link. after this call, esp_wireguard start
        establishing connection. */
      err = esp_wireguard_connect(&this->ctx);

      /* after some time, see if the link is up. note that it takes some time to
        establish the link */
      err = esp_wireguardif_peer_is_up(&this->ctx);
      if (err == ESP_OK) {
          /* the link is up */
      }
      else {
          /* the link is not up */
      }

      /* do something */

      //err = esp_wireguard_disconnect(&ctx);
    }
    void loop() override {
    }

    void ~WgSimpleComponent(){
      esp_wireguard_disconnect(&ctx);
    }

    private:
    const char * private_key;
    const char * public_key;     // [Peer] PublicKey
    const char * local_ip;
    const char * endpoint_address;    // [Peer] Endpoint
    const int endpoint_port;              // [Peer] Endpoint
    esp_err_t err;
    wireguard_config_t wg_config;
    wireguard_ctx_t ctx;
};
