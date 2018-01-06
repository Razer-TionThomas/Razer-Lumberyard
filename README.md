# Chroma GEM for Amazon Lumberyard

## Table of Contents

- Related Tools
- Frameworks supported
- Engine Versions
- Prerequisites
- Chroma Gem Overview 
  - Editor Component
  - Runtime Components
- Chroma Ebus APIs
- Usage Tutorials
- Todo


## Related Tools

[CChromaEditor][Windows ChromaEditorDLL github] - C++ Native MFC Library for playing and editing Chroma animations

[GameMakerChromaExtension][GameMaker github] - GameMaker Extension to control lighting for Razer Chroma

[UE4ChromaSDK][UE4 github] - Unreal Engine 4 (UE4) Blueprint library to control lighting for Razer Chroma

[UnityNativeChromaSDK][UnityNative github] - Unity native library for the ChromaSDK

## Frameworks supported

Windows 10 OS

Windows Chroma SDK (32-bit) 

Windows Chroma SDK (64-bit)

## Engine Versions

The latest Chroma GEM for Amazon Lumberyard was developed and validated on version 1.11 beta ([Latest Download][lmb-download])

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

## Chroma Gem Overview
The Chroma Gem is an extension designed for running in Amazon's Lumberyard Game Engine that will allow developers to integrate Razer Chroma lighting technology into their applications. 
It allows for a user to both author Chroma animations across a variety of Chroma enabled devices directly in the Lumberyard editor and to integrate Chroma animations 
into their games at runtime via Lumberyard's built in scripting engine. The Chroma Gem is fully compatible with the .chroma effect file format used in other Chroma plugin tools such as [Unreal 
Engine 4][UE4 github], [Unity][UnityNative github], [GameMaker][GameMaker github], and native [Windows Chroma Editor][Windows ChromaEditorDLL github] allowing for importing of effects created in 
those tools as well as exporting Lumberyard authored effects into those tools.  

### Chroma Gem Editor Component
The Chroma Gem editor component provides a GUI interface to create new Chroma effects directly in the Lumberyard Editor. The user can preview the effects in the GUI and then export the effect into
a .chroma effect file to be read later during runtime.

For a more detailed overview and visual aids to understanding the Chroma Gem editor UI, please see the tutorial video title "Chroma Gem Setup" within the 9-26 Update folder of the tuturial videos link: 
[Chroma Gem Video Tutorials][chroma-vids] 

#### Steps to enable the Chroma Editor UI in Lumberyard Editor
1. Enable the Chroma Gem in the project configurator for your project (see Prerequisites above)
2. Create an entity in your scene
3. Under the Entity Inspector tab, click on the add component button for that entity
4. Locate the Chroma Gem interface under "Periphiral Lighting"
5. Expand the Chroma Editor UI to see the full menu

#### Chroma Editor Menu Overview
![](Images/ChromaGemUI1.png?raw=true)

The Chroma editor UI has features for creating and modifying chroma effects on all supported Razer Chroma devices. Below is a description of each of the components in the UI.

##### Global Settings
At the top, you'll find several global settings that are needed to define the parameters of the effect. They are as follows:
 -  **Brightness** - set the intensity of the color for the effect
 -  **Speed** - set the delay between each frame of the animation. Note that this setting applies the same delay to every frame
 -  **Color** - define the Chroma color using the Lumberyard Color Picker tool. *By default, the color is blank and thus no effect will show. You must click the space directly to the left 
    of the "R: 0 G: 0 B: 0255" line in the image above to open the color picker*
 - **Repeat** - checkbox to indicate that the effect should loop on playback
 - **Select Device** - Combobox to specify the Chroma device on which to display the effect. Choices are keyboard, mouse, mousepad, headset, keypad, and chromalink
 - **Stop Effect** - button to stop a looping effect
 - **Clear Effects** - button to clear all chroma effects on all connected Chroma devices (i.e. global reset)

