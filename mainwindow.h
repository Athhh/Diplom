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

    double on_choose_clicked();

    void slotMousePress(QMouseEvent * event);

    void slotMouseMove(QMouseEvent * event);

    void mousePress();

    void contextMenuRequest(QPoint pos);

    void removeSelectedGraph();

    void removeAllGraphs();

    void selectionChanged();

    void createMenus();

    void wrongBorders();

    void createLegend();

private:
    Ui::MainWindow *ui;
    QCPItemTracer *tracer;
    QCPLegend *Legend;
    QCPGraph *graph1;
    QCPGraph *graph2;
    QCPGraph *graph3;
    double leftX,rightX;
    int counter = 0;
    int numberOfGraphs = 0;
};
#endif // MAINWINDOW_H
