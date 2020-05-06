//
//  PitchDetectorDefs.h
//  PitchDetector
//
//  Created by Nobuhiro Kuroiwa on 2015/04/09.
//  Copyright (c) 2015年 Nobuhiro Kuroiwa. All rights reserved.
//

#ifndef PitchDetector_PitchDetectorDefs_h
#define PitchDetector_PitchDetectorDefs_h

#include <cstdint>
#include <cmath>

typedef struct PitchInfo_ {
    bool         error;
    float        freq;
    unsigned int midi;
    const char*  noteStr;
    uint8_t      octave;
} PitchInfo;

static const float kNoteConst = log10(pow(2.0f, 1.0f / 12.0f));

static const char* kNoteStrings[] {
    "A", "B♭", "B", "C",
    "C#", "D", "D#", "E",
    "F", "F#", "G", "G#",
};

#endif
