# Snipek
SailfishOS app that converts phone into remote microphone and speaker for Snips voice assistant system

Snips (https://snips.ai/) provides voice assistant system that doesn’t require external cloud service. All processing (including voice) is entirely done locally, on device where Snips is installed.

Snipek acts as remote microphone and speaker. It connects to Snips as an additional audio server. It sends audio stream captured from the microphone and plays audio files received from the Snips system.

At this moment, Snips supports installation on Raspberry Pi and Debian system. Snipek was tested on both configurations and it works with almost zero configuration effort.

## License
Snipek is a free application. The source code is subject to the terms of the [Mozilla Public License Version 2.0](https://www.mozilla.org/MPL/2.0/).
