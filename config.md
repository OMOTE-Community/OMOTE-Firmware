- [Config Module](#org42e5bb4)
  - [Usage](#org4a6f522)
  - [Functionality](#orgbd09c88)
    - [Devices](#org68bb64e)
    - [Scenes](#orge0d9e30)
  - [Schema](#org086f29e)
    - [Keys](#org02a5fe7)
  - [Example](#org2d8e11c)


<a id="org42e5bb4"></a>

# Config Module

The *Config Module* for OMOTE introduces the option to define the remote's functionality such as the controlled [Devices](#org68bb64e) and [Scenes](#orge0d9e30) in a declarative way (using a YAML configuration file) instead of the standard imperative approach based on defining the respective configurations by implementing appropriate functions and data structures as code.

Please note that while, in principle, the [Devices](#org68bb64e) and [Scenes](#orge0d9e30) defined declaratively can live side-by-side with code driven definitions, it is not advised as certain functionality (such as [Device List](#org773dcc4) or [Automated Power on/off](#orgea7f413)) will not be available for the imperative definitions.


<a id="org4a6f522"></a>

## Usage

To enable the *Config Module* one needs to define the YAML\_CONFIG build flag in the `platformio.ini` file. Defining that flag has the following effects:

1.  The *Config Module* dependencies (such as the ArduinoJson library) will be **conditionally** enabled and linked.
2.  The main YAML configuration file (defined according to the [Schema](#org086f29e) in a file named `config.yml` - see [Example](#org2d8e11c) ) located in the main OMOTE project directory, will be translated to a header file and compiled into the binary.

The translation and conditional dependencies logic is located in a script file under `tools/conditional_config.py`.


<a id="orgbd09c88"></a>

## Functionality


<a id="org68bb64e"></a>

### Devices

Devices defined via the configuration file (see [Schema](#org086f29e)) are equivalent to the code driven definitions that live in `src/devices`. In addition to the standard set of attributes and customization options inherited from the imperative device definitions, the configuration driven device definitions also allow to define a standard key-map for the device (see `map_short` and `map_long` attributes in the [Schema](#org086f29e)) that can be used as a convenient baseline for a scene key-map (see `scene` / `keys_default` attribute) and will be used in the [Device List](#org773dcc4).

Each command defined within a device configuration, may also include a new `category` attribute (see [Schema](#org086f29e)) that is used to enable the [Automated Power on/off](#orgea7f413) capability.

1.  Device List

    The *Device List* is a new top-level GUI screen which aggregates all devices defined in the configuration file along with their commands. This gives the capability (that would be immediately familiar to any Logitech Harmony user) to invoke any arbitrary command, from any configured device, even if it's not mapped to a physical key.
    
    The GUI presents as a list of devices which expand to the list of their respective commands once a device is "clicked". As long as a device is expanded, the physical key-map will be replaced with the key mappings from that device definition (if available - see `map_short` and `map_long` from [Schema](#org086f29e)). The key-map will be restored to default once a device command list is collapsed


<a id="orge0d9e30"></a>

### Scenes

Scenes, similarly to [Devices](#org68bb64e), are a direct analog to the standard OMOTE's imperatively constructed "scene" entity. The config file [Schema](#org086f29e) allows to define all the standard properties (including "start" and "end" sequences) available in a classic, code driven, scene definition. On top of that the following additional capabilities are available:

1.  Scene GUI

    Each scene gets its own standardized GUI that gives easy access to the scene's "shortcuts" (see below), ability to quickly swipe do the list of devices (see [Device List](#org773dcc4))as well as a quick way to turn all devices involved in the scene off.

2.  Shortcuts

    The configuration file [Schema](#org086f29e) enables to define a list of "shortcuts" for each scene that include semi-frequently accessed commands that for one reason or another are not mapped to physical buttons but should be available for easy access. The list of "shortcuts" is presented on the [Scene GUI](#org543767e).

3.  Automated Power on/off

    The *Config Module* infrastructure tracks the devices that are being turned on (via the `category` attribute attached to commands) in the configured scenes' startup sequences and, as the user switches from one scene to another, makes sure that:
    
    1.  Only the devices required in particular scene remain turned on after switching from a previous scene.
    2.  Only the devices that haven't been already turned on in the previous scene receive a power on or power toggle command.
    3.  Only the devices that are actually on (that is used in the current scene) are switched off when requesting the "all off" function.
    
    This is both handy and necessary in case of devices that are turned on/off via a toggle command, as without that a device would be inadvertently turned off if a new scene was requested that includes powering on a device that have already been turned on in the previous scene.


<a id="org086f29e"></a>

## Schema

| Entity        | Attribute      | Type                                           | Description                                                                                                                                                                                                                                                                                               |
|------------- |-------------- |---------------------------------------------- |--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `devices`     |                | dictionary: string -> `device`                 | A dictionary mapping a string representing the unique identifier for a device to a corresponding `device` object                                                                                                                                                                                          |
| `device`      | `display_name` | string                                         | The display name for this device in UI                                                                                                                                                                                                                                                                    |
|               | `protocol`     | enum(string)                                   | The default protocol used in commands communicating with the device. Currently list: SIRC, NEC, RCS, DENON, KASEIKYO (easily expandable in config/parser.cpp)                                                                                                                                             |
|               | `commands`     | list: `command`                                | The list of `command` objects defined for the device                                                                                                                                                                                                                                                      |
| `command`     | `name`         | string                                         | The name of the command. Will be used both for UI and internal accounting.                                                                                                                                                                                                                                |
|               | `data`         | hex number                                     | The data to be sent for the command based on the supported protocol.                                                                                                                                                                                                                                      |
|               | `protocol`     | (optional) enum(string)                        | Overwrites the `device` / `protocol` if specified                                                                                                                                                                                                                                                         |
|               | `nbits`        | (optional) integer                             | Number of bits used for the command. Not required if constant for a protocol with no variants                                                                                                                                                                                                             |
|               | `repeats`      | integer                                        | Number of repeats to send with the command.                                                                                                                                                                                                                                                               |
|               | `category`     | (optional) string                              | A comma delimited string representing the set of standard functional categories that the command belongs to. Two categories are currently supported: *on* and *off*. For example a "power on" command would support the *on* category, while "power toggle" would be categorized as *on,off*              |
|               | `map_short`    | (optional) enum(string)                        | The default key associated with the command on the remote for a "short" press. See [Keys](#org02a5fe7) for the list of supported key designations. This will be used when the device is accessed via the "Device List GUI" or if a scene uses the `keys_default` attribute pointing to the parent device. |
|               | `map_long`     | (optional) enum(string)                        | The default key associated with the command on the remote for a "long" press. See [Keys](#org02a5fe7) for the list of supported key designations. This will be used when the device is accessed via the "Device List GUI" or if a scene uses the `keys_default` attribute pointing to the parent device.  |
| `scenes`      |                | dictionary: string -> `scene`                  | A dictionary mapping a string representing the unique identifier for a scene to a corresponding `scene` object                                                                                                                                                                                            |
| `scene`       | `display_name` | string                                         | The name used for this scene in UI                                                                                                                                                                                                                                                                        |
|               | `keys_default` | (optional) string                              | The unique identifier of a `device` section to be used as source for default physical key mapping in the scene (as retrieved from `command` attributes)                                                                                                                                                   |
|               | `keys_short`   | (optional) dictionary: string -> `command_ref` | A dictionary mapping physical key designators (see [Keys](#org02a5fe7)) to corresponding  commands (via `command_ref` structures) that should be triggered on short press while the scene is active. Any key mappings define here overwrite the default definition inherited via `keys_default`.          |
|               | `keys_long`    | (optional) dictionary: string -> `command_ref` | A dictionary mapping physical key designators (see [Keys](#org02a5fe7)) to corresponding  commands (via `command_ref` structures) that should be triggered on long press while the scene is active. Any key mappings define here overwrite the default definition inherited via `keys_default`.           |
|               | `start`        | list: `command_ref` &vert; `delay`             | A sequence of device commands (represented via `command_ref` structures) or delays (represented via `delay` structure) to be executed when the scene starts.                                                                                                                                              |
|               | `end`          | list: `command_ref` &vert; `delay`             | A sequence of device commands (represented via `command_ref` structures) or delays (represented via `delay` structure) to be executed when the scene ends.                                                                                                                                                |
|               | `shortcuts`    | list: `command_ref`                            | A list of commands (represented by `command_ref` structures) to be presented on this scene's GUI as buttons.                                                                                                                                                                                              |
| `command_ref` | `device`       | string                                         | The unique identifier of the device (see `devices`) that this reference points to.                                                                                                                                                                                                                        |
|               | `command`      | string                                         | The name of the `command` with the device's `commands` list that this reference points to. This attributre is only allowed if `device` attribute is present.                                                                                                                                              |
|               | `scene`        | string                                         | The `display_name` valye of the `scene` to be activated. This attribute is only allowed if neither `device` nor `command` is present.                                                                                                                                                                     |
| `delay`       |                | integer                                        | The number of milliseconds to delay before executing the next step of a sequence.                                                                                                                                                                                                                         |


<a id="org02a5fe7"></a>

### Keys

The designators for physical keys used in the configuration YAML scheme. Abbreviations are used to limit the amount of typing required when authoring a configuration.

-   OFF
-   STOP
-   REWI
-   PLAY
-   FORW
-   CONF
-   INFO
-   UP
-   DOWN
-   LEFT
-   RIGHT
-   OK
-   BACK
-   SRC
-   VOLUP
-   VOLDO
-   MUTE
-   REC
-   CHUP
-   CHDOW
-   RED
-   GREEN
-   YELLO
-   BLUE


<a id="org2d8e11c"></a>

## Example

```yaml
scenes:
  watch_tv:
    display_name: Watch TV
    keys_default: sony_bravia
    shortcuts:
      - device: sony_bravia
        command: Netflix
      - device: sony_bravia
        command: Swap

    keys_short:
      UP:
        device: sony_bravia
        command: Down
      DOWN:
        device: sony_bravia
        command: Up
    start:
      - device: sony_bravia
        command: Power

  watch_tv_receiver:
    display_name: Watch TV Receiver
    keys_default: sony_bravia    
    keys_short:
      VOLUP:
        device: denon_x4700h
        command: Volume Up
      VOLDO:
        device: denon_x4700h
        command: Volume Down
      MUTE:
        device: denon_x4700h
        command: Mute

    start:
      - device: sony_bravia
        command: Power        
      - delay: 500
      - device: denon_x4700h
        command: Power Toggle
      - device: denon_x4700h
        command: Input TV


devices:
  denon_x4700h:
    display_name: Denon Receiver
    protocol: DENON
    commands:
    - name: Power Toggle
      data: "0x2A4C028A0088"
      nbits: 48
      repeats: 0
      category: on,off
      map_short: OFF
    - name: Volume Up
      data: "0x2A4C0280E86A"
      nbits: 48
      repeats: 0
      map_short: VOLUP
    - name: Volume Down
      data: "0x2A4C0288E862"
      nbits: 48
      repeats: 0
      map_short: VOLDO
    - name: Input TV
      data: "0x2A4C0284B432"
      nbits: 48
      repeats: 0

  sony_bravia:
    display_name: Sony Bravia
    protocol: SIRC    
    commands:
    - name: Power
      data: '0xA90'
      nbits: 12
      map_short: OFF
      category: on,off
    - name: Input
      data: '0xA50'
      nbits: 12
      map_short: SRC
    - name: Home
      data: '0x070'
      nbits: 12
      map_short: INFO
    - name: Apps
      data: '0x2A23'
      nbits: 15
    - name: Tv
      data: '0x250'
      nbits: 12
    - name: Left
      data: '0x2D0'
      nbits: 12
      map_short: LEFT
    - name: Right
      data: '0xCD0'
      nbits: 12
      map_short: RIGHT
    - name: Up
      data: '0x2F0'
      nbits: 12
      map_short: UP
    - name: Down
      data: '0xAF0'
      nbits: 12
      map_short: DOWN
    - name: Enter
      data: '0xA70'
      nbits: 12
      map_short: OK
    - name: Back
      data: '0x62E9'
      nbits: 15
      map_short: BACK
    - name: Mute
      data: '0x290'
      nbits: 12
      map_short: MUTE
    - name: Vol Up
      data: '0x490'
      nbits: 12
      map_short: VOLUP
    - name: Vol Down
      data: '0xC90'
      nbits: 12
      map_short: VOLDO
    - name: Netflix
      data: '0x1F58'
      nbits: 15
    - name: Swap
      data: '0xDD0'
      nbits: 12

```
