#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QMouseEvent>

class ClickableAreaWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ClickableAreaWidget(QWidget *parent = nullptr);

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
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
    void interactif();
    void lancerPresentationSDL();

private:
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
    ClickableAreaWidget *tool[7];
};

#endif // MAINWINDOW_H
