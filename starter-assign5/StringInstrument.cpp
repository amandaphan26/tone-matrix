/*
 * File: StringInstrument.cpp
 * Author: Amanda Phan (amandapt@stanford.edu)
 * Course/Section: CS 106B, Thorton 210
 * This file performs a simplified physical simulation of a plucked string instrument.
 */


#include "Demos/AudioSystem.h"
#include "StringInstrument.h"
#include "error.h"
using namespace std;

const double AVERAGE = 0.995;
const double STANDARD = 0.05;


/* The StringInstrument constructor function takes in a frequency and initializes
 * the _waveform array.  */
StringInstrument::StringInstrument(double frequency) {
    if (frequency <= 0 || frequency >= AudioSystem::sampleRate()){
        error("Frequency must be positive or non-zero.");
    }

    _length = AudioSystem::sampleRate() / frequency;
    _cursor = 0;
    _waveform = new Sample[_length];


    for (int i = 0; i < _length; i++){
        _waveform[i] = 0;
    }
}


/* The StringInstrument destructor function deletes the _waveform array
 * and releases the dynamically allocated memory back to the system. */
StringInstrument::~StringInstrument() {
    delete[] _waveform;
}


/* The pluck() function fills the first half of the array to +0.05 and the second half to -0.05.
 * If the array length is odd, the middle element is +0.05. */
void StringInstrument::pluck() {
    // Set the first half of the array to +0.05
    for (int i = 0; i < _length / 2; i++){
        _waveform[i] = STANDARD;
    }

    // Set the second half of the array to -0.05
    for (int i = _length / 2; i < _length; i++){
        _waveform[i] = -STANDARD;
    }
    _cursor = 0;

}


/* The nextSample function returns the next sound sample and updates the _waveform buffer and cursor position. */
Sample StringInstrument::nextSample() {
    Sample thisOne = _waveform[_cursor];

    Sample average = AVERAGE * ((_waveform[_cursor] + _waveform[(_cursor + 1) % _length]) / 2);
    _waveform[_cursor] = average;
    _cursor = (_cursor + 1) % _length;
    return thisOne;

}

/* * * * * Test Cases Below This Point * * * * */

#include "GUI/SimpleTest.h"


STUDENT_TEST("Milestone 2: Waveform array initialized correctly.") {
    /* Change the sample rate to 3, just to make the numbers come out nice. */
    AudioSystem::setSampleRate(10);

    /* Create a string that vibrates at 1hz. This is well below the human hearing
     * threshold and exists purely for testing purposes.
     */
    StringInstrument instrument(1);

    /* Make sure something was allocated. */
    EXPECT_NOT_EQUAL(instrument._waveform, nullptr);

    /* Length should be 10 / 1 = 10. */
    EXPECT_EQUAL(instrument._length, 10);

    /* All entries should be zero. */
    EXPECT_EQUAL(instrument._waveform[0], 0);
    EXPECT_EQUAL(instrument._waveform[1], 0);
    EXPECT_EQUAL(instrument._waveform[2], 0);
    EXPECT_EQUAL(instrument._waveform[3], 0);
    EXPECT_EQUAL(instrument._waveform[4], 0);
    EXPECT_EQUAL(instrument._waveform[5], 0);
    EXPECT_EQUAL(instrument._waveform[6], 0);
    EXPECT_EQUAL(instrument._waveform[7], 0);
    EXPECT_EQUAL(instrument._waveform[8], 0);
    EXPECT_EQUAL(instrument._waveform[9], 0);
}

STUDENT_TEST("Milestone 2: Constructor sets cursor to position 0.") {
    AudioSystem::setSampleRate(30);

    StringInstrument instrument(1);
    EXPECT_NOT_EQUAL(instrument._waveform, nullptr);

    EXPECT_EQUAL(instrument._cursor, 0);
}

STUDENT_TEST("Milestone 3: pluck sets values to +0.05 and -0.05.") {
    AudioSystem::setSampleRate(5);

    /* 5 samples per sec / 1Hz = 5 samples. */
    StringInstrument instrument(1);
    EXPECT_NOT_EQUAL(instrument._waveform, nullptr);
    EXPECT_EQUAL(instrument._length, 5);

    instrument.pluck();
    EXPECT_EQUAL(instrument._waveform[0], +0.05);
    EXPECT_EQUAL(instrument._waveform[1], +0.05);
    EXPECT_EQUAL(instrument._waveform[2], -0.05);
    EXPECT_EQUAL(instrument._waveform[3], -0.05);
    EXPECT_EQUAL(instrument._waveform[4], -0.05);
}

