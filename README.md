# ardunio

A project that uses ESP8266 WiFi modules that communicate with a Raspberry Pi server to control accessories on the Garfield-Clarendon Model Railroad Club.

### Prerequisites

The code running on the ESP8266 is developed using the Arduino IDE

The ControlServer application running on the Raspberry Pi makes use of the Qt framework.  The ESP8266 uses WebSockets to communicate with the ControlServer application.  A separate RESTful API provides access to commands to activate routes, set turnouts and update data in the configuration database.

[API Reference](https://github.com/garfieldclarendon/ardunio/api/index.html)

## Built With

* [Arduino](https://www.arduino.cc/en/Main/Software) - Arduino Tools
* [Qt](https://www.qt.io/) - Qt Framework

## Contributing

Please read [CONTRIBUTING.md](https://gist.github.com/PurpleBooth/b24679402957c63ec426) for details on our code of conduct, and the process for submitting pull requests to us.

## Authors

* **John Reilly** - *Initial work*

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details
