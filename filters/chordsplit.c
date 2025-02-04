MFD_FILTER(chordsplit)

#ifdef MX_TTF

    mflt:chordsplit
    TTF_DEFAULTDEF("MIDI Chord Split", "MIDI Chord Split")
    , TTF_IPORT(0, "channelf", "Filter Channel", 1, 16, 1, PORTENUM16
            rdfs:comment "MIDI channel that is split. All others are blocked.")
    ; rdfs:comment "MIDI Chord Split. Split incoming channel to channel 1-16 with a single note per channel." ;
    .

#elif defined MX_CODE

void filter_init_chordsplit(MidiFilter *self) {
    for (uint8_t c=0; c<16; c++) {
        self->memI[c] = -1;
    }
}

void filter_midi_chordsplit(MidiFilter *self,
                            uint32_t tme,
                            const uint8_t *const buffer,
                            uint32_t size) {
    if (size != 3) {
        forge_midimessage(self, tme, buffer, size);
        return;
    }

    const uint8_t chf = rintf(*self->cfg[0]);
    uint8_t chn = buffer[0] & 0x0f;
    uint8_t mst = buffer[0] & 0xf0;

    if (chf != chn || (mst != MIDI_NOTEON && mst != MIDI_NOTEOFF)) {
        //forge_midimessage(self, tme, buffer, size);
        return;
    }

    const uint8_t key = buffer[1] & 0x7f;
    const uint8_t vel = buffer[2] & 0x7f;

    if (vel == 0) {
        mst = MIDI_NOTEOFF;
    }

    int c;

    if (mst == MIDI_NOTEON) {
        for (c=0; c<16; c++) {
            if (self->memI[c] == -1) {
                self->memI[c] = key;
                chn = c;
                break;
            }
        }
        if (c == 16) {  // no empty channel found, drop note
            return;
        }
    } else {        // MIDI_NOTEOFF
        for (c=0; c<16; c++) {
            if (self->memI[c] == key) {
                self->memI[c] = -1;
                chn = c;
                break;
            }
        }
        if (c == 16) {  // note-off for unknown/dropped key
            return;
        }
    }

    uint8_t msg[3] = { mst | chn, key, vel };
    forge_midimessage(self, tme, msg, 3);
}

#endif