STUDENT_TEST("Milestone 4: nextSample works if pluck not called.") {
    AudioSystem::setSampleRate(15);

    /* 10 samples per sec / 1Hz = 10 samples. */
    StringInstrument instrument(1);
    EXPECT_NOT_EQUAL(instrument._waveform, nullptr);
    EXPECT_EQUAL(instrument._length, 15);

    EXPECT_EQUAL(instrument.nextSample(), 0);
    EXPECT_EQUAL(instrument.nextSample(), 0);
    EXPECT_EQUAL(instrument.nextSample(), 0);
    EXPECT_EQUAL(instrument.nextSample(), 0);
    EXPECT_EQUAL(instrument.nextSample(), 0);
    EXPECT_EQUAL(instrument.nextSample(), 0);
    EXPECT_EQUAL(instrument._cursor, 6);
}


PROVIDED_TEST("Milestone 2: Waveform array initialized correctly.") {
    /* Change the sample rate to 3, just to make the numbers come out nice. */
    AudioSystem::setSampleRate(3);

    /* Create a string that vibrates at 1hz. This is well below the human hearing
     * threshold and exists purely for testing purposes.
     */
    StringInstrument instrument(1);

    /* Make sure something was allocated. */
    EXPECT_NOT_EQUAL(instrument._waveform, nullptr);

    /* Length should be 3 / 1 = 3. */
    EXPECT_EQUAL(instrument._length, 3);

    /* All entries should be zero. */
    EXPECT_EQUAL(instrument._waveform[0], 0);
    EXPECT_EQUAL(instrument._waveform[1], 0);
    EXPECT_EQUAL(instrument._waveform[2], 0);
}

PROVIDED_TEST("Milestone 2: Constructor reports errors on bad inputs.") {
    /* To make the math easier. */
    AudioSystem::setSampleRate(10);

    EXPECT_ERROR(StringInstrument error(-1));  // Negative frequency
    EXPECT_ERROR(StringInstrument error(0));   // Zero frequency
    EXPECT_ERROR(StringInstrument error(10));  // Array would have length 1
    EXPECT_ERROR(StringInstrument error(100)); // Array would have length 0

    /* But we shouldn't get errors for good values. */
    StringInstrument peachyKeen(1);
    EXPECT_NOT_EQUAL(peachyKeen._waveform, nullptr);
}

PROVIDED_TEST("Milestone 2: Constructor sets cursor to position 0.") {
    AudioSystem::setSampleRate(10);

    StringInstrument instrument(1);
    EXPECT_NOT_EQUAL(instrument._waveform, nullptr);

    EXPECT_EQUAL(instrument._cursor, 0);
}

PROVIDED_TEST("Milestone 3: pluck does not allocate a new array.") {
    AudioSystem::setSampleRate(10);

    StringInstrument instrument(1);
    EXPECT_NOT_EQUAL(instrument._waveform, nullptr);

    /* Plucking the string should change the contents of the array, but not
     * which array we're pointing at.
     */
    Sample* oldArray = instrument._waveform;
    instrument.pluck();

    EXPECT_EQUAL(instrument._waveform, oldArray);
}

PROVIDED_TEST("Milestone 3: pluck sets values to +0.05 and -0.05.") {
    AudioSystem::setSampleRate(4);

    /* 4 samples per sec / 1Hz = 4 samples. */
    StringInstrument instrument(1);
    EXPECT_NOT_EQUAL(instrument._waveform, nullptr);
    EXPECT_EQUAL(instrument._length, 4);

    instrument.pluck();
    EXPECT_EQUAL(instrument._waveform[0], +0.05);
    EXPECT_EQUAL(instrument._waveform[1], +0.05);
    EXPECT_EQUAL(instrument._waveform[2], -0.05);
    EXPECT_EQUAL(instrument._waveform[3], -0.05);
}

PROVIDED_TEST("Milestone 3: pluck resets the cursor.") {
    AudioSystem::setSampleRate(4);

    /* 4 samples per sec / 1Hz = 4 samples. */
    StringInstrument instrument(1);
    EXPECT_NOT_EQUAL(instrument._waveform, nullptr);

    /* Invasively move the cursor forward. This is called an "invasive"
     * test because it manipulates internal state of the type we're
     * testing, rather than just using the interface.
     */
    instrument._cursor = 3;
    instrument.pluck();
    EXPECT_EQUAL(instrument._cursor, 0);
}

PROVIDED_TEST("Milestone 4: nextSample works if pluck not called.") {
    AudioSystem::setSampleRate(10);

    /* 10 samples per sec / 1Hz = 10 samples. */
    StringInstrument instrument(1);
    EXPECT_NOT_EQUAL(instrument._waveform, nullptr);
    EXPECT_EQUAL(instrument._length, 10);

    EXPECT_EQUAL(instrument.nextSample(), 0);
    EXPECT_EQUAL(instrument.nextSample(), 0);
    EXPECT_EQUAL(instrument.nextSample(), 0);
    EXPECT_EQUAL(instrument.nextSample(), 0);
    EXPECT_EQUAL(instrument._cursor, 4);
}

