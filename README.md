Hi,  this project started of with an absence of anything available which suited my needs.

I was designing a MODBUS RTU network, but it had to be accessable by WiFi/Internet.

The two gateways are identicle except for the WiFi configuration, one is via the epresssiff smart config app, the other is via the standard WPS on the router)

The project has the following connections on the ESP......

standard serial port connects to a RS485 there is no control pin included in the script, i like the auto send circuit to make the software simpler(or serial for simplicity)



Pin 4 - pullup is internal. Connect a switch to ground. This used to trigger the smart config listening when the device is turned on.

Pin 14 - connect an led with a resistor to ground ( this led flashes when you are in smart config mode).

Pin 12 - Connect an led with a resistor to ground (this led flashes whenconnecting to wiFi, solid when connected).

Pin 13 - Connect an led with a resistor to ground (this led goes solid when mDNS is espablished. i used this to find the device on the LAN via ping, although the smart app displays the IP address on the Android app).

Links to apps for android devices

https://play.google.com/store/apps/details?id=com.khoazero123.iot_esptouch_demo&hl=en_US&gl=US



