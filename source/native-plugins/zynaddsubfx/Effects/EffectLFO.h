/*
  ZynAddSubFX - a software synthesizer

  EffectLFO.h - Stereo LFO used by some effects
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Author: Nasca Octavian Paul

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.
*/

#ifndef EFFECT_LFO_H
#define EFFECT_LFO_H

/**LFO for some of the Effect objects
 * \todo see if this should inherit LFO*/
class EffectLFO
{
    public:
        EffectLFO(float srate_f, float bufsize_f);
        ~EffectLFO();
        void effectlfoout(float *outl, float *outr);
        void updateparams(void);
        unsigned char Pfreq;
        unsigned char Prandomness;
        unsigned char PLFOtype;
        unsigned char Pstereo; // 64 is centered
    private:
        float getlfoshape(float x);

        float xl, xr;
        float incx;
        float ampl1, ampl2, ampr1, ampr2; //necessary for "randomness"
        float lfornd;
        char  lfotype;

        // current setup
        float samplerate_f;
        float buffersize_f;
};

#endif
