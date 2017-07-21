define({ "api": [
  {
    "type": "get",
    "url": "/panel_list/",
    "title": "Get a list of Panels",
    "name": "PanelList",
    "group": "Panels",
    "description": "<p>Returns a list of defined panels</p>",
    "success": {
      "fields": {
        "Success 200": [
          {
            "group": "Success 200",
            "type": "Object[]",
            "optional": false,
            "field": "panels",
            "description": "<p>List of panels.</p>"
          },
          {
            "group": "Success 200",
            "type": "Number",
            "optional": false,
            "field": "panels.moduleIndex",
            "description": "<p>Index of the module.</p>"
          },
          {
            "group": "Success 200",
            "type": "Number",
            "optional": false,
            "field": "panels.panelID",
            "description": ""
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "panels.panelName",
            "description": "<p>Descriptive name of the panel.</p>"
          },
          {
            "group": "Success 200",
            "type": "Number",
            "optional": false,
            "field": "panels.serialNumber",
            "description": "<p>Controller's serial number.</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Success-Response:",
          "content": "  HTTP/1.1 200 OK\n[\n   {\n       \"moduleIndex\": \"0\",\n       \"panelID\": \"2\",\n       \"panelName\": \"CA Panel1\",\n       \"serialNumber\": \"637866\"\n   },\n   {\n       \"moduleIndex\": \"0\",\n       \"panelID\": \"1\",\n       \"panelName\": \"Gaskill Mine\",\n       \"serialNumber\": \"1551385\"\n   },\n]",
          "type": "json"
        }
      ]
    },
    "examples": [
      {
        "title": "Example usage:",
        "content": "http://localhost:8080/api/panel_list",
        "type": "json"
      }
    ],
    "version": "0.0.0",
    "filename": "./APIHandler.h",
    "groupTitle": "Panels"
  },
  {
    "type": "get",
    "url": "/panel_routes/:panelID",
    "title": "Get a list of Routes assigned to a Panel",
    "name": "PanelRoutes",
    "group": "Panels",
    "parameter": {
      "fields": {
        "Parameter": [
          {
            "group": "Parameter",
            "type": "Number",
            "optional": false,
            "field": "panelID",
            "description": "<p>The panel's ID.</p>"
          }
        ]
      }
    },
    "description": "<p>Returns a list of routes (buttons) assigned to a panel</p>",
    "success": {
      "fields": {
        "Success 200": [
          {
            "group": "Success 200",
            "type": "Object[]",
            "optional": false,
            "field": "routes",
            "description": "<p>List of routes assigned to the panel.</p>"
          },
          {
            "group": "Success 200",
            "type": "Bool",
            "optional": false,
            "field": "routes.isActive",
            "description": "<p>indicates if the route is active.</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "routes.routeDescription",
            "description": "<p>Descriptive name of the route</p>"
          },
          {
            "group": "Success 200",
            "type": "Number",
            "optional": false,
            "field": "routes.routeID",
            "description": "<p>The route's ID.</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "routes.routeName",
            "description": "<p>Route's name.</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Success-Response:",
          "content": "  HTTP/1.1 200 OK\n[\n   {\n       \"isActive\": \"false\",\n       \"routeDescription\": \"Gaskill Mine Route 1\",\n       \"routeID\": \"1\",\n       \"routeName\": \"GM 1\"\n   },\n   {\n       \"isActive\": \"true\",\n       \"routeDescription\": \"Gaskill Mine Route 2\",\n       \"routeID\": \"2\",\n       \"routeName\": \"GM 2\"\n   },\n]",
          "type": "json"
        }
      ]
    },
    "examples": [
      {
        "title": "Example usage:",
        "content": "http://localhost:8080/api/panel_routes?panelID=1",
        "type": "json"
      }
    ],
    "version": "0.0.0",
    "filename": "./APIHandler.h",
    "groupTitle": "Panels"
  },
  {
    "type": "get",
    "url": "/activate_route/:routeID",
    "title": "Activate Route",
    "name": "ActivateRoute",
    "group": "Routes",
    "parameter": {
      "fields": {
        "Parameter": [
          {
            "group": "Parameter",
            "type": "Number",
            "optional": false,
            "field": "routeID",
            "description": "<p>The route's ID.</p>"
          }
        ]
      }
    },
    "description": "<p>Activates the desired route.  When activated, all turnouts are set to the directing as defined in the route configuration.</p>",
    "examples": [
      {
        "title": "Example usage:",
        "content": "http://localhost:8080/api/activate_route?routeID=1",
        "type": "json"
      }
    ],
    "version": "0.0.0",
    "filename": "./APIHandler.h",
    "groupTitle": "Routes"
  },
  {
    "type": "get",
    "url": "/activate_turnout/:deviceID,turnoutState",
    "title": "Activate Turnout",
    "name": "ActivateTurnout",
    "group": "Turnouts",
    "parameter": {
      "fields": {
        "Parameter": [
          {
            "group": "Parameter",
            "type": "Number",
            "optional": false,
            "field": "deviceID",
            "description": "<p>The turnout's Device ID.</p>"
          },
          {
            "group": "Parameter",
            "type": "Number",
            "allowedValues": [
              "1",
              "3"
            ],
            "optional": false,
            "field": "turnoutState",
            "description": "<p>The desired state to set the turnout to.</p>"
          }
        ]
      }
    },
    "description": "<p>Sets the turnout to the desired state (direction) 1 = Normal 2 = Diverging</p>",
    "examples": [
      {
        "title": "Example usage:",
        "content": "http://localhost:8080/api/activate_turnout?deviceID=7&turnoutState=3",
        "type": "json"
      }
    ],
    "version": "0.0.0",
    "filename": "./APIHandler.h",
    "groupTitle": "Turnouts"
  }
] });
