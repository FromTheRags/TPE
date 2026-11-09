#include "mainwindow.h"
#include "utils.h"
#include "sdl_presentation.h"

#include <QApplication>
#include <QMenuBar>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextBrowser>
#include <QWhatsThis>
#include <QFile>
#include <QTextStream>
#include <cmath>

ClickableAreaWidget::ClickableAreaWidget(QWidget *parent)
    : QWidget(parent)
{
    setCursor(Qt::PointingHandCursor);
}

void ClickableAreaWidget::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    emit clicked();
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setFixedSize(1100, 580);
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

    // --- Central Widget ---
    QWidget *zoneCentrale = new QWidget(this);

    // Background Image
    QLabel *image = new QLabel(zoneCentrale);
    image->setGeometry(0, 0, 830, 528);
    image->setPixmap(QPixmap(getAssetPath("ENR5.png")));
    image->setScaledContents(true);

    // Interactive hotspot areas overlaid on ENR5.png
    for (int i = 0; i < 7; ++i) {
        tool[i] = new ClickableAreaWidget(zoneCentrale);
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
    QLabel *texte1 = new QLabel("<u>Énergie éolienne</u> :", zoneCentrale);
    texte1->setFont(headerFont);
    texte1->setGeometry(840, 25, 250, 30);

    box1bis = new QDoubleSpinBox(zoneCentrale);
    box1bis->setRange(0, 100000);
    box1bis->setValue(100);
    box1bis->setDecimals(0);
    box1bis->setGeometry(840, 58, 70, 26);
    QLabel *texte1bis = new QLabel("Nombre d'éoliennes", zoneCentrale);
    texte1bis->setFont(labelFont);
    texte1bis->setGeometry(918, 58, 175, 26);

    box1 = new QDoubleSpinBox(zoneCentrale);
    box1->setRange(0, 200);
    box1->setValue(80);
    box1->setDecimals(0);
    box1->setGeometry(840, 88, 70, 26);
    QLabel *texte2 = new QLabel("Diamètre du rotor (en m)", zoneCentrale);
    texte2->setFont(labelFont);
    texte2->setGeometry(918, 88, 175, 26);

    box2 = new QDoubleSpinBox(zoneCentrale);
    box2->setRange(0, 60);
    box2->setValue(8.0);
    box2->setDecimals(1);
    box2->setGeometry(840, 118, 70, 26);
    QLabel *texte3 = new QLabel("Vitesse moyenne du vent (m/s)", zoneCentrale);
    texte3->setFont(labelFont);
    texte3->setGeometry(918, 118, 175, 26);

    box3 = new QDoubleSpinBox(zoneCentrale);
    box3->setRange(0, 8766);
    box3->setValue(6000);
    box3->setDecimals(0);
    box3->setGeometry(840, 148, 70, 26);
    QLabel *texte4 = new QLabel("Heures de fonctionnement / an", zoneCentrale);
    texte4->setFont(labelFont);
    texte4->setGeometry(918, 148, 175, 26);

    // Section 2: Solaire
    QLabel *texte5 = new QLabel("<u>Énergie solaire</u> :", zoneCentrale);
    texte5->setFont(headerFont);
    texte5->setGeometry(840, 185, 250, 30);

    box4 = new QDoubleSpinBox(zoneCentrale);
    box4->setRange(0, 100000000);
    box4->setValue(50000);
    box4->setDecimals(0);
    box4->setGeometry(840, 218, 70, 26);
    QLabel *texte6 = new QLabel("Surface des panneaux (en m²)", zoneCentrale);
    texte6->setFont(labelFont);
    texte6->setGeometry(918, 218, 175, 26);

    box5 = new QDoubleSpinBox(zoneCentrale);
    box5->setRange(0, 50);
    box5->setValue(16);
    box5->setDecimals(0);
    box5->setGeometry(840, 248, 70, 26);
    QLabel *texte7 = new QLabel("Rendement du module (%)", zoneCentrale);
    texte7->setFont(labelFont);
    texte7->setGeometry(918, 248, 175, 26);

    box6 = new QDoubleSpinBox(zoneCentrale);
    box6->setRange(0, 3000);
    box6->setValue(1500);
    box6->setDecimals(0);
    box6->setGeometry(840, 278, 70, 26);
    QLabel *texte8 = new QLabel("Ensoleillement (kWh/m²/an)", zoneCentrale);
    texte8->setFont(labelFont);
    texte8->setGeometry(918, 278, 175, 26);

    box7 = new QDoubleSpinBox(zoneCentrale);
    box7->setRange(0.01, 1.0);
    box7->setSingleStep(0.01);
    box7->setValue(0.75);
    box7->setDecimals(2);
    box7->setGeometry(840, 308, 70, 26);
    QLabel *texte9 = new QLabel("Coefficient de performance (Cp)", zoneCentrale);
    texte9->setFont(labelFont);
    texte9->setGeometry(918, 308, 175, 26);

    // Section 3: Nucléaire
    QLabel *texte10 = new QLabel("<u>Énergie nucléaire</u> :", zoneCentrale);
    texte10->setFont(headerFont);
    texte10->setGeometry(840, 345, 250, 30);

    box8 = new QDoubleSpinBox(zoneCentrale);
    box8->setRange(1, 3000);
    box8->setValue(1000);
    box8->setDecimals(0);
    box8->setGeometry(840, 378, 70, 26);
    QLabel *texte11 = new QLabel("Puissance moyenne (MW)", zoneCentrale);
    texte11->setFont(labelFont);
    texte11->setGeometry(918, 378, 175, 26);

    box9 = new QDoubleSpinBox(zoneCentrale);
    box9->setRange(0, 100);
    box9->setValue(58);
    box9->setDecimals(0);
    box9->setGeometry(840, 408, 70, 26);
    QLabel *texte12 = new QLabel("Nombre de réacteurs", zoneCentrale);
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
    m_resultat = new QLabel(zoneCentrale);
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
    QString fichier = QFileDialog::getSaveFileName(this, "Enregistrer la simulation", QString(), "Fichier texte (*.txt)");
    if (fichier.isEmpty()) {
        return;
    }

    QFile file(fichier);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Erreur", "Impossible de créer le fichier :\n" + fichier);
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    out << "\n Énergie éolienne : \n\n"
        << " Nombre d'éoliennes : " << QString::number(box1bis->value(), 'f', 0) << " \n"
        << " Diamètre du rotor (en m) : " << QString::number(box1->value(), 'f', 0) << " \n"
        << " Vitesse moyenne du vent (en m/s) : " << QString::number(box2->value(), 'f', 1) << " \n"
        << " Temps de fonctionnement (en heures) : " << QString::number(box3->value(), 'f', 0) << " \n\n"
        << " Énergie solaire : \n\n"
        << " Surface du champ photovoltaïque (en m²) : " << QString::number(box4->value(), 'f', 2) << " \n"
        << " Rendement du module (en %) : " << QString::number(box5->value(), 'f', 0) << " \n"
        << " Ensoleillement/rayonnement sur la surface inclinée en Kwh/m² : " << QString::number(box6->value(), 'f', 0) << " \n"
        << " Coefficient de perte : " << QString::number(box7->value(), 'f', 2) << " \n\n"
        << " Énergie nucléaire : \n\n"
        << " Puissance moyenne des réacteurs (en MW) : " << QString::number(box8->value(), 'f', 0) << " \n"
        << " Nombre de réacteurs : " << QString::number(box9->value(), 'f', 0) << " \n\n"
        << " " << m_resultat->text() << " \n";

    file.close();
    QMessageBox::information(this, "Sauvegarde réussie", "Simulation enregistrée avec succès dans :\n" + fichier);
}

