/*
 * File: StringInstrument.cpp
 * Author: Amanda Phan (amandapt@stanford.edu)
 * Course/Section: CS 106B, Thorton 210
 * This file maintains a grid of lights and plays sounds based on a user's input.
 */

#include "ToneMatrix.h"
#include "Demos/DrawRectangle.h"
#include <cmath>
using namespace std;

/* Color of a light that is off and that is on. */
const Color kLightOffColor( 64,  64,  64);
const Color kLightOnColor (250, 250, 100);
const int PLUCK_STRING = 8192;

/* Given a row index, returns the frequency of the note played by the
 * instrument at that index.
 *
 * For those of you who are musically inclined: the base frequency is
 * chosen to be a low C. The remaining notes are then repeated major
 * pentatonic scales stacked on top. To see why, note that one half step
 * corresponds to multiplying the frequency by the twelfth root of two.
 * Therefore, multiplying a frequency by pow(2, n / 12.0) corresponds
 * to taking the note n half-steps above the base frequency.
 *
 * Feel free to tinker and tweak these frequencies as an extension.
 * However, don't modify them when working on the base assignment;
 * our test cases make some assumptions based on how they work.
 */
double frequencyForRow(int rowIndex) {
    if (rowIndex < 0) error("Invalid row index: " + to_string(rowIndex));

    /* Pentatonic note offsets from the base note, in number of
     * half steps. Each row lowers the frequency, so we count
     * down by the number of half steps.
     */
    const int kPentatonicSteps[] = {
        0, -3, -5, -8, -10
    };

    /* High C. */
    const double kBaseFrequency = 220 * pow(2, (30.0 + 9.0) / 12.0);

    /* Convert offset to how many octaves down to shift. */
    int octave = rowIndex / 5;

    /* Determine how many half steps we need to shift down. */
    int halfSteps = (-(12 * octave) + kPentatonicSteps[rowIndex % 5]);

    /* Each half step corresponds to scaling the frequency by the twelfth root of
     * two. Therefore, taking a bunch of half steps down is equivalent to
     * multiplying by some power of the twelfth root of two.
     */
    return kBaseFrequency * pow(2.0, halfSteps / 12.0);
}


/* The ToneMatrix function takes in a gridSize and a lightSize. The function
* stores the state of each of the gridSize x gridSize lights, and initalizes
* an array of StringInstruments.
*/
ToneMatrix::ToneMatrix(int gridSize, int lightSize) {
    _gridSize = gridSize;
    _lightSize = lightSize;
    _instruments = new StringInstrument[_gridSize];
    _time = 0;
    _col = 0;

    // Initialize an arrav of bools but set each element equal to false
    _grid = new bool[_gridSize * _gridSize];
    for (int i = 0; i < _gridSize * _gridSize; i++){
        _grid[i] = false;
    }

    for (int j = 0; j < _gridSize; j++){
        double freq = frequencyForRow(j);
        // Create a string instrument and assign it to the array
        // StringInstrument specificInstrument(freq);
        _instruments[j] = StringInstrument(freq);
    }
}

/* The ToneMatrix destructor function cleans up all the memory allocated
 * by the ToneMatrix type.
 */
ToneMatrix::~ToneMatrix() {
    delete[] _grid;
    delete[] _instruments;
}

/* The mousePressed function takes in two arguments: mouseX and mouseY.
 * Based on the location of the mouse, the function determines which light
 * in the grid the mouse was pressed on.
 */
void ToneMatrix::mousePressed(int mouseX, int mouseY) {
    // Converting mouseX and mouseY to grid coordinates
    int x = mouseX / _lightSize;
    int y = mouseY / _lightSize;

    if (_grid[_gridSize * y + x] == false) {
        _grid[_gridSize * y + x] = true;
        _pressed = true;
    }
    else if (_grid[_gridSize * y + x] == true) {
        _grid[_gridSize * y + x] = false;
        _pressed = false;
    }
}

/* The mouseDragged function takes in two arguments: mouseX and mouseY,
 * indicating where the mouse was dragged within the Tone Matrix. The function
 * updates the state of the light directly under the mouse.
 */
void ToneMatrix::mouseDragged(int mouseX, int mouseY) {
    int x = mouseX / _lightSize;
    int y = mouseY / _lightSize;
    _grid[_gridSize * y + x] = _pressed;

}

/* The draw function draws the lights of the Tone Matrix, specifically computing
 * the rectangular bounding boxes for all the lights in the grid.
 */
void ToneMatrix::draw() const {
    for (int i = 0; i < _gridSize; i++){
        for (int j = 0; j < _gridSize; j++){
            Rectangle rec = {i * _lightSize, j * _lightSize, _lightSize, _lightSize};
            if (_grid[_gridSize * j + i]) {
                drawRectangle(rec, kLightOnColor);
            }
            else if (!_grid[_gridSize * j + i]) {
                drawRectangle(rec, kLightOffColor);
            }
        }
    }

}

/* The nextSample function determines whether it is time to pluck more strings
 * and plucks the approrpriate strings. Then, it adds up the samples returned
 * by all the strings and sends that to the speakers.
 */
Sample ToneMatrix::nextSample() {
    Sample total = 0;

    // If the call is a multiple of 8192, pluck the string
    if (_time % PLUCK_STRING == 0) {
        for (int i = 0; i < _gridSize; i++) {
            if (_grid[_gridSize * i + _col] == true) {
                _instruments[i].pluck();
            }
        }
        _col = (_col + 1) % _gridSize;
    }

    _time++;


    // Loop through the rows and add up all the samples
    for (int j = 0; j < _gridSize; j++){
        total += _instruments[j].nextSample();
    }
    return total;
}


/* The resize function takes in a newGridSize and dynamically updates the tone matrix to a
 * new newGridSize x newGridSize. The function resizes both the light grid and the instrument array.
 * It also resets time and the playback position to behin at column 0.
 */
void ToneMatrix::resize(int newGridSize) {
    if (newGridSize <= 0) {
        error("This is not a valid grid size.");
    }

    // Create a new, dynamically allocated array of StringInstruments of the new size
    StringInstrument* _newInstruments = new StringInstrument[newGridSize];

    // Resize the instruments array and copy over the old instruments to the new array
    for (int i = 0; i < newGridSize; i++) {
        if (i <= _gridSize - 1){
            _newInstruments[i] = _instruments[i];
        }
        else {
            double freq = frequencyForRow(i);
            _newInstruments[i] = StringInstrument(freq);

        }
    }

    delete [] _instruments;
    _instruments = _newInstruments;


    // Resize the light grid
    bool* _newGrid = new bool[newGridSize * newGridSize];
    for (int j = 0; j < newGridSize; j++) {
        for (int k = 0; k < newGridSize; k++) {
            if (j <= _gridSize - 1 && k <= _gridSize - 1) {
                _newGrid[newGridSize * j + k] = _grid[_gridSize * j + k];
            }
            else {
                _newGrid[newGridSize * j + k] == false;
            }

        }
    }
    delete [] _grid;
    _grid = _newGrid;

    _col = 0;
    _time = 0;
    _gridSize = newGridSize;

}





















