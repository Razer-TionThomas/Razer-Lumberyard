# Chroma GEM for Amazon Lumberyard (WIP)

## Table of Contents

- Related
- Frameworks supported
- Prerequisites
- Engine Versions
- Usage
.
.
.

## Related

CChromaEditor - C++ Native MFC Library for playing and editing Chroma animations

GameMakerChromaExtension - GameMaker Extension to control lighting for Razer Chroma

UE4ChromaSDK - Unreal Engine 4 (UE4) Blueprint library to control lighting for Razer Chroma

UnityNativeChromaSDK - Unity native library for the ChromaSDK

UnityChromaSDK - Unity C# library for the Chroma Rest API


## Frameworks supported

Windows Chroma SDK (32-bit) 

Windows Chroma SDK (64-bit)

## Engine Versions

The Chroma GEM for Amazon Lumberyard was developed and validated on version 1.9 beta. 

## Prerequisites
1 [Download latest Lumberyard Engine][lmb-download] 

2 Follow Amazon's tutorial on how to install, configure your system, and run Lumberyard ([Amazon's Lumberyard tutorial page][lmbtut])

3 Download the Chroma GEM folder and copy it to the Lumberyard Install Path\dev\Gems folder. For example, assuming the default install path for installing Lumberyard 1.9.0.1, you will copy the Chroma Gem folder to the following path:
```sh
C:\Amazon\Lumberyard\1.9.0.1\dev\Gems
```
4 Open the Projector Configurator and select *Enable Gems* under the sample project
![](Images/ProjectConfig.png?raw=true)

5 Verify that the Chroma GEM is visible and enabled (via check mark) in the Gems Extensions menu
![](Images/ProjectConfigChromaGem.png?raw=true)


## Usage
See video overview on how to use the Chroma GEM component in the Lumberyard editor: [Chroma Gem Video Tutorials][chroma-vids] 

Tutorial Videos are categoried by major releases as follows:
[9-16 Update] **Chroma Gem Setup** - tutorial on how to setup the Chroma Gem in the project configurator, enable it, and bring up the Chroma UI in the Lumberyard editor
[9-16 Update] **Chroma Gem Menu Overview** - tutorial that provides a step by step overview of the components in the Chroma Gem UI
[9-16 Update] **Chroma Gem Pong Example** - tutorial video that demonstrates a simple pong game animation on the keyboard as an example of how to use the features in the Chroma editor UI

[9-28 Update] **Chroma Gem Image Import** - tutorial video that demonstrates how to import images (.jpg, .bmp, .png) and animations (.gif) into the Chroma editor UI and render the image as a keyboard effect
[9-28 Update] **Chroma Gem Custom LED** - tutorial video that explains how to use the custom LED feature to set colors to individual LEDs on non-keyboard devices
[9-28 Update] **Chroma Gem Play All Devices** - tutorial video that demonstrates how to play a *composite* effect across all devices simultaneously

[10-20 Update] **Chroma Gem Effect File Import/Export** - tutorial video that demonstrates how save an animation into an effect file (.chroma) and how to import effect files into the Lumberyard Editor


## Todo
- Fix image import feature to use QFileDialog instead of CFileDialog (Completed in 9/28 update)
- Add support for composite effect across all devices (Completed in 9/28 update)
- Add support to save effects into .chroma file format for cross engine support (Completed in 10/20 update)
- Add more support for ebus events for triggering effects (Will add support for Lumberyard version 1.11 scripting)
- Create deplyable sample project using scripts to communicate with Chroma GEM (Will add in Lumberyard version 1.11)
- Integrate CChromaEditor native plugin (Completed in 10/20 update)
- Add visual grid overlay for easier custom key/led (Added compatibility with CChromaEditor in 10/20 update)
- Add curve editor to modify effect timings (Added compatibility with CChromaEditor in 10/20 update. May add in Lumberyard natively in future update)

[//]: #
[lmbtut]: https://gamedev.amazon.com/forums/tutorials
[lmb-download]: https://aws.amazon.com/lumberyard/downloads/
[chroma-vids]: https://www.dropbox.com/sh/fpc2cyx43rqkezf/AABejHkC74H0Bk_v1p_cSWFLa?dl=0
