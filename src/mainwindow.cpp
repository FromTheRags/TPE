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
#include <QPainter>
#include <QSvgRenderer>
#include <QFont>
#include <QDir>
#include <QUrl>
#include <QToolTip>
#include <QCursor>
#include <QFileInfo>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScreen>
#include <QFontMetrics>
#include <QTimer>
#include <cmath>

namespace {
// Reference size of the 2014 design (window client area without menu bar).
constexpr int kBaseWidth = 1100;
constexpr int kBaseHeight = 560;
constexpr int kImageWidth = 830;
constexpr int kImageHeight = 528;

// Descriptions of the three simulated energies (landscape tooltips).
const QString kDescEolien = QStringLiteral(
    "Les éoliennes utilisent la force du vent qui actionne leurs hélices et un alternateur pour créer de l'électricité.");
const QString kDescNucleaire = QStringLiteral(
    "La centrale nucléaire utilise le procédé de fission des atomes d’uranium. "
    "Cela produit de la chaleur qui transforme ensuite de l'eau en vapeur et fait tourner la turbine (production d'électricité).");
const QString kDescPhotov = QStringLiteral(
    "Les panneaux photovoltaïques produisent de l’électricité grâce à l'énergie solaire. "
    "C'est aussi un moyen pratique pour alimenter des lieux isolés du réseau électrique.<br>"
    "Il existe trois types de panneaux : à cellules amorphes, à cellules polycristallines ou à cellules monocristallines.");
}

// ---------------------------------------------------------------------------
// ClickableAreaWidget
// ---------------------------------------------------------------------------
ClickableAreaWidget::ClickableAreaWidget(QWidget *parent)
    : QWidget(parent)
{
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_Hover, true);
}

void ClickableAreaWidget::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if (!toolTip().isEmpty()) {
        QToolTip::showText(QCursor::pos(), toolTip(), this);
    }
}

// ---------------------------------------------------------------------------
// BackgroundView
// ---------------------------------------------------------------------------
BackgroundView::BackgroundView(QWidget *parent)
    : QWidget(parent)
    , m_svg(nullptr)
{
    // ENR5.svg is a vector redraw of the 2014 landscape (same 830x528 framing):
    // it is rendered at the exact on-screen pixel size, so it stays uniformly
    // sharp at any window size, in full screen and on HiDPI screens.
    // ENR5_hd.png (a 1660x1056 export of the SVG) and the original ENR5.png
    // are only fallbacks if the SVG cannot be loaded.
    m_sourceArea = QRectF(0, 0, kImageWidth, kImageHeight);
    m_svg = new QSvgRenderer(getAssetPath("ENR5.svg"), this);
    if (!m_svg->isValid()) {
        delete m_svg;
        m_svg = nullptr;
        QString hdPath = getAssetPath("ENR5_hd.png");
        if (!(QFileInfo::exists(hdPath) && m_source.load(hdPath))) {
            m_source.load(getAssetPath("ENR5.png"));
        }
    }
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

QSize BackgroundView::sizeHint() const
{
    return QSize(kImageWidth, kImageHeight);
}

QSize BackgroundView::minimumSizeHint() const
{
    return QSize(kImageWidth, kImageHeight);
}

ClickableAreaWidget *BackgroundView::addHotspot(const QRect &rect, const QString &toolTip)
{
    ClickableAreaWidget *w = new ClickableAreaWidget(this);
    w->setToolTip(toolTip);
    m_hotspots.append(qMakePair(w, rect));
    w->setGeometry(mapFromOriginal(rect));
    return w;
}

QRect BackgroundView::imageRect() const
{
    if (width() <= 0 || height() <= 0) {
        return rect();
    }
    QSize s = QSize(kImageWidth, kImageHeight).scaled(size(), Qt::KeepAspectRatio);
    // Bottom-aligned so the landscape always sits right on the result bar.
    return QRect(QPoint((width() - s.width()) / 2, height() - s.height()), s);
}

QRect BackgroundView::mapFromOriginal(const QRectF &r) const
{
    const QRect img = imageRect();
    const double sx = img.width() / m_sourceArea.width();
    const double sy = img.height() / m_sourceArea.height();
    QRectF mapped(img.left() + (r.left() - m_sourceArea.left()) * sx,
                  img.top() + (r.top() - m_sourceArea.top()) * sy,
                  r.width() * sx,
                  r.height() * sy);
    return mapped.toAlignedRect().intersected(img);
}

void BackgroundView::updateScaledPixmap()
{
    const QRect img = imageRect();
    const qreal dpr = devicePixelRatioF();
    const QSize target(qRound(img.width() * dpr), qRound(img.height() * dpr));
    if (!target.isValid() || m_scaled.size() == target) {
        return;
    }
    // Rendered once per size (and per screen scale factor) at the physical
    // pixel resolution, then simply blitted on every repaint.
    if (m_svg) {
        QPixmap pm(target);
        pm.fill(Qt::transparent);
        QPainter p(&pm);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setRenderHint(QPainter::SmoothPixmapTransform, true);
        m_svg->render(&p, QRectF(0, 0, target.width(), target.height()));
        p.end();
        m_scaled = pm;
    } else if (!m_source.isNull()) {
        m_scaled = m_source.scaled(target, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    } else {
        return;
    }
    m_scaled.setDevicePixelRatio(dpr);
}

void BackgroundView::updateLayout()
{
    updateScaledPixmap();
    for (const auto &h : m_hotspots) {
        h.first->setGeometry(mapFromOriginal(h.second));
    }
}

void BackgroundView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateLayout();
}

void BackgroundView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);
    // The top of the landscape is plain sky: filling the spare area with the
    // same colour makes the picture blend seamlessly when it is letterboxed.
    p.fillRect(rect(), QColor(0xb9, 0xe0, 0xfb)); // top colour of the ENR5.svg sky
    const QRect img = imageRect();
    updateScaledPixmap(); // no-op unless the size or the screen DPI changed
    if (!m_scaled.isNull()) {
        p.drawPixmap(img.topLeft(), m_scaled);
    }
}