/* * * * * Test Cases Below This Point * * * * */
#include "GUI/SimpleTest.h"
#include "Demos/AudioSystem.h"
#include "GUI/TextUtils.h"

STUDENT_TEST("Milestone 1: mousePressed toggles the light at row 0, col 0.") {
    AudioSystem::setSampleRate(44300);

    /* Make each light 2x2. This makes the light at position 2*col + 1, 2*row + 1 dead
     * center in the middle of position (row, col).
     */
    ToneMatrix matrix(20, 5);
    EXPECT_NOT_EQUAL(matrix._grid, nullptr);
    EXPECT_EQUAL(matrix._gridSize, 20);

    /* Light should be off. */
    EXPECT_EQUAL(matrix._grid[0], false);

    /* Pressing at position (1, 1) presses the upper-left corner. It's off, so
     * this should turn it on.
     */
    matrix.mousePressed(1, 1);
    EXPECT_EQUAL(matrix._grid[0], true);

    /* Make sure that every other light is still off. */
    for (int row = 0; row < 20; row++) {
        for (int col = 0; col < 20; col++) {
            /* Skip (0, 0) */
            if (row != 0 || col != 0) {
                EXPECT_EQUAL(matrix._grid[20 * row + col], false);
            }
        }
    }

    /* Do this again, which should turn the light back off. */
    matrix.mousePressed(1, 1);
    EXPECT_EQUAL(matrix._grid[0], false);

    /* Make sure that every other light is still off. */
    for (int row = 0; row < 20; row++) {
        for (int col = 0; col < 20; col++) {
            /* Skip (0, 0) */
            if (row != 0 || col != 0) {
                EXPECT_EQUAL(matrix._grid[20 * row + col], false);
            }
        }
    }
}

STUDENT_TEST("Milestone 2: mouseDragged turns on all lights in the top row.") {
    AudioSystem::setSampleRate(44300);

    /* Make each light 2x2. This makes the light at position 2*col + 1, 2*row + 1 dead
     * center in the middle of position (row, col).
     */
    ToneMatrix matrix(20, 2);
    EXPECT_NOT_EQUAL(matrix._grid, nullptr);

    /* Press the mouse at (1, 1) to turn on the top-left light. */
    EXPECT_EQUAL(matrix._grid[0], false);
    matrix.mousePressed(1, 1);
    EXPECT_EQUAL(matrix._grid[0], true);

    /* Drag the mouse to (3, 1) to turn on the light at row 0, column 1. */
    EXPECT_EQUAL(matrix._grid[1], false);
    matrix.mouseDragged(3, 1);
    EXPECT_EQUAL(matrix._grid[1], true);

    /* Drag the mouse back to (1, 1). This should not have any effect because
     * the light is already on.
     */
    EXPECT_EQUAL(matrix._grid[0], true);
    matrix.mouseDragged(1, 1);
    EXPECT_EQUAL(matrix._grid[0], true);

    /* Drag the mouse back to (3, 1). This should not have any effect because
     * the light is already on.
     */
    EXPECT_EQUAL(matrix._grid[1], true);
    matrix.mouseDragged(3, 1);
    EXPECT_EQUAL(matrix._grid[1], true);

    /* Drag the mouse across the rest of the top row to turn on all lights. */
    for (int col = 2; col < 20; col++) {
        /* This is the light at index 0 * 16 + col = col within the grid. */
        const int index = col;

        /* This light should be off. */
        EXPECT_EQUAL(matrix._grid[index], false);

        /* Press at x = 2*col + 1, y = 1 to press the light. */
        matrix.mouseDragged(2 * col + 1, 1);
        EXPECT_EQUAL(matrix._grid[index], true);
    }

    /* Make sure everything in the top row is still turned on. */
    for (int col = 0; col < 20; col++) {
        EXPECT_EQUAL(matrix._grid[col], true);
    }

    /* Make sure everything not in row 0 is turned off. */
    for (int row = 1; row < 20; row++) {
        for (int col = 0; col < 20; col++) {
            EXPECT_EQUAL(matrix._grid[20 * row + col], false);
        }
    }

    /* Turn all the lights in the top row back off. Begin by pressing the
     * mouse at row 0, column 15.
     */
    EXPECT_EQUAL(matrix._grid[19], true);
    matrix.mousePressed(2 * 19 + 1, 1);
    EXPECT_EQUAL(matrix._grid[19], false);

    /* Drag the mouse to row 0, column 14, to turn that light off. */
    EXPECT_EQUAL(matrix._grid[18], true);
    matrix.mouseDragged(2 * 18 + 1, 1);
    EXPECT_EQUAL(matrix._grid[18], false);

    /* Drag the mouse back to row 0, column 15, which should have no
     * effect because the light is already off.
     */
    EXPECT_EQUAL(matrix._grid[19], false);
    matrix.mouseDragged(2 * 19 + 1, 1);
    EXPECT_EQUAL(matrix._grid[19], false);

    /* Drag the mouse back to row 0, column 14, which should have no
     * effect because the light is already off.
     */
    EXPECT_EQUAL(matrix._grid[18], false);
    matrix.mouseDragged(2 * 18 + 1, 1);
    EXPECT_EQUAL(matrix._grid[18], false);

    /* Now drag from right to left back across the row to turn all the
     * lights off.
     */
    for (int col = 17; col >= 0; col--) {
        /* This is the light at index 0 * 16 + col = col within the grid. */
        const int index = col;

        /* This light should be on. */
        EXPECT_EQUAL(matrix._grid[index], true);

        /* Press at x = 2*col + 1, y = 1 to press the light. */
        matrix.mousePressed(2 * col + 1, 1);
        EXPECT_EQUAL(matrix._grid[index], false);
    }

    /* Make sure all lights are off. */
    for (int row = 0; row < 20; row++) {
        for (int col = 0; col < 20; col++) {
            EXPECT_EQUAL(matrix._grid[20 * row + col], false);
        }
    }
}


STUDENT_TEST("Milestone 4: Checks whether new data members are initialized") {
    AudioSystem::setSampleRate(2500);

    ToneMatrix matrix(20, 5);
    EXPECT_EQUAL(matrix._gridSize, 20);
    EXPECT_EQUAL(matrix._lightSize, 5);
    EXPECT_EQUAL(matrix._time, 0);
    EXPECT_EQUAL(matrix._col, 0);
}


STUDENT_TEST("Milestone 4: Keeps track of the current column and how many more calls to nextSample") {
    AudioSystem::setSampleRate(44100);

    /* Make each light 2x2. This makes the light at position 2*col + 1, 2*row + 1 dead
     * center in the middle of position (row, col).
     */
    ToneMatrix matrix(16, 2);

    int col = 0;
    int time = 0;


    for (int i = 0; i < 8193 ; i++) {
        matrix.nextSample();
        if (i % 8192 == 0) {
            col++;
            EXPECT_EQUAL(matrix._col, col);
        }
    }

}