##### Preset Effects
In this section of the Chroma Gem Editor UI, the user can select an effect from a dropdown list and play it using the global parameters selected above. The current list of preset effects are:
   - Random Effect - colors all device LED a random color
   - Flash Effect - a single flash/blink of all device LEDs a single color
   - Wave LR Effect - a single colored column moves across the device from left to right
   - Waterfall Effect - a vertical ripple from top to bottom that fades in and out 
   - Cross Wave Effect - 2 colored columns start on opposite ends and cross each other as they move to the opposite side 
   - Breathing Effect - all device LEDs pulse a single color as they fade from max intensity to min intensity (black)
   - Ripple LR Effect - a single colored column moves across the device from left to right
   - Ripple UD Effect - a single colored row moves across the device from top to bottom
   - Explosion Effect - a colored explosion from the center of the device
   - Rainbow Effect - a spectrum cycling of rainbow colors across all device LEDs

***Note that the list of preset effects may change in future updates***

##### Custom Effects
![](Images/ChromaGemUI2.png?raw=true)

In this section of the Chroma Gem Editor UI, the user can specify a wide selection of parameters to create custom Chroma animation, preview them on the Chroma device, and save them into a .chroma 
effect file among other actions. Below is an overview of the key components of the custom animation UI.

- *Current Frame* - a read only field that display the current frame for the animation. Any actions performed will be done on this frame
- *Select Key* - dropdown box to specify a single key on the Chroma enabled keyboard to color based on the global parameters
- *Select Led* - dropdown box to specify a single LED light on non-keyboard Chroma enabled devices
- *Set Key Color* - button to display/preview the color for the selected key on the keyboard
- *Set Led Color* - button to display/preview the color for the selected Led on non-keyboard devices
- *Row* - text field to specify a row number on the keyboard to paint (max row = 6)
- *Column* - text field to specify a column number on the keyboard to paint (max column = 22)
- *Paint Row* - button to display/preview the color for the selected row on the keyboard
- *Paint Column* - button to display/preview the color for the selected column on the keyboard
- *Jump to Frame* - text field to specify a frame within your animation to jump to for faster navigation through animation when editing
- *Next Frame* - button to move to the next frame in the animation (assuming it's been added before). If you are currently at the max frame of the animation, then next frame does not perform any action
- *Prev Frame* - button to move to the previous frame in the animation. If you are currently at the first frame of the animation, then previous frame does not perform any action
- *Copy Frame* - button to copy/save the color data of the current frame
- *Paste Frame* - button to paste/write the color data of the last copied frame to the current frame
- *Fill Frame* - button to paint all LEDs for the selected device a single color based on the global color value. Only fills the current frame
- *Clear Frame* - button to clear the contents of the current frame only. Useful for editing and correcting mistakes on current frame without change other frames of the animation
- *Reset Animation* - button to erase all data from all frames of the animation. Essentially, used start a new animation from scratch
- *Add Frame* - button to add a new frame to the animation (i.e. to increase the frame count of the animation by 1). The new frame is added to the end of the current animation
- *Delete Frame* - button to delete the **current** frame from the animation and reduce the total frame count by 1
- *Import Image* - a button to bring up a file browser to select an image file (.jpg, .png, .bmp) to sample/map onto the Chroma keyboard. Allows for artists to create Chroma animations using prerendered art stored as images
- *Import Animation* - a button to bring up a file browser to select an animation file (.gif only) to sample/map onto the Chroma keyboard. Allows for artists to create Chroma animations using prerendered art stored as gif animations
- *Play Single Device* - button to playback/preview current animation on the **currently selected device only**
- *Play All Devices* - button to playback/preview composite animation **across all devices simultaneously**
- *Import Effect* - button to bring up a file browser to select a chroma effect file (.chroma) to import into the Lumberyard editor for previewing and editing
- *Export Effect* - button to bring up a file browser to save the current animation into a chroma effect file (.chroma)
- *Export Header File* - button to bring up a file browser to save the current animation color data into a C++ header file format for easy integration into C++ applications

***For more details and visual aids on the Chroma Editor UI including preset animations, custom animation, image import, and file import/export, please view the [Chroma Gem Video Tutorials][chroma-vids] outlined in the *Usage* section 
of this document***


### Chroma Gem Runtime Component
The Chroma runtime component is intended to provide the functions to hook in-game events to Chroma effects via Lumberyard's scripting engine. This will allow a developer to trigger 
chroma effects based on certain in game script events for fully dynamic and immersive lighting effects. For example, a developer can trigger a door opening animation once the player opens 
a door or can display a lighting flash animation when the player fires a weapon. The runtime APIs are accessbile via the Ebus mechanism in Lumberyard and can be called via LUA scripts,
C++ code, or Lumberyard's script canvas (visual scripting tool). Note that there is no UI associated with this component.

There are 2 types of effects that can be accessed via the Chroma runtime component. 
1. **Preset Effects** - these animations are pre-coded into the Gem and represent basic effects that can be used for common game screnarios. The advantage of using these effects is that they 
are already created so they are quick to implement and there is no authoring needed by the developer. However, since they are precoded their designs are fixed and modification is limited to 
high level features like color and effect speed only. Note that the full set of Chroma devices are supported by the preset animations.

    The list of preset Chroma animations will continue to expand as the Gem is further developed. The current list of preset effects is below:
   - Random Effect - colors all device LED a random color
   - Flash Effect - a single flash/blink of all device LEDs a single color
   - Wave LR Effect - a single colored column moves across the device from left to right
   - Waterfall Effect - a vertical ripple from top to bottom that fades in and out 
   - Cross Wave Effect - 2 colored columns start on opposite ends and cross each other as they move to the opposite side 
   - Breathing Effect - all device LEDs pulse a single color as they fade from max intensity to min intensity (black)
   - Ripple LR Effect - a single colored column moves across the device from left to right
   - Ripple UD Effect - a single colored row moves across the device from top to bottom
   - Explosion Effect - a colored explosion from the center of the device
   - Rainbow Effect - a spectrum cycling of rainbow colors across all device LEDs

2. **Custom Effects** - these animations are the .chroma effect files that can be read in and played back on Chroma devices at runtime. The effect files are not limited to files exported from the 
Chroma Gem editor component and can be any .chroma effect files generated from any supported tools. The advantage here is that the developer has complete control over the effect design and can 
develop very unqiue effects that suite their title. The disadvantage to this approach is the additional time needed to design and author the Chroma effects before integrating them into the game.

#### Ebus APIs
The Chroma runtime component broadcasts a number of APIs via the Lumberyard Event bus communication mechanism. This allows it to be accessed globally by other Gems or via the Lumberyard scripting engine. 
The current list of implemented APIs is below. **Note that this list will continue to expand in future updates.**

##### ChromaSDKInit
Function to initialize the Chroma SDK. This is necessary in order for any chroma animation to display on chroma devices.

```C++
void ChromaSDKInit() override;
```

##### ChromaSDKUninit
Function to uninitialize the Chroma SDK. This should be used upon exiting the application/game or to disable Chroma effect playback in game.

```C++
void ChromaSDKUnInit() override;
```

##### SetEffectBrightness
Function to set the brightness/intensity of the Chroma color(s) to be rendered. This function should be called before calling any API to play a Chroma animation.
This function takes in a single input arguement:
- brightness - an float to specify the brightness value between 0.0 - 1.0 

```C++
void SetEffectBrightness(float brightness) override;
```

##### SetEffectSpeed
Function to set the delay between each frame of an animation. This function should be called before calling any API to play a Chroma animation. 
This function takes in a single input arguement:
- speed - an int value to specify the frame delay in ms 

```C++
void SetEffectSpeed(int speed) override;
```

##### SetEffectColor
Function to set the RGBA color value. This value will be used for any subsequent Chroma effects that are rendered until the `SetEffectColor` is called again.
This function takes in the following input arguements:
- r - a float value between 0.0 - 1.0 for the Red component of the RGBA color
- g - a float value between 0.0 - 1.0 for the Green component of the RGBA color
- b - a float value between 0.0 - 1.0 for the Blue component of the RGBA color
- a - a float value between 0.0 - 1.0 for the Alpha component of the RGBA color

```C++
void SetEffectColor(float r, float g, float b, float a) override;
```

##### PlayPresetChromaEffect
Function to play a preset (i.e. precoded) chroma animation. This function takes in the following input parameters:
- effectId - an int that specifies an enum for which effect to play. The enums values are:
```C++
		enum ChromaEffectValue
		{
			RANDOM = 0,
			FLASH,
			WAVELR1,
			WATERFALL,
			WAVECROSS,
			BREATHING,
			RIPPLELR,
			RIPPLEUD,
			EXPLOSION,
			RAINBOW
		};
```
- device - specify the enum value for the device to display the chroma effect. The device enum list is:
```C++
		enum ChromDeviceType
		{
			KEYBOARD = 0,
			MOUSE,
			MOUSEPAD,
			HEADSET,
			KEYPAD,
			CHROMALINK
		};
```
- repeat - boolean flag to specify whether to play the chroma animation a single time (false) or on a loop (true)

```C++
void PlayPresetChromaEffect(int effectId, int device, bool repeat ) override;
```


##### StopChromaEffect
Function to stop a looping effect from playing. This must be used to interrupt a looping effect with a new effect.

```C++
void StopChromaEffect() override;
```

##### ClearChromaEffects
Function to clear all chroma effects from all chroma devices.

```C++
void ClearChromaEffects() override;
```

#### Using Chroma Gem Via Script Canvas (as of Lumberyard 1.12) 
Coming Soon


## Usage
See video overview on how to use the Chroma GEM component in the Lumberyard editor: [Chroma Gem Video Tutorials][chroma-vids] 

Tutorial Videos are categoried by major releases as follows:
- [9-16 Update] **Chroma Gem Setup** - tutorial on how to setup the Chroma Gem in the project configurator, enable it, and bring up the Chroma UI in the Lumberyard editor
- [9-16 Update] **Chroma Gem Menu Overview** - tutorial that provides a step by step overview of the components in the Chroma Gem UI
- [9-16 Update] **Chroma Gem Pong Example** - tutorial video that demonstrates a simple pong game animation on the keyboard as an example of how to use the features in the Chroma editor UI

- [9-28 Update] **Chroma Gem Image Import** - tutorial video that demonstrates how to import images (.jpg, .bmp, .png) and animations (.gif) into the Chroma editor UI and render the image as a keyboard effect
- [9-28 Update] **Chroma Gem Custom LED** - tutorial video that explains how to use the custom LED feature to set colors to individual LEDs on non-keyboard devices
- [9-28 Update] **Chroma Gem Play All Devices** - tutorial video that demonstrates how to play a *composite* effect across all devices simultaneously

- [10-20 Update] **Chroma Gem Effect File Import/Export** - tutorial video that demonstrates how save an animation into an effect file (.chroma) and how to import effect files into the Lumberyard Editor

- [1-4-2018 Update] **Chroma Gem Runtime Preset Effect** - example video shows Chroma animation integrated into Scripting sample level using Lua script triggers  

## Todo
- Fix image import feature to use QFileDialog instead of CFileDialog (Completed in 9/28 update)
- Add support for composite effect across all devices (Completed in 9/28 update)
- Add support to save effects into .chroma file format for cross engine support (Completed in 10/20 update)
- Add more support for ebus events for triggering effects (Completed in 1/4/18 Update)
- Create deplyable sample project using scripts to communicate with Chroma GEM (Will add in Lumberyard version 1.11)
- Integrate CChromaEditor native plugin (Completed in 10/20 update)
- Add visual grid overlay for easier custom key/led (Added compatibility with CChromaEditor in 10/20 update)
- Add curve editor to modify effect timings (Added compatibility with CChromaEditor in 10/20 update. May add in Lumberyard natively in future update)

[//]: #
[lmbtut]: https://gamedev.amazon.com/forums/tutorials
[lmb-download]: https://aws.amazon.com/lumberyard/downloads/
[chroma-vids]: https://www.dropbox.com/sh/fpc2cyx43rqkezf/AABejHkC74H0Bk_v1p_cSWFLa?dl=0
[UE4 github]: https://github.com/razerofficial/UE4ChromaSDK
[UnityNative github]: https://github.com/razerofficial/UnityNativeChromaSDK
[GameMaker github]: https://github.com/razerofficial/GameMakerChromaExtension
[Windows ChromaEditorDLL github]: https://github.com/razerofficial/CChromaEditor
