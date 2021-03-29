#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <qcustomplot.h>
#include <QFile>
#include <QDebug>
#include <QTextStream>
#include <QStatusBar>

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
    //ui->widget->QCustomPlot::setAutoAddPlottableToLegend(false);
// connect slot that ties some axis selections together (especially opposite axes):
     connect(ui->widget, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));
// connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
    connect(ui->widget, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
    connect(ui->widget, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));

    ui->widget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->widget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));

    createMenus();

}

void MainWindow::createMenus()
{
//Панель управления
    QAction *newa = new QAction("&Новый Файл", this);
    QAction *open = new QAction("&Открыть файл", this);
    QAction *quit = new QAction("&Выход", this);
    QAction *file = new QAction("&File", this);

//Шорткаты

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

//Количество отрисовываемых точекq

    double N = (b-a)/(h)+1;

    switch(counter)
    {
        rewind:
        case 0:
        {
            if (N - int(N) != 0)
            {
                QMessageBox msgBox;
                msgBox.setIcon(QMessageBox::Critical);
                msgBox.setWindowTitle("Внимание");
                msgBox.setText("Введите другой шаг");
                msgBox.exec();
            }

        //Вычисляем наши данные
            else
            {
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

                ui->widget->addGraph();
                ui->widget->graph(counter)->setData(x, y0);
                ui->widget->graph(counter)->setPen(QPen(Qt::green));

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
            {
                QMessageBox msgBox;
                msgBox.setIcon(QMessageBox::Critical);
                msgBox.setWindowTitle("Внимание");
                msgBox.setText("Введите другой шаг");
                msgBox.exec();
            }

    //Вычисляем наши данные
        else
            {
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
                    y1[i] = X*X+on_choose_clicked();
                    in << x[i] << "\t" << y1[i] << "\n";
                    i++;
                }
                x[i]=b;
                y1[i]=b*b+on_choose_clicked();
                in << x[i] << "\t" << y1[i]  << "\n";

                fileOut.close();
            }
        //Отрисовка графика

            ui->widget->addGraph();
            ui->widget->graph(counter)->setData(x, y1);
            ui->widget->graph(counter)->setPen(QPen(Qt::red));

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
            }
        counter++;
        break;
        }
        case 2:
        {
            if (N - int(N) != 0)
            {
                QMessageBox msgBox;
                msgBox.setIcon(QMessageBox::Critical);
                msgBox.setWindowTitle("Внимание");
                msgBox.setText("Введите другой шаг");
                msgBox.exec();
            }

        //Вычисляем наши данные
            else
            {
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
                    y2[i] = X*X+on_choose_clicked();
                    in << x[i] << "\t" << y2[i] << "\n";
                    i++;
                }
                x[i]=b;
                y2[i]=b*b+on_choose_clicked();
                in << x[i] << "\t" << y2[i]  << "\n";

                fileOut.close();
            }
        //Отрисовка графика

            ui->widget->addGraph();
            ui->widget->graph(counter)->setData(x, y2);
            ui->widget->graph(counter)->setPen(QPen(Qt::yellow));

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
            ui->widget->QCustomPlot::setAutoAddPlottableToLegend(false);
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
    QFile file(QCoreApplication::applicationDirPath() + "/Materials/Materials.txt");
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
  counter = 0;
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

  for (int i=0; i<3; ++i)
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
