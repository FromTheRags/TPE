#include "mainwindow.h"
#include "sdl_presentation.h"
#include "utils.h"

#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QIcon>
#include <QPixmap>
#include <QFont>
#include <cmath>

ClickableAreaWidget::ClickableAreaWidget(QWidget *parent)
    : QWidget(parent)
{
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_Hover, true);
}

void ClickableAreaWidget::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    emit clicked();
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , contentContainer(nullptr)
    , actionPleinEcran(nullptr)
{
    setMinimumSize(1100, 580);
    resize(1100, 580);
    setWindowTitle("TPE - Simulateur de Production Énergétique");

    QString iconPath = getAssetPath("icone.png");
    if (!iconPath.isEmpty()) {
        setWindowIcon(QIcon(iconPath));
    }

    // --- Menu Fichier ---
    QMenu *menuFichier = menuBar()->addMenu("&Fichier");
    QAction *ouvrir = new QAction("&Ouvrir...", this);
    ouvrir->setShortcut(QKeySequence::Open);
    ouvrir->setIcon(QIcon(getAssetPath("fileopen.png")));
    connect(ouvrir, &QAction::triggered, this, &MainWindow::importation);
    menuFichier->addAction(ouvrir);

    QAction *sauvegarder = new QAction("&Sauvegarder en .txt...", this);
    sauvegarder->setShortcut(QKeySequence::Save);
    sauvegarder->setIcon(QIcon(getAssetPath("icone_save.png")));
    connect(sauvegarder, &QAction::triggered, this, &MainWindow::exportation);
    menuFichier->addAction(sauvegarder);

    menuFichier->addSeparator();

    QAction *actionQuitter = new QAction("&Quitter", this);
    actionQuitter->setShortcut(QKeySequence::Quit);
    actionQuitter->setIcon(QIcon(getAssetPath("icone_quit.png")));
    connect(actionQuitter, &QAction::triggered, qApp, &QApplication::quit);
    menuFichier->addAction(actionQuitter);

    // --- Menu Affichage (Plein écran) ---
    QMenu *menuAffichage = menuBar()->addMenu("&Affichage");
    actionPleinEcran = new QAction("&Plein écran", this);
    actionPleinEcran->setShortcut(QKeySequence(Qt::Key_F11));
    actionPleinEcran->setCheckable(true);
    actionPleinEcran->setStatusTip("Basculer en mode plein écran (F11)");
    connect(actionPleinEcran, &QAction::triggered, this, &MainWindow::toggleFullScreen);
    menuAffichage->addAction(actionPleinEcran);

    // --- Menu Calculs ---
    QMenu *menuCalcul = menuBar()->addMenu("&Calculs");
    QAction *actionCalculEolien = new QAction("&Éolien", this);
    actionCalculEolien->setIcon(QIcon(getAssetPath("eolienne_fixe.png")));
    connect(actionCalculEolien, &QAction::triggered, this, &MainWindow::eolien);
    menuCalcul->addAction(actionCalculEolien);

    QAction *actionCalculSolaire = new QAction("&Photovoltaïque", this);
    actionCalculSolaire->setIcon(QIcon(getAssetPath("Photo.png")));
    connect(actionCalculSolaire, &QAction::triggered, this, &MainWindow::photov);
    menuCalcul->addAction(actionCalculSolaire);

    QAction *actionCalculNucleaire = new QAction("&Nucléaire", this);
    actionCalculNucleaire->setIcon(QIcon(getAssetPath("nuclo.png")));
    connect(actionCalculNucleaire, &QAction::triggered, this, &MainWindow::nuclo);
    menuCalcul->addAction(actionCalculNucleaire);

    // --- Menu Présentation (SDL 1.2) ---
    QMenu *menuPresentation = menuBar()->addMenu("&Présentation");
    QAction *actionSDL = new QAction("&Lancer la présentation interactive (SDL 1.2)", this);
    actionSDL->setShortcut(QKeySequence("Ctrl+P"));
    actionSDL->setIcon(QIcon(getAssetPath("eolienne_fixe.png")));
    connect(actionSDL, &QAction::triggered, this, &MainWindow::lancerPresentationSDL);
    menuPresentation->addAction(actionSDL);

    // --- Menu Interactif & Aide ---
    QAction *menuInteractif = menuBar()->addAction("&Interactif");
    menuInteractif->setShortcut(QKeySequence("Ctrl+I"));
    connect(menuInteractif, &QAction::triggered, this, &MainWindow::interactif);

    QAction *menuAide = menuBar()->addAction("&?");
    connect(menuAide, &QAction::triggered, this, &MainWindow::aide);

    // --- Central Widget & Centered Content Container ---
    QWidget *zoneCentrale = new QWidget(this);
    zoneCentrale->setStyleSheet("background-color: #f0f3f6;");

    contentContainer = new QWidget(zoneCentrale);
    contentContainer->setFixedSize(1100, 560);
    contentContainer->setStyleSheet("background-color: transparent;");

    // Background Image (authentic ENR5.png)
    QLabel *image = new QLabel(contentContainer);
    image->setGeometry(0, 0, 830, 528);
    image->setPixmap(QPixmap(getAssetPath("ENR5.png")));
    image->setScaledContents(true);

    // Interactive hotspot areas overlaid on ENR5.png
    for (int i = 0; i < 7; ++i) {
        tool[i] = new ClickableAreaWidget(contentContainer);
        connect(tool[i], &ClickableAreaWidget::clicked, this, &MainWindow::interactif);
    }

    // 0: Éoliennes
    tool[0]->setGeometry(201, 120, 216, 115);
    tool[0]->setToolTip("<strong>Éoliennes</strong>");
    tool[0]->setWhatsThis("Les éoliennes utilisent l'énergie cinétique du vent pour actionner leurs hélices et faire tourner un alternateur.");

    // 1: Centrale nucléaire
    tool[1]->setGeometry(352, 264, 206, 125);
    tool[1]->setToolTip("<strong>Centrale nucléaire</strong>");
    tool[1]->setWhatsThis("La centrale nucléaire utilise la fission des atomes d'uranium pour produire de la chaleur, transformée en vapeur entraînant une turbine.");

    // 2: Panneaux photovoltaïques
    tool[2]->setGeometry(557, 377, 272, 113);
    tool[2]->setToolTip("<strong>Panneaux photovoltaïques</strong>");
    tool[2]->setWhatsThis("Les panneaux photovoltaïques produisent de l'électricité directement grâce au rayonnement solaire via l'effet photoélectrique.");

    // 3: Centrale thermique
    tool[3]->setGeometry(0, 190, 183, 140);
    tool[3]->setToolTip("<strong>Centrale thermique</strong> (à titre indicatif)");
    tool[3]->setWhatsThis("La centrale thermique utilise la vapeur générée par la combustion d'énergies fossiles (charbon, gaz, fioul).");

    // 4: Centrale géothermique
    tool[4]->setGeometry(134, 412, 228, 116);
    tool[4]->setToolTip("<strong>Centrale géothermique</strong> (à titre indicatif)");
    tool[4]->setWhatsThis("La géothermie puise la chaleur naturelle de la croûte terrestre pour produire de la vapeur et de l'électricité.");

    // 5: Centrale hydroélectrique
    tool[5]->setGeometry(644, 125, 186, 101);
    tool[5]->setToolTip("<strong>Centrale hydroélectrique</strong> (à titre indicatif)");
    tool[5]->setWhatsThis("La centrale hydroélectrique exploite l'énergie potentielle et cinétique de l'eau retenue par un barrage.");

    // 6: Biomasse
    tool[6]->setGeometry(644, 226, 186, 108);
    tool[6]->setToolTip("<strong>Biomasse</strong> (à titre indicatif)");
    tool[6]->setWhatsThis("La biomasse valorise les matières organiques végétales ou agricoles pour produire de la chaleur et de l'électricité.");

    for (int i = 0; i < 7; ++i) {
        tool[i]->show();
    }

    // --- Right Side Parameter Controls ---
    QFont headerFont("Segoe UI", 12, QFont::Bold);
    QFont labelFont("Segoe UI", 8);

    // Section 1: Éolien
    QLabel *texte1 = new QLabel("<u>Énergie éolienne</u> :", contentContainer);
    texte1->setFont(headerFont);
    texte1->setGeometry(840, 25, 250, 30);

    box1bis = new QDoubleSpinBox(contentContainer);
    box1bis->setRange(0, 100000);
    box1bis->setValue(100);
    box1bis->setDecimals(0);
    box1bis->setGeometry(840, 58, 70, 26);
    QLabel *texte1bis = new QLabel("Nombre d'éoliennes", contentContainer);
    texte1bis->setFont(labelFont);
    texte1bis->setGeometry(918, 58, 175, 26);

    box1 = new QDoubleSpinBox(contentContainer);
    box1->setRange(0, 200);
    box1->setValue(80);
    box1->setDecimals(0);
    box1->setGeometry(840, 88, 70, 26);
    QLabel *texte2 = new QLabel("Diamètre du rotor (en m)", contentContainer);
    texte2->setFont(labelFont);
    texte2->setGeometry(918, 88, 175, 26);

    box2 = new QDoubleSpinBox(contentContainer);
    box2->setRange(0, 60);
    box2->setValue(8.0);
    box2->setDecimals(1);
    box2->setGeometry(840, 118, 70, 26);
    QLabel *texte3 = new QLabel("Vitesse moyenne du vent (m/s)", contentContainer);
    texte3->setFont(labelFont);
    texte3->setGeometry(918, 118, 175, 26);

    box3 = new QDoubleSpinBox(contentContainer);
    box3->setRange(0, 8766);
    box3->setValue(6000);
    box3->setDecimals(0);
    box3->setGeometry(840, 148, 70, 26);
    QLabel *texte4 = new QLabel("Heures de fonctionnement / an", contentContainer);
    texte4->setFont(labelFont);
    texte4->setGeometry(918, 148, 175, 26);

    // Section 2: Solaire
    QLabel *texte5 = new QLabel("<u>Énergie solaire</u> :", contentContainer);
    texte5->setFont(headerFont);
    texte5->setGeometry(840, 185, 250, 30);

    box4 = new QDoubleSpinBox(contentContainer);
    box4->setRange(0, 100000000);
    box4->setValue(50000);
    box4->setDecimals(0);
    box4->setGeometry(840, 218, 70, 26);
    QLabel *texte6 = new QLabel("Surface des panneaux (en m²)", contentContainer);
    texte6->setFont(labelFont);
    texte6->setGeometry(918, 218, 175, 26);

    box5 = new QDoubleSpinBox(contentContainer);
    box5->setRange(0, 50);
    box5->setValue(16);
    box5->setDecimals(0);
    box5->setGeometry(840, 248, 70, 26);
    QLabel *texte7 = new QLabel("Rendement du module (%)", contentContainer);
    texte7->setFont(labelFont);
    texte7->setGeometry(918, 248, 175, 26);

    box6 = new QDoubleSpinBox(contentContainer);
    box6->setRange(0, 3000);
    box6->setValue(1500);
    box6->setDecimals(0);
    box6->setGeometry(840, 278, 70, 26);
    QLabel *texte8 = new QLabel("Ensoleillement (kWh/m²/an)", contentContainer);
    texte8->setFont(labelFont);
    texte8->setGeometry(918, 278, 175, 26);

    box7 = new QDoubleSpinBox(contentContainer);
    box7->setRange(0.01, 1.0);
    box7->setSingleStep(0.01);
    box7->setValue(0.75);
    box7->setDecimals(2);
    box7->setGeometry(840, 308, 70, 26);
    QLabel *texte9 = new QLabel("Coefficient de performance (Cp)", contentContainer);
    texte9->setFont(labelFont);
    texte9->setGeometry(918, 308, 175, 26);

    // Section 3: Nucléaire
    QLabel *texte10 = new QLabel("<u>Énergie nucléaire</u> :", contentContainer);
    texte10->setFont(headerFont);
    texte10->setGeometry(840, 345, 250, 30);

    box8 = new QDoubleSpinBox(contentContainer);
    box8->setRange(1, 3000);
    box8->setValue(1000);
    box8->setDecimals(0);
    box8->setGeometry(840, 378, 70, 26);
    QLabel *texte11 = new QLabel("Puissance moyenne (MW)", contentContainer);
    texte11->setFont(labelFont);
    texte11->setGeometry(918, 378, 175, 26);

    box9 = new QDoubleSpinBox(contentContainer);
    box9->setRange(0, 100);
    box9->setValue(58);
    box9->setDecimals(0);
    box9->setGeometry(840, 408, 70, 26);
    QLabel *texte12 = new QLabel("Nombre de réacteurs", contentContainer);
    texte12->setFont(labelFont);
    texte12->setGeometry(918, 408, 175, 26);

    // Connect spinbox signals
    connect(box1bis, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::resultat);
    connect(box1,    QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::resultat);
    connect(box2,    QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::resultat);
    connect(box3,    QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::resultat);
    connect(box4,    QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::resultat);
    connect(box5,    QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::resultat);
    connect(box6,    QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::resultat);
    connect(box7,    QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::resultat);
    connect(box8,    QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::resultat);
    connect(box9,    QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::resultat);

    // Result display label
    m_resultat = new QLabel(contentContainer);
    m_resultat->setGeometry(0, 525, 830, 32);
    m_resultat->setFrameStyle(QFrame::Box | QFrame::Raised);
    m_resultat->setFont(QFont("Segoe UI", 12, QFont::Bold));
    m_resultat->setAlignment(Qt::AlignCenter);
    m_resultat->setLineWidth(2);

    resultat(0.0);

    setCentralWidget(zoneCentrale);
}