PROVIDED_TEST("Milestone 1: ToneMatrix constructor stores the light dimensions.") {
    /* Other tests may have changed the sample rate. This is necessary to ensure that
     * the sample rate is set to a value large enough for all StringInstruments can
     * behave correctly.
     */
    AudioSystem::setSampleRate(44100);

    ToneMatrix matrix1(16, 137);
    EXPECT_EQUAL(matrix1._gridSize, 16);
    EXPECT_EQUAL(matrix1._lightSize, 137);

    ToneMatrix matrix2(5, 106);
    EXPECT_EQUAL(matrix1._gridSize, 16);
    EXPECT_EQUAL(matrix2._lightSize, 106);
}

PROVIDED_TEST("Milestone 1: ToneMatrix constructor sets instrument frequencies.") {
    AudioSystem::setSampleRate(44100);

    ToneMatrix matrix(16, 137);
    EXPECT_NOT_EQUAL(matrix._instruments, nullptr);

    /* Check that the frequencies are right by computing what they should be and comparing
     * against the expected value.
     */
    for (int i = 0; i < 16; i++) {
        EXPECT_EQUAL(matrix._instruments[i]._length, AudioSystem::sampleRate() / frequencyForRow(i));
    }
}

PROVIDED_TEST("Milestone 1: ToneMatrix constructor initializes lights to off.") {
    AudioSystem::setSampleRate(44100);

    ToneMatrix matrix(13, 137);
    EXPECT_NOT_EQUAL(matrix._grid, nullptr);

    for (int i = 0; i < 13 * 13; i++) {
        EXPECT_EQUAL(matrix._grid[i], false);
    }
}

PROVIDED_TEST("Milestone 1: mousePressed toggles the light at row 0, col 0.") {
    AudioSystem::setSampleRate(44100);

    /* Make each light 2x2. This makes the light at position 2*col + 1, 2*row + 1 dead
     * center in the middle of position (row, col).
     */
    ToneMatrix matrix(14, 2);
    EXPECT_NOT_EQUAL(matrix._grid, nullptr);
    EXPECT_EQUAL(matrix._gridSize, 14);

    /* Light should be off. */
    EXPECT_EQUAL(matrix._grid[0], false);

    /* Pressing at position (1, 1) presses the upper-left corner. It's off, so
     * this should turn it on.
     */
    matrix.mousePressed(1, 1);
    EXPECT_EQUAL(matrix._grid[0], true);

    /* Make sure that every other light is still off. */
    for (int row = 0; row < 14; row++) {
        for (int col = 0; col < 14; col++) {
            /* Skip (0, 0) */
            if (row != 0 || col != 0) {
                EXPECT_EQUAL(matrix._grid[14 * row + col], false);
            }
        }
    }

    /* Do this again, which should turn the light back off. */
    matrix.mousePressed(1, 1);
    EXPECT_EQUAL(matrix._grid[0], false);

    /* Make sure that every other light is still off. */
    for (int row = 0; row < 14; row++) {
        for (int col = 0; col < 14; col++) {
            /* Skip (0, 0) */
            if (row != 0 || col != 0) {
                EXPECT_EQUAL(matrix._grid[14 * row + col], false);
            }
        }
    }
}

PROVIDED_TEST("Milestone 1: mousePressed toggles the light at row 9, col 6.") {
    AudioSystem::setSampleRate(44100);

    /* Make each light 2x2. This makes the light at position 2*col + 1, 2*row + 1 dead
     * center in the middle of position (row, col).
     */
    ToneMatrix matrix(10, 2);
    EXPECT_NOT_EQUAL(matrix._grid, nullptr);

    const int rowIndex = 9;
    const int colIndex = 6;
    const int lightIndex = 10 * rowIndex + colIndex;

    /* Dead center inside the light. */
    const int lightX = 1 + 2 * colIndex;
    const int lightY = 1 + 2 * rowIndex;

    /* Light should be off. */
    EXPECT_EQUAL(matrix._grid[lightIndex], false);

    /* Pressing at position (lightX, lightY) presses the light. It's off, so
     * this should turn it on.
     */
    matrix.mousePressed(lightX, lightY);
    EXPECT_EQUAL(matrix._grid[lightIndex], true);

    /* Make sure that every other light is still off. */
    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < 10; col++) {
            /* Skip (rowIndex, colIndex) */
            if (row != rowIndex || col != colIndex) {
                EXPECT_EQUAL(matrix._grid[10 * row + col], false);
            }
        }
    }

    /* Do this again, which should turn the light back off. */
    matrix.mousePressed(lightX, lightY);
    EXPECT_EQUAL(matrix._grid[lightIndex], false);

    /* Make sure that every other light is still off. */
    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < 10; col++) {
            /* Skip (rowIndex, colIndex) */
            if (row != rowIndex || col != colIndex) {
                EXPECT_EQUAL(matrix._grid[10 * row + col], false);
            }
        }
    }
}

PROVIDED_TEST("Milestone 1: mousePressed works across the top row.") {
    AudioSystem::setSampleRate(44100);

    /* Make each light 2x2. This makes the light at position 2*col + 1, 2*row + 1 dead
     * center in the middle of position (row, col).
     */
    ToneMatrix matrix(16, 2);
    EXPECT_NOT_EQUAL(matrix._grid, nullptr);

    /* Turn all the lights in the top row on. */
    for (int col = 0; col < 16; col++) {
        /* This is the light at index 0 * 16 + col = col within the grid. */
        const int index = col;

        /* This light should be off. */
        EXPECT_EQUAL(matrix._grid[index], false);

        /* Press at x = 2*col + 1, y = 1 to press the light. */
        matrix.mousePressed(2 * col + 1, 1);
        EXPECT_EQUAL(matrix._grid[index], true);
    }

    /* Make sure everything in the top row is still turned on. */
    for (int col = 0; col < 16; col++) {
        EXPECT_EQUAL(matrix._grid[col], true);
    }

    /* Make sure everything not in row 0 is turned off. */
    for (int row = 1; row < 16; row++) {
        for (int col = 0; col < 16; col++) {
            EXPECT_EQUAL(matrix._grid[16 * row + col], false);
        }
    }

    /* Turn all the lights in the top row back off. */
    for (int col = 0; col < 16; col++) {
        /* This is the light at index 0 * 16 + col = col within the grid. */
        const int index = col;

        /* This light should be on. */
        EXPECT_EQUAL(matrix._grid[index], true);

        /* Press at x = 2*col + 1, y = 1 to press the light. */
        matrix.mousePressed(2 * col + 1, 1);
        EXPECT_EQUAL(matrix._grid[index], false);
    }

    /* Make sure all lights are off. */
    for (int row = 0; row < 16; row++) {
        for (int col = 0; col < 16; col++) {
            EXPECT_EQUAL(matrix._grid[16 * row + col], false);
        }
    }
}

