# De-Noise

Plugin audio de réduction de bruit (VST3 + application autonome), **Windows uniquement** pour l'instant.

## Préréglages

| Preset | Réduction mesurée (bruit blanc) | Usage |
|--------|---------------------------------|-------|
| **Bas** | ~5 dB | Bruit léger, très peu d'artefacts |
| **Normal** | ~10 dB | Compromis par défaut |
| **Haut** | ~16 dB | Bruit marqué, réduction forte |

Valeurs obtenues sur un prototype de l'algorithme (bruit blanc stationnaire). À ajuster à l'oreille sur de vrais enregistrements.

Un curseur **Mix** (0–100 %) règle le dosage entre le signal d'origine et le signal nettoyé.

## Fonctionnement

Réduction spectrale (STFT 1024 points, recouvrement 75 %) : le plancher de bruit est suivi en continu pour chaque bande de fréquence, puis soustrait avec un gain lissé en temps et en fréquence. Latence : 1024 échantillons (déclarée à l'hôte, compensée automatiquement par la plupart des DAW).

## Compilation

Le build Windows est fait par GitHub Actions (onglet **Actions**, artefact `De-Noise-Windows`).
Pour publier une version : `git tag v0.1.0 && git push origin v0.1.0` (le zip est alors attaché à la Release).

En local (Windows, Visual Studio 2022 + CMake) :

```
cmake -S . -B build -A x64
cmake --build build --config Release
```

Le VST3 se trouve dans `build/DeNoise_artefacts/Release/VST3/`. Copie `De-Noise.vst3` dans `C:\Program Files\Common Files\VST3`.

## Licence

Le plugin utilise [JUCE](https://juce.com), disponible sous AGPLv3 ou licence commerciale. Choisis la licence adaptée avant toute distribution.
