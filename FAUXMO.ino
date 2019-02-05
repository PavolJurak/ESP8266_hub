void fauxmoSetup()
{
  fauxmo.addDevice("light one");
  fauxmo.addDevice("light two");
  fauxmo.addDevice("light three");
  fauxmo.addDevice("light four");
  fauxmo.addDevice("light five");
  fauxmo.addDevice("light six");
  fauxmo.addDevice("bedroom");
  fauxmo.enable(true);

  fauxmo.onSetState([](unsigned char device_id, const char * device_name, boolean state) {
    switch (device_id) {
      case 0:
        light1 = state;
        actionLight1 = true;
        break;
      case 1:
        light2 = state;
        actionLight2 = true;
        break;
      case 2:
        light3 = state;
        actionLight3 = true;
        break;
      case 3:
        light4 = state;
        actionLight4 = true;
        break;
      case 4:
        light5 = state;
        actionLight5 = true;
        break;
      case 5:
        light6 = state;
        actionLight6 = true;
        break;
      case 6:
        bedroom = state;
        actionBedroom = true;
        break;
    }
    String sta = state ? "ON" : "OFF";
    Serial.println("MAIN Device " + (String)device_id + " " + (String)device_name + " " + sta);
    //Serial.printf("[MAIN] Device #%d (%s) state: %s\n", device_id, device_name, state ? "ON" : "OFF");
  });
  fauxmo.onGetState([](unsigned char device_id, const char * device_name) {
    switch (device_id) {
      case 0:
        return light1;
        break;
      case 1:
        return light2;
        break;
      case 2:
        return light3;
        break;
      case 3:
        return light4;
        break;
      case 4:
        return light5;
        break;
      case 5:
        return light6;
        break;
      case 6:
        return bedroom;
        break;
    }
  });
}

void handleRemoteCommand()
{
  while (true)
  {
    if (actionLight1 == true) {
      if (light1 == true) {
        mySwitch.send(LIGHT_1_ON, 24); // ON
        delay(delayTimeBetweenSendRC);
        Serial.println("Turn on light 1");
      } else {
        mySwitch.send(LIGHT_1_OFF, 24); //OFF
        delay(delayTimeBetweenSendRC);
        Serial.println("Turn off light 1");
      }
      actionLight1 = false;
      break;
    }
    if (actionLight2 == true) {
      if (light2 == true) {
        mySwitch.send(LIGHT_2_ON, 24); // ON
        Serial.println("Turn on light 2");
      } else {
        mySwitch.send(LIGHT_2_OFF, 24); //OFF
        Serial.println("Turn off light 2");
      }
      actionLight2 = false;
      delay(delayTimeBetweenSendRC);
      break;
    }
    if (actionLight3 == true) {
      if (light3 == true) {
        mySwitch.send(LIGHT_3_ON, 24); // ON
        Serial.println("Turn on light 3");
      } else {
        mySwitch.send(LIGHT_3_OFF, 24); //OFF
        Serial.println("Turn off light 3");
      }
      actionLight3 = false;
      delay(delayTimeBetweenSendRC);
      break;
    }
    if (actionLight4 == true) {
      if (light4 == true) {
        mySwitch.send(LIGHT_4_ON, 24); // ON
        Serial.println("Turn on light 4");
      } else {
        mySwitch.send(LIGHT_4_OFF, 24); //OFF
        Serial.println("Turn off light 4");
      }
      actionLight4 = false;
      delay(delayTimeBetweenSendRC);
      break;
    }
    if (actionLight5 == true) {
      if (light5 == true) {
        mySwitch.send(LIGHT_5_ON, 24); // ON
        Serial.println("Turn on light 5");
      } else {
        mySwitch.send(LIGHT_5_OFF, 24); //OFF
        Serial.println("Turn off light 5");
      }
      actionLight5 = false;
      delay(delayTimeBetweenSendRC);
      break;
    }
    if (actionLight6 == true) {
      if (light6 == true) {
        mySwitch.send(LIGHT_6_ON, 24); // ON
        Serial.println("Turn on light 6");
      } else {
        mySwitch.send(LIGHT_6_OFF, 24); //OFF
        Serial.println("Turn off light 6");
      }
      actionLight6 = false;
      delay(delayTimeBetweenSendRC);
      break;
    }
    if (actionBedroom == true) {
      if (bedroom == true) {
        mySwitch.send(ALL_LIGHTS_BEDROOM_ON, 24); // ON
        Serial.println("Turn on all lights in the bedroom");
      } else {
        mySwitch.send(ALL_LIGHTS_BEDROOM_OFF, 24); //OFF
        Serial.println("Turn off all lights in the bedroom");
      }
      actionBedroom = false;
      delay(delayTimeBetweenSendRC);
      break;
    }
    break;
  }
}