PROVIDED_TEST("Milestone 1: mousePressed works across the leftmost column.") {
    AudioSystem::setSampleRate(44100);

    /* Make each light 2x2. This makes the light at position 2*col + 1, 2*row + 1 dead
     * center in the middle of position (row, col).
     */
    ToneMatrix matrix(16, 2);
    EXPECT_NOT_EQUAL(matrix._grid, nullptr);

    /* Turn all the lights in the leftmost column on. */
    for (int row = 0; row < 16; row++) {
        /* This is the light at index 16 * row + col = 16 * row within the grid. */
        const int index = 16 * row;

        /* This light should be off. */
        EXPECT_EQUAL(matrix._grid[index], false);

        /* Press at x = 1, y = 2*row + 1 to press the light. */
        matrix.mousePressed(1, 2 * row + 1);
        EXPECT_EQUAL(matrix._grid[index], true);
    }

    /* Make sure everything in the leftmost column is still turned on. */
    for (int row = 0; row < 16; row++) {
        EXPECT_EQUAL(matrix._grid[16 * row], true);
    }

    /* Make sure everything not in col 0 is turned off. */
    for (int row = 0; row < 16; row++) {
        for (int col = 1; col < 16; col++) {
            EXPECT_EQUAL(matrix._grid[16 * row + col], false);
        }
    }

    /* Turn all the lights in the leftmost column back off. */
    for (int row = 0; row < 16; row++) {
        /* This is the light at index 16 * row + col = 16 * row within the grid. */
        const int index = 16 * row;

        /* This light should be on. */
        EXPECT_EQUAL(matrix._grid[index], true);

        /* Press at x = 1, y = 2*row + 1 to press the light. */
        matrix.mousePressed(1, 2 * row + 1);
        EXPECT_EQUAL(matrix._grid[index], false);
    }

    /* Make sure all lights are off. */
    for (int row = 0; row < 16; row++) {
        for (int col = 0; col < 16; col++) {
            EXPECT_EQUAL(matrix._grid[16 * row + col], false);
        }
    }
}

PROVIDED_TEST("Milestone 2: mouseDragged turns on all lights in the top row.") {
    AudioSystem::setSampleRate(44100);

    /* Make each light 2x2. This makes the light at position 2*col + 1, 2*row + 1 dead
     * center in the middle of position (row, col).
     */
    ToneMatrix matrix(16, 2);
    EXPECT_NOT_EQUAL(matrix._grid, nullptr);

    /* Press the mouse at (1, 1) to turn on the top-left light. */
    EXPECT_EQUAL(matrix._grid[0], false);
    matrix.mousePressed(1, 1);
    EXPECT_EQUAL(matrix._grid[0], true);

    /* Drag the mouse to (3, 1) to turn on the light at row 0, column 1. */
    EXPECT_EQUAL(matrix._grid[1], false);
    matrix.mouseDragged(3, 1);
    EXPECT_EQUAL(matrix._grid[1], true);

    /* Drag the mouse back to (1, 1). This should not have any effect because
     * the light is already on.
     */
    EXPECT_EQUAL(matrix._grid[0], true);
    matrix.mouseDragged(1, 1);
    EXPECT_EQUAL(matrix._grid[0], true);

    /* Drag the mouse back to (3, 1). This should not have any effect because
     * the light is already on.
     */
    EXPECT_EQUAL(matrix._grid[1], true);
    matrix.mouseDragged(3, 1);
    EXPECT_EQUAL(matrix._grid[1], true);

    /* Drag the mouse across the rest of the top row to turn on all lights. */
    for (int col = 2; col < 16; col++) {
        /* This is the light at index 0 * 16 + col = col within the grid. */
        const int index = col;

        /* This light should be off. */
        EXPECT_EQUAL(matrix._grid[index], false);

        /* Press at x = 2*col + 1, y = 1 to press the light. */
        matrix.mouseDragged(2 * col + 1, 1);
        EXPECT_EQUAL(matrix._grid[index], true);
    }

    /* Make sure everything in the top row is still turned on. */
    for (int col = 0; col < 16; col++) {
        EXPECT_EQUAL(matrix._grid[col], true);
    }

    /* Make sure everything not in row 0 is turned off. */
    for (int row = 1; row < 16; row++) {
        for (int col = 0; col < 16; col++) {
            EXPECT_EQUAL(matrix._grid[16 * row + col], false);
        }
    }

    /* Turn all the lights in the top row back off. Begin by pressing the
     * mouse at row 0, column 15.
     */
    EXPECT_EQUAL(matrix._grid[15], true);
    matrix.mousePressed(2 * 15 + 1, 1);
    EXPECT_EQUAL(matrix._grid[15], false);

    /* Drag the mouse to row 0, column 14, to turn that light off. */
    EXPECT_EQUAL(matrix._grid[14], true);
    matrix.mouseDragged(2 * 14 + 1, 1);
    EXPECT_EQUAL(matrix._grid[14], false);

    /* Drag the mouse back to row 0, column 15, which should have no
     * effect because the light is already off.
     */
    EXPECT_EQUAL(matrix._grid[15], false);
    matrix.mouseDragged(2 * 15 + 1, 1);
    EXPECT_EQUAL(matrix._grid[15], false);

    /* Drag the mouse back to row 0, column 14, which should have no
     * effect because the light is already off.
     */
    EXPECT_EQUAL(matrix._grid[14], false);
    matrix.mouseDragged(2 * 14 + 1, 1);
    EXPECT_EQUAL(matrix._grid[14], false);

    /* Now drag from right to left back across the row to turn all the
     * lights off.
     */
    for (int col = 13; col >= 0; col--) {
        /* This is the light at index 0 * 16 + col = col within the grid. */
        const int index = col;

        /* This light should be on. */
        EXPECT_EQUAL(matrix._grid[index], true);

        /* Press at x = 2*col + 1, y = 1 to press the light. */
        matrix.mousePressed(2 * col + 1, 1);
        EXPECT_EQUAL(matrix._grid[index], false);
    }

    /* Make sure all lights are off. */
    for (int row = 0; row < 16; row++) {
        for (int col = 0; col < 16; col++) {
            EXPECT_EQUAL(matrix._grid[16 * row + col], false);
        }
    }
}

#include "set.h"
#include "Demos/RectangleCatcher.h"
#include "GUI/TextUtils.h"

PROVIDED_TEST("Milestone 3: Draws a grid where all lights are off.") {
    AudioSystem::setSampleRate(44100);

    /* Each cell has size 137 * 137 */
    const int lightSize = 137;
    ToneMatrix matrix(16, lightSize);

    EXPECT_NOT_EQUAL(matrix._grid, nullptr);

    /* RectangleCatcher is a type that captures all rectangles drawn by
     * drawRectangle rather than rendering them to the screen. We'll use
     * this to determine which rectangles are drawn.
     */
    RectangleCatcher catcher;
    matrix.draw();

    /* Scan the rectangles. Confirm that...
     *
     * 1. We didn't get any duplicates.
     * 2. Every rectangle's coordinates are a multiple of lightSize.
     * 3. Every rectangle upper-corner is within the box [0, 0] x [lightSize * 15, lightSize * 15].
     * 4. Every rectangle has width and height equal to lightSize.
     * 5. Every light is given the OFF color.
     */
    Set<DrawnRectangle> drawn;
    for (int i = 0; i < catcher.numDrawn(); i++) {
        /* Check for duplicates. */
        if (drawn.contains(catcher[i])) {
            SHOW_ERROR(format("Duplicate rectangle drawn: %s", catcher[i]));
        }
        drawn += catcher[i];

        /* Check that x and y coordinates are multiples of the light size. */
        auto rectangle = catcher[i].rectangle;
        EXPECT_EQUAL(rectangle.x % lightSize, 0);
        EXPECT_EQUAL(rectangle.y % lightSize, 0);

        /* Check that everything is within the appropriate bounding box. */
        EXPECT_GREATER_THAN_OR_EQUAL_TO(rectangle.x, 0);
        EXPECT_GREATER_THAN_OR_EQUAL_TO(rectangle.y, 0);
        EXPECT_LESS_THAN(rectangle.x, 16 * lightSize);
        EXPECT_LESS_THAN(rectangle.y, 16 * lightSize);

        /* Check the width and height of the rectangles. */
        EXPECT_EQUAL(rectangle.width,  lightSize);
        EXPECT_EQUAL(rectangle.height, lightSize);

        /* Make sure the light is off. */
        auto color = catcher[i].color;
        EXPECT_EQUAL(color, kLightOffColor);
    }

    /* If there are 256 total rectangles, the above constraints ensure that
     * every possible rectangle has been drawn and they've all been drawn
     * once.
     */
    EXPECT_EQUAL(catcher.numDrawn(), 16 * 16);
}

