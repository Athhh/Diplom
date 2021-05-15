#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qcustomplot.h>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    double on_acc_selectionChanged();

    double on_leftX_editingFinished();

    double on_rightX_editingFinished();

    void on_plot_clicked();

    void mouseWheel();

    void on_choosePipe_clicked();

    double oslableniyePipe(double X);

    void slotMousePress(QMouseEvent * event);

    void slotMouseMove(QMouseEvent * event);

    void mousePress();

    void selectionChanged();

    void createMenus();

    void wrongBorders();

    void createLegend();

    void on_removeAllGraphs_clicked();

    void on_chooseFirst_clicked();

    void on_chooseSecond_clicked();

    void on_chooseThird_clicked();

    void on_chooseEnv_clicked();

    double on_voltage_editingFinished();

    double on_current_editingFinished();

    double on_targetThick_editingFinished();

    double on_windowThick_editingFinished();

    double on_distance_editingFinished();

private:
    Ui::MainWindow *ui;
    QCPItemTracer *tracer;
    QCPLegend *Legend;
    QCPGraph *graph1;
    QCPGraph *graph2;
    QCPGraph *graph3;

    double pipeNumber;
    double pipeMass;
    double pipePlotnost;

    double firstNumber;
    double firstMass;
    double firstPlotnost;

    double secondNumber;
    double secondMass;
    double secondPlotnost;

    double thirdNumber;
    double thirdMass;
    double thirdPlotnost;

    double envNumber;
    double envMass;
    double envPlotnost;

    double weakening;

    double leftX,rightX;
    double voltage, current, targetThick, windowThick, distance;
    int counter = 0;
    int numberOfGraphs = 0;
};
#endif // MAINWINDOW_H
