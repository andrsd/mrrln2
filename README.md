mrrsln2 - Two Aspect Model Railroad Signaling for LocoNet
=========================================================

This implements a simple 2-aspect signaling for small model railroad layouts wired with Digitrax LocoNet.
To use this code, you will need:

* Arduino Nano (or its clone)
* Digitrax PR3XTRA
* SE8c configured for signaling and all signals physically connected
* USB cable (type-B male to mini-B male) to connect Arduino Nano to PR3XTRA


Before you use the code, you will want to set up your signals. The signal heads guarding a block are stored
in `signals` variable. The first index is the block ID and then the two numbers correspond to the signal head ID
(-1 means no signal head).

For example:
```
const int signals[NUM_OF_BLOCKS][SIGNALS_PER_BLOCK] = {
  { 3, -1 },
  { 4, 16},
  ...
```
means that block 1 if guarded by signal head 3, block 1 by signal heads 4 and 16, etc.

You will want to change `NUM_OF_BLOCKS` to match the highest block ID you have on your layout guarded by signals. The
unused blocks will have `{ -1, -1}` entry. The `signals` array must be continuous.


To program the Arduino Nano:

* Open Arduino IDE
* Connect the board via USB cable
* Compile and upload the code on the board
* Connect the board to PR3XTRA


Note on the USB cable
---------------------

You will need to connect the Arduino board to your computer to program it. For that, you will need mini-B male to type-A male USB.
Then to connect to PR3XTRA you will need type-A female to type-B male adapter.