void MainWindow::importation()
{
    QString fichier = QFileDialog::getOpenFileName(this, "Ouvrir une simulation", QString(), "Fichier texte (*.txt)");
    if (fichier.isEmpty()) {
        return;
    }

    QFile file(fichier);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Erreur", "Impossible d'ouvrir le fichier :\n" + fichier);
        return;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);

    double recup[10] = {0};
    int valIndex = 0;

    while (!in.atEnd() && valIndex < 10) {
        QString line = in.readLine();
        int colonIdx = line.indexOf(':');
        if (colonIdx != -1) {
            QString valStr = line.mid(colonIdx + 1).trimmed();
            // Extract the first numeric token
            QStringList tokens = valStr.split(' ', Qt::SkipEmptyParts);
            if (!tokens.isEmpty()) {
                bool ok = false;
                double val = tokens.first().toDouble(&ok);
                if (ok) {
                    recup[valIndex++] = val;
                }
            }
        }
    }
    file.close();

    if (valIndex >= 9) {
        box1bis->setValue(recup[0]);
        box1->setValue(recup[1]);
        box2->setValue(recup[2]);
        box3->setValue(recup[3]);
        box4->setValue(recup[4]);
        box5->setValue(recup[5]);
        box6->setValue(recup[6]);
        box7->setValue(recup[7]);
        box8->setValue(recup[8]);
        if (valIndex >= 10) {
            box9->setValue(recup[9]);
        }
        resultat(0.0);
        QMessageBox::information(this, "Importation", "Simulation chargée avec succès !");
    } else {
        QMessageBox::warning(this, "Format invalide", "Le fichier ne contient pas les données attendues pour la simulation.");
    }
}