MainWindow::~MainWindow()
{
}

void MainWindow::toggleFullScreen()
{
    if (isFullScreen()) {
        showNormal();
        if (actionPleinEcran) actionPleinEcran->setChecked(false);
    } else {
        showFullScreen();
        if (actionPleinEcran) actionPleinEcran->setChecked(true);
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F11) {
        toggleFullScreen();
        event->accept();
    } else if (event->key() == Qt::Key_Escape && isFullScreen()) {
        toggleFullScreen();
        event->accept();
    } else {
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if (contentContainer && centralWidget()) {
        int cx = (centralWidget()->width() - contentContainer->width()) / 2;
        int cy = (centralWidget()->height() - contentContainer->height()) / 2;
        contentContainer->move(qMax(0, cx), qMax(0, cy));
    }
}

void MainWindow::resultat(double)
{
    // 1. Éolien : Betz limit 16/27, factor 0.70, air density 1.23 kg/m3
    double radius = box1->value() / 2.0;
    double surface_rotor = 3.14159265359 * radius * radius;
    double wind_speed_cubed = std::pow(box2->value(), 3);
    double puissance_eolienne_w = 0.70 * (16.0 / 27.0) * 0.5 * 1.23 * surface_rotor * wind_speed_cubed * box1bis->value();
    double prod_eolienne_wh = puissance_eolienne_w * box3->value();

    // 2. Solaire : E = S * r * H * Cp (en Wh)
    double prod_solaire_wh = box4->value() * (box5->value() / 100.0) * box6->value() * box7->value() * 1000.0;

    // 3. Nucléaire : 24 * 365.25 * 0.85 = 7451.1 h/an
    double prod_nucleaire_wh = box8->value() * box9->value() * 1000000.0 * 365.25 * 24.0 * 0.85;

    double resultat_final = prod_eolienne_wh + prod_solaire_wh + prod_nucleaire_wh;

    QString texte;
    if (resultat_final < 1000.0) {
        texte = QString("Production annuelle cumulée : %1 Wh").arg(resultat_final, 0, 'f', 1);
    } else if (resultat_final < 1e6) {
        texte = QString("Production annuelle cumulée : %1 kWh").arg(resultat_final / 1e3, 0, 'f', 2);
    } else if (resultat_final < 1e9) {
        texte = QString("Production annuelle cumulée : %1 MWh").arg(resultat_final / 1e6, 0, 'f', 2);
    } else if (resultat_final < 1e12) {
        texte = QString("Production annuelle cumulée : %1 GWh").arg(resultat_final / 1e9, 0, 'f', 2);
    } else {
        texte = QString("Production annuelle cumulée : %1 TWh").arg(resultat_final / 1e12, 0, 'f', 3);
    }

    m_resultat->setText(texte);
}

void MainWindow::exportation()
{
    QString chemin = QFileDialog::getSaveFileName(
        this,
        "Enregistrer les paramètres de simulation",
        "simulation.txt",
        "Fichiers texte (*.txt);;Tous les fichiers (*.*)"
    );

    if (chemin.isEmpty()) {
        return;
    }

    QFile file(chemin);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Erreur", "Impossible d'écrire dans le fichier sélectionné.");
        return;
    }

    QTextStream flux(&file);
    flux.setEncoding(QStringConverter::Utf8);

    flux << box1bis->value() << "\n"
         << box1->value()    << "\n"
         << box2->value()    << "\n"
         << box3->value()    << "\n"
         << box4->value()    << "\n"
         << box5->value()    << "\n"
         << box6->value()    << "\n"
         << box7->value()    << "\n"
         << box8->value()    << "\n"
         << box9->value()    << "\n";

    file.close();
    QMessageBox::information(this, "Sauvegarde réussie", "Les paramètres de la simulation ont été enregistrés avec succès.");
}