// ---------------------------------------------------------------------------
// MainWindow
// ---------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_background(nullptr)
    , m_panel(nullptr)
    , m_fullScreenAction(nullptr)
    , m_uiScale(1.0)
{
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

    // --- Menu Affichage ---
    QMenu *menuAffichage = menuBar()->addMenu("&Affichage");
    m_fullScreenAction = new QAction("&Plein écran", this);
    m_fullScreenAction->setCheckable(true);
    m_fullScreenAction->setShortcut(QKeySequence(Qt::Key_F11));
    m_fullScreenAction->setShortcutContext(Qt::ApplicationShortcut);
    connect(m_fullScreenAction, &QAction::triggered, this, &MainWindow::toggleFullScreen);
    menuAffichage->addAction(m_fullScreenAction);

    // --- Aide / Crédits (?) ---
    QAction *menuAide = menuBar()->addAction("?");
    connect(menuAide, &QAction::triggered, this, &MainWindow::aide);

    // --- Central widget: [ landscape + result ] | [ parameters ] ---
    QWidget *zoneCentrale = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(zoneCentrale);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(0);

    m_background = new BackgroundView(zoneCentrale);
    leftLayout->addWidget(m_background, 1);

    m_background->addHotspot(QRect(201, 120, 216, 115),
        "<b>Éoliennes</b><br>" + kDescEolien);
    m_background->addHotspot(QRect(352, 264, 206, 125),
        "<b>Centrale nucléaire</b><br>" + kDescNucleaire);
    m_background->addHotspot(QRect(557, 377, 272, 113),
        "<b>Panneaux photovoltaïques</b><br>" + kDescPhotov);
    m_background->addHotspot(QRect(0, 190, 183, 140),
        "<b>Centrale thermique</b> <i>(à titre indicatif)</i><br>"
        "La centrale thermique utilise la force de la vapeur dégagée en brûlant des énergies fossiles "
        "(charbon, pétrole, gaz naturel) ou de la biomasse (déchets végétaux ou ménagers).");
    m_background->addHotspot(QRect(134, 412, 228, 116),
        "<b>Centrale géothermique</b> <i>(à titre indicatif)</i><br>"
        "La centrale géothermique utilise la chaleur de la Terre pour chauffer des foyers ou créer de l'électricité.");
    m_background->addHotspot(QRect(644, 125, 186, 101),
        "<b>Centrale hydroélectrique</b> <i>(à titre indicatif)</i><br>"
        "La centrale hydroélectrique utilise la force de l’eau, créée par un puissant déplacement, pour produire de l'électricité. "
        "Il peut s’agir d’une chute d’eau naturelle, de l’eau stockée dans un barrage ou des mouvements de la marée.");
    m_background->addHotspot(QRect(644, 226, 186, 108),
        "<b>Biomasse</b> <i>(à titre indicatif)</i><br>"
        "La biomasse est une énergie constituée de matières organiques végétales ou animales (déchets ménagers ou agricoles). "
        "Sa combustion permet de produire de l’électricité.");

    // Result Label (authentic 2014 raised bevel frame)
    m_resultat = new QLabel(zoneCentrale);
    m_resultat->setFrameStyle(QFrame::Box | QFrame::Raised);
    m_resultat->setLineWidth(3);
    m_resultat->setFont(QFont("Comic Sans MS", 13, QFont::Bold));
    m_resultat->setAlignment(Qt::AlignCenter);
    m_resultat->setMinimumHeight(32);
    m_resultat->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    registerScalable(m_resultat);
    leftLayout->addWidget(m_resultat);

    mainLayout->addLayout(leftLayout, 1);

    // --- Right parameter panel ---
    // A grid keeps every caption aligned with its spin box and every section
    // title aligned with the column, whatever the font metrics / DPI.
    m_panel = new QWidget(zoneCentrale);
    m_panel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    QGridLayout *grid = new QGridLayout(m_panel);
    grid->setContentsMargins(10, 12, 10, 8);
    grid->setHorizontalSpacing(8);
    grid->setVerticalSpacing(4);
    grid->setColumnStretch(1, 1);

    QFont fontEolien("Calibri", 15, QFont::Bold);
    QFont fontSection("Calibri", 14, QFont::Bold);

    int row = 0;
    addSection(grid, row++, "<u>Énergie éolienne</u> :", fontEolien);
    box1bis = addParam(grid, row++, "Nombre d'éoliennes", 0, 100000, 100, 0);
    box1    = addParam(grid, row++, "Diamètre du rotor (en m)", 0, 200, 80, 0);
    box2    = addParam(grid, row++, "Vitesse moyenne du vent (en m/s)", 0, 60, 8.0, 1);
    box3    = addParam(grid, row++, "Temps de fonctionnement<br>(en heures)", 0, 8766, 6000, 0);

    grid->setRowMinimumHeight(row++, 10);
    addSection(grid, row++, "<u>Énergie solaire</u> :", fontSection);
    box4 = addParam(grid, row++, "Surface du champ<br>photovoltaïque (en m<sup>2</sup>)", 0, 100000000, 50000, 0);
    box5 = addParam(grid, row++, "Rendement du module (en %)", 0, 50, 14, 0);
    box6 = addParam(grid, row++, "Ensoleillement/rayonnement<br>sur la surface inclinée en kWh/m²", 0, 2500, 1500, 0);
    box7 = addParam(grid, row++, "Coefficient de perte", 0.01, 0.90, 0.75, 2);
    box7->setSingleStep(0.01);

    grid->setRowMinimumHeight(row++, 10);
    addSection(grid, row++, "<u>Énergie nucléaire</u> :", fontSection);
    box8 = addParam(grid, row++, "Puissance moyenne des<br>réacteurs (en MW)", 1, 2000, 1000, 0);
    box9 = addParam(grid, row++, "Nombre de réacteurs", 0, 58, 58, 0);

    grid->setRowStretch(row, 1);
    mainLayout->addWidget(m_panel, 0);

    for (QDoubleSpinBox *b : {box1bis, box1, box2, box3, box4, box5, box6, box7, box8, box9}) {
        connect(b, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::resultat);
    }

    updateSpinBoxWidths();
    resultat(1.0);

    setCentralWidget(zoneCentrale);
    resize(kBaseWidth, kBaseHeight + menuBar()->sizeHint().height());
}