PROVIDED_TEST("Milestone 4: nextSample updates waveform array.") {
    AudioSystem::setSampleRate(4);

    /* 4 samples per sec / 1Hz = 4 samples. */
    StringInstrument instrument(1);
    EXPECT_NOT_EQUAL(instrument._waveform, nullptr);
    EXPECT_EQUAL(instrument._length, 4);

    instrument.pluck();
    EXPECT_EQUAL(instrument.nextSample(), +0.05);
    EXPECT_EQUAL(instrument.nextSample(), +0.05);
    EXPECT_EQUAL(instrument.nextSample(), -0.05);
    EXPECT_EQUAL(instrument._cursor, 3);

    /* The first array value is the average of +0.05 and +0.05, scaled by 0.995.
     * The two values are the same, so we should get back +0.05 scalled by 0.995.
     */
    EXPECT_EQUAL(instrument._waveform[0], +0.05 * 0.995);

    /* The next array value is the average of +0.05 and -0.05, scaled by 0.995.
     * This is exactly zero.
     */
    EXPECT_EQUAL(instrument._waveform[1], 0.0);

    /* The next array value is the average of -0.05 and -0.05, scaled by 0.995.
     * As with the first entry, this is -0.05 scaled by 0.995.
     */
    EXPECT_EQUAL(instrument._waveform[2], -0.05 * 0.995);
}

PROVIDED_TEST("Milestone 4: nextSample wraps around properly.") {
    AudioSystem::setSampleRate(2);

    /* 2 samples per sec / 1Hz = 2 samples. */
    StringInstrument instrument(1);
    EXPECT_NOT_EQUAL(instrument._waveform, nullptr);
    EXPECT_EQUAL(instrument._length, 2);

    /* Pluck the string, forming the array [+0.05, -0.05] */
    instrument.pluck();

    /* Read two samples, which should be +0.05 and -0.05. */
    EXPECT_EQUAL(instrument.nextSample(), +0.05);
    EXPECT_EQUAL(instrument._cursor, 1);
    EXPECT_EQUAL(instrument.nextSample(), -0.05);
    EXPECT_EQUAL(instrument._cursor, 0);

    /* The first array value is the average of +0.05 and -0.05, scaled by 0.995.
     * This is zero.
     */
    EXPECT_EQUAL(instrument._waveform[0], 0.0);

    /* The next array value is the average of -0.05 and 0, scaled by 0.995. */
    Sample decayedTerm = 0.995 * (-0.05 + 0) / 2.0;
    EXPECT_EQUAL(instrument._waveform[1], decayedTerm);

    /* Get two more samples. The waveform is [0, decayedTerm], so we should
     * get back 0, then decayedTerm.
     */
    EXPECT_EQUAL(instrument.nextSample(), 0.0);
    EXPECT_EQUAL(instrument._cursor, 1);
    EXPECT_EQUAL(instrument.nextSample(), decayedTerm);
    EXPECT_EQUAL(instrument._cursor, 0);

    /* The first array value is the average of 0.0 and decayedTerm, scaled by
     * 0.995.
     */
    Sample moreDecayed = 0.995 * (decayedTerm + 0) / 2.0;
    EXPECT_EQUAL(instrument._waveform[0], moreDecayed);

    /* The second array value is the average of decayedTerm and moreDecayed,
     * scaled by 0.995.
     */
    EXPECT_EQUAL(instrument._waveform[1], 0.995 * (decayedTerm + moreDecayed) / 2.0);
}

// STUDENT_TEST("Milestone 4: Keeps track of current column and how may more calls to nextSample()") {
//     _col = 0;

//     for int





// }











/* * * * * Special Functions Below This Point * * * * */

#include <algorithm>

/* Copy constructor for StringInstrument. This is called automatically by C++
 * if you need to make a copy of a StringInstrument; say, if you were to pass
 * one into a function by value. Take CS106L for more details about how this
 * works.
 */
StringInstrument::StringInstrument(const StringInstrument& rhs) {
    /* Not normally part of a copy constructor, this line is designed
     * to ensure that if you haven't implemented StringInstrument,
     * you don't get weird and wild crashes.
     */
    if (rhs._waveform == nullptr) {
        return;
    }

    _length   = rhs._length;
    _waveform = new Sample[_length];
    _cursor   = rhs._cursor;

    for (int i = 0; i < _length; i++) {
        _waveform[i] = rhs._waveform[i];
    }
}

/* Assignment operator for StringInstrument. This is called automatically by
 * C++ when you assign one StringInstrument to another. Take CS106L for more
 * details about how this works. This specific implementation uses an idiom
 * called "copy-and-swap."
 */
StringInstrument& StringInstrument::operator =(StringInstrument rhs) {
    swap(_length,   rhs._length);
    swap(_waveform, rhs._waveform);
    swap(_cursor,   rhs._cursor);
    return *this;
}
