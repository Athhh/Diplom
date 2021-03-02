#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
//Название графика
    ui->widget->plotLayout()->insertRow(0);
    QCPTextElement *title = new QCPTextElement(ui->widget, "График зависимости интенсивности от энергии", QFont("sans", 18, QFont::Bold));
    ui->widget->plotLayout()->addElement(0, 0, title);
//Перетаскивание графика
    ui->widget->setInteraction(QCP::iRangeDrag, true);
//Название осей
    ui->widget->xAxis->setLabel("Энергия");
    ui->widget->yAxis->setLabel("Интенсивность");
//Приближение графика
    ui->widget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes);
    connect(ui->widget, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

double MainWindow::on_acc_editingFinished()
{
    double acc = ui->acc->text().toDouble();
    return acc;
}

double MainWindow::on_leftX_editingFinished()
{
    leftX = ui->leftX->text().toDouble();
    return leftX;
}

double MainWindow::on_rightX_editingFinished()
{
    rightX = ui->rightX->text().toDouble();
    return rightX;
}

void MainWindow::on_plot_clicked()
{
    double a = on_leftX_editingFinished();
    double b = on_rightX_editingFinished();
    double h = on_acc_editingFinished();

//Количество отрисовываемых точек

    double N = (b-a)/(h)+1;

    N = round(N);

    QVector<double> x(N), y(N);

//Вычисляем наши данные

    int i=0;
    for (double X=a; fabs(X - b) >= 0.00001; X+= h)
    {
        x[i] = X;
        y[i] = X*X+on_choose_clicked();
        i++;
    }
    x[i]=b;
    y[i]=b*b+on_choose_clicked();

//Отрисовка графика

    ui->widget->clearGraphs();
    ui->widget->addGraph();

    ui->widget->graph(0)->setData(x, y);

    ui->widget->xAxis->setRange(on_leftX_editingFinished(), on_rightX_editingFinished());

    double minY = y[0], maxY = y[0];
    for (int i=1; i<N; i++)
    {
        if (y[i]<minY) minY = y[i];
        if (y[i]>maxY) maxY = y[i];
    }

    ui->widget->yAxis->setRange(minY, maxY);

    ui->widget->replot();
    ui->widget->rescaleAxes();
}

void MainWindow::mouseWheel()
{
    if (ui->widget->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->widget->axisRect()->setRangeZoom(ui->widget->xAxis->orientation());
    else if (ui->widget->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->widget->axisRect()->setRangeZoom(ui->widget->yAxis->orientation());
    else
    ui->widget->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
}

double MainWindow::on_choose_clicked()
{
    QString func = ui->material->currentText();
    int n;
    if (func == "Медь")
    {
        n = 1;
    }
    if (func == "Алюминий")
    {
        n = 2;
    }
    if (func == "Железо")
    {
        n = 3;
    }

    return n;
}

