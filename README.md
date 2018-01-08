# Vrinda Switch (ESP8266 Starter Pack Sketch with single relay module)

> There is no description for now.

## Basic Concept
![Alt text](diagram/diagram.png?raw=true "Diagram Concept")

## Example Wire
This example wire using Wemos D1 Mini and Relay Module
![Alt text](wemos_relay_bb.png?raw=true "Wemos")

## Related Packages

| Package | Docs | Description |
|---------|------|-------------|
| [`VrindaApp`](https://github.com/bonkzero404/VrindaApp) |  [![](https://img.shields.io/badge/API%20Docs-markdown-lightgrey.svg?style=flat-square)](/packages/react-router/docs) | Mobile Apps using React Native |
| [`VrindaApi`](https://github.com/bonkzero404/VrindaApp) | [![](https://img.shields.io/badge/API%20Docs-markdown-lightgrey.svg?style=flat-square)](/packages/react-router-dom/docs) | RestfFul Service for IoT Platform (Include MQTT Server) |
| [`VrindaSwitch`](https://github.com/bonkzero404/VrindaSwitch) | [![](https://img.shields.io/badge/API%20Docs-markdown-lightgrey.svg?style=flat-square)](/packages/react-router-native/docs) |  Arduino Sketch for single relay module |

## Install

```
Open an arduino IDE then upload to your wemos/nodemcu/ESP8266 board
```

## Usage

After finish compile and upload to your board, open Wifi setting from your smartphone, and search network SSID (ex: Vrinda Switch ESP8266) then insert wpa2 password ('Note: default password is "password" without double quotes')

<img align="center" width="300" src="https://raw.githubusercontent.com/bonkzero404/VrindaSwitch/master/screen-1.png" />

Open 192.168.4.1 from your browser to open captive portal

<img align="center" width="300" src="https://raw.githubusercontent.com/bonkzero404/VrindaSwitch/master/screen-2.png" />

Configure local network and MQTT server from captive portal, MQTT server is directed to <a href="https://github.com/bonkzero404/VrindaApi">VrindaApi</a>


<img align="center" width="300" src="https://raw.githubusercontent.com/bonkzero404/VrindaSwitch/master/screen-3.png" />

Save your configuration then enjoy it.


## Contribute

NOTES : this code still Quick and Dirty. PRs accepted.

## License

MIT © Janitra Panji
