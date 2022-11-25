# OpenSmartBattery
 Open-source smart battery firmware targeting the ATtiny84 platform

### About
This started as a soft fork of [iam4722202468's ThinkpadBattery project](https://github.com/iam4722202468/ThinkpadBattery), but after sending a few pull requests I decided to do a (more or less) complete rewrite.

**Like `ThinkpadBattery`, OpenSmartBattery is still signifcantly lacking in basic functionality and should be considered alpha or pre-alpha in its current state**

### Configuration
Configuration of most values is done in `lib/OpenSmartBattery/config.hpp`. `config::HardwareConfig::Pins` should give you a good idea of the hardware configuration needed until I get around to writing a guide.

You will also need to implement the specific authentication proceedure that your battery needs in `lib/OpenSmartBattery/authentication.hpp` (there is an implementation of the SHA-1 method there if you need it).

### Building
This project it set up to use VSCode + PlatformIO for building and deploying the firmware. As such, you will need VSCode and the PlatformIO extension to build it without some work on your own. I will not provide support for building using other methods, as this method is the easiest to maintain.

You'll need to download [these files from cryptosuite2](https://github.com/daknuett/cryptosuite2/tree/master/sha) and put them in `lib/SHA/` in order to use the existing SHA-1 authentication implementation. `cryptosuite2` is needed because most modern batteries use SHA-1 hashing to validate their authenticity. Older batteries simply transmit the unlock code instead of doing a signing proceedure.

### Development tips
I recommend developing on a more forgiving Android device like a Uno or Mega before flashing to your ATtiny84, as it makes it far easier to debug your code.