void MainWindow::interactif()
{
    if (QWhatsThis::inWhatsThisMode()) {
        QWhatsThis::leaveWhatsThisMode();
    } else {
        QWhatsThis::enterWhatsThisMode();
    }
}

void MainWindow::photov()
{
    QTextBrowser *browser = new QTextBrowser();
    browser->setAttribute(Qt::WA_DeleteOnClose);
    browser->setWindowIcon(QIcon(getAssetPath("Photo.png")));
    browser->setWindowTitle("Panneaux Photovoltaïques - Principes & Rendements");
    browser->setReadOnly(true);
    browser->setOpenExternalLinks(true);

    QString statsImg = getAssetPath("stats.png");

    QString html = QString(
        "<div style='font-family: Segoe UI, sans-serif; padding: 15px;'>"
        "<h2 style='color: #1a4a8a; text-align: center; border-bottom: 2px solid #1a4a8a; padding-bottom: 8px;'>Panneaux Solaires Photovoltaïques</h2>"
        "<p>Ce module calcule la production électrique annuelle en utilisant la formule standard de dimensionnement :</p>"
        "<div style='background: #eef4fc; padding: 12px; border-radius: 6px; text-align: center; font-size: 16px; font-weight: bold; color: #103060;'>"
        "E = S × r × H × Cp"
        "</div>"
        "<ul>"
        "<li><b>E</b> : Énergie produite annuellement (en kWh)</li>"
        "<li><b>S</b> : Surface totale du champ photovoltaïque (en m²)</li>"
        "<li><b>r</b> : Rendement moyen des modules (typiquement 14 % à 22 %)</li>"
        "<li><b>H</b> : Ensoleillement global annuel sur plan incliné (en kWh/m²/an, de 1100 dans le Nord à plus de 1500 dans le Sud)</li>"
        "<li><b>Cp</b> : Facteur de performance global (généralement compris entre 0.75 et 0.82)</li>"
        "</ul>"
        "<p><b>Pertes prises en compte dans le coefficient Cp :</b></p>"
        "<ul>"
        "<li>Pertes onduleur : 8 % à 15 %</li>"
        "<li>Pertes thermiques (échauffement des cellules) : 5 % à 12 %</li>"
        "<li>Pertes câbles, connectiques et salissures : 3 % à 5 %</li>"
        "</ul>"
        "<div style='text-align: center; margin-top: 20px;'>"
        "<img src='%1' width='500' style='border: 1px solid #ccc; border-radius: 6px;'/>"
        "</div>"
        "</div>"
    ).arg(statsImg);

    browser->setHtml(html);
    browser->resize(850, 700);
    browser->show();
}

