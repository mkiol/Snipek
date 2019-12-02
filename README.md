# Snipek

Remote microphone and speaker for Snips voice assistant system on Sailfish OS

## Description

Snipek is a [Sailfish OS](https://sailfishos.org/) app that provides remote
microphone and speaker capability for [Snips voice assistant system](https://snips.ai/).

Snips is a voice assistant system that doesn’t require external cloud service.
All processing (including voice) is entirely done locally, on a device where
Snips is installed. To make Snipek app works you have to separately install Snips
on your Sailfish OS phone or on any another device in your local network.
More information about Snips installation options are in
[Snips installation section](#snips-installation).

Snipek app connects to Snips as an additional audio server.
It sends audio stream captured from the microphone and plays audio data
received from Snips. It also provides few built-in skills that
enable phone control with the voice commands. Currently implemented skills are
described in [Snipek built-in skills section](#snipek-built-in-skills).
Possible use cases for Snipek are outlined in [Use cases section](#use-cases-section).

*Snipek does not provide Snips software. Snips components should be installed separately
on a Sailfish OS phone or different machine (e.g. Raspberry Pi, any
Debian based computer).*

## Use cases

Snipek can be used in several configuration options. Here are few examples:

1. Snipek is used as a primary mic/speaker for Snips installed on another computer
   in your local network. Computer where Snips is installed may not have any
   mic/speaker connected.
2. Snipek is used as secondary mic/speaker for Snips. Computer where Snips is
   installed may have mic/speaker connected but it also listens voice commands from Snipek.
3. Snipek is used as a mic/speaker for Snips and as a voice assistant
   ([voice commands](#snipek-built-in-skills))
   that let you control your phone. Snips software as well as
   [Snipek assistant](#snipek-assistant-installation) are installed on another computer
   in your local network.
4. Snipek is used as a pure off-line voice assistant. Snips software and
   Snipek assistant are installed on a Sailfish OS phone.

## Snipek built-in skills

Skills are the capabilities of voice assistant i.e. the things that assistant can do with a voice command.

Following skills are implemented:

| Name | Skill behaviour | Intents | Trigger phrase (examples) |
|:-----|:----------------|:--------|:--------------------------|
| Date & Time | Reads current time or date. | getTime, getDate | What's the time? What's the date? |
| Call History | Reads events from call history e.g. all missed calls. | getCalls, getMissedCalls | Read today's calls. Read missed calls since yesterday. |

More skills will be added in the future...

## Snips installation

According [Snips documentation](https://docs.snips.ai/), Snips officially supports
following platforms:

- Raspberry Pi,
- Debian 9 (stretch).

Bare in mind that **Snips is not an open source software**. The use of Snips is is governed by
[Snips Terms of Use](https://docs.snips.ai/additional-resources/legal-and-privacy/website-terms-of-use).
The source code is not publicly available. Snips publishes only binaries for ARM and x86_64.

Luckily with some hack, installation on Sailfish OS (ARM only) is possible.

### Snips installation on Raspberry Pi

This is the easiest option because Raspberry Pi is officially supported platform
and very well documented. The guide how to install Snips on Raspberry Pi is available
[here](https://docs.snips.ai/getting-started/quick-start-raspberry-pi).

Remember to install [Snipek assistant](#snipek-assistant-installation) if you want
to use [Snipek voice commands](#snipek-built-in-skills).

### Snips installation on Debian

Debian installation is also supported by Snips but official documentation is less clear.
Here is quick step-by-step guide for Snips installation on a fresh Debian 9 (stretch) system:

1. Make sure that your Debian machine is connected to local network and its
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

6. Download and install Snipek assistant file (skip this step and install your own
   assistant if you don't want to use [Snipek built-in skills](#snipek-built-in-skills) or
   want to [create the assistant with Snipek Intents from the console](#snipek-assistant-installation)).

   ```
   $ wget https://github.com/mkiol/Snipek/raw/master/assistant/assistant_en.zip
   # apt-get install unzip
   # mkdir -p /usr/share/snips
   # unzip assistant_en.zip -d /usr/share/snips
   ```

7. Start Snips and check if all components are active.

   ```
   # systemctl start snips-*
   # systemctl status snips-*
   ```

8. Start Snipek app on your phone, configure IP address and port of
   MQTT broker (default MQTT port is 1883) and connect.
9. Say "Hey Snips" (default wake word) and check if Snipek reacts.

### Snips installation on Sailfish OS

Snips is not an open source software. Only binaries for ARM and x86_64 are available.
There is no x86 build (32 bit) therefore (at least right now) installation on Jolla Tablet
and any other non-ARM Sailfish OS devices is not possible.

To make Snips installation process as easy as possible, Snipek app provides two bash scripts:

- `snips_download.sh` - downloads all needed binaries from Snips/Raspbian/Debian reposotories
and downloads Snipek assistant (script can be executed on SFOS directly or on any Linux machine)
- `snips_start.sh` - starts/stops Snips components (script must be executed on SFOS directly)

After Snipek app installation, both scripts are in `/usr/share/harbour-snipek/snips`.

Here are few example usages:

```
# Display usage help for snips_download.sh:
$ snips_download.sh -h

# Download Snips to default dir (on SFOS):
$ snips_download.sh

# Download Snips to specific dir (can be executed on SFOS or any Linux machine):
$ snips_download.sh -d <dir>

# Check is all needed files exist in specific dir:
$ snips_download.sh -c -d <dir>

# Display usage help for snips_start.sh:
$ snips_start.sh -h

# Start Snips that has been downloaded to default dir:
$ snips_start.sh

# Start Snips that has been downloaded to specific dir:
$ snips_start.sh -d <dir>

# Stop Snips:
$ snips_start.sh -k

# Check if Snips is running:
$ snips_start.sh -c
```

Installation (`snips_download.sh`) has to be executed mannually from terminal
but starting/stopping (`snips_start.sh`) can be managed via Snipek app.

## Snipek assistant installation

Snipek assistant provides intents for [Snipek built-in skills](#snipek-built-in-skills).
Intents are defined for particular language. Currently only English is supported.

There are two ways to install Snipek intents:

1. Download already created [Snipek assistant file](https://github.com/mkiol/Snipek/tree/master/assistant)
   and unpack it on the computer where Snips is installed. This assistant only contains intents
   for built-in skills.

   To download and install on Debian/Raspbian, execute following commands:

   ```
   $ wget https://github.com/mkiol/Snipek/raw/master/assistant/assistant_en.zip
   # mkdir -p /usr/share/snips
   # unzip assistant_en.zip -d /usr/share/snips
   ```

   To download and install on Sailfish OS use `snips_download.sh` script:

   ```
   $ cd /usr/share/harbour-snipek/snips
   $ ./snips_download.sh -a
   ```

2. Alternatively, create your own assistant with [Snips console](https://console.snips.ai)
   and add Snipek intents from the store.
   Snipek intents are published [here](https://console.snips.ai/store/en/skill_4YMgn1YavPo).
   If you decide to fork Snipek intents, most likely namespace will change
   (e.g. `muki:getTime` to `userX:getTime`). You can update the namespace to
   new one on Snipek app settings page.

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
It just forwards and receives audio samples to/from Snips.

When Snipek is used for [voice commands that let you control your phone](#snipek-built-in-skills),
language matters.
Currently built-in skills and Snipek assistant support only English language.

## Downloads

Binary packages for Sailfish OS can be downloaded from
[OpenRepos](https://openrepos.net/content/mkiol/snipek) and from the official Jolla Store app.

## License

Snipek is developed as an open source project under
[Mozilla Public License Version 2.0](https://www.mozilla.org/MPL/2.0/).

The use of Snips assistant file is governed by
[Snips Terms of Use](https://docs.snips.ai/additional-resources/legal-and-privacy/terms-of-use-highlights).
