#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QPixmap>
#include <QVector>
#include <QPair>
#include <QFont>

class QAction;
class QSvgRenderer;

class ClickableAreaWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ClickableAreaWidget(QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;
};

// Landscape background: paints the HD image scaled to the available space
// (aspect ratio kept, high-quality smoothing, HiDPI aware) and keeps the
// clickable hotspots aligned with it at any size.
class BackgroundView : public QWidget
{
    Q_OBJECT

public:
    explicit BackgroundView(QWidget *parent = nullptr);

    // rect is expressed in the 830x528 landscape coordinates (ENR5.svg / ENR5.png)
    ClickableAreaWidget *addHotspot(const QRect &rect, const QString &toolTip);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QRect imageRect() const;
    QRect mapFromOriginal(const QRectF &r) const;
    void updateScaledPixmap();
    void updateLayout();

    QSvgRenderer *m_svg;  // vector landscape (preferred)
    QPixmap m_source;     // raster fallback
    QPixmap m_scaled;
    QRectF m_sourceArea; // area of the original image covered by m_source
    QVector<QPair<ClickableAreaWidget *, QRect>> m_hotspots;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

public slots:
    void exportation();
    void importation();
    void aide();
    void resultat(double value = 0.0);
    void eolien();
    void photov();
    void nuclo();
    void toggleFullScreen();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    QDoubleSpinBox *addParam(class QGridLayout *grid, int row, const QString &label,
                             double min, double max, double value, int decimals);
    QLabel *addSection(class QGridLayout *grid, int row, const QString &title, const QFont &font);
    void registerScalable(QWidget *w);
    void applyUiScale();
    void setUiScale(double s);
    void updateSpinBoxWidths();

    BackgroundView *m_background;
    QWidget *m_panel;
    QAction *m_fullScreenAction;

    QDoubleSpinBox *box1bis; // Nombre d'éoliennes
    QDoubleSpinBox *box1;    // Diamètre du rotor (m)
    QDoubleSpinBox *box2;    // Vitesse moyenne du vent (m/s)
    QDoubleSpinBox *box3;    // Temps de fonctionnement (h)
    QDoubleSpinBox *box4;    // Surface photovoltaïque (m²)
    QDoubleSpinBox *box5;    // Rendement module (%)
    QDoubleSpinBox *box6;    // Ensoleillement (kWh/m²)
    QDoubleSpinBox *box7;    // Coefficient de perte
    QDoubleSpinBox *box8;    // Puissance moyenne réacteurs (MW)
    QDoubleSpinBox *box9;    // Nombre de réacteurs

    QLabel *m_resultat;

    QVector<QPair<QWidget *, QFont>> m_scalable; // widgets whose font follows the window size
    double m_uiScale;
    QSize m_normalSize; // windowed size to restore when leaving full screen
};

#endif // MAINWINDOW_H
