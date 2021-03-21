## STM32F411 "Black Pill" USB WAV Player

<!-- MarkdownTOC -->

* [Project description](#description)
* [How to use](#use)
* [Firmware](#firmware)

<!-- /MarkdownTOC -->

<a id="description"></a>
## Project description<br>

This is just a training project that reads, parses and plays WAV files using PWM.<br>
Uses a STM32F411CE "Black Pill" board running at 96MHz.<br>

Supported WAV formats:
- 8, 16 bit depths (16bit is converted to 8bit).<br>
- Stereo / mono (In mono mode, right channel is copied to the left channel).<br>
- 8, 16, 22.05, 32, 44.1, 48, 96KHz sample rates.<br>

22.05 and 44.1KHz have 0.04% error (22059Hz, 44117Hz), it plays only 1.5 seconds faster every hour.<br>
Other WAV formats will be skipped.<br>

In 8/16/32/48/96KHz PWM frequency is 384KHz, max PWM value 249, audio values over that will cause minor clipping.<br>
I used Audacity and normalized the level to -0.1dB, that limited the output within the limits.<br>

In 22.05/44.1KHz PWM frequency is 352.941KHz, max PWM value 271, so there's no clipping problem.<br>


The PWM uses DMA burst mode, a very efficient and clean method to update multiple PWM channels at once.<br>

PWM outputs are as follows:<br>
- Right channel: PA8<br>
- Left channel: PA9<br>


For filtering the PWM, I used this calculator: http://sim.okawa-denshi.jp/en/Sallen3tool.php<br>
The parameters were Chebyshev filtering, fc=22KHz.<br>
The result was really good for coming from 8-bit PWM.<br>

The SWO output is enabled on PB3, you can see the debug messages using SWO console.<br>
You can see them without debugging, just connect the ST-Link utility, open "Prinf via SWO viewer", setting clock to 96000000Hz.br>


Example log:<br>

      Device connected
      Device ready
      Drive mounted
      Opening file: 1-STER~1.WAV
      WAV Info: Stereo, 48KHz, 8bit
      Playback started
      Playback stopped

      Opening file: 7.WAV
      WAV Info: Stereo, 44.1KHz, 16bit
      Playback started
      Playback stopped

      Opening file: 1.WAV
      WAV Info: Stereo, 48KHz, 16bit
      Playback started
      Playback stopped

      Opening file: 6.WAV
      WAV Info: Stereo, 96KHz, 16bit
      Playback started
      Playback stopped

For more details, check:<br>

      /Inc/files.h
      /Inc/pwmAudio.h
      /Src/files.c
      /Src/pwmAudio.c
  
<a id="use"></a>
## How to use

Connect a USB drive with wav files in the root folder (/), it will automatically start playing all files found in a loop .<br>

<a id="firmware"></a>
## Firmware

You can download already compiled binaries in the Release folder (411.bin file)<br>
If you want to compile your own:<br>
- Download STM32 Cube IDE<br>
- Clone or download the code<br>
- Open STM32 Cube IDE, import existing project and select the folder where the code is.<br>
- It should recognize it and be ready for compiling or modifying for your own needs.<br>  

 The code is stripped from all the CubeMX libraries, it needs to regenerate them.<br>
 Open the CubeMX (.IOC) file and make any small change (Ex. set an unused GPIO as input), then and revert it and close saving changes.<br>
 CubeMx will make new code, and now it will compile correctly.<br>
