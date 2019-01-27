# Layout Control System (LCS)

A project that uses ESP8266 WiFi modules that communicate with a Raspberry Pi server to control accessories on the Garfield-Clarendon Model Railroad Club.

### Prerequisites

The code running on the ESP8266 is developed using the Arduino IDE

The LCSServer application running on the Raspberry Pi makes use of the Qt framework.  The ESP8266 uses http calls to communicate with the LCSServer application  and UDP unicast messages to communicate with other ESP8266 controllers on the layout.  A separate RESTful API provides access to commands to activate routes, set turnouts and update data in the configuration database.

[Layout Control System Overview](https://github.com/garfieldclarendon/garfieldclarendon.github.io/blob/master/html/Documents/LCSOverview.pdf)

[Source Code Documentation](https://garfieldclarendon.github.io/html/index.html)

[API Documentation](https://garfieldclarendon.github.io/apidoc/index.html)


## Built With

* [Arduino](https://www.arduino.cc/en/Main/Software) - Arduino Tools
* [Qt](https://www.qt.io/) - Qt Framework
* [Eagle](https://www.autodesk.com/products/eagle/overview) - Eagle Printed Circuit Board (PCB) design software

## Contributing

Please read [CONTRIBUTING.md](https://gist.github.com/PurpleBooth/b24679402957c63ec426) for details on our code of conduct, and the process for submitting pull requests to us.

## Authors

* **John Reilly** - *Initial work*
* **Ryan Balla** - *Web Development*

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details
