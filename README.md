# Snipek

Remote microphone and speaker for Snips voice assistant system

## Description

Snipek is a [Sailfish OS](https://sailfishos.org/) app that provides remote
microphone and speaker capability for [Snips voice assistant system](https://snips.ai/).

Snips is a voice assistant system that doesn’t require external cloud service.
All processing (including voice) is entirely done locally, on a device where
Snips server is installed. To make Snipek app works you have to install Snips
server somewhere in your local network. More information about Snips server
installation options are in
[Snips server installation section](#snips-server-installation).

Snipek app connects to Snips server as an additional audio server.
It sends audio stream captured from the microphone and plays audio data
received from Snips server. It also provides few built-in skills that
enable phone control with the voice commands. Currently implemented skills are
described in [Snipek built-in skills section](#snipek-built-in-skills).
Possible use cases for Snipek are in [Use cases section](#use-cases-section).

*Snipek does not provide Snips server components. They should be installed seperatly
on a different machine (e.g. Raspberry Pi or any Debian machine).*

## Use cases

Snipek can be used in several configuration options. Here are few examples:

1. Snipek is used as primary mic/speaker for Snips.
   Snips server doesn't have any mic/speaker connected.
2. Snipek is used as secondary mic/speaker for Snips.
   Snips server has mic/speaker connected but it also listens voice commands from Snipek.
3. Snipek is used as mic/speaker for [voice commands](#snipek-built-in-skills) that let you control your phone.
   [Snipek assistant](#snipek-assistant-installation) has to be installed on Snips server.

## Snipek built-in skills

| Name | Skill behaviour | Intents | Trigger phrase (examples) |
|:-----|:----------------|:--------|:--------------------------|
| Date & Time | Reads current time or date. | getTime, getDate | What's the time? What's the date? |
| Call History | Reads events from call history e.g. all missed calls. | getCalls, getMissedCalls | Read today's calls. Read missed calls since yesterday. |

More skills are under implementation...

## Snipek assistant installation

Snipek assistant provides Intents for [Snipek built-in skills](#snipek-built-in-skills).
Intents are defined for particular language. Currently only English is supported.

To download and install Snipek assistant file on Debian/Raspbian, execute following commands:

   ```
   $ wget https://github.com/mkiol/Snipek/raw/master/assistant/assistant_proj_BAYAr2l4k5z.zip
   # mkdir -p /usr/share/snips
   # unzip assistant_proj_BAYAr2l4k5z.zip -d /usr/share/snips
   ```

## Snips server installation

According [Snips documentation](https://docs.snips.ai/), Snips supports installation
on Raspberry Pi and any Debian machine.
The easiest way is to use Raspberry Pi because it is officially supported platform
and well documented. The guide how to do it is available
[here](https://docs.snips.ai/getting-started/quick-start-raspberry-pi).

Alternatively you can also manually install Snips on Debian system
(it can be installed on phisical or virtual machine).

Here is quick step-by-step guide for Snips installation on a fresh Debian 9:

1. Make sure that your Debian server is connected to local network and its
   IP address is "visible" for your phone.
2. Enable "non-free" packages in `/etc/apt/sources.list`.
   For details see [Debian guide](https://wiki.debian.org/SourcesList).
3. Add Snips repository.

   ```
   # apt install dirmngr apt-transport-https
   # bash -c 'echo "deb https://debian.snips.ai/stretch stable main" > /etc/apt/sources.list.d/snips.list'
   # apt-key adv --fetch-keys  https://debian.snips.ai/5FFCD0DEB5BA45CD.pub
   # apt update
   ```

4. Install Mosquitto MQTT broker.

   ```
   # apt install mosquitto
   ```

5. Install essential Snips packages.

   ```
   # apt install snips-platform-voice snips-tts snips-watch
   ```

6. Download and install Snipek assistant file (skip this step and install your own assistant if you don't want to use [Snipek built-in skills](#snipek-built-in-skills)).

   ```
   $ wget https://github.com/mkiol/Snipek/raw/master/assistant/assistant_proj_BAYAr2l4k5z.zip
   # apt-get install unzip
   # mkdir -p /usr/share/snips
   # unzip assistant_proj_BAYAr2l4k5z.zip -d /usr/share/snips
   ```

7. Start Snips and check if all components are active.

   ```
   # systemctl start snips-*
   # systemctl status snips-*
   ```

8. Start Snipek app on your phone, configure IP address and port of
   MQTT broker (default MQTT port is 1883) and connect.
9. Say "Hey Snips" (default wake word) and check if Snipek reacts.

## Languages support

Snips supports following languages:

- German
- English
- Spanish
- French
- Italian
- Japanese
- Portuguese (Brazil)

When Snipek is used only as remote mic/speaker, it's language-agnostic.
It just forwards and receives audio samples to/from Snips server.

When Snipek is used for [voice commands](#snipek-built-in-skills)
that let you control your phone, language matters.
Currently built-in skills and Snipek assistant support only English language.

## Downloads

Binary packages for Sailfish OS can be downloaded from
[OpenRepos](https://openrepos.net/content/mkiol/snipek) and from official Jolla Store app.

## License

Snipek is developed as an open source project under
[Mozilla Public License Version 2.0](https://www.mozilla.org/MPL/2.0/).

The use of Snips assistant file is governed by
[Snips Terms of Use](https://docs.snips.ai/additional-resources/legal-and-privacy/terms-of-use-highlights).