void MainWindow::eolien()
{
    QTextBrowser *browser = new QTextBrowser();
    browser->setAttribute(Qt::WA_DeleteOnClose);
    browser->setWindowIcon(QIcon(getAssetPath("eolienne_fixe.png")));
    browser->setWindowTitle("Énergie Éolienne - Théorie & Potentiel");
    browser->setReadOnly(true);
    browser->setOpenExternalLinks(true);

    QString potImg = getAssetPath("potentiel.png");
    QString tailImg = getAssetPath("eoliennes_taille.png");
    QString puisImg = getAssetPath("Eo_puiss.png");

    QString html = QString(
        "<div style='font-family: Segoe UI, sans-serif; padding: 15px;'>"
        "<h2 style='color: #1a6b3e; text-align: center; border-bottom: 2px solid #1a6b3e; padding-bottom: 8px;'>L'Énergie Éolienne</h2>"
        "<p>Le rendement théorique maximal d'un aérogénérateur est fixé par la <b>limite de Betz</b> à 16/27 (soit environ 59,3 % de l'énergie cinétique du flux d'air).</p>"
        "<p>En tenant compte des pertes aérodynamiques, mécaniques (multiplicateur) et électriques (génératrice, onduleur, transformateur), on considère 70 % de la limite de Betz :</p>"
        "<div style='background: #eef7f2; padding: 12px; border-radius: 6px; text-align: center; font-size: 16px; font-weight: bold; color: #0d4a27;'>"
        "P = 0.70 × (16/27) × 0.5 × ρ × S × v³ × n"
        "</div>"
        "<ul>"
        "<li><b>P</b> : Puissance instantanée moyenne (en W)</li>"
        "<li><b>ρ</b> : Masse volumique de l'air sec (1.23 kg/m³ à 15 °C et 1.013 bar)</li>"
        "<li><b>S</b> : Surface balayée par le rotor (S = π × r²)</li>"
        "<li><b>v</b> : Vitesse moyenne du vent (m/s) – <i>attention à la dépendance au cube !</i></li>"
        "<li><b>n</b> : Nombre d'éoliennes dans le parc</li>"
        "</ul>"
        "<div style='text-align: center; margin: 20px 0;'>"
        "<h4>Courbe de puissance typique :</h4>"
        "<img src='%1' width='450' style='border: 1px solid #ccc; border-radius: 6px;'/><br/><br/>"
        "<h4>Évolution des dimensions et de la puissance :</h4>"
        "<img src='%2' width='750' style='border: 1px solid #ccc; border-radius: 6px;'/><br/><br/>"
        "<h4>Potentiel éolien français :</h4>"
        "<img src='%3' width='500' style='border: 1px solid #ccc; border-radius: 6px;'/>"
        "</div>"
        "</div>"
    ).arg(puisImg, tailImg, potImg);

    browser->setHtml(html);
    browser->resize(900, 750);
    browser->show();
}

void MainWindow::nuclo()
{
    QTextBrowser *browser = new QTextBrowser();
    browser->setAttribute(Qt::WA_DeleteOnClose);
    browser->setWindowIcon(QIcon(getAssetPath("nuclo.png")));
    browser->setWindowTitle("Énergie Nucléaire - Fission & Disponibilité");
    browser->setReadOnly(true);
    browser->setOpenExternalLinks(true);

    QString nucImg = getAssetPath("Nucléaire.png");

    QString html = QString(
        "<div style='font-family: Segoe UI, sans-serif; padding: 15px;'>"
        "<h2 style='color: #8c3b10; text-align: center; border-bottom: 2px solid #8c3b10; padding-bottom: 8px;'>L'Énergie Nucléaire</h2>"
        "<p>L'électricité d'origine nucléaire constitue la base de la production électrique française. Elle est pilotable et quasiment décarbonée en phase d'exploitation.</p>"
        "<div style='background: #fdf5f0; padding: 12px; border-radius: 6px; text-align: center; font-size: 16px; font-weight: bold; color: #612809;'>"
        "E = P × n × 7451.1"
        "</div>"
        "<ul>"
        "<li><b>E</b> : Énergie annuelle produite (en MWh)</li>"
        "<li><b>P</b> : Puissance électrique nominale unitaire (en MW)</li>"
        "<li><b>n</b> : Nombre de réacteurs en service</li>"
        "<li><b>7451.1 h</b> : Nombre d'heures annuelles équivalentes à pleine puissance, basé sur un facteur de disponibilité moyen de 85 % (24 × 365.25 × 0.85).</li>"
        "</ul>"
        "<div style='text-align: center; margin-top: 20px;'>"
        "<h4>Cartographie du parc électronucléaire français :</h4>"
        "<img src='%1' width='700' style='border: 1px solid #ccc; border-radius: 6px;'/>"
        "</div>"
        "</div>"
    ).arg(nucImg);

    browser->setHtml(html);
    browser->resize(850, 750);
    browser->show();
}

void MainWindow::aide()
{
    QLabel *image = new QLabel();
    image->setAttribute(Qt::WA_DeleteOnClose);
    image->setWindowTitle("À propos - TPE Simulateur");
    image->setWindowIcon(QIcon(getAssetPath("icone.png")));
    image->setPixmap(QPixmap(getAssetPath("credits3.png")));
    image->resize(878, 572);
    image->show();
}

void MainWindow::lancerPresentationSDL()
{
    launchSdlPresentation();
}