MainWindow::~MainWindow()
{
}

QLabel *MainWindow::addSection(QGridLayout *grid, int row, const QString &title, const QFont &font)
{
    QLabel *label = new QLabel(title, m_panel);
    label->setFont(font);
    grid->addWidget(label, row, 0, 1, 2, Qt::AlignLeft | Qt::AlignVCenter);
    registerScalable(label);
    return label;
}

QDoubleSpinBox *MainWindow::addParam(QGridLayout *grid, int row, const QString &text,
                                     double min, double max, double value, int decimals)
{
    QDoubleSpinBox *box = new QDoubleSpinBox(m_panel);
    box->setRange(min, max);
    box->setDecimals(decimals);
    box->setValue(value);
    box->setAlignment(Qt::AlignRight);
    // No alignment flag: every spin box fills the column, so they all share the
    // width needed by the widest range (no truncated values such as "600" for 6000).
    grid->addWidget(box, row, 0);
    registerScalable(box);

    QLabel *label = new QLabel(text, m_panel);
    label->setFont(QFont("Segoe UI", 9));
    label->setBuddy(box);
    grid->addWidget(label, row, 1, Qt::AlignLeft | Qt::AlignVCenter);
    registerScalable(label);
    return box;
}

void MainWindow::registerScalable(QWidget *w)
{
    m_scalable.append(qMakePair(w, w->font()));
}

