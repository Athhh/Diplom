#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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

private:
    Ui::MainWindow *ui;
    double leftX,rightX;
};
#endif // MAINWINDOW_H
