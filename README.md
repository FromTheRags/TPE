# TPE - Simulateur de Production Énergétique

Simulateur interactif modélisant le mix de production électrique en France (**Énergies Renouvelables vs Nucléaire**), combinant une interface graphique riche en **Qt** (Qt 5 / Qt 6) et un module multimédia interactif en **SDL 1.2**.

Ce projet modernisé compile nativement sous **Windows** et **Linux**, est 100% compatible avec **CLion** et **CMake**, intègre des workflows d'intégration continue **GitHub Actions** (CI/CD) et produit un installeur Windows avec **Inno Setup**.

---

## Fonctionnalités

1. **Simulateur de Production Énergétique (Qt)** :
   - **Design initial authentique (2014-2015)** : interface compacte et fidèle (1100x580) avec l'image paysagère d'origine `ENR5.png` restaurée sans artefact ni pixellisation, et cadre de résultat en relief.
   - **Infobulles au survol** : affichage immédiat d'un texte explicatif riche (titre et description de chaque filière énergétique) au survol de chaque zone du schéma paysager (*Éolien, Solaire photovoltaïque, Nucléaire, Thermique, Géothermie, Hydroélectricité, Biomasse*).
   - **Mode Plein Écran** : bascule dynamique en plein écran via la touche **F11** (sortie via **Échap** ou **F11**).
   - **Paramétrage en temps réel** :
     - *Éolien* : nombre d'éoliennes, diamètre de rotor, vitesse moyenne du vent, heures annuelles (formule avec limite théorique de Betz à 16/27 et facteur 0.70).
     - *Solaire photovoltaïque* : surface de captage, rendement des modules, ensoleillement global, coefficient de performance global (Cp).
     - *Nucléaire* : puissance unitaire des réacteurs, nombre de tranches en exploitation, facteur de disponibilité moyen (85%).
   - **Calcul dynamique** et conversion automatique de la production annuelle globale (Wh, kWh, MWh, GWh, TWh).
   - **Importation et exportation** des scénarios de simulation au format texte (`.txt`), compatible avec les fichiers d'origine.
   - **Fiches thématiques authentiques (HTML)** : reprise intégrale des fiches d'origine sous `QTextBrowser` plein écran pour l'éolien, le solaire photovoltaïque et le nucléaire, avec formules mathématiques, sources et images d'époque.

2. **Module de Présentation Interactive (SDL 1.2)** :
   - Accessible depuis le menu de l'application (**Ctrl+P**).
   - Présentation interactive avec la mascotte originale *Volt*, bulles explicatives progressives et animation de l'éolienne.
   - **Support du mode plein écran** : bascule dynamique en plein écran via **F11** ou la touche **F** (sortie via **Échap**).
   - Rendu d'arrière-plan uniformisé et centré sans bandes disparates.

---

## Organisation du Projet

```
TPE/
├── .github/
│   └── workflows/
│       ├── build.yml               # CI GitHub Actions (Linux & Windows)
│       └── release.yml             # CD automatique sur tag git
├── assets/                         # Ressources partagées (icônes, images, polices OFL)
├── cmake/
│   ├── CopyRuntimeDependencies.cmake
│   └── sdl12/                      # Bibliothèques et en-têtes SDL 1.2 Windows (x86 & x64)
├── dist/                           # Sortie de packaging (installeur .exe, zip portable)
├── examples/                       # Fichiers de simulation d'exemple (.txt)
├── images/                         # Graphismes d'origine (PNG, BMP, GIF, ICO)
├── innosetup/
│   └── tpe_setup.iss               # Script Inno Setup moderne
├── scripts/
│   └── restore_original_assets.py  # Script de traitement et d'uniformisation des assets
├── src/                            # Dossier source ("source")
│   ├── main.cpp                    # Point d'entrée de l'application Qt
│   ├── mainwindow.h / .cpp         # Interface graphique Qt principale, calculs & plein écran
│   ├── sdl_presentation.h / .cpp   # Module interactif SDL 1.2 & plein écran
│   ├── utils.h / .cpp              # Résolution robuste et portable des ressources
│   └── tpe.rc                      # Fichier de ressources Windows (icône binaire)
├── CMakeLists.txt                  # Configuration CMake multiplateforme (CLion)
├── LICENSE                         # Licence MIT (Créé par fromtherags en 2014)
├── OFL.txt                         # Licence SIL Open Font License
└── README.md
```

---

## Guide d'Installation pour Rebuild

Ce guide décrit pas à pas l'installation des dépendances nécessaires pour recompiler le projet sous Windows et Linux.

### 1. Prérequis sous Windows

#### A. Outils généraux & Compilateur
- **CLion** (qui intègre nativement MinGW GCC, CMake et Ninja dans `bin/mingw/bin`, `bin/cmake/` et `bin/ninja/`).
- Ou alternativement, **MSYS2** (`C:\msys64`).

#### B. Installation de Qt (recommandé via `aqtinstall`)
L'outil officiel en ligne de commande `aqtinstall` permet d'installer les bibliothèques Qt précompilées sans installer l'interface graphique volumineuse de Qt :

1. Installer `aqtinstall` (via winget ou pip) :
   ```powershell
   winget install miurahr.aqtinstall
   # ou via python : pip install aqtinstall
   ```

2. Télécharger et installer **Qt 6.7.2 MinGW 64-bit** dans `C:\Qt` :
   ```powershell
   aqt install-qt windows desktop 6.7.2 win64_mingw -O C:\Qt
   ```
   *Note : Le fichier `CMakeLists.txt` détecte automatiquement `C:\Qt\6.7.2\mingw_64`.*

