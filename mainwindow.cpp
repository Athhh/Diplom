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

    createMenus();

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
    createLegend();
// connect slot that ties some axis selections together (especially opposite axes):
     connect(ui->widget, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));
// connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
    connect(ui->widget, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
    connect(ui->widget, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));

    //ui->widget->setContextMenuPolicy(Qt::CustomContextMenu);
    //connect(ui->widget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createMenus()
{
//Панель управления
    QAction *newa = new QAction("&Новый Файл", this);
    QAction *open = new QAction("&Открыть файл", this);
    QAction *quit = new QAction("&Выход", this);
    QAction *file = new QAction("&File", this);

//Хоткеи

    quit->setShortcut(tr("CTRL+Q"));
    file->setShortcut(tr("ALT+F"));

    QMenu *fileMenu;
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newa);
    fileMenu->addAction(open);
    fileMenu->addSeparator();
    fileMenu->addAction(quit);

    connect(quit, &QAction::triggered, qApp, &QApplication::quit);

    statusBar()->showMessage("Готово к работе");
}

void MainWindow::createLegend()
{
    //Первый график

    graph1 = ui->widget->addGraph();

    graph1->addToLegend();
    ui->widget->legend->itemWithPlottable(graph1)->setVisible(false);

    //Второй график

    graph2 = ui->widget->addGraph();
    graph2->setName(ui->material->currentText());
    graph2->addToLegend();
    ui->widget->legend->itemWithPlottable(graph2)->setVisible(false);

    //Третий график

    graph3 = ui->widget->addGraph();
    graph3->setName(ui->material->currentText());
    graph3->addToLegend();
    ui->widget->legend->itemWithPlottable(graph3)->setVisible(false);

    //Формат легенды

    QFont legendFont = font();
    legendFont.setPointSize(10);
    ui->widget->legend->setSelectedFont(legendFont);
    ui->widget->legend->setSelectableParts(QCPLegend::spItems);
    ui->widget->setAutoAddPlottableToLegend(false);
}

double MainWindow::on_acc_selectionChanged()
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

double MainWindow::on_choose_clicked() //возвращает атомный номер
{
    QFile atomNumber(QCoreApplication::applicationDirPath() + "/Materials/atomniy_nomer.txt");
    QStringList number;
    if (atomNumber.open(QIODevice::ReadOnly))
    {
        while(!atomNumber.atEnd())
        {
            number << atomNumber.readLine();
        }
    }

    eqNumber = number[ui->material->currentIndex()].toDouble();

    atomNumber.close();

    QFile atomMass(QCoreApplication::applicationDirPath() + "/Materials/atom_mass.txt");
    QStringList mass;
    if (atomMass.open(QIODevice::ReadOnly))
    {
        while(!atomMass.atEnd())
        {
            mass << atomMass.readLine();
        }
    }

    eqMass = mass[ui->material->currentIndex()].toDouble();

    atomMass.close();

    QFile atomPlotnost(QCoreApplication::applicationDirPath() + "/Materials/plotnost.txt");
    QStringList plotnost;
    if (atomPlotnost.open(QIODevice::ReadOnly))
    {
        while(!atomPlotnost.atEnd())
        {
            plotnost << atomPlotnost.readLine();
        }
    }

    eqPlotnost = plotnost[ui->material->currentIndex()].toDouble();

    atomPlotnost.close();
}

