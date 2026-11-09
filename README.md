# TPE - Simulateur de Production Énergétique

Simulateur interactif modélisant le mix de production électrique en France (Énergies Renouvelables vs Nucléaire), combinant une interface graphique riche en **Qt** (Qt 5 / Qt 6) et un module interactif multimédia en **SDL 1.2**.

Ce projet modernisé compile nativement sous **Linux** et **Windows**, est nativement compatible avec **CLion** et **CMake**, intègre des workflows d'intégration continue **GitHub Actions** (CI/CD) et produit un installeur Windows avec **Inno Setup**.

Toutes les ressources graphiques, sons et polices ont été intégralement remplacées par des versions **100% libres de droits** (licences CC0 / MIT / OFL).

---

## Fonctionnalités

1. **Simulateur de Production Énergétique (Qt)** :
   - Schéma paysager interactif cliquable représentant les différentes sources d'énergie (Éolien, Solaire, Nucléaire, Thermique, Géothermie, Hydroélectricité, Biomasse).
   - Paramétrage en temps réel des capacités :
     - *Éolien* : nombre d'éoliennes, diamètre de rotor, vitesse moyenne du vent, heures annuelles (formule avec limite théorique de Betz à 16/27).
     - *Solaire photovoltaïque* : surface de captage, rendement des modules, ensoleillement global, coefficient de performance global (Cp).
     - *Nucléaire* : puissance unitaire des réacteurs, nombre de tranches en exploitation, facteur de disponibilité moyen (85%).
   - Calcul dynamique et conversion automatique de la production annuelle globale (Wh, kWh, MWh, GWh, TWh).
   - Importation et exportation des scénarios de simulation au format texte (`.txt`).
   - Fiches documentées interactives avec formules physiques, diagrammes et cartes thématiques françaises (gisement éolien, irradiation solaire, cartographie du parc nucléaire).

2. **Module de Présentation Interactive (SDL 1.2)** :
   - Accessible depuis le menu de l'application (Ctrl+P).
   - Présentation interactive avec la mascotte originale *Volt*, bulles explicatives progressives et animation fluide.

---

## Organisation du Projet

Le projet a été restructuré pour respecter les standards modernes C++ et CMake :

```
TPE/
├── .github/
│   └── workflows/
│       ├── build.yml               # CI GitHub Actions (Linux & Windows)
│       └── release.yml             # CD automatique sur tag git
├── assets/                         # Ressources partagées (icônes, polices OFL)
├── cmake/
│   ├── CopyRuntimeDependencies.cmake
│   └── sdl12/                      # Bibliothèques et en-têtes SDL 1.2 pour Windows
├── examples/                       # Fichiers de simulations d'exemple (.txt)
├── images/                         # Graphismes libres de droits (PNG, BMP, GIF, ICO)
├── innosetup/
│   └── tpe_setup.iss               # Script Inno Setup moderne
├── src/                            # Dossier source ("source")
│   ├── main.cpp                    # Point d'entrée de l'application Qt
│   ├── mainwindow.h / .cpp         # Interface graphique Qt principale & calculs
│   ├── sdl_presentation.h / .cpp   # Module interactif SDL 1.2
│   ├── utils.h / .cpp              # Résolution robuste et portable des ressources
│   └── tpe.rc                      # Fichier de ressources Windows (icône binaire)
├── CMakeLists.txt                  # Configuration CMake multiplateforme (CLion)
├── LICENSE                         # Licence MIT
├── OFL.txt                         # Licence de police SIL Open Font License
└── README.md
```

---

## Compilation

### Prérequis

- **CMake** 3.16 ou supérieur
- Un compilateur C++ compatible **C++17** (GCC, Clang ou MSVC)
- **Qt** (version 5 ou 6 avec le module Widgets)
- **SDL 1.2** (`libsdl1.2-dev` sous Linux, ou bibliothèques incluses sous Windows)

### Sous Linux (Ubuntu / Debian)

```bash
# Installation des dépendances
sudo apt-get update
sudo apt-get install -y cmake ninja-build build-essential \
    libsdl1.2-dev libsdl-ttf2.0-dev \
    qtbase5-dev qttools5-dev-tools

# Configuration et compilation
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Lancement
./build/tpe
```

### Sous Windows (MinGW-w64 / MSYS2)

Dans un terminal MSYS2 MinGW64 :

```bash
# Installation des paquets nécessaires
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja \
          mingw-w64-x86_64-SDL mingw-w64-x86_64-SDL_ttf mingw-w64-x86_64-qt5-base

# Compilation
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Sous CLion (JetBrains)

Le projet s'ouvre directement dans **CLion** :
1. `Fichier` > `Ouvrir...` > Sélectionner le dossier racine du projet `TPE`.
2. CLion détecte automatiquement `CMakeLists.txt` et configure l'environnement.
3. Cliquer sur le bouton **Exécuter** (Run `tpe`). Les images et polices sont copiées automatiquement dans le répertoire de sortie lors de la compilation.

---

## Création de l'Installeur Windows (Inno Setup)

Pour générer l'installeur binaire sous Windows :

```powershell
# Préparer le dossier dist/tpe-windows
mkdir -p dist/tpe-windows
cp build/tpe.exe dist/tpe-windows/
cp -r images assets examples LICENSE OFL.txt README.md dist/tpe-windows/

# Déployer les dépendances Qt et runtime MinGW
windeployqt dist/tpe-windows/tpe.exe

# Compiler l'installeur Inno Setup
& "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" innosetup/tpe_setup.iss
```

L'installeur `tpe-setup-v1.5.exe` sera généré dans le dossier `dist/`.

---

## Licence

- Code source : [MIT License](LICENSE) (© 2015-2026 Rémi Fache)
- Polices de caractères : [SIL Open Font License 1.1](OFL.txt) (Droid Sans, Patrick Hand)
- Graphismes et illustrations : 100% Libres de droits (CC0 / Domaine Public)
