#include "looper.h"

#include "../debug/debug.h"

#include <Arduino.h>

namespace midier
{

Looper::Looper(ILayers layers) :
    layers(layers)
{}

char Looper::start(Degree degree)
{
    for (char i = 0; i < layers.count(); ++i)
    {
        auto & layer = layers[i];

        if (layer.tag != -1)
        {
            continue; // this layer is used
        }

        auto start = Time::now;

        if (_started.bar == -1)
        {
            TRACE_1(F("First layer starting"));
            _started = Time::now;
        }
        else if (assist != Assist::No)
        {
            const auto jumps = (unsigned)assist;
            const auto round = Time::Subdivisions / jumps;

            while (((start - _started).subdivisions % round) != 0)
            {
                ++start;
            }
        }

        layer = Layer(i, degree, start);

        if (layers.config != nullptr)
        {
            // all layers share common configuration by default
            layer.config = layers.config;
        }

        if (state == State::Record || state == State::Overlay)
        {
            layer.record();
        }

        // we were in pre-record state and the first layer was just played
        // changing the state to `Record` will cause to actually start recording at
        // the next click of `run()` and will record this newly inserted layer as well
        if (state == State::Prerecord)
        {
            TRACE_1(F("Will record after prerecord"));
            state = State::Record;
        }

        return i;
    }

    TRACE_1(F("There's no place for a new layer"));
    return -1;
}

void Looper::stop(Layer::Tag tag)
{
    Layer * const layer = layers.find(tag);

    if (layer != nullptr)
    {
        if (layer->state == Layer::State::Wait || layer->state == Layer::State::Wander)
        {
            layer->revoke();
        }
        else if (layer->state == Layer::State::Record)
        {
            layer->playback();
        }

        // do nothing if the layer is already in playback mode (let it keep playbacking)
        // this could happen when the button is kept being pressed even after the entire
        // recorded loop is over
    }
}

void Looper::revoke(Layer::Tag tag)
{
    if (state == State::Wander)
    {
        TRACE_1(F("Not revoking any layer as we are wandering"));
        return; // nothing to do when wandering
    }

    if (tag == -1)
    {
        // we want to revoke the last recorded (or being recorded) layer
        // we cannot tell for sure which layer was the last one to be recorded,
        // so we assume it is the layer with the highest tag (and the highest index)

        for (unsigned i = layers.count(); i > 0; --i)
        {
            auto & layer = layers[i - 1];

            if (layer.tag == -1)
            {
                continue;
            }

            if (layer.state == Layer::State::Record || layer.state == Layer::State::Playback)
            {
                layer.revoke();
                break;
            }
        }
    }
    else
    {
        Layer * const layer = layers.find(tag);

        if (layer != nullptr)
        {
            layer->revoke();
        }
    }

    // check if there is no more recorded layers and go back to wandering if so

    if (layers.none([](const Layer & layer) { return layer.state == Layer::State::Record || layer.state == Layer::State::Playback; }))
    {
        TRACE_1(F("Going back to wandering as there are no recorded layers anymore"));
        state = State::Wander;
    }
}

void Looper::record()
{
    if (state == State::Wander)
    {
        // we want to set the state to `Prerecord` if there are no layers at the moment,
        // so we will start recording when the first layer will be played
        // we want to start recording immediately if there's a layer playing at the moment
        state = layers.idle() ? State::Prerecord : State::Record;
    }
    else if (state == State::Prerecord)
    {
        state = State::Wander;
    }
    else if (state == State::Record || state == State::Overlay)
    {
        state = State::Playback;
    }
    else if (state == State::Playback)
    {
        state = State::Overlay;
    }
}

void Looper::wander()
{
    state = State::Wander;
}

Looper::Bar Looper::click()
{
    Bar bar = Bar::Same;

    if (_started.bar != -1) // should we reset 'started'?
    {
        if (_started.subdivision == Time::now.subdivision && layers.idle())
        {
            TRACE_1(F("Reseting start beat as no more layers are being played"));
            _started.bar = -1;
        }
    }

    if (state != _previous) // the state has changed since the last click
    {
        if (state == State::Prerecord)
        {
            TRACE_1(F("Marked for pre-record"));
        }
        else if (state == State::Record)
        {
            TRACE_1(F("Starting to record"));

            _record.when = Time::now;
            _record.bars = 0; // reset the number of bars recorded

            layers.record();
        }
        else if (state == State::Wander)
        {
            TRACE_1(F("Starting to wander"));

            layers.revoke();

            if (_previous != State::Prerecord)
            {
                bar = Bar::None;
            }
        }
        else if (state == State::Playback)
        {
            TRACE_3(F("Starting to playback "), (int)_record.bars, F(" recorded bars"));
        }
        else if (state == State::Overlay)
        {
            TRACE_1(F("Starting to overlay"));

            layers.eval([](Layer & layer)
                {
                    if (layer.state == Layer::State::Wander)
                    {
                        layer.record();
                    }
                });
        }

        _previous = state; // save the state after we finished comparing it
    }

    if (state == State::Record || state == State::Playback || state == State::Overlay)
    {
        const auto difference = Time::now - _record.when;

        if (difference.subdivisions == 0)
        {
            if (state == State::Record)
            {
                if (_record.bars < Time::Bars)
                {
                    ++_record.bars; // we count recorded bars from the time they start
                    TRACE_3(F("Recording bar #"), (int)_record.bars, F(" for the first time"));
                }
                else
                {
                    TRACE_3(F("Recorded maximum of "), (int)_record.bars, F(" bars"));
                    _previous = state = State::Overlay; // setting `_previous` also for it to not count as a state change
                }
            }
            else // playback or overlay
            {
                if (difference.bars == _record.bars) // just passed the # of recorded bars
                {
                    TRACE_5(F("Resetting beat to "), _record.when, F(" after "), (int)difference.bars, F(" bars"));
                    Time::now = _record.when;
                }
            }

            // using `Time::now` and not `difference` to support the cases when we reset the beat
            bar = (Bar)((Time::now - _record.when).bars + 1);
        }
    }

    // let all layers click
    layers.click();

    // after playing all the layers, we advance the global time
    ++Time::now;

    // let the client know if the bar has changed
    return bar;
}

} // midier