void MainWindow::on_plot_clicked()
{
    ui->widget->legend->setVisible(true);

    double a = on_leftX_editingFinished();
    double b = on_rightX_editingFinished();
    double h = on_acc_selectionChanged();
    double N = (b-a)/(h)+1;

    if (h<0)
    {
        QMessageBox stepBox;
        stepBox.setIcon(QMessageBox::Critical);
        stepBox.setWindowTitle("Внимание");
        stepBox.setText("Шаг должен быть неотрицательным");
        stepBox.exec();
    }
    else
    {
        switch(counter)
        {
            rewind:
            case 0:
            {
                if (N - int(N) != 0)
                wrongBorders();
            //Вычисляем наши данные
                else
                {
                    graph1->setName(ui->material->currentText());

                    QVector<double> x(N), y0(N);

                    int i=0;

                //Записываем данные в переменную

                    QFile fileOut(QCoreApplication::applicationDirPath() + "/Output/fileOut.txt");
                    if (fileOut.open(QIODevice::WriteOnly| QIODevice::Text))
                    {
                        QTextStream in(&fileOut);
                        for (double X=a; fabs(X - b) >= 0.00001; X+= h)
                        {
                            x[i] = X;
                            y0[i] = X*X+on_choose_clicked();
                            in << x[i] << "\t" << y0[i] << "\n";
                            i++;
                        }
                        x[i]=b;
                        y0[i]=b*b+on_choose_clicked();
                        in << x[i] << "\t" << y0[i]  << "\n";

                        fileOut.close();
                    }
                //Отрисовка графика

                    ui->widget->legend->itemWithPlottable(graph1)->setVisible(true);
                    graph1->setData(x, y0);
                    graph1->setPen(QPen(Qt::green));

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
                    counter++;
                    break;
                }
            }
            case 1:
            {
            if (N - int(N) != 0)
            wrongBorders();
        //Вычисляем наши данные
            else
                {
                graph2->setName(ui->material->currentText());

                QVector<double> x(N), y1(N);

                int i=0;

                //Записываем данные в переменную

                QFile fileOut(QCoreApplication::applicationDirPath() + "/Output/fileOut.txt");
                if (fileOut.open(QIODevice::WriteOnly| QIODevice::Text))
                {
                    QTextStream in(&fileOut);
                    for (double X=a; fabs(X - b) >= 0.00001; X+= h)
                    {
                        x[i] = X;
                        y1[i] = -X*X+on_choose_clicked();
                        in << x[i] << "\t" << y1[i] << "\n";
                        i++;
                    }
                    x[i]=b;
                    y1[i]=-b*b+on_choose_clicked();
                    in << x[i] << "\t" << y1[i]  << "\n";

                    fileOut.close();
                }
            //Отрисовка графика
                ui->widget->legend->itemWithPlottable(graph2)->setVisible(true);
                graph2->setData(x, y1);
                graph2->setPen(QPen(Qt::red));

            //Границы по оси х

                ui->widget->xAxis->setRange(on_leftX_editingFinished(), on_rightX_editingFinished());

            //Границы по оси y

                double minY = y1[0], maxY = y1[0];
                for (int i=1; i<N; i++)
                {
                    if (y1[i]<minY) minY = y1[i];
                    if (y1[i]>maxY) maxY = y1[i];
                }

                ui->widget->yAxis->setRange(minY, maxY);

                ui->widget->replot();
                ui->widget->rescaleAxes();
                counter++;
                }

            break;
            }
            case 2:
            {
                if (N - int(N) != 0)
                wrongBorders();
            //Вычисляем наши данные
                else
                {
                graph3->setName(ui->material->currentText());

                QVector<double> x(N), y2(N);

                int i=0;

            //Записываем данные в переменную

                QFile fileOut(QCoreApplication::applicationDirPath() + "/Output/fileOut.txt");
                if (fileOut.open(QIODevice::WriteOnly| QIODevice::Text))
                {
                    QTextStream in(&fileOut);
                    for (double X=a; fabs(X - b) >= 0.00001; X+= h)
                    {
                        x[i] = X;
                        y2[i] = qExp(X/2)+on_choose_clicked();
                        in << x[i] << "\t" << y2[i] << "\n";
                        i++;
                    }
                    x[i]=b;
                    y2[i]=qExp(b/2)+on_choose_clicked();
                    in << x[i] << "\t" << y2[i]  << "\n";

                    fileOut.close();
                }
            //Отрисовка графика

                ui->widget->legend->itemWithPlottable(graph3)->setVisible(true);
                graph3->setData(x, y2);
                graph3->setPen(QPen(Qt::yellow));

            //Границы по оси х

                ui->widget->xAxis->setRange(on_leftX_editingFinished(), on_rightX_editingFinished());

            //Границы по оси y

                double minY = y2[0], maxY = y2[0];
                for (int i=1; i<N; i++)
                {
                    if (y2[i]<minY) minY = y2[i];
                    if (y2[i]>maxY) maxY = y2[i];
                }

                ui->widget->yAxis->setRange(minY, maxY);

                ui->widget->replot();
                ui->widget->rescaleAxes();
                counter++;
                }
             break;
            }
            default:
            {
                counter = 0;
                goto rewind;
            }
        }
    }
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

void MainWindow::mouseWheel()
{
    if (ui->widget->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->widget->axisRect()->setRangeZoom(ui->widget->xAxis->orientation());
    else if (ui->widget->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->widget->axisRect()->setRangeZoom(ui->widget->yAxis->orientation());
    else
    ui->widget->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
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

  for (counter=0; counter<ui->widget->graphCount(); ++counter)
  {
    QCPGraph *graph = ui->widget->graph(counter);
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

void MainWindow::wrongBorders()
{
    QMessageBox stepBox;
    stepBox.setIcon(QMessageBox::Critical);
    stepBox.setWindowTitle("Внимание");
    stepBox.setText("Шаг должен быть неотрицательным");
    stepBox.exec();
}

void MainWindow::on_removeAllGraphs_clicked()
{
    ui->widget->clearGraphs();
    createLegend();
    ui->widget->legend->setVisible(false);
    ui->widget->replot();
    counter = 0;
}
