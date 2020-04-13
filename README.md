# nRF91TCPHTTPTest

## Test BSD library - NB-IoT TCP HTTP client connect to worldtimeapi.org

### nRF Connect SDK!
    https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/index.html

    Installing the nRF Connect SDK through nRF Connect for Desktop
    https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/gs_assistant.html

### Nordicsemi nRF9160 NB-IoT 
    https://www.nordicsemi.com/Software-and-tools/Development-Kits/nRF9160-DK

### CLion - A cross-platform IDE for C
    https://devzone.nordicsemi.com/f/nordic-q-a/49730/howto-nrf9160-development-with-clion

### Application Description
    curl Test 
    # curl "http://worldtimeapi.org/api/timezone/Africa/Johannesburg"

    {
       "week_number": 16,
       "utc_offset": "+02:00",
       "utc_datetime": "2020-04-13T07:08:04.066847+00:00",
       "unixtime": 1586761684,
       "timezone": "Africa/Johannesburg",
       "raw_offset": 7200,
       "dst_until": null,
       "dst_offset": 0,
       "dst_from": null,
       "dst": false,
       "day_of_year": 104,
       "day_of_week": 1,
       "datetime": "2020-04-13T09:08:04.066847+02:00",
       "client_ip": "172.104.140.63",
       "abbreviation": "SAST"
    }


    Client: Connect to http://worldtimeapi.org/api/timezone/Africa/Johannesburg
    Client: Parse JSON and print  
       datetime   -> 2020-04-13T13:45:03.626399+02:00
       timezone   -> Africa/Johannesburg
       utc_offset -> +02:00
    Client: Close socket

### Test Server 
    # Test Server
    # Proxy
    CONFIG_SERVER_HOST="172.104.140.63"
    CONFIG_SERVER_PORT=8000
    # worldtimeapi.org
    CONFIG_SERVER_HOST="52.48.90.17"
    CONFIG_SERVER_PORT=80

### Build hex 
    $ export ZEPHYR_BASE=/????
    $ west build -b nrf9160_pca10090ns

### Program nRF9160-DK using nrfjprog
    $ nrfjprog --program build/zephyr/merged.hex -f nrf91 --chiperase --reset --verify


### nRF Connect
![alt text](https://raw.githubusercontent.com/FrancisSieberhagen/nRF91TCPHTTPTest/master/images/nRFConnect.jpg)
