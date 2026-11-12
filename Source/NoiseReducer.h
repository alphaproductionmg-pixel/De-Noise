#pragma once

#include <juce_dsp/juce_dsp.h>
#include <algorithm>
#include <cmath>
#include <vector>

// Réducteur de bruit spectral (STFT, soustraction spectrale avec suivi du plancher de bruit).
// Un objet traite UN canal. FFT 1024, recouvrement 75 %, fenêtre racine de Hann.
// Latence : fftSize échantillons.
class NoiseReducer
{
public:
    static constexpr int fftOrder = 10;
    static constexpr int fftSize  = 1 << fftOrder;
    static constexpr int hopSize  = fftSize / 4;
    static constexpr int numBins  = fftSize / 2 + 1;

    NoiseReducer() : fft (fftOrder)
    {
        window.resize (fftSize);
        for (int i = 0; i < fftSize; ++i)
        {
            const double hann = 0.5 - 0.5 * std::cos (2.0 * juce::MathConstants<double>::pi * i / fftSize);
            window[(size_t) i] = (float) std::sqrt (hann);
        }
        fftData.assign ((size_t) fftSize * 2, 0.0f);
        reset();
    }

    void prepare (double sampleRate)
    {
        const double framesPerSecond = sampleRate / (double) hopSize;
        // Le plancher de bruit estimé remonte lentement (6 dB par seconde).
        riseFactor = (float) std::pow (10.0, (6.0 / 20.0) / framesPerSecond);
        reset();
    }

    void reset()
    {
        inRing.assign ((size_t) fftSize, 0.0f);
        outRing.assign ((size_t) fftSize, 0.0f);
        noise.assign ((size_t) numBins, 0.0f);
        smoothMag.assign ((size_t) numBins, 0.0f);
        prevGain.assign ((size_t) numBins, 1.0f);
        gain.assign ((size_t) numBins, 1.0f);
        pos = 0;
        initialised = false;
    }

    // over : facteur de sur-soustraction, floorDb : atténuation maximale (dB, négatif)
    void setParameters (float overSubtraction, float floorDb)
    {
        over = overSubtraction;
        floorGain = std::pow (10.0f, floorDb / 20.0f);
    }

    // Traite un échantillon. Retourne le signal nettoyé (wet) et le signal d'origine
    // retardé de fftSize échantillons (dry), pour un mélange aligné en phase.
    inline void processSample (float in, float& wet, float& dry)
    {
        const size_t p = (size_t) pos;
        wet = outRing[p];
        dry = inRing[p];
        outRing[p] = 0.0f;
        inRing[p] = in;

        pos = (pos + 1) % fftSize;

        if (pos % hopSize == 0)
            processFrame();
    }

private:
    void processFrame()
    {
        // Rassemble les fftSize derniers échantillons (du plus ancien au plus récent).
        for (int i = 0; i < fftSize; ++i)
            fftData[(size_t) i] = inRing[(size_t) ((pos + i) % fftSize)] * window[(size_t) i];
        std::fill (fftData.begin() + fftSize, fftData.end(), 0.0f);

        fft.performRealOnlyForwardTransform (fftData.data(), true);

        // Estimation du bruit et calcul du gain par bin.
        for (int k = 0; k < numBins; ++k)
        {
            const float re = fftData[(size_t) (2 * k)];
            const float im = fftData[(size_t) (2 * k + 1)];
            const float mag = std::sqrt (re * re + im * im);

            // Magnitude lissée dans le temps : le suivi du bruit est plus stable.
            float& sm = smoothMag[(size_t) k];
            float& n = noise[(size_t) k];
            if (! initialised)
            {
                sm = mag;
                n = mag;
            }
            else
            {
                sm = 0.6f * sm + 0.4f * mag;
                if (sm < n)
                    n += 0.3f * (sm - n);
                else
                    n = std::min (n * riseFactor, sm);
            }

            const float g = std::max (floorGain, 1.0f - over * n / (mag + 1.0e-9f));
            gain[(size_t) k] = g;
        }
        initialised = true;

        // Lissage en fréquence (3 points) puis en temps pour limiter le « bruit musical ».
        for (int k = 0; k < numBins; ++k)
        {
            const float a = gain[(size_t) std::max (k - 1, 0)];
            const float b = gain[(size_t) k];
            const float c = gain[(size_t) std::min (k + 1, numBins - 1)];
            const float smoothedF = 0.25f * a + 0.5f * b + 0.25f * c;

            const float smoothedT = 0.5f * prevGain[(size_t) k] + 0.5f * smoothedF;
            prevGain[(size_t) k] = smoothedT;

            fftData[(size_t) (2 * k)]     *= smoothedT;
            fftData[(size_t) (2 * k + 1)] *= smoothedT;
        }

        // Symétrie conjuguée pour les fréquences négatives.
        for (int k = 1; k < fftSize / 2; ++k)
        {
            fftData[(size_t) (2 * (fftSize - k))]     =  fftData[(size_t) (2 * k)];
            fftData[(size_t) (2 * (fftSize - k) + 1)] = -fftData[(size_t) (2 * k + 1)];
        }

        fft.performRealOnlyInverseTransform (fftData.data());

        // Fenêtre de synthèse + recouvrement-addition. Somme des fenêtres² = 2 à 75 % → /2.
        for (int i = 0; i < fftSize; ++i)
            outRing[(size_t) ((pos + i) % fftSize)] += 0.5f * fftData[(size_t) i] * window[(size_t) i];
    }

    juce::dsp::FFT fft;
    std::vector<float> window, fftData, inRing, outRing, noise, smoothMag, prevGain, gain;
    int pos = 0;
    bool initialised = false;

    float riseFactor = 1.001f;
    float over = 1.6f;
    float floorGain = 0.18f;
};
