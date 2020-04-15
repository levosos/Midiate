#pragma once

#include "../layers/layers.h"

namespace midier
{

struct Looper
{
    enum class State : char
    {
        Wander,
        Prerecord,
        Record,
        Playback,
        Overlay,
    };

    enum class Assist : char
    {
        No,

        // numbers represeting the # of subdivisions to round
        _12 = 12,
        _8 = 8,
        _6 = 6,
        _4 = 4,
        _3 = 3,
    };

    enum class Bar : char
    {
        None = -1,
        Same = 0,

        // bar index
    };

    // creation
    Looper(ILayers layers);

    // start and stop layers
    Layer::Tag start(Degree degree); // return corresponding tag or (-1) if could not play
    void stop(Layer::Tag tag);

    // revoke a recorded layer
    // passing (-1) as `tag` means to revoke the last recorded layer
    // doing nothing if wandering
    void revoke(Layer::Tag tag = -1);

    // state changes
    void record(); // toggle between record/playback/overlay modes
    void wander(); // go back to wandering

    // runs all the logic of the looper layers once
    Bar click();

    State state = State::Wander;
    Assist assist = Assist::No;
    ILayers layers;

private:
    struct {
        Time when; // when we started to record
        char bars; // # of recorded bars
    } _record;

    Time _started; // first note ever played
    State _previous = state;
};

} // midier