PROVIDED_TEST("Milestone 3: Draws a grid where all lights are on.") {
    AudioSystem::setSampleRate(44100);

    /* Each cell has size 137 * 137 */
    const int lightSize = 137;
    ToneMatrix matrix(16, lightSize);

    EXPECT_NOT_EQUAL(matrix._grid, nullptr);

    /* Turn all the lights on. To do so, press at position (137 * col + 1, 137 * row + 1)
     * for all rows and columns.
     */
    for (int row = 0; row < 16; row++) {
        for (int col = 0; col < 16; col++) {
            matrix.mousePressed(lightSize * col + 1, lightSize * row);
            EXPECT_EQUAL(matrix._grid[16 * row + col], true);
        }
    }


    /* RectangleCatcher is a type that captures all rectangles drawn by
     * drawRectangle rather than rendering them to the screen. We'll use
     * this to determine which rectangles are drawn.
     */
    RectangleCatcher catcher;
    matrix.draw();

    /* Scan the rectangles. Confirm that...
     *
     * 1. We didn't get any duplicates.
     * 2. Every rectangle's coordinates are a multiple of lightSize.
     * 3. Every rectangle upper-corner is within the box [0, 0] x [lightSize * 15, lightSize * 15].
     * 4. Every rectangle has width and height equal to lightSize.
     * 5. Every light is given the ON color.
     */
    Set<DrawnRectangle> drawn;
    for (int i = 0; i < catcher.numDrawn(); i++) {
        /* Check for duplicates. */
        if (drawn.contains(catcher[i])) {
            SHOW_ERROR(format("Duplicate rectangle drawn: %s", catcher[i]));
        }
        drawn += catcher[i];

        /* Check that x and y coordinates are multiples of the light size. */
        auto rectangle = catcher[i].rectangle;
        EXPECT_EQUAL(rectangle.x % lightSize, 0);
        EXPECT_EQUAL(rectangle.y % lightSize, 0);

        /* Check that everything is within the appropriate bounding box. */
        EXPECT_GREATER_THAN_OR_EQUAL_TO(rectangle.x, 0);
        EXPECT_GREATER_THAN_OR_EQUAL_TO(rectangle.y, 0);
        EXPECT_LESS_THAN(rectangle.x, 16 * lightSize);
        EXPECT_LESS_THAN(rectangle.y, 16 * lightSize);

        /* Check the width and height of the rectangles. */
        EXPECT_EQUAL(rectangle.width,  lightSize);
        EXPECT_EQUAL(rectangle.height, lightSize);

        /* Make sure the light is off. */
        auto color = catcher[i].color;
        EXPECT_EQUAL(color, kLightOnColor);
    }

    /* If there are 256 total rectangles, the above constraints ensure that
     * every possible rectangle has been drawn and they've all been drawn
     * once.
     */
    EXPECT_EQUAL(catcher.numDrawn(), 16 * 16);
}

PROVIDED_TEST("Milestone 3: Draws a grid with one offset light that's on.") {
    AudioSystem::setSampleRate(44100);

    /* Each cell has size 137 * 137 */
    const int lightSize = 137;
    ToneMatrix matrix(16, lightSize);

    EXPECT_NOT_EQUAL(matrix._grid, nullptr);

    /* Row 9, column 6. */
    const int rowIndex = 9;
    const int colIndex = 6;
    const int lightIndex = 16 * rowIndex + colIndex;

    /* Dead center inside the light. */
    const int lightX = 1 + lightSize * colIndex;
    const int lightY = 1 + lightSize * rowIndex;

    matrix.mousePressed(lightX, lightY);
    EXPECT_EQUAL(matrix._grid[lightIndex], true);

    /* Set up a RectangleCatcher to catch all drawn rectangles. */
    RectangleCatcher catcher;
    matrix.draw();

    /* Confirm we drew 256 rectangles. */
    EXPECT_EQUAL(catcher.numDrawn(), 16 * 16);

    /* Store all rectangles with the ON color. */
    Set<Rectangle> lightsOn;
    for (int i = 0; i < catcher.numDrawn(); i++) {
        if (catcher[i].color == kLightOnColor) {
            lightsOn += catcher[i].rectangle;
        }
    }

    /* There should just be one. */
    EXPECT_EQUAL(lightsOn.size(), 1);
    auto rect = lightsOn.first();

    /* Check its width/height. */
    EXPECT_EQUAL(rect.width,  lightSize);
    EXPECT_EQUAL(rect.height, lightSize);

    /* Confirm it's where it should be. */
    EXPECT_EQUAL(rect.x, colIndex * lightSize);
    EXPECT_EQUAL(rect.y, rowIndex * lightSize);
}

PROVIDED_TEST("Milestone 4: First call to nextSample() plucks appropriate strings.") {
    AudioSystem::setSampleRate(44100);

    /* Make each light 2x2. This makes the light at position 2*col + 1, 2*row + 1 dead
     * center in the middle of position (row, col).
     */
    ToneMatrix matrix(16, 2);
    EXPECT_NOT_EQUAL(matrix._grid, nullptr);
    EXPECT_NOT_EQUAL(matrix._instruments, nullptr);
    EXPECT_NOT_EQUAL(matrix._instruments[0]._waveform, nullptr);

    /* Press the lights in column 0 in all even-numbered rows. */
    for (int row = 0; row < 16; row += 2) {
        int lightIndex = 16 * row;

        /* Should be off. */
        EXPECT_EQUAL(matrix._grid[lightIndex], false);

        /* Now it's on. */
        matrix.mousePressed(1, 2 * row + 1);
        EXPECT_EQUAL(matrix._grid[lightIndex], true);
    }

    /* Confirm that none of the instruments have been plucked by seeing if
     * the first sound sample in each instrument is 0.
     */
    for (int i = 0; i < 16; i++) {
        EXPECT_EQUAL(matrix._instruments[i]._cursor, 0);
        EXPECT_EQUAL(matrix._instruments[i]._waveform[0], 0);
    }

    /* Get the next sample from the Tone Matrix. There are eight instruments
     * active. Each of them, when plucked, returns +0.05 as its sample. We should
     * therefore have our sample come back as +0.40.
     */
    EXPECT_EQUAL(matrix.nextSample(), +0.40);

    /* Inspect the even-numbered instruments. Each should have a cursor at
     * position 1. The sample there should be equal to +0.05.
     */
    for (int row = 0; row < 16; row += 2) {
        EXPECT_EQUAL(matrix._instruments[row]._cursor, 1);
        EXPECT_EQUAL(matrix._instruments[row]._waveform[1], +0.05);
    }

    /* Inspect the odd-numbered instruments. Their cursors should also have
     * moved forward to position 1, but all the entries should be 0.
     */
    for (int row = 1; row < 16; row += 2) {
        EXPECT_EQUAL(matrix._instruments[row]._cursor, 1);
        EXPECT_EQUAL(matrix._instruments[row]._waveform[0], 0.0);
        EXPECT_EQUAL(matrix._instruments[row]._waveform[1], 0.0);
    }
}

