# Hot water temperature monitor and logger
This is a project to try and make more efficient use of our hot water system both in terms of saving money and being more eco-friendly.

## Overview
Our hotwater system is typical of many of those incoporated into houses built in the UK between ~1970 and probably late 90s known as an [indirect boiler system](https://www.diydoctor.org.uk/projects/domestic_hot_water_systems.htm). It comprises of a hot water cylinder, gravity fed from a cold water tank in the loft. The tank incorporates a heat exchanger which is then heater via a gas boiler. The tank is (currently) heated twice a day via a timer (which forms part of the hotwater/central heating system).

There are 2 parts to this project. The first involves collecting (logging) temperature data from the hotwater tank then using this to analyse our usage. The second part then uses that data with a view to implementing one or more of the following:

- adjusting our current heating timer to try and only heat water when we need it 
- implementing some form of timer override such that we can extend/reduce the heating time dynamically based on external factors
- using surplus solar energy to heat the tank (via an existing immersion heater), rather than exporting it to the grid

## Logger
The hardware for the logger is little more that an ESP-32 (in this case a [C3 supermini](https://www.espboards.dev/esp32/esp32-c3-super-mini/)) connected to a couple of DS18B20 temperature sensors attached to it. There are [plenty of tutorials online which describe how to connect these things up](https://randomnerdtutorials.com/esp32-ds18b20-temperature-arduino-ide/). I've connected one of these to the lower third of the tank (next to the existing thermostat) and the other near the top.

The software for the logger can be found in the [logger](/logging) folder. This comprises an Arduino sketch for the ESP, which publishes the temperature sensor data to an MQTT broker every 15 seconds. There is also a Python script which logs the data to a MySQL/MariaDB database every 15 minutes (the database schema is also included). Along with the tank data, I also store the outside temperature from my [Raspberry Pi weather station](https://github.com/fridgemagnet3/rpi-weatherstation). This was to try and ascertain if this had any bearing on behaviour both in terms of heating and cooling down times. Since the water to the cylinder is fed from a tank in the loft, the water in that will be warmer during the summer months and equally colder during the winter. It may also affect the starting temperature of the water in the heat exchanger itself.

That's really all there is to this part, it's then just a case of exporting the data into a spreadsheet for manipulation.

## Analysis
This of course is purely based on our circumstances and usage however I'm summarising it here because it does then drive what I implemented as a result.

During the summer months (no central heating) we spend ~£20/month (excluding standing charge) on hot water. Ultimately this mostly gets used for showers, cleaning plus we also feed it to the dishwasher. As a result of analysing the logged data, I made some changes to the timer schedule which (in theory) should reduce the amount of heating time by a quarter with no real changes to our lifestyle - so we're looking at £15/month now.

There was no real change in tank hotwater temperatures as a result of seasonal changes (external temperature). In part this may have been helped by the heat exchanger water already being hot from running the central heating (which typically turns on about an hour before the hot water is heated). It was slightly interesting to note that with **just** the central heating running, the temperature in the lower part of the tank actually goes up. As a result of this, I've not seen the need to implement the 2nd bullet point in the summary (_implementing some form of timer override such that we can extend/reduce the heating time_). 

Of some interest is that quite often the temperature in the lower part of the tank is **higher** than that in the upper. This confused me initially because it's fairly common knowledge that heat rises so you would expect this not to be the case. However given that the heat exchanger itself sits directly below where the sensor is located this would indeed be the case. Over a period of the ensuing hours, the temperatures then level out, the lower part showing a drop & the upper one rising.
