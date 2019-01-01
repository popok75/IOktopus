# IOktopus	
>>>>see also the website/blog :https://esp8266life.wordpress.com/ 

IOktopus is a sensor logger and controller for ESP8266. Current version runs a c++ server on board and provide a javascript client that :
	- allows to monitor sensor values from your browser (supported sensors are : DHT22,DS18B20,HTU21,SHT15)
	- provides a browser alarm on each sensor value
	- keeps and show max min values 
	- allows to explore sensors history in a zoomable graph (saved values in board memory, up to 1000items)
	- display board state (responsiveness, last boot)
	
Full version of IOktopus will additionally allow to :
	- display current and history of actuators values (relays, leds, motors,piezo)
	- directely control actuators values from browser
	- activate rules that control actuators from sensor values or timestamp (thermo/humidistat, timer)
	- build custom rules from basic operations, such as assign, compare, time, expressions (tinyexpression integration)
	
	
## How to run IOktopus
### Run the binary
### Compile the code
