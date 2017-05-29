# Reaper Generic Control
A plugin for [Reaper DAW](http://www.reaper.fm/), to use the [KORG Nanokontrol2](http://www.korg.com/us/products/computergear/nanokontrol2/) MIDI control surface, and could probably support similar surfaces as well.

# Why ?
So, I bought a MIDI control surface, KORG's Nanokontrol2.
![Nanokontrol2](https://github.com/Pierousseau/reaper_generic_control/raw/master/Doc/nanokontrol2.png)
I thought it would be somehow plug and play with my favorite Digital Audio Workstation Software, Reaper, but well... not so much.
I found a few tutorials to make it work, but nothing worked as well as I hoped. 
So I made my own plugin, to have my control surface work jut the way I want it.
I may very well have missed an easy and efficient way to make it work, but it was rather instructive anyway.

# What is it ?
I looked at reaper's SDK, and discovered it was poorly documented and most existing control surface plugins shipped with it had button codes hardcoded all over the place, which is... well, not my favorite kind of code.
So I decided to write my plugin without hardcoding MIDI codes.
To do that, I made the plugin to use a simple json (text) list of the MIDI codes used by the control surface. This package is actually two-fold ; on one side, there is the plugin DLL and its folder of json MIDI codes presets, and on the other side is a super-basic console program that will assist you in generating a json preset.

# Does it work ?
All buttons work, except for "Track previous" / "Track next" (what should I use them for ?). Reaper will send feedback and appropriately turn on/off the lights on your surface. The right-most controls are bound to the master.

# How do I use it ?
1. Modify your surface's behaviour, in **Korg Kontrol Editor**. We want the *Cycle*, *Play*, *Record*, and all the tracks' *S*, *M*, and *R* to be used as toggles. We also allow the leds control by our plugin (set *LED Mode* to *External*). You can do it yourself, or simply open the file I made which is available in the **Dist** folder. Dont forget to do *Communications* -> *Write Scene Data* to apply your changes !
![Korg Kontrol Editor](https://github.com/Pierousseau/reaper_generic_control/raw/master/Doc/kontrol_editor.png)

2. Copy the content of **Dist/Plugins/** to **C:\Program Files\REAPER (x64)\Plugins** (or wherever your Reaper plugins are installed).

3. Launch Reaper. Open its preferences (Ctrl+P), browse down to *Control/OSC/Web*. Click *Add*, select *Generic Surface Controller*. Set your control surface as MIDI input and output. Select the json preset you want from the last list. Click OK.
![Reaper Preferences](https://github.com/Pierousseau/reaper_generic_control/raw/master/Doc/reaper.png)

4. Play Music !

# What about presets ?
Maybe my default preset won't be to your taste. For instance, I reserved the right-most track controls for the master, which may not be what you like. Or, you may usually work half of the time in Cubase or Sonar, and the other half in Reaper, and you don't want to reconfigure your surface all the time. Well, that's what presets are for. Use *Dist/PluginPresetGenerator/control_surface_map_generator.exe* to generate another preset suited to your needs, put the file in *C:\Program Files\REAPER (x64)\Plugins\reaper_plugin_control_surface_generic_presets*, 
select it in reaper preferences, and go back to playing music !
If you make a different preset that you're happy with, please share it back :)

# Could it work with my *whatever* control surface ?
I don't know ! But feel free to try, or to grab the code and modify it to adapt to your gear, and give feedback !

# Licence
This work is covered by the LGPL3 License.
Third parties include the Reaper SDK, and Rapidjson library, which are covered by their own license sets.

Within these licenses bonds, use freely, as long as you acknowledge that should smoke come out of your control surface, or out of your hands after a great jam, I'm not responsible.