void MainWindow::importation()
{
    QString chemin = QFileDialog::getOpenFileName(
        this,
        "Charger des paramètres de simulation",
        "",
        "Fichiers texte (*.txt);;Tous les fichiers (*.*)"
    );

    if (chemin.isEmpty()) {
        return;
    }

    QFile file(chemin);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Erreur", "Impossible de lire le fichier sélectionné.");
        return;
    }

    QTextStream flux(&file);
    flux.setEncoding(QStringConverter::Utf8);

    double recup[10] = {0.0};
    int count = 0;

    while (!flux.atEnd() && count < 10) {
        QString line = flux.readLine().trimmed();
        if (!line.isEmpty()) {
            bool ok = false;
            double val = line.toDouble(&ok);
            if (ok) {
                recup[count++] = val;
            }
        }
    }
    file.close();

    if (count == 10) {
        box1bis->setValue(recup[0]);
        box1->setValue(recup[1]);
        box2->setValue(recup[2]);
        box3->setValue(recup[3]);
        box4->setValue(recup[4]);
        box5->setValue(recup[5]);
        box6->setValue(recup[6]);
        box7->setValue(recup[7]);
        box8->setValue(recup[8]);
        box9->setValue(recup[9]);

        resultat(0.0);
        QMessageBox::information(this, "Importation réussie", "Les 10 paramètres de simulation ont été chargés avec succès.");
    } else {
        QMessageBox::warning(
            this,
            "Fichier invalide",
            QString("Le fichier doit contenir 10 valeurs numériques (seules %1 ont été lues).").arg(count)
        );
    }
}

