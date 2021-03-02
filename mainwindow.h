#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qcustomplot.h>

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
    double on_acc_editingFinished();

    double on_leftX_editingFinished();

    double on_rightX_editingFinished();

    void on_plot_clicked();

    void mouseWheel();

    double on_choose_clicked();

    void slotMousePress(QMouseEvent * event);

    void slotMouseMove(QMouseEvent * event);

private:
    Ui::MainWindow *ui;
    QCPCurve *verticalLine;
    QCPItemTracer *tracer;
    double leftX,rightX;
};
#endif // MAINWINDOW_H
