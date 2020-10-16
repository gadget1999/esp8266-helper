#ifndef RF_h
#define RF_h

#include <RCSwitch.h>
//#include <FirebaseArduino.h>

class RFSensor {
protected:
  bool  _bReceiveEnabled = false;
  bool  _bSendEnabled = false;

  RCSwitch _rcSensor;

  String getRFCode(unsigned long decCode, unsigned int bitLen, unsigned int pulse, unsigned int protocol) {
    String sRet;
    char buffer[256] = { 0 };

    if (decCode == 0) {
      sRet = "Unknown encoding.";
    }
    else {
      snprintf(buffer, 255, "%lu, %u (Pulse=%uus,Protocol=%u)",
        decCode, bitLen, pulse, protocol);
      sRet = buffer;
    }

    return sRet;
  }

public:
  void Setup(byte pinSend, byte pinReceive = -1) {
    if (pinReceive >= 0) {
      _rcSensor.enableReceive(pinReceive);
      _bReceiveEnabled = true;
    }

    if (pinSend >= 0) {
      _rcSensor.enableTransmit(pinSend);
      _bSendEnabled = true;
    }

    SetParams(2, 800, 5); // per this article: https://forum.arduino.cc/index.php?topic=211294.0, set to 800  
  }

  void Loop() {
    if (_bReceiveEnabled && _rcSensor.available()) {
      String code = getRFCode(_rcSensor.getReceivedValue(), _rcSensor.getReceivedBitlength(), _rcSensor.getReceivedDelay(), _rcSensor.getReceivedProtocol());
      INFO("RF code: %s", code.c_str());
      _rcSensor.resetAvailable();
    }
  }

  void SetParams(int protocol, int pulseLen, int repeatNum) {
    _rcSensor.setProtocol(protocol);
    _rcSensor.setPulseLength(pulseLen);
    _rcSensor.setRepeatTransmit(repeatNum);
  }

  void SendRFCode(int code, int bitLen) {
    if (!_bSendEnabled) {
      ERROR("Send pin was not setup.");
      return;
    }

    _rcSensor.send(code, bitLen);
    delay(200);
    _rcSensor.send(code, bitLen);
  }
};

#endif