void MainWindow::applyUiScale()
{
    if (!centralWidget()) {
        return;
    }
    // In full screen, grow the text with the screen so the parameter panel and
    // the result stay proportionate to the landscape. Windowed mode keeps the
    // 2014 sizes (scaling there would also inflate the window minimum size).
    double s = 1.0;
    if (isFullScreen()) {
        s = qMin(centralWidget()->width() / double(kBaseWidth),
                 centralWidget()->height() / double(kBaseHeight));
        s = qBound(1.0, std::floor(s * 10.0) / 10.0, 2.5);
    }
    setUiScale(s);
}

void MainWindow::setUiScale(double s)
{
    if (qFuzzyCompare(s, m_uiScale)) {
        return;
    }
    m_uiScale = s;
    for (const auto &entry : m_scalable) {
        QFont f = entry.second;
        f.setPointSizeF(f.pointSizeF() * s);
        entry.first->setFont(f);
    }
    m_resultat->setMinimumHeight(qRound(32 * s));
    updateSpinBoxWidths();
}

void MainWindow::updateSpinBoxWidths()
{
    // The Windows 11 style draws the up/down arrows side by side and its size
    // hint leaves too little room for the digits (6000 was shown as "600").
    // Size every box for 7 digits plus the arrows, from the current font.
    for (QDoubleSpinBox *b : m_panel->findChildren<QDoubleSpinBox *>()) {
        const QFontMetrics fm(b->font());
        const int text = fm.horizontalAdvance(QStringLiteral("0000000"));
        const int buttons = qRound(fm.height() * 2.8);
        b->setMinimumWidth(qMax(b->sizeHint().width(), text + buttons + 12));
    }
}

void MainWindow::toggleFullScreen()
{
    if (isFullScreen()) {
        // Shrink the fonts first so the enlarged panel does not keep the
        // restored window wider than it was before entering full screen.
        setUiScale(1.0);
        setWindowState(windowState() & ~Qt::WindowFullScreen);
        if (!isMaximized() && m_normalSize.isValid()) {
            const QSize target = m_normalSize;
            QTimer::singleShot(0, this, [this, target] {
                if (!isFullScreen() && !isMaximized()) {
                    resize(target);
                }
            });
        }
    } else {
        m_normalSize = size();
        setWindowState(windowState() | Qt::WindowFullScreen);
    }
}

void MainWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
    if (event->type() == QEvent::WindowStateChange) {
        const bool fs = isFullScreen();
        if (m_fullScreenAction) {
            m_fullScreenAction->setChecked(fs);
        }
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape && isFullScreen()) {
        toggleFullScreen();
        event->accept();
    } else {
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    applyUiScale();
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
    QWidget *fenetre = new QWidget();
    fenetre->setWindowTitle("TPE");
    fenetre->setWindowIcon(QIcon(getAssetPath("icone.png")));
    fenetre->setAttribute(Qt::WA_DeleteOnClose);
    QVBoxLayout *layout = new QVBoxLayout(fenetre);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QLabel *image = new QLabel();
    image->setPixmap(QPixmap(getAssetPath("credits3.png")));
    layout->addWidget(image);

    QLabel *version = new QLabel(QString("TPE version %1 — Qt %2")
                                     .arg(QApplication::applicationVersion(), QString::fromLatin1(qVersion())));
    version->setAlignment(Qt::AlignCenter);
    version->setMargin(6);
    layout->addWidget(version);

    fenetre->show();
}