3. *(Optionnel)* Si vous utilisez **MSYS2 MinGW-w64** au lieu de l'archive Qt :
   ```bash
   pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja \
             mingw-w64-x86_64-SDL mingw-w64-x86_64-SDL_ttf mingw-w64-x86_64-qt5-base mingw-w64-x86_64-qt5-tools
   ```

#### C. Installation d'Inno Setup 6 (pour l'installeur)
```powershell
winget install JRSoftware.InnoSetup
# ou via chocolatey : choco install innosetup -y
```

---

### 2. Compilation sous CLion (JetBrains)

Le projet est configuré pour une prise en charge immédiate sous CLion :

1. Lancer **CLion**.
2. **Fichier** > **Ouvrir...** > Sélectionner le dossier racine du projet `TPE`.
3. CLion charge le `CMakeLists.txt` et détecte automatiquement le compilateur MinGW inclus ainsi que l'installation Qt dans `C:/Qt/6.7.2/mingw_64`.
   *(Si votre installation Qt est située dans un dossier personnalisé, ajoutez simplement `-DCMAKE_PREFIX_PATH=C:/chemin/vers/Qt` dans **Settings** > **Build, Execution, Deployment** > **CMake** > **CMake options**).*
4. Cliquer sur **Build** (Marteau vert / `Ctrl+F9`) ou sur **Run** (`Shift+F10`).
5. Les images et assets sont automatiquement copiés dans le dossier d'exécution (`cmake-build-debug-mingw`) lors de la compilation.

---

### 3. Compilation en ligne de commande sous Windows (PowerShell)

En utilisant les outils fournis par CLion :

```powershell
# 1. Configurer l'environnement PATH
$env:PATH = "C:\Users\$env:USERNAME\AppData\Local\Programs\CLion\bin\mingw\bin;C:\Users\$env:USERNAME\AppData\Local\Programs\CLion\bin\ninja\win\x64;$env:PATH"

# 2. Configurer le projet avec CMake et Ninja
& "C:\Users\$env:USERNAME\AppData\Local\Programs\CLion\bin\cmake\win\x64\bin\cmake.exe" `
    -DCMAKE_BUILD_TYPE=Release `
    -DCMAKE_MAKE_PROGRAM="C:/Users/$env:USERNAME/AppData/Local/Programs/CLion/bin/ninja/win/x64/ninja.exe" `
    -G Ninja -S . -B build

# 3. Compiler l'exécutable
cmake --build build --config Release

# 4. Déployer les dépendances Qt avec windeployqt
& "C:\Qt\6.7.2\mingw_64\bin\windeployqt.exe" --no-translations build/tpe.exe

# 5. Lancer l'application
.\build\tpe.exe
```

---

### 4. Compilation sous Linux (Ubuntu / Debian)

```bash
# 1. Installation des dépendances (Qt5 ou Qt6 et SDL 1.2)
sudo apt-get update
sudo apt-get install -y cmake ninja-build build-essential \
    libsdl1.2-dev libsdl-ttf2.0-dev \
    qtbase5-dev qttools5-dev-tools

# (Si vous êtes sous Ubuntu 24.04+, le paquet équivalent est :)
# sudo apt-get install -y cmake ninja-build build-essential libsdl1.2-compat-dev libsdl-ttf2.0-dev qt6-base-dev qt6-tools-dev

# 2. Configuration et compilation
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# 3. Lancer l'application
./build/tpe
```

---

### 5. Génération de l'Installeur Windows (Inno Setup)

Pour générer l'installeur autonome `dist/tpe-setup-v1.5.exe` :

```powershell
# 1. Préparer le dossier de distribution portable
New-Item -ItemType Directory -Force -Path dist\tpe-windows
Copy-Item cmake-build-debug-mingw\tpe.exe dist\tpe-windows\ -Force
Copy-Item cmake-build-debug-mingw\*.dll dist\tpe-windows\ -Force
Copy-Item -Recurse -Force images dist\tpe-windows\
Copy-Item -Recurse -Force assets dist\tpe-windows\
Copy-Item -Recurse -Force examples dist\tpe-windows\
Copy-Item -Recurse -Force cmake-build-debug-mingw\platforms dist\tpe-windows\
Copy-Item LICENSE, OFL.txt, README.md dist\tpe-windows\ -Force

# 2. Compiler l'installeur Inno Setup
& "${env:LOCALAPPDATA}\Programs\Inno Setup 6\ISCC.exe" innosetup\tpe_setup.iss
# Ou si installé dans Program Files :
# & "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe" innosetup\tpe_setup.iss
```

L'installeur final est produit dans `dist/tpe-setup-v1.5.exe`.

---

## Raccourcis et Commandes

| Raccourci | Action |
|---|---|
| **F11** | Basculer en mode Plein Écran |
| **Échap** | Quitter le mode Plein Écran |
| **Ctrl+O** | Ouvrir un scénario de simulation (`.txt`) |
| **Ctrl+S** | Sauvegarder les paramètres actuels (`.txt`) |
| **Ctrl+Q** | Quitter l'application |

---

## Licence & Crédits

- **Code source** : [MIT License](LICENSE) — Créé par fromtherags en 2014.
- **Polices de caractères** : [SIL Open Font License 1.1](OFL.txt) (Patrick Hand, Droid Sans).
- **Ressources graphiques** : Fichiers d'origine avec mentions de nom remplacées par *Créé par fromtherags en 2014* et rendu uniformisé.