PROVIDED_TEST("Milestone 4: All strings are sampled at each step.") {
    AudioSystem::setSampleRate(44100);

    /* Make each light 2x2. This makes the light at position 2*col + 1, 2*row + 1 dead
     * center in the middle of position (row, col).
     */
    ToneMatrix matrix(16, 2);
    EXPECT_NOT_EQUAL(matrix._grid, nullptr);
    EXPECT_NOT_EQUAL(matrix._instruments, nullptr);
    EXPECT_NOT_EQUAL(matrix._instruments[0]._waveform, nullptr);

    /* Press the lights in column 0 in all even-numbered rows. */
    for (int row = 0; row < 16; row += 2) {
        int lightIndex = 16 * row;

        /* Should be off. */
        EXPECT_EQUAL(matrix._grid[lightIndex], false);

        /* Now it's on. */
        matrix.mousePressed(1, 2 * row + 1);
        EXPECT_EQUAL(matrix._grid[lightIndex], true);
    }

    /* Confirm that none of the instruments have been plucked by seeing if
     * the first sound sample in each instrument is 0.
     */
    for (int i = 0; i < 16; i++) {
        EXPECT_EQUAL(matrix._instruments[i]._cursor, 0);
        EXPECT_EQUAL(matrix._instruments[i]._waveform[0], 0);
    }

    /* Get the next sample from the Tone Matrix. There are eight instruments
     * active. Each of them, when plucked, returns +0.05 as its sample. We should
     * therefore have our sample come back as +0.40.
     */
    EXPECT_EQUAL(matrix.nextSample(), +0.40);

    /* Inspect the even-numbered instruments. Each should have a cursor at
     * position 1. The sample there should be equal to +0.05.
     */
    for (int row = 0; row < 16; row += 2) {
        EXPECT_EQUAL(matrix._instruments[row]._cursor, 1);
        EXPECT_EQUAL(matrix._instruments[row]._waveform[1], +0.05);
    }

    /* Inspect the odd-numbered instruments. Their cursors should also have
     * moved forward to position 1, but all the entries should be 0.
     */
    for (int row = 1; row < 16; row += 2) {
        EXPECT_EQUAL(matrix._instruments[row]._cursor, 1);
        EXPECT_EQUAL(matrix._instruments[row]._waveform[0], 0.0);
        EXPECT_EQUAL(matrix._instruments[row]._waveform[1], 0.0);
    }

    /* Run ten time steps forward, ensuring all the cursors move. */
    for (int i = 2; i < 10; i++) {
        matrix.nextSample();

        for (int row = 0; row < 16; row++) {
            EXPECT_EQUAL(matrix._instruments[row]._cursor, i);
        }
    }
}

PROVIDED_TEST("Milestone 4: The 8192nd call to nextSample() after the first plucks strings.") {
    AudioSystem::setSampleRate(44100);

    /* Make each light 2x2. This makes the light at position 2*col + 1, 2*row + 1 dead
     * center in the middle of position (row, col).
     */
    ToneMatrix matrix(16, 2);
    EXPECT_NOT_EQUAL(matrix._grid, nullptr);
    EXPECT_NOT_EQUAL(matrix._instruments, nullptr);
    EXPECT_NOT_EQUAL(matrix._instruments[0]._waveform, nullptr);

    /* Press the lights in column 1 in all even-numbered rows. */
    for (int row = 0; row < 16; row += 2) {
        int lightIndex = 16 * row + 1;

        /* Should be off. */
        EXPECT_EQUAL(matrix._grid[lightIndex], false);

        /* Now it's on. */
        matrix.mousePressed(3, 2 * row + 1);
        EXPECT_EQUAL(matrix._grid[lightIndex], true);
    }

    /* Run time forward for a bit. */
    for (int time = 0; time < 8192; time++) {
        /* None of the instruments were plucked, so there should be
         * no sound.
         */
        EXPECT_EQUAL(matrix.nextSample(), 0.0);

        /* All sound samples in the instruments should still be zero
         * because nothing has been plucked yet.
         */
        for (int i = 0; i < 16; i++) {
            int cursor = matrix._instruments[i]._cursor;
            EXPECT_EQUAL(matrix._instruments[i]._waveform[cursor], 0);
        }
    }

    /* The next call to matrix.nextSample() will move into the first column.
     * When that happens, we should pluck all even-numbered strings.
     */

    /* Get the next sample from the Tone Matrix. There are eight instruments
     * active. Each of them, when plucked, returns +0.05 as its sample. We should
     * therefore have our sample come back as +0.40.
     *
     * If you are failing this test but all the instruments seem to be properly
     * plucked, make sure that you're resetting the cursor for each instrument
     * to 0 when calling pluck(). Otherwise, the cursors of the different
     * instruments might be in arbitrary positions within each waveform and you
     * may have contributions of +0.05 from some plucked strings and -0.05 from
     * others.
     */
    EXPECT_EQUAL(matrix.nextSample(), +0.40);

    /* Inspect the even-numbered instruments. The current sample should
     * be +0.05.
     */
    for (int row = 0; row < 16; row += 2) {
        EXPECT_EQUAL(matrix._instruments[row]._cursor, 1);
        EXPECT_EQUAL(matrix._instruments[row]._waveform[1], +0.05);
    }

    /* Inspect the odd-numbered instruments. The item under their cursors should
     * still be 0 because they haven't been plucked yet.
     */
    for (int row = 1; row < 16; row += 2) {
        int cursor = matrix._instruments[row]._cursor;
        EXPECT_EQUAL(matrix._instruments[row]._waveform[cursor], 0.0);
    }
}

#include <cmath>

