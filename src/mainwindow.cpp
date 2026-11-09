#include "mainwindow.h"
#include "utils.h"

#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QTextBrowser>
#include <QIcon>
#include <QPixmap>
#include <QFont>
#include <QDir>
#include <QUrl>
#include <QToolTip>
#include <QCursor>
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
    if (!toolTip().isEmpty()) {
        QToolTip::showText(QCursor::pos(), toolTip(), this);
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , contentContainer(nullptr)
{
    setFixedSize(1100, 580);
    setWindowTitle("TPE");

    QString iconPath = getAssetPath("icone.png");
    if (!iconPath.isEmpty()) {
        setWindowIcon(QIcon(iconPath));
    }

    // Configure image search paths so QTextBrowser can resolve images
    QFileInfo iconInfo(iconPath);
    if (iconInfo.exists()) {
        QDir::addSearchPath("images", iconInfo.absolutePath());
    }

    // --- Menu Fichiers ---
    QMenu *menuFichier = menuBar()->addMenu("&Fichiers");

    QAction *ouvrir = new QAction("&Ouvrir", this);
    ouvrir->setShortcut(QKeySequence("Ctrl+O"));
    ouvrir->setIcon(QIcon(getAssetPath("fileopen.png")));
    connect(ouvrir, &QAction::triggered, this, &MainWindow::importation);
    menuFichier->addAction(ouvrir);

    QAction *sauvegarder = new QAction("&Sauvegarder en .txt", this);
    sauvegarder->setShortcut(QKeySequence("Ctrl+S"));
    sauvegarder->setIcon(QIcon(getAssetPath("icone_save.png")));
    connect(sauvegarder, &QAction::triggered, this, &MainWindow::exportation);
    menuFichier->addAction(sauvegarder);

    menuFichier->addSeparator();

    QAction *actionQuitter = new QAction("&Quitter", this);
    actionQuitter->setShortcut(QKeySequence("Ctrl+Q"));
    actionQuitter->setIcon(QIcon(getAssetPath("icone_quit.png")));
    connect(actionQuitter, &QAction::triggered, qApp, &QApplication::quit);
    menuFichier->addAction(actionQuitter);

    // --- Menu Calculs ---
    QMenu *menuCalcul = menuBar()->addMenu("&Calculs");

    QAction *actionCalcul = new QAction("&Éolien", this);
    actionCalcul->setIcon(QIcon(getAssetPath("eolienne_fixe.png")));
    connect(actionCalcul, &QAction::triggered, this, &MainWindow::eolien);
    menuCalcul->addAction(actionCalcul);

    QAction *actionCalcul1 = new QAction("&Photovolaïque", this);
    actionCalcul1->setIcon(QIcon(getAssetPath("Photo.png")));
    connect(actionCalcul1, &QAction::triggered, this, &MainWindow::photov);
    menuCalcul->addAction(actionCalcul1);

    QAction *actionCalcul2 = new QAction("&Nucléaire", this);
    actionCalcul2->setIcon(QIcon(getAssetPath("nuclo.png")));
    connect(actionCalcul2, &QAction::triggered, this, &MainWindow::nuclo);
    menuCalcul->addAction(actionCalcul2);

    // --- Aide / Crédits (?) ---
    QAction *menuAide = menuBar()->addAction("?");
    connect(menuAide, &QAction::triggered, this, &MainWindow::aide);

    // --- Central Widget & Layout ---
    QWidget *zoneCentrale = new QWidget(this);

    contentContainer = new QWidget(zoneCentrale);
    contentContainer->setGeometry(0, 0, 1100, 560);

    // Background Image (authentic ENR5.png, exactly 830x528)
    QLabel *image = new QLabel(contentContainer);
    image->setGeometry(0, 0, 830, 528);
    image->setPixmap(QPixmap(getAssetPath("ENR5.png")));

    // 7 Hotspot Areas overlaid on the landscape
    for (int i = 0; i < 7; ++i) {
        tool[i] = new ClickableAreaWidget(contentContainer);
    }

    // 0: Éoliennes
    tool[0]->setGeometry(201, 120, 216, 115);
    tool[0]->setToolTip(
        "<b>Éoliennes</b><br>"
        "Les éoliennes utilisent la force du vent qui actionne leurs hélices et un alternateur pour créer de l'électricité."
    );

    // 1: Centrale nucléaire
    tool[1]->setGeometry(352, 264, 206, 125);
    tool[1]->setToolTip(
        "<b>Centrale nucléaire</b><br>"
        "La centrale nucléaire utilise le procédé de fission des atomes d’uranium. "
        "Cela produit de la chaleur qui transforme ensuite de l'eau en vapeur et fait tourner la turbine (production d'électricité)."
    );

    // 2: Panneaux photovoltaïques
    tool[2]->setGeometry(557, 377, 272, 113);
    tool[2]->setToolTip(
        "<b>Panneaux photovoltaïques</b><br>"
        "Les panneaux photovoltaïques produisent de l’électricité grâce à l'énergie solaire. "
        "C'est aussi un moyen pratique pour alimenter des lieux isolés du réseau électrique.<br>"
        "Il existe trois types de panneaux : à cellules amorphes, à cellules polycristallines ou à cellules monocristallines."
    );

    // 3: Centrale thermique
    tool[3]->setGeometry(0, 190, 183, 140);
    tool[3]->setToolTip(
        "<b>Centrale thermique</b> <i>(à titre indicatif)</i><br>"
        "La centrale thermique utilise la force de la vapeur dégagée en brûlant des énergies fossiles "
        "(charbon, pétrole, gaz naturel) ou de la biomasse (déchets végétaux ou ménagers)."
    );

    // 4: Centrale géothermique
    tool[4]->setGeometry(134, 412, 228, 116);
    tool[4]->setToolTip(
        "<b>Centrale géothermique</b> <i>(à titre indicatif)</i><br>"
        "La centrale géothermique utilise la chaleur de la Terre pour chauffer des foyers ou créer de l'électricité."
    );

    // 5: Centrale hydroélectrique
    tool[5]->setGeometry(644, 125, 186, 101);
    tool[5]->setToolTip(
        "<b>Centrale hydroélectrique</b> <i>(à titre indicatif)</i><br>"
        "La centrale hydroélectrique utilise la force de l’eau, créée par un puissant déplacement, pour produire de l'électricité. "
        "Il peut s’agir d’une chute d’eau naturelle, de l’eau stockée dans un barrage ou des mouvements de la marée."
    );

    // 6: Biomasse
    tool[6]->setGeometry(644, 226, 186, 108);
    tool[6]->setToolTip(
        "<b>Biomasse</b> <i>(à titre indicatif)</i><br>"
        "La biomasse est une énergie constituée de matières organiques végétales ou animales (déchets ménagers ou agricoles). "
        "Sa combustion permet de produire de l’électricité."
    );

    for (int i = 0; i < 7; ++i) {
        tool[i]->show();
    }

    // --- Right Parameter Panel: Authentic 2014 Layout ---
    QFont fontEolien("Calibri", 15, QFont::Bold);
    QFont fontSection("Calibri", 14, QFont::Bold);
    QFont fontLabel("Segoe UI", 8);

    // 1. Énergie éolienne
    QLabel *texte1 = new QLabel(contentContainer);
    texte1->setText("<u>Énergie éolienne</u> :");
    texte1->setFont(fontEolien);
    texte1->setGeometry(850, 30, 240, 30);

    box1bis = new QDoubleSpinBox(contentContainer);
    box1bis->setMinimum(0);
    box1bis->setMaximum(100000);
    box1bis->setValue(100);
    box1bis->setDecimals(0);
    box1bis->setGeometry(840, 60, 70, 26);
    QLabel *texte1bis = new QLabel(contentContainer);
    texte1bis->setText("Nombre d'éoliennes");
    texte1bis->setFont(fontLabel);
    texte1bis->setGeometry(918, 60, 175, 26);

    box1 = new QDoubleSpinBox(contentContainer);
    box1->setMinimum(0);
    box1->setMaximum(200);
    box1->setValue(80);
    box1->setDecimals(0);
    box1->setGeometry(840, 90, 70, 26);
    QLabel *texte2 = new QLabel(contentContainer);
    texte2->setText("Diamètre du rotor (en m)");
    texte2->setFont(fontLabel);
    texte2->setGeometry(918, 90, 175, 26);

    box2 = new QDoubleSpinBox(contentContainer);
    box2->setMinimum(0);
    box2->setMaximum(60);
    box2->setValue(8.0);
    box2->setDecimals(1);
    box2->setGeometry(840, 120, 70, 26);
    QLabel *texte3 = new QLabel(contentContainer);
    texte3->setText("Vitesse moyenne du vent (en m/s)");
    texte3->setFont(fontLabel);
    texte3->setGeometry(918, 120, 175, 26);

    box3 = new QDoubleSpinBox(contentContainer);
    box3->setMinimum(0);
    box3->setMaximum(8766);
    box3->setValue(6000);
    box3->setDecimals(0);
    box3->setGeometry(840, 150, 70, 26);
    QLabel *texte4 = new QLabel(contentContainer);
    texte4->setText("Temps de fonctionnement<br> (en heures)");
    texte4->setFont(fontLabel);
    texte4->setGeometry(918, 147, 175, 32);

    // 2. Énergie solaire
    QLabel *texte5 = new QLabel(contentContainer);
    texte5->setText("<u>Énergie solaire</u> :");
    texte5->setFont(fontSection);
    texte5->setGeometry(850, 195, 240, 30);

    box4 = new QDoubleSpinBox(contentContainer);
    box4->setMinimum(0);
    box4->setMaximum(100000000);
    box4->setValue(50000);
    box4->setDecimals(0);
    box4->setGeometry(840, 225, 70, 26);
    QLabel *texte6 = new QLabel(contentContainer);
    texte6->setText("Surface du champ <br>photovoltaïque (en m<sup>2</sup>)");
    texte6->setFont(fontLabel);
    texte6->setGeometry(918, 222, 175, 32);

    box5 = new QDoubleSpinBox(contentContainer);
    box5->setRange(0, 50);
    box5->setValue(14);
    box5->setDecimals(0);
    box5->setGeometry(840, 260, 70, 26);
    QLabel *texte7 = new QLabel(contentContainer);
    texte7->setText("Rendement du module (en %)");
    texte7->setFont(fontLabel);
    texte7->setGeometry(918, 260, 175, 26);

    box6 = new QDoubleSpinBox(contentContainer);
    box6->setRange(0, 2500);
    box6->setValue(1500);
    box6->setDecimals(0);
    box6->setGeometry(840, 290, 70, 26);
    QLabel *texte8 = new QLabel(contentContainer);
    texte8->setText("Ensoleillement/rayonnement <br>sur la surface inclinée en kWh/m²");
    texte8->setFont(fontLabel);
    texte8->setGeometry(918, 287, 175, 32);

    box7 = new QDoubleSpinBox(contentContainer);
    box7->setRange(0.01, 0.90);
    box7->setDecimals(2);
    box7->setSingleStep(0.01);
    box7->setValue(0.75);
    box7->setGeometry(840, 325, 70, 26);
    QLabel *texte9 = new QLabel(contentContainer);
    texte9->setText("Coefficient de perte");
    texte9->setFont(fontLabel);
    texte9->setGeometry(918, 325, 175, 26);

    // 3. Énergie nucléaire
    QLabel *texte10 = new QLabel(contentContainer);
    texte10->setText("<u>Énergie nucléaire</u> :");
    texte10->setFont(fontSection);
    texte10->setGeometry(850, 370, 240, 30);

    box8 = new QDoubleSpinBox(contentContainer);
    box8->setRange(1, 2000);
    box8->setValue(1000);
    box8->setDecimals(0);
    box8->setGeometry(840, 400, 70, 26);
    QLabel *texte11 = new QLabel(contentContainer);
    texte11->setText("Puissance moyenne des<br> réacteurs(en MW)");
    texte11->setFont(fontLabel);
    texte11->setGeometry(918, 397, 175, 32);

    box9 = new QDoubleSpinBox(contentContainer);
    box9->setRange(0, 58);
    box9->setValue(58);
    box9->setDecimals(0);
    box9->setGeometry(840, 435, 70, 26);
    QLabel *texte12 = new QLabel(contentContainer);
    texte12->setText("Nombre de réacteurs");
    texte12->setFont(fontLabel);
    texte12->setGeometry(918, 435, 175, 26);

    // Connect spinbox valueChanged signals
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

    // Result Label (authentic 2014 raised bevel frame)
    m_resultat = new QLabel(contentContainer);
    m_resultat->setGeometry(0, 525, 830, 32);
    m_resultat->setFrameStyle(QFrame::Box | QFrame::Raised);
    m_resultat->setLineWidth(3);
    m_resultat->setFont(QFont("Comic Sans MS", 13, QFont::Bold));
    m_resultat->setAlignment(Qt::AlignCenter);

    resultat(1.0);

    setCentralWidget(zoneCentrale);
}

MainWindow::~MainWindow()
{
}

void MainWindow::toggleFullScreen()
{
    if (isFullScreen()) {
        showNormal();
        setFixedSize(1100, 580);
    } else {
        setMinimumSize(0, 0);
        setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        showFullScreen();
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
    // Éolien : 70% de la limite de Betz (16/27) * 0.5 * rho (1.23) * S * v^3 * n * heures
    double eolien_puiss_w = 0.70 * (16.0 / 27.0) * 0.5 * 1.230000
                          * (std::pow(box1->value() / 2.0, 2) * 3.141593)
                          * std::pow(box2->value(), 3)
                          * box1bis->value();
    double resultat_final = eolien_puiss_w * box3->value();

    // Solaire : E = S * r * H * Cp (en Wh)
    resultat_final += (box4->value() * box5->value() * ((box6->value() + 0.0) / 100.0) * box7->value()) * 1000.0;

    // Nucléaire : P * n * 1 000 000 * 365.25 * 24 * 0.85 (en Wh)
    resultat_final += box8->value() * box9->value() * 1000000.0 * 365.25 * 24.0 * 0.85;

    QString texte("");
    if (resultat_final < 1000.0) {
        texte = QString("Production annuelle : %1 Wh").arg(resultat_final, 0, 'f', 1);
    } else if (resultat_final >= 1000.0 && resultat_final < 1000000.0) {
        resultat_final /= 1000.0;
        texte = QString("Production annuelle : %1 KWh").arg(resultat_final, 0, 'f', 2);
    } else if (resultat_final >= 1000000.0 && resultat_final < 1000000000.0) {
        resultat_final /= 1e6;
        texte = QString("Production annuelle : %1 MWh").arg(resultat_final, 0, 'f', 2);
    } else if (resultat_final >= 1000000000.0 && resultat_final < 1e12) {
        resultat_final /= 1e9;
        texte = QString("Production annuelle : %1 GWh").arg(resultat_final, 0, 'f', 2);
    } else {
        resultat_final /= 1e12;
        texte = QString("Production annuelle : %1 TWh").arg(resultat_final, 0, 'f', 2);
    }
    m_resultat->setText(texte);
}

void MainWindow::exportation()
{
    QString chemin = QFileDialog::getSaveFileName(
        this,
        "Enregistrer un fichier",
        QString(),
        "Texte (*.txt)"
    );

    if (chemin.isEmpty()) {
        return;
    }

    QFile file(chemin);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::information(this, "Fichier", "Erreur fichier non créé : " + chemin);
        return;
    }

    QTextStream flux(&file);
    flux.setEncoding(QStringConverter::Utf8);

    flux << "\n Énergie éolienne : \n\n"
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
}

void MainWindow::importation()
{
    QString chemin = QFileDialog::getOpenFileName(
        this,
        "Ouvrir un fichier",
        QString(),
        "Texte (*.txt)"
    );

    if (chemin.isEmpty()) {
        return;
    }

    QFile file(chemin);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::information(this, "Fichier", "Erreur le fichier : " + chemin + " n'existe pas");
        return;
    }

    QTextStream flux(&file);
    flux.setEncoding(QStringConverter::Utf8);

    double recup[10] = {0.0};
    int count = 0;

    // Supports both the 2014 colon-separated format and raw value lines
    while (!flux.atEnd() && count < 10) {
        QString line = flux.readLine();
        if (line.contains(':')) {
            QString valStr = line.section(':', -1).trimmed().section(' ', 0, 0);
            bool ok = false;
            double val = valStr.toDouble(&ok);
            if (ok) {
                recup[count++] = val;
            }
        } else {
            QString trimmed = line.trimmed();
            if (!trimmed.isEmpty()) {
                bool ok = false;
                double val = trimmed.toDouble(&ok);
                if (ok) {
                    recup[count++] = val;
                }
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
    } else {
        QMessageBox::critical(this, "Fichier", "Erreur le fichier : " + chemin + " n'est pas un fichier de simulation valide");
    }
}

void MainWindow::interactif()
{
}

void MainWindow::eolien()
{
    QTextBrowser *eole = new QTextBrowser();
    eole->setWindowIcon(QIcon(getAssetPath("eolienne_fixe.png")));
    eole->setWindowTitle("L'énergie éolienne");
    eole->setReadOnly(true);

    QString imgPotentiel = QUrl::fromLocalFile(getAssetPath("potentiel.png")).toString();
    QString imgTaille = QUrl::fromLocalFile(getAssetPath("eoliennes_taille.png")).toString();

    QString html = QString(
        "<br><center><strong><font size=6><u>L'énergie éolienne</u> : </font></strong></center><br/>"
        "<font size=4><p>&nbsp; &nbsp; &nbsp; Le rendement maximal théorique d'une éolienne est fixé à 16/27, soit environ 59,3 % de l'énergie cinétique du vent dans un mètre cube (en effet l'éolienne n'arrête pas le vent, elle ne récupère donc pas toute l'énergie cinétique de celui-ci). Ce chiffre ne prend pas en compte les pertes d'énergie occasionnées lors de la conversion de l'énergie mécanique du vent en énergie électrique.</p><br>"
        "&nbsp; &nbsp; &nbsp; L'énergie fournie par l'aérogénérateur étant convertit d’une forme à une autre, cette limite est donc affectée par tous les rendements propres aux différentes transformations."
        "<ul style=list-style-type:disc>"
        "<li>Hélice : entre 0,20 et 0,85 </li>"
        "<li>Le multiplicateur ou le réducteur : entre 0,7 et 0,98 </li>"
        "<li>L'alternateur ou la génératrice continue : entre 0,80 et 0,98 </li>"
        "<li>Le transformateur : entre 0,85 et 0,98 </li>"
        "<li>Le redresseur : entre 0,9 et 0,98 </li>"
        "<li>Les batteries : entre 0,7 et 0,8</li>"
        "<li>Les pertes de lignes : entre 0,9 et 0,99</li>"
        "</ul>"
        "&nbsp; &nbsp; &nbsp; <p>&nbsp; &nbsp; &nbsp; Les rendements de chaque élément varient avec le régime de fonctionnement lié à la vitesse de rotation de l'hélice, ce qui en dehors du régime nominal diminue encore le rendement global du dispositif, il semble difficile de dépasser 70% de la limite de Betz.<br/>"
        "On considèrera donc pour les calculs de ce simulateur 70% de la limite de Betz (16/27), soit la formule suivante :</p><br/>"
        "<center><strong>E = (70/100) * (16/27) * (1/2) * P * S * v<sup>3</sup> * n</strong></center><br/><br>"
        "&nbsp; E = énergie produite en Wh<br/>"
        "&nbsp; P : masse volumique de l'air (air atmosphérique sec, environ : 1,23 kg/m3 à 15 °C et à pression atmosphérique 1,0132 bar)<br/>"
        "&nbsp; S : surface du capteur éolien (en m2), S = π * r²<br/>"
        "&nbsp; V : vitesse du vent (en m/s)<br/>"
        "&nbsp; n : nombre d'éoliennes</p><br>"
        "<p>&nbsp; &nbsp; &nbsp; On ne considèrera pas les pertes dûes à la multiplication des éoliennes, de même on considèrera la vitesse des vents moyenne comme représentant un vent de vitesse constante (une simplification nécessaire pour garder des calculs abordables).<br/></p>"
        "<a href='http://eolienne.f4jr.org/eolienne_etude_theorique'>Source 1</a> &nbsp; &nbsp; &nbsp; &nbsp; "
        "<a href='http://www.energieplus-lesite.be/index.php?id=16657'>Source 2</a> &nbsp; &nbsp; &nbsp; &nbsp; "
        "<a href='http://alizes.jimdo.com/monter-un-projet/calculer-la-production'>Source 3</a> &nbsp; &nbsp; &nbsp; &nbsp; "
        "<a href='http://www.eoleres.com/fr/ressources/faqs'>Source 4</a> &nbsp; &nbsp; &nbsp; &nbsp; "
        "<a href='http://www.quandonypense.be/node/101'>Source 5</a><br><br>"
        "<center>Ci-dessous une carte du potentiel éolien en France :</center><br>"
        "<center><img src='%1' alt='Potentiel éolien'></center><br>"
        "Ci-dessous une représentation de la puissance installée par taille d'éoliennes et années d'installation :<br>"
        "<center><img src='%2' alt='Taille éoliennes'></center></font>"
    ).arg(imgPotentiel, imgTaille);

    eole->setHtml(html);
    eole->setOpenExternalLinks(true);
    eole->showMaximized();
}

void MainWindow::photov()
{
    QTextBrowser *photov = new QTextBrowser();
    photov->setWindowIcon(QIcon(getAssetPath("pico.png")));
    photov->setWindowTitle("Panneaux Photovolaïque");
    photov->setReadOnly(true);

    QString imgStats = QUrl::fromLocalFile(getAssetPath("stats.png")).toString();

    QString html = QString(
        "<center><strong><font size=6><u>Panneaux Photovolaïque</u> : </font></strong></center><br/>"
        "<font size=4><p>&nbsp; &nbsp; &nbsp; Ce simulateur calcule de l'énergie produite en utilisant le rendement du panneau (ou de la cellule). Cela est fait en utilisant la formule suivante :</p><br>"
        "<center><strong>E = S * r * H * Cp</strong></center><br><br>"
        "&nbsp; E = énergie produite en kWh<br>"
        "&nbsp; S = surface du champ photovoltaïque (exemple 7 m²)<br>"
        "&nbsp; r = rendement du module (14 % par exemple)<br>"
        "&nbsp; H = ensoleillement/rayonnement sur la surface inclinée en kWh/m² (>1500 kWh/m².an pour le sud de la France)<br>"
        "&nbsp; Cp = coefficient de perte (varie entre 0.9 et ... très bas, soit un minimum de 10 %, la valeur fréquente étant entre 0.75 et 0.8)<br><br>"
        "&nbsp;&nbsp;&nbsp;Détail des pertes (varie selon les installations) :"
        "<ul style=list-style-type:disc>"
        "<li>Pertes onduleurs 8% à 15 %</li>"
        "<li>Pertes température 5% à 12%</li>"
        "<li>Pertes câbles et connexion 2%</li>"
        "<li>Pertes masque 0 % à 50% (dépend de l'implantation)</li>"
        "<li>Pertes faible éclairement 3% à 7%</li>"
        "<li>Pertes liées à la réflectivité environ 3%</li>"
        "</ul><br/>"
        "<a href='http://www.photovoltaique.info/Rendement-d-un-systeme.html'>Source 1</a> &nbsp; &nbsp; &nbsp; &nbsp; "
        "<a href='http://www.photovoltaique-energie.fr/estimer-la-production-photovoltaique.html'>Source 2</a><br><br>"
        "Ci-dessous une carte de l'ensoleillement en France :<br>"
        "<center><img src='%1' alt='Carte ensoleillement'></center></font>"
    ).arg(imgStats);

    photov->setHtml(html);
    photov->setOpenExternalLinks(true);
    photov->showMaximized();
}

void MainWindow::nuclo()
{
    QTextBrowser *nuclo = new QTextBrowser();
    nuclo->setWindowIcon(QIcon(getAssetPath("nuclo.png")));
    nuclo->setWindowTitle("Énergie nucléaire");
    nuclo->setReadOnly(true);

    QString nucloPath = getAssetPath("Nucléaire.png");
    if (!QFileInfo::exists(nucloPath)) {
        nucloPath = getAssetPath("nucleaire.png");
    }
    QString imgNuclo = QUrl::fromLocalFile(nucloPath).toString();

    QString html = QString(
        "<br><center><strong><font size=6><u>Énergie nucléaire</u> : </font></strong></center><br/>"
        "<font size=4><p>&nbsp; &nbsp; &nbsp; L'électricité d'origine nucléaire est la principale énergie produite et consommée en France (78,1% de la production annuelle). Elle provient de 58 réacteurs de différents niveaux de puissance constituant un parc réparti sur l'ensemble du territoire (dans dix-neuf centrales nucléaires). On estime que les réacteurs nucléaires en France ont une puissance moyenne de 1 000 MW. Ce qui nous donne donc la formule suivante :</p><br/>"
        "<center><strong>E = 7451,1 * P * n</strong></center><br/><br>"
        "&nbsp; E = énergie produite en Wh<br/>"
        "&nbsp; Constante : Nombre d'heures moyen d'activité obtenu sur la base d'un fonctionnement durant 85% de l'année soit :<br>"
        "<mark>&nbsp; &nbsp; 24 * 365,25 * 0,85 = 7451,1 h</mark><br/>"
        "&nbsp; P : Puissance moyenne des réacteurs en MW<br/>"
        "&nbsp; n : Nombre de réacteurs</p><br/><br>"
        "<p>&nbsp; &nbsp; <u>NB</u> : La durée de vie des réacteurs s'allongeant, le taux de disponibilité du parc nucléaire tend à baisser (maintenances plus fréquentes).</p><br><br>"
        "<a href='http://www.planetoscope.com/Source-d-energie/233-production-d-energie-electrique-en-france-en-kwh.html'>Source 1</a> &nbsp; &nbsp; &nbsp; &nbsp; "
        "<a href='http://www.sortirdunucleaire.org/EDF-objectif-de-disponibilite-du'>Source 2</a> &nbsp; &nbsp; &nbsp; &nbsp; "
        "<a href='http://fr.wikipedia.org/wiki/Disponibilit%C3%A9'>Source 3</a> &nbsp; &nbsp; &nbsp; &nbsp; "
        "<a href='http://jeunes.edf.com/article/le-nucleaire-en-france,72'>Source 4</a><br><br>"
        "Ci-dessous une carte des centrales françaises en activité, classées selon leur puissance :<br>"
        "<center><img src='%1' alt='Carte centrales nucléaires'></center></font>"
    ).arg(imgNuclo);

    nuclo->setHtml(html);
    nuclo->setOpenExternalLinks(true);
    nuclo->showMaximized();
}

void MainWindow::aide()
{
    QLabel *image = new QLabel();
    image->setWindowTitle("TPE");
    image->setWindowIcon(QIcon(getAssetPath("icone.png")));
    image->setPixmap(QPixmap(getAssetPath("credits3.png")));
    image->setAttribute(Qt::WA_DeleteOnClose);
    image->show();
}
