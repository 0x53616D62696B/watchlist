#include <iostream>
#include <vector>
#include <cmath> // For sin() and M_PI
#ifdef _MSC_VER
    #include <corecrt_math_defines.h> // Correcting of M_PI - platform specific for MSVC compiler
#endif

#include "sine_wave.hpp"

namespace SineWave{

class SineWaveGenerator {
public:
    SineWaveGenerator(double amplitude, double frequency, double phase, double step, int num_samples)
        : amplitude(amplitude), frequency(frequency), phase(phase), step(step), num_samples(num_samples) {
        generateSineWave();
    }

    void printSineWave() const {
        for (const auto& value : sine_wave) {
            std::cout << value << std::endl;
        }
    }

private:
    double amplitude;
    double frequency;
    double phase;
    double step;
    int num_samples;
    std::vector<double> sine_wave;

    void generateSineWave() {
        for (int i = 0; i < num_samples; ++i) {
            double t = i * step;
            double value = amplitude * std::sin(2 * M_PI * frequency * t + phase);
            sine_wave.push_back(value);
        }
    }
};

void sine_wave_main() {
    double amplitude = 1.0; // Amplitude of the sine wave
    double frequency = 1.0; // Frequency of the sine wave in Hz
    double phase = 0.0;     // Phase shift in radians
    double step = 0.01;     // Time step in seconds
    int num_samples = 100;  // Number of samples to generate

    SineWaveGenerator generator(amplitude, frequency, phase, step, num_samples);
    generator.printSineWave();

}

} // namespace SineWave