PROVIDED_TEST("Milestone 4: Each instrument is plucked at the appropriate time.") {
    AudioSystem::setSampleRate(44100);

    /* Make each light 2x2. This makes the light at position 2*col + 1, 2*row + 1 dead
     * center in the middle of position (row, col).
     */
    ToneMatrix matrix(16, 2);
    EXPECT_NOT_EQUAL(matrix._grid, nullptr);
    EXPECT_NOT_EQUAL(matrix._instruments, nullptr);
    EXPECT_NOT_EQUAL(matrix._instruments[0]._waveform, nullptr);

    /* Press the lights all the way down the main diagonal. This will cause each
     * instrument to be plucked when its column comes up.
     */
    for (int row = 0; row < 16; row++) {
        int lightIndex = 16 * row + row;

        /* Should be off. */
        EXPECT_EQUAL(matrix._grid[lightIndex], false);

        /* Now it's on. */
        matrix.mousePressed(2 * row + 1, 2 * row + 1);
        EXPECT_EQUAL(matrix._grid[lightIndex], true);
    }

    /* Run the Tone Matrix for full sweep-through. */
    for (int i = 0; i < 16; i++) {
        /* This call to nextSample() should pluck the instrument
         * in row i. All previous instruments will have been
         * plucked before this, and all future instruments will
         * not have been plucked.
         */
        matrix.nextSample();

        /* Because of the decay rate, none of the previous instrument
         * amplitudes should be +-0.05. If they were, it means they
         * were plucked.
         */
        for (int before = 0; before < i; before++) {
            /* 'fabs' is "floating-point absolute value." It's basically
             * the absolute value function.
             */
            int cursor = matrix._instruments[before]._cursor;
            Sample amplitude = fabs(matrix._instruments[before]._waveform[cursor]);
            EXPECT_LESS_THAN(amplitude, +0.05);
            EXPECT_GREATER_THAN(amplitude, -0.05);
        }

        /* Confirm instrument in row i is plucked. */
        EXPECT_EQUAL(matrix._instruments[i]._cursor, 1);
        EXPECT_EQUAL(matrix._instruments[i]._waveform[1], +0.05);

        /* Nothing after us should be plucked. */
        for (int after = i + 1; after < 16; after++) {
            int cursor = matrix._instruments[after]._cursor;
            EXPECT_EQUAL(matrix._instruments[after]._waveform[cursor], 0.0);
        }

        /* Advance time forward 8191 steps. */
        for (int time = 0; time < 8191; time++) {
            matrix.nextSample();
        }
    }
}

PROVIDED_TEST("Milestone 4: nextSample() wraps back around to first column.") {
    AudioSystem::setSampleRate(44100);

    /* Make each light 2x2. This makes the light at position 2*col + 1, 2*row + 1 dead
     * center in the middle of position (row, col).
     */
    ToneMatrix matrix(16, 2);
    EXPECT_NOT_EQUAL(matrix._grid, nullptr);
    EXPECT_NOT_EQUAL(matrix._instruments, nullptr);
    EXPECT_NOT_EQUAL(matrix._instruments[0]._waveform, nullptr);

    /* Set only the top-left light to on. */
    matrix.mousePressed(1, 1);
    EXPECT_EQUAL(matrix._grid[0], true);

    /* Run several loops through the full Tone Matrix cycle. */
    for (int loopsThrough = 0; loopsThrough < 10; loopsThrough++) {
        for (int col = 0; col < 16; col++) {
            /* Column 0 will cause the first string to be plucked. */
            if (col == 0) {
                /* Only the first string vibrates, and we just plucked it. */
                EXPECT_EQUAL(matrix.nextSample(), +0.05);

                /* First string should have been plucked. */
                EXPECT_EQUAL(matrix._instruments[0]._cursor, 1);
                EXPECT_EQUAL(matrix._instruments[0]._waveform[1], +0.05);
            }
            /* Otherwise, nothing was plucked. We can't easily calculate what
             * the amplitude of the sample is.
             */
            else {
                matrix.nextSample();
            }

            /* No other strings should have been plucked. */
            for (int row = 1; row < 16; row++) {
                int cursor = matrix._instruments[row]._cursor;
                EXPECT_EQUAL(matrix._instruments[row]._waveform[cursor], 0.0);
            }

            /* Move through 8191 more samples, which gets to the point where we are
             * about to pluck things again.
             */
            for (int time = 0; time < 8191; time++) {
                matrix.nextSample();
            }
        }
    }
}

PROVIDED_TEST("Milestone 5: resize() chooses instruments of correct frequencies.") {
    AudioSystem::setSampleRate(44100);

    /* Initially, a 4x4 grid. */
    ToneMatrix matrix(4, 2);
    EXPECT_NOT_EQUAL(matrix._grid, nullptr);
    EXPECT_NOT_EQUAL(matrix._instruments, nullptr);
    EXPECT_EQUAL(matrix._gridSize, 4);
    EXPECT_EQUAL(matrix._lightSize, 2);

    /* Check the existing frequencies. */
    for (int row = 0; row < 4; row++) {
        EXPECT_EQUAL(matrix._instruments[row]._length, AudioSystem::sampleRate() / frequencyForRow(row));
    }

    /* Now expand up to 20 rows. */
    matrix.resize(20);
    EXPECT_EQUAL(matrix._gridSize, 20);
    EXPECT_EQUAL(matrix._lightSize, 2); // Unchanged

    /* Check the new frequencies. */
    for (int row = 0; row < 20; row++) {
        EXPECT_EQUAL(matrix._instruments[row]._length, AudioSystem::sampleRate() / frequencyForRow(row));
    }

    /* Now resize back down to 3 instruments. */
    matrix.resize(3);
    EXPECT_EQUAL(matrix._gridSize, 3);
    EXPECT_EQUAL(matrix._lightSize,     2); // Unchanged

    /* Check the new frequencies. */
    for (int row = 0; row < 3; row++) {
        EXPECT_EQUAL(matrix._instruments[row]._length, AudioSystem::sampleRate() / frequencyForRow(row));
    }
}

PROVIDED_TEST("Milestone 5: resize() preserves existing instruments.") {
    AudioSystem::setSampleRate(44100);

    /* Make each light 2x2. This makes the light at position 2*col + 1, 2*row + 1 dead
     * center in the middle of position (row, col).
     */
    ToneMatrix matrix(16, 2);
    EXPECT_NOT_EQUAL(matrix._grid, nullptr);
    EXPECT_NOT_EQUAL(matrix._instruments, nullptr);
    EXPECT_NOT_EQUAL(matrix._instruments[0]._waveform, nullptr);

    /* Press the lights all the way down the first column. This will cause all
     * instruments to play on the first call to nextSample().
     */
    for (int row = 0; row < 16; row++) {
        int lightIndex = 16 * row + 0;

        /* Should be off. */
        EXPECT_EQUAL(matrix._grid[lightIndex], false);

        /* Now it's on. */
        matrix.mousePressed(1, 2 * row + 1);
        EXPECT_EQUAL(matrix._grid[lightIndex], true);
    }

    /* Sample the Tone Matrix once to pluck all the instruments. */
    matrix.nextSample();

    /* All instruments should have been plucked, which we can measure
     * by looking at the underlying waveforms.
     */
    for (int row = 0; row < 16; row++) {
        EXPECT_EQUAL(matrix._instruments[row]._cursor, 1);
        EXPECT_EQUAL(matrix._instruments[row]._waveform[1], +0.05);
    }

    /* Now, resize the matrix down from 16 instruments to 8. This should
     * preserve the first eight instruments.
     */
    matrix.resize(8);
    EXPECT_EQUAL(matrix._gridSize, 8);
    EXPECT_EQUAL(matrix._lightSize, 2);

    /* All instruments should have been plucked, which we can measure
     * by looking at the underlying waveforms.
     */
    for (int row = 0; row < 8; row++) {
        EXPECT_EQUAL(matrix._instruments[row]._cursor, 1);
        EXPECT_EQUAL(matrix._instruments[row]._waveform[1], +0.05);
    }
}

