# electrosew

Because all clothes should light up.

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