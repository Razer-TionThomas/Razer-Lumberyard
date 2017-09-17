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

2 Follow Amazon's tutorial on how to configure your system to run Lumberyard (see link below)

3 Download the Chroma GEM folder and copy it to the Lumberyard Install Path\dev\Gems folder. For example, assuming the default install path for installing Lumberyard 1.9.0.1, you will copy the Chroma Gem folder to the following path:
```sh
C:\Amazon\Lumberyard\1.9.0.1\dev\Gems
```
4 Open the Projector Configurator and select *Enable Gems* under the sample project
![](Images/ProjectConfig.png?raw=true)

5 Verify that the Chroma GEM is visible and enabled (via check mark) in the Gems Extensions menu
![](Images/ProjectConfigChromaGem.png?raw=true)

For information on how to get started in Lumberyard, including instructions on installation, setting up your development environment, and navigating the UI please see [Amazon's Lumberyard tutorial page][lmbtut]



## Usage
See video overview on how to use the Chroma GEM component in the Lumberyard editor: [Chroma Gem Video Tutorial][ChromaGemTutorialVids] 


## Todo
- Fix image import feature to use QFileDialog instead of CFileDialog
- Add support for composite effect across all devices
- Add more support for ebus events for triggering effects
- Create deplyable sample project using scripts to communicate with Chroma GEM
- Integrate CChromaEditor native plugin
- Add visual grid overlay for easier custom key/led (May just use native plugin integration)
- Add curve editor to modify effect timings (May just use native plugin integration)

[//]: #
[lmbtut]: https://gamedev.amazon.com/forums/tutorials
[lmb-download]: https://aws.amazon.com/lumberyard/downloads/
[ChromaGemTutorialVids]: https://www.dropbox.com/sh/fpc2cyx43rqkezf/AABejHkC74H0Bk_v1p_cSWFLa?dl=0