PROVIDED_TEST("Milestone 5: resize() extends existing instruments with new ones.") {
    AudioSystem::setSampleRate(44100);

    /* Make each light 2x2. This makes the light at position 2*col + 1, 2*row + 1 dead
     * center in the middle of position (row, col).
     */
    ToneMatrix matrix(8, 2);
    EXPECT_NOT_EQUAL(matrix._grid, nullptr);
    EXPECT_NOT_EQUAL(matrix._instruments, nullptr);
    EXPECT_NOT_EQUAL(matrix._instruments[0]._waveform, nullptr);

    /* Press the lights all the way down the first column. This will cause all
     * instruments to play on the first call to nextSample().
     */
    for (int row = 0; row < 8; row++) {
        int lightIndex = 8 * row + 0;

        /* Should be off. */
        EXPECT_EQUAL(matrix._grid[lightIndex], false);

        /* Now it's on. */
        matrix.mousePressed(1, 2 * row + 1);
        EXPECT_EQUAL(matrix._grid[lightIndex], true);
    }

    /* Sample the Tone Matrix once to pluck all the instruments. */
    matrix.nextSample();

    /* All instruments should have been plucked, which we can measure
     * by looking at the underlying waveforms.
     */
    for (int row = 0; row < 8; row++) {
        EXPECT_EQUAL(matrix._instruments[row]._cursor, 1);
        EXPECT_EQUAL(matrix._instruments[row]._waveform[1], +0.05);
    }

    /* Now, resize the matrix up from 8 instruments to 15. This should leave
     * the first 8 instruments the same and add seven new ones.
     */
    matrix.resize(15);
    EXPECT_EQUAL(matrix._gridSize, 15);
    EXPECT_EQUAL(matrix._lightSize, 2);

    /* First eight instruments should remain plucked. */
    for (int row = 0; row < 8; row++) {
        EXPECT_EQUAL(matrix._instruments[row]._cursor, 1);
        EXPECT_EQUAL(matrix._instruments[row]._waveform[1], +0.05);
    }

    /* Next seven instruments should be unplucked. */
    for (int row = 8; row < 15; row++) {
        EXPECT_EQUAL(matrix._instruments[row]._cursor, 0);
        EXPECT_EQUAL(matrix._instruments[row]._waveform[0], 0.0);
    }
}

PROVIDED_TEST("Milestone 5: resize() preserves old lights when expanding.") {
    AudioSystem::setSampleRate(44100);

    ToneMatrix matrix(2, 2);
    EXPECT_NOT_EQUAL(matrix._grid, nullptr);
    EXPECT_NOT_EQUAL(matrix._instruments, nullptr);
    EXPECT_NOT_EQUAL(matrix._instruments[0]._waveform, nullptr);
    EXPECT_EQUAL(matrix._gridSize, 2);

    /* Turn every light in the grid on. The grid should now look like
     * this:
     *
     *    ON ON
     *    ON ON
     *
     */
    for (int i = 0; i < 4; i++) {
        matrix._grid[i] = true;
    }

    /* Now resize the grid to dimension 3x3. Preserving the lights
     * should result in a grid that looks like this:
     *
     *    ON ON __
     *    ON ON __
     *    __ __ __
     */
    matrix.resize(3);
    EXPECT_EQUAL(matrix._gridSize, 3);

    EXPECT_EQUAL(matrix._grid[0],  true);
    EXPECT_EQUAL(matrix._grid[1],  true);
    EXPECT_EQUAL(matrix._grid[2], false);
    EXPECT_EQUAL(matrix._grid[3],  true);
    EXPECT_EQUAL(matrix._grid[4],  true);
    EXPECT_EQUAL(matrix._grid[5], false);
    EXPECT_EQUAL(matrix._grid[6], false);
    EXPECT_EQUAL(matrix._grid[7], false);
    EXPECT_EQUAL(matrix._grid[8], false);
}

PROVIDED_TEST("Milestone 5: resize() preserves old lights when contracting.") {
    AudioSystem::setSampleRate(44100);

    ToneMatrix matrix(3, 2);
    EXPECT_NOT_EQUAL(matrix._grid, nullptr);
    EXPECT_NOT_EQUAL(matrix._instruments, nullptr);
    EXPECT_NOT_EQUAL(matrix._instruments[0]._waveform, nullptr);
    EXPECT_EQUAL(matrix._gridSize, 3);

    /* Turn every light in the grid on. The grid should now look like
     * this:
     *
     *    ON ON ON
     *    ON ON ON
     *    ON ON ON
     */
    for (int i = 0; i < 9; i++) {
        matrix._grid[i] = true;
    }

    /* Now resize the grid to dimension 2x2. Preserving the lights
     * should result in a grid that looks like this:
     *
     *    ON ON
     *    ON ON
     */
    matrix.resize(2);
    EXPECT_EQUAL(matrix._gridSize, 2);

    EXPECT_EQUAL(matrix._grid[0],  true);
    EXPECT_EQUAL(matrix._grid[1],  true);
    EXPECT_EQUAL(matrix._grid[2],  true);
    EXPECT_EQUAL(matrix._grid[3],  true);
}

PROVIDED_TEST("Milestone 5: resize() resets the left-to-right sweep.") {
    AudioSystem::setSampleRate(44100);

    ToneMatrix matrix(3, 2);
    EXPECT_NOT_EQUAL(matrix._grid, nullptr);
    EXPECT_NOT_EQUAL(matrix._instruments, nullptr);
    EXPECT_NOT_EQUAL(matrix._instruments[0]._waveform, nullptr);
    EXPECT_EQUAL(matrix._gridSize, 3);

    /* Turn on the lights in the second column. */
    matrix._grid[1] = matrix._grid[4] = matrix._grid[7] = true;

    /* Play some sounds for a bit, nothing should be generated. */
    for (int i = 0; i < 1000; i++) {
        EXPECT_EQUAL(matrix.nextSample(), 0);
    }

    /* Resize down to a 2x2 grid. */
    matrix.resize(2);
    EXPECT_EQUAL(matrix._gridSize, 2);

    /* Confirm the lights are as follows:
     *
     *   __ ON
     *   __ ON
     *
     */
    EXPECT_EQUAL(matrix._grid[0], false);
    EXPECT_EQUAL(matrix._grid[1],  true);
    EXPECT_EQUAL(matrix._grid[2], false);
    EXPECT_EQUAL(matrix._grid[3],  true);

    /* Play 8,192 samples. Nothing should happen. */
    for (int i = 0; i < 8192; i++) {
        EXPECT_EQUAL(matrix.nextSample(), 0);
    }

    /* The next one, though, should trigger something. */
    double result = matrix.nextSample();
    EXPECT_GREATER_THAN(result, 0);
}
