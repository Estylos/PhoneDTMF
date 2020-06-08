# DTMF Decoder

A simple AVR project which decodes DTFM tones from an audio signal.

## Context

Having an old analog telephone approved by France Telecom from the 90s which was no longer useful to me, I wanted to reuse it to get the DTMF signals generated at the press of the keys, and by software, being able to display the corresponding key by serial communication. The project is not perfect but it has the merit of working without too many major issues.

## Hardware

I use for this project an ATmega168A (internally clocked at 8 MHz), an FTDI, and an old landline phone. I power the telephone in 12V DC to be able to make it work and I get the audio signal as [indicated here](http://pafgadget.free.fr/bidouillages/telephone-intercom.htm). Finally, I use a voltage divider to lower the 12v to 5v before entering in the ADC.

## Software

The code is already commented to facilitate understanding. The heart of the code which is the detection of DTMF signals is ensured by the Goertzel algorithm. 
For more information on the algorithm and its application check :
- https://en.wikipedia.org/wiki/Goertzel_algorithm
- https://www.embedded.com/the-goertzel-algorithm/