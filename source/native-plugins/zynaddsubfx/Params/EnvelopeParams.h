/*
  ZynAddSubFX - a software synthesizer

  EnvelopeParams.h - Parameters for Envelope
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Author: Nasca Octavian Paul

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.
*/

#ifndef ENVELOPE_PARAMS_H
#define ENVELOPE_PARAMS_H

#include "../globals.h"
#include "../Misc/XMLwrapper.h"
#include "Presets.h"

class EnvelopeParams:public Presets
{
    public:
        EnvelopeParams(unsigned char Penvstretch_=64,
                       unsigned char Pforcedrelease_=0,
                       const AbsTime *time_ = nullptr);
        ~EnvelopeParams();
        void paste(const EnvelopeParams &ep);
        void ADSRinit(char A_dt, char D_dt, char S_val, char R_dt);
        void ADSRinit_dB(char A_dt, char D_dt, char S_val, char R_dt);
        void ASRinit(char A_val, char A_dt, char R_val, char R_dt);
        void ADSRinit_filter(char A_val,
                             char A_dt,
                             char D_val,
                             char D_dt,
                             char R_dt,
                             char R_val);
        void ASRinit_bw(char A_val, char A_dt, char R_val, char R_dt);
        void converttofree();

        void add2XML(XMLwrapper& xml);
        void defaults();
        void getfromXML(XMLwrapper& xml);

        float getdt(char i) const;
        static float dt(char val);

        /* MIDI Parameters */
        unsigned char Pfreemode; //1 for free mode, 0 otherwise
        unsigned char Penvpoints;
        unsigned char Penvsustain; //127 for disabled
        unsigned char Penvdt[MAX_ENVELOPE_POINTS];
        unsigned char Penvval[MAX_ENVELOPE_POINTS];
        unsigned char Penvstretch; //64=normal stretch (piano-like), 0=no stretch
        unsigned char Pforcedrelease; //0 - OFF, 1 - ON
        unsigned char Plinearenvelope; //if the amplitude envelope is linear

        unsigned char PA_dt, PD_dt, PR_dt,
                      PA_val, PD_val, PS_val, PR_val;



        int Envmode; // 1 for ADSR parameters (linear amplitude)
                     // 2 for ADSR_dB parameters (dB amplitude)
                     // 3 for ASR parameters (frequency LFO)
                     // 4 for ADSR_filter parameters (filter parameters)
                     // 5 for ASR_bw parameters (bandwidth parameters)

        const AbsTime *time;
        int64_t last_update_timestamp;

        static const rtosc::Ports &ports;
    private:
        void store2defaults();

        /* Default parameters */
        unsigned char Denvstretch;
        unsigned char Dforcedrelease;
        unsigned char Dlinearenvelope;
        unsigned char DA_dt, DD_dt, DR_dt,
                      DA_val, DD_val, DS_val, DR_val;
};

#endif
