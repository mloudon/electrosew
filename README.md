# electrosew

Because all clothes should light up.

## On `Scarf.ino`

Trying to put some framework together. A couple of things:

### Framerates

We try to decouple animation speed from processing time here. The idea is:

* Each animation loop starts by taking a reading of current program time with `millis()`
* Uses the `getClock` function to get a number between 0-255 for various cycling purposes,
with a specified cycle length.
* Use these variables to render the strip, in a stateless fashion. (i.e. current pattern 
is only a function of time.
* Draw to pixel.
* `delay(20)`, but mainly to save processor cycle/power (is that true?). This also serves
to give a 50fps limit. But the parameter really doesn't affect _what_ is shown, just how
smoothly it goes.

### getClock

Lot of animations is driving by some cycling parameter. This allows us to generate one 
easily with a specified frequency. Current options are powers of 2 away from ~1hz (more like
0.97hz, but whateves.) This is mainly so we can use a bitwise shift to calculate it.

Outputs a byte that cycles 0-255, then back to 0 again.

### Ramping up brightness

Generally, it seems linear works well. Try that before anything more fancy.

## Notes:

The 5v, 16Mhz Trinket needs some extra code in `setup()` to work:

      #if defined (__AVR_ATtiny85__)
        if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
      #endif

On Gamma curves

`Adafruit_NeoPixel` seems to do Gamma correction (it does some weird stuff where it's
actually 16bit on an underlying layer... but ja.

So, `strip.Color(127,127,127)` should be perceived 50% brightness, or <25% current.

Not sure what `strip.setBrightness(127)` does, on the other hand.