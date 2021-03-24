#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <qcustomplot.h>
#include <QFile>
#include <QDebug>
#include <QTextStream>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
//Название графика
    ui->widget->plotLayout()->insertRow(0);
    QCPTextElement *title = new QCPTextElement(ui->widget, "График зависимости интенсивности от энергии", QFont("sans", 18, QFont::Bold));
    ui->widget->plotLayout()->addElement(0, 0, title);
//Название осей
    ui->widget->xAxis->setLabel("Энергия");
    ui->widget->yAxis->setLabel("Интенсивность");
//Установка взаимодействий
    ui->widget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes | QCP::iSelectLegend | QCP::iSelectPlottables);
//Трейсер
    connect(ui->widget, &QCustomPlot::mousePress, this, &MainWindow::slotMousePress);
    connect(ui->widget, &QCustomPlot::mouseMove, this, &MainWindow::slotMouseMove);
    tracer = new QCPItemTracer(ui->widget);
    tracer->setStyle(QCPItemTracer::tsNone);
    tracer->setSize(10);
//Легенда
    QFont legendFont = font();
    legendFont.setPointSize(10);
    ui->widget->legend->setSelectedFont(legendFont);
    ui->widget->legend->setSelectableParts(QCPLegend::spItems);
// connect slot that ties some axis selections together (especially opposite axes):
     connect(ui->widget, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));
// connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
    connect(ui->widget, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
    connect(ui->widget, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));

    connect(ui->widget, SIGNAL(plottableClick(QCPAbstractPlottable*,int,QMouseEvent*)), this, SLOT(graphClicked(QCPAbstractPlottable*,int)));

    ui->widget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->widget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));
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
    ui->widget->legend->setVisible(true);
    double a = on_leftX_editingFinished();
    double b = on_rightX_editingFinished();
    double h = on_acc_editingFinished();

//Количество отрисовываемых точек

    double N = (b-a)/(h)+1;

    N = round(N);

    QVector<double> x(N), y0(N), y1(N);

//Вычисляем наши данные

    int i=0;


//Записываем данные в переменную
    QFile fileOut("E:\\Work\\projects\\Diplom\\Diplom\\fileOut.txt");
    if (fileOut.open(QIODevice::WriteOnly| QIODevice::Text))
    {
        QTextStream in(&fileOut);
        for (double X=a; fabs(X - b) >= 0.00001; X+= h)
        {
            x[i] = X;
            in << x[i];
            y0[i] = X*X+on_choose_clicked();
            y1[i] = 3+X+on_choose_clicked();
            i++;
        }
        x[i]=b;
        y0[i]=b*b+on_choose_clicked();
        y1[i]=3+b+on_choose_clicked();

        fileOut.close();
    }
//Отрисовка графика

    ui->widget->clearGraphs();

//Первый график

    ui->widget->addGraph();
    ui->widget->graph(0)->setData(x, y0);
    ui->widget->graph(0)->setPen(QPen(Qt::blue));

//Второй график

    ui->widget->addGraph();
    ui->widget->graph(1)->setData(x, y1);
    ui->widget->graph(1)->setPen(QPen(Qt::red));
    ui->widget->graph(1)->setVisible(false);

//Границы по оси х

    ui->widget->xAxis->setRange(on_leftX_editingFinished(), on_rightX_editingFinished());

//Границы по оси y

    double minY = y0[0], maxY = y0[0];
    for (int i=1; i<N; i++)
    {
        if (y0[i]<minY) minY = y0[i];
        if (y0[i]>maxY) maxY = y0[i];
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

void MainWindow::mousePress()
{
  if (ui->widget->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->widget->axisRect()->setRangeDrag(ui->widget->xAxis->orientation());
  else if (ui->widget->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->widget->axisRect()->setRangeDrag(ui->widget->yAxis->orientation());
  else
    ui->widget->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
}

double MainWindow::on_choose_clicked()
{
    QString func = ui->material->currentText();
    QFile file("E:\\Work\\projects\\Diplom\\Diplom\\Materials.txt");
    QStringList strList;
    if (file.open(QIODevice::ReadOnly))
    {
        while(!file.atEnd())
        {
            strList << file.readLine();
        }
    }

    return strList[ui->material->currentIndex()].toDouble();

    file.close();
}

    /*QString func = ui->material->currentText(); //Передаём в константу выбранный пользователем материал
    QFile file("E:\\Work\\projects\\Diplom\\Diplom\\Materials.txt");
    int n;
    if (func == "Медь")
    {
        n = 0;
    }
    else if (func == "Алюминий")
    {
        n = 1;
    }
    else //(func == "Железо")
    {
        n = 2;
    }
    if (file.open(QIODevice::ReadOnly))
    {
        int i=0;
        while((i<n)&&(!file.atEnd()))
        {
            file.readLine();
            i++;
        };
    }

    return file.readLine().toDouble();
    file.close();
*/
    /*
    QString func = ui->material->currentText(); //Передаём в константу выбранный пользователем материал
    QFile file("E:\\Work\\projects\\Diplom\\Diplom\\Materials.txt");
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning("Cannot open file for reading"); // если файл не найден, то выводим предупреждение и завершаем выполнение программы
        return 1;
    }
    QStringList strList;
    int n;
    if (func == "Медь")
    {
        n = 1;
    }
    else if (func == "Алюминий")
    {
        n = 2;
    }
    else //(func == "Железо")
    {
        n = 3;
    }
    if (file.open(QIODevice::ReadOnly))
    {
        while(!file.atEnd())
        {
            strList << file.readLine();
        }
        strList[n];
    }
    file.close();

    return strList[n].toDouble();*/

    /*QString func = ui->material->currentText();
    FILE *fp;
    double n;
    #define MAXLIN 80
    char string[MAXLIN];
    fp = fopen("Materials.txt", "r");
    while((fgets(string, MAXLIN, fp))!=func)
        fscanf(fp,"%d",&n);
    return n;*/


    /*
    QString func = ui->material->currentText();
    QFile file("Materials.txt");

    int n;
    if (func == "Медь")
    {
        n = 1;
    }
    else if (func == "Алюминий")
    {
        n = 2;
    }
    else //(func == "Железо")
    {
        n = 3;
    }
    int i=0;
    QString str[5];
    while(!file.atEnd())
    {
        str[i] = file.readLine();
        i++;
    }
    file.close();
    return str[0].toDouble();*/


void MainWindow::slotMouseMove(QMouseEvent *event)
{
    if(QApplication::mouseButtons()) slotMousePress(event);
}

void MainWindow::slotMousePress(QMouseEvent *event)
{
// Определяем координату X на графике, где был произведён клик мышью

    double coordX = ui->widget->xAxis->pixelToCoord(event->pos().x());

// По координате X клика мыши определим ближайшие координаты для трассировщика

    tracer->setGraphKey(coordX);
    tracer->updatePosition();

// Выводим координаты точки графика, где установился трассировщик, в label

    ui->coord->setText("Координата x: " + QString::number(tracer->position->key()) + " y: " + QString::number(tracer->position->value()));

// Перерисовываем содержимое полотна графика

    ui->widget->replot();
}

void MainWindow::contextMenuRequest(QPoint pos)
{
  QMenu *menu = new QMenu(this);
  menu->setAttribute(Qt::WA_DeleteOnClose);
  {
    if (ui->widget->selectedGraphs().size() > 0)
    menu->addAction("Удалить выбранный график", this, SLOT(removeSelectedGraph()));
    if (ui->widget->graphCount() > 0)
    menu->addAction("Удалить все графики", this, SLOT(removeAllGraphs()));
  }
  menu->popup(ui->widget->mapToGlobal(pos));
}


void MainWindow::removeSelectedGraph()
{
  if (ui->widget->selectedGraphs().size() > 0)
  {
    ui->widget->removeGraph(ui->widget->selectedGraphs().first());
    ui->widget->replot();
  }
}

void MainWindow::removeAllGraphs()
{
  ui->widget->clearGraphs();
  ui->widget->replot();
}

void MainWindow::selectionChanged()
{
  if (ui->widget->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->widget->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
      ui->widget->xAxis2->selectedParts().testFlag(QCPAxis::spAxis) || ui->widget->xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
  {
    ui->widget->xAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    ui->widget->xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
  }

  if (ui->widget->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->widget->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
      ui->widget->yAxis2->selectedParts().testFlag(QCPAxis::spAxis) || ui->widget->yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
  {
    ui->widget->yAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    ui->widget->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
  }

  for (int i=0; i<ui->widget->graphCount(); ++i)
  {
    QCPGraph *graph = ui->widget->graph(i);
    QCPPlottableLegendItem *item = ui->widget->legend->itemWithPlottable(graph);
    if (item->selected() || graph->selected())
    {
      item->setSelected(true);
      tracer->setGraph(graph);
      tracer->setStyle(QCPItemTracer::tsPlus);
      graph->setSelection(QCPDataSelection(graph->data()->dataRange()));
    }
  }
}

void MainWindow::on_set_visible_clicked()
{
    ui->widget->graph(1)->setVisible(true);
    ui->widget->replot();
}
