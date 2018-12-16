# Snipek
Remote microphone and speaker for Snips voice assistant system

## Description
Snipek is a Linux desktop and [Sailfish OS](https://sailfishos.org/) app that converts phone into remote microphone and speaker for [Snips](https://snips.ai/) voice assistant system.

Snips provides voice assistant system that doesn’t require external cloud service. All processing (including voice) is entirely done locally, on device where Snips is installed.

Snipek app acts as remote microphone and speaker. It connects to Snips as an additional audio server. It sends audio stream captured from the microphone and plays audio files received from Snips system. At this moment Snips supports installation on Raspberry Pi 3 (but it works also on Raspberry Pi 2) and Debian desktop. App was tested on both platforms and it works with almost zero configuration effort.

## Downloads
Binary packages for Jolla phone and Jolla tablet can be downloaded
from [OpenRepos](https://openrepos.net/content/mkiol/snipek).

As an experiment, [Linux desktop version](https://github.com/mkiol/Snipek/tree/master/desktop/packages) is also available in following binary packages:
* DEB (tested on Ubuntu 18.04)
* RPM (tested on Fedora 28 and OpenSuse Leap 15.0)
* Arch Linux

## License
Snipek is developed as an open source project under
[Mozilla Public License Version 2.0](https://www.mozilla.org/MPL/2.0/).