void MainWindow::eolien()
{
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Énergie Éolienne - Potentiel et caractéristiques");
    dialog->setMinimumSize(850, 720);

    QVBoxLayout *layout = new QVBoxLayout(dialog);

    QLabel *titre = new QLabel(
        "<div style='background: #eef4fc; padding: 12px; border-radius: 6px; text-align: center; font-size: 16px; font-weight: bold; color: #103060;'>"
        "Potentiel éolien en France & Évolution de la taille des aérogénérateurs"
        "</div>",
        dialog
    );
    layout->addWidget(titre);

    QHBoxLayout *imgLayout = new QHBoxLayout();

    QLabel *imgPotentiel = new QLabel(dialog);
    imgPotentiel->setPixmap(QPixmap(getAssetPath("potentiel.png")).scaled(400, 520, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    imgPotentiel->setAlignment(Qt::AlignCenter);
    imgLayout->addWidget(imgPotentiel);

    QVBoxLayout *sideLayout = new QVBoxLayout();

    QLabel *imgTaille = new QLabel(dialog);
    imgTaille->setPixmap(QPixmap(getAssetPath("eoliennes_taille.png")).scaled(400, 260, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    imgTaille->setAlignment(Qt::AlignCenter);
    sideLayout->addWidget(imgTaille);

    QLabel *imgCourbe = new QLabel(dialog);
    imgCourbe->setPixmap(QPixmap(getAssetPath("Eo_puiss.png")).scaled(400, 240, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    imgCourbe->setAlignment(Qt::AlignCenter);
    sideLayout->addWidget(imgCourbe);

    imgLayout->addLayout(sideLayout);
    layout->addLayout(imgLayout);

    dialog->setLayout(layout);
    dialog->exec();
}

void MainWindow::photov()
{
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Énergie Solaire Photovoltaïque - Ensoleillement");
    dialog->setMinimumSize(700, 650);

    QVBoxLayout *layout = new QVBoxLayout(dialog);

    QLabel *titre = new QLabel(
        "<div style='background: #eef7f2; padding: 12px; border-radius: 6px; text-align: center; font-size: 16px; font-weight: bold; color: #0d4a27;'>"
        "Carte de l'irradiation solaire globale annuelle en France (kWh / m² / an)"
        "</div>",
        dialog
    );
    layout->addWidget(titre);

    QLabel *imgStats = new QLabel(dialog);
    imgStats->setPixmap(QPixmap(getAssetPath("stats.png")).scaled(550, 530, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    imgStats->setAlignment(Qt::AlignCenter);
    layout->addWidget(imgStats);

    QLabel *desc = new QLabel(
        "<p style='color: #404040; font-size: 11px; text-align: center;'>"
        "Formule de calcul : <em>E = S × r × H × Cp</em> (avec <em>S</em> en m², <em>r</em> rendement, <em>H</em> ensoleillement, <em>Cp</em> performance)."
        "</p>",
        dialog
    );
    layout->addWidget(desc);

    dialog->setLayout(layout);
    dialog->exec();
}

void MainWindow::nuclo()
{
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Énergie Nucléaire - Répartition du parc en France");
    dialog->setMinimumSize(750, 750);

    QVBoxLayout *layout = new QVBoxLayout(dialog);

    QLabel *titre = new QLabel(
        "<div style='background: #fdf5f0; padding: 12px; border-radius: 6px; text-align: center; font-size: 16px; font-weight: bold; color: #612809;'>"
        "Carte du parc électronucléaire français (19 centrales nucléaires)"
        "</div>",
        dialog
    );
    layout->addWidget(titre);

    QLabel *imgNuclo = new QLabel(dialog);
    imgNuclo->setPixmap(QPixmap(getAssetPath("Nucléaire.png")).scaled(650, 650, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    imgNuclo->setAlignment(Qt::AlignCenter);
    layout->addWidget(imgNuclo);

    dialog->setLayout(layout);
    dialog->exec();
}

void MainWindow::interactif()
{
    QMessageBox::information(
        this,
        "Mode Interactif",
        "Cliquez directement sur l'image paysagère pour découvrir chaque source d'énergie :\n\n"
        " • Éoliennes : conversion de l'énergie cinétique du vent.\n"
        " • Centrale nucléaire : réacteur thermique à fission d'uranium.\n"
        " • Panneaux photovoltaïques : captage de l'énergie solaire.\n"
        " • Centrale thermique : combustion de gaz, charbon ou fioul.\n"
        " • Géothermie : exploitation de la chaleur du sous-sol terrestre.\n"
        " • Hydroélectricité : énergie motrice des cours d'eau et barrages.\n"
        " • Biomasse : valorisation énergétique des matières organiques."
    );
}

void MainWindow::aide()
{
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Crédits & Informations");
    dialog->setFixedSize(880, 600);

    QVBoxLayout *layout = new QVBoxLayout(dialog);

    QLabel *image = new QLabel(dialog);
    image->setPixmap(QPixmap(getAssetPath("credits3.png")));
    image->setAlignment(Qt::AlignCenter);
    layout->addWidget(image);

    dialog->setLayout(layout);
    dialog->exec();
}

void MainWindow::lancerPresentationSDL()
{
    launchSdlPresentation();
}
