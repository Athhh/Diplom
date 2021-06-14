#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <qcustomplot.h>
#include <QFile>
#include <QDebug>
#include <QTextStream>
#include <QtMath>
#include <QScriptEngine>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    createMenus();

//Название графика
    ui->widget->plotLayout()->insertRow(0);
    QCPTextElement *title = new QCPTextElement(ui->widget, "График зависимости интенсивности от энергии", QFont("sans", 14, QFont::Bold));
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
     connect(ui->widget, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));
//Если выделена ось, то приближаем и перетаскиваем только относительно неё
    connect(ui->widget, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
    connect(ui->widget, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));
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
    graph2->setName(ui->materialPipe->currentText());
    graph2->addToLegend();
    ui->widget->legend->itemWithPlottable(graph2)->setVisible(false);

    //Третий график

    graph3 = ui->widget->addGraph();
    graph3->setName(ui->materialPipe->currentText());
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

double MainWindow::on_voltage_editingFinished()
{
    voltage = ui->voltage->text().toDouble();
    return voltage;
}

void MainWindow::character()
{
    //Для к серии
    double yK = 3.8*qPow(10,-2);
    double wK = qPow((3.342*qPow(10,-2)*pipeNumber+0.008-qPow(10,-6)*qPow(pipeNumber,3)),4)*qPow((1+qPow((3.342*qPow(10,-2)*pipeNumber+0.008-qPow(10,-6)*qPow(pipeNumber,3)),4)),-1);
    double R = 1-(7*pipeNumber-80)/(14*pipeNumber-80);
    double pKa = 1-0.002*pipeNumber;
    double pKb = 0.002*pipeNumber;

    EK = 0.037-0.063*(0.01*pipeNumber)+125.4*qPow((0.01*pipeNumber),2.12)+22.44*qPow((0.01*pipeNumber),6);
    EKa = -0.097+1.463*(0.01*pipeNumber)+106.2*qPow((0.01*pipeNumber),2.12)+13.53*qPow((0.01*pipeNumber),6);
    EKb = 0.034-0.061*(0.01*pipeNumber)+123.2*qPow((0.01*pipeNumber),2.12)+13.42*qPow((0.01*pipeNumber),6);

    iKa = 5*qPow(10,14)*current*qPow(10,-3)*((yK*wK*pKa*R)/pipeNumber)*qPow(voltage/EK-1,1.67);
    iKb = 5*qPow(10,14)*current*qPow(10,-3)*((yK*wK*pKb*R)/pipeNumber)*qPow(voltage/EK-1,1.67);
}

double MainWindow::on_current_editingFinished()
{
    current = ui->current->text().toDouble();
    return current;
}

double MainWindow::on_targetThick_editingFinished()
{
    targetThick = ui->targetThick->text().toDouble();
    return targetThick;
}

double MainWindow::on_windowThick_editingFinished()
{
    windowThick = ui->windowThick->text().toDouble();
    return windowThick;
}

double MainWindow::on_distance_editingFinished()
{
    distance = ui->distance->text().toDouble();
    return distance;
}

void MainWindow::on_choosePipe_clicked()
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

    pipeNumber = number[ui->materialPipe->currentIndex()].toDouble();

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

    pipeMass = mass[ui->materialPipe->currentIndex()].toDouble();

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

    pipePlotnost = plotnost[ui->materialPipe->currentIndex()].toDouble();

    atomPlotnost.close();
}

void MainWindow::on_chooseFirst_clicked()
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

    firstNumber = number[ui->materialFirst->currentIndex()].toDouble();

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

    firstMass = mass[ui->materialFirst->currentIndex()].toDouble();

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

    firstPlotnost = plotnost[ui->materialFirst->currentIndex()].toDouble();

    atomPlotnost.close();
}

void MainWindow::on_chooseSecond_clicked()
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

    secondNumber = number[ui->materialSecond->currentIndex()].toDouble();

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

    secondMass = mass[ui->materialSecond->currentIndex()].toDouble();

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

    secondPlotnost = plotnost[ui->materialSecond->currentIndex()].toDouble();

    atomPlotnost.close();
}

void MainWindow::on_chooseThird_clicked()
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

    thirdNumber = number[ui->materialThird->currentIndex()].toDouble();

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

    thirdMass = mass[ui->materialThird->currentIndex()].toDouble();

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

    thirdPlotnost = plotnost[ui->materialThird->currentIndex()].toDouble();

    atomPlotnost.close();
}

void MainWindow::on_chooseEnv_clicked()
{
    QFile atomNumber(QCoreApplication::applicationDirPath() + "/Materials/enviroment_nomer.txt");
    QStringList number;
    if (atomNumber.open(QIODevice::ReadOnly))
    {
        while(!atomNumber.atEnd())
        {
            number << atomNumber.readLine();
        }
    }

    envNumber = number[ui->materialEnv->currentIndex()].toDouble();

    atomNumber.close();

    QFile atomMass(QCoreApplication::applicationDirPath() + "/Materials/enviroment_mass.txt");
    QStringList mass;
    if (atomMass.open(QIODevice::ReadOnly))
    {
        while(!atomMass.atEnd())
        {
            mass << atomMass.readLine();
        }
    }

    envMass = mass[ui->materialEnv->currentIndex()].toDouble();

    atomMass.close();

    QFile atomPlotnost(QCoreApplication::applicationDirPath() + "/Materials/enviroment_plotnost.txt");
    QStringList plotnost;
    if (atomPlotnost.open(QIODevice::ReadOnly))
    {
        while(!atomPlotnost.atEnd())
        {
            plotnost << atomPlotnost.readLine();
        }
    }

    envPlotnost = plotnost[ui->materialEnv->currentIndex()].toDouble();

    atomPlotnost.close();
}

double MainWindow::on_firstThick_editingFinished()
{
    firstThick = ui->firstThick->text().toDouble();
    return firstThick;
}

double MainWindow::on_secondThick_editingFinished()
{
    secondThick = ui->secondThick->text().toDouble();
    return secondThick;
}

double MainWindow::on_thirdThick_editingFinished()
{
    thirdThick = ui->thirdThick->text().toDouble();
    return thirdThick;
}

double MainWindow::oslableniyePipe(double x)
{
    QString tau;

    QString sigma;
    QString sigmaK;
    QString sigmaNK;

    QScriptEngine engine;

    QScriptValue tauValue;
    QScriptValue sigmaValue;
    QScriptValue sigmaKValue;
    QScriptValue sigmaNKValue;

    switch(ui->materialPipe->currentIndex())
    {
    case 0: //Air
        {
            if(x<3.203)
            {
                tau = "-(0.1991*Math.pow(10,-2))*X+(0.1169)*Math.pow(X,-1)-(0.1514*100)*Math.pow(X,-2)+(0.4759*10000)*Math.pow(X,-3)-(0.9060*1000)*Math.pow(X,-4)";
            }
            else
            {
               tau = "-(0.3514*Math.pow(10,2))*Math.pow(X,-2)+(0.5059*10000)*Math.pow(X,-3)-(0.1060*1000)*Math.pow(X,-4)+Math.pow((26.44*Math.pow(X,-1)+4.724+0.01809*X),-1)";
            }
            tau.replace("X", QString::number(x));
            tauValue = engine.evaluate(tau);
            weakening = tauValue.toNumber();
            return weakening;
            break;
        }
    case 1: //H
        {
            tau = "-(0.2645*Math.pow(10,-2))*X+(0.7086*0.1)*Math.pow(X,-1)-(0.7487)*Math.pow(X,-2)+(0.5575*10)*Math.pow(X,-3)-(-0.1936*10)*Math.pow(X,-4)";
            sigma = "0.4005*(1/1.008)*Math.pow((1+2*X/511),-2)*(1+2*X/511+0.3*Math.pow(2*X/511,2)-0.0625*Math.pow(2*X/511,3))";
            tau.replace("X", QString::number(x));
            sigma.replace("X", QString::number(x));
            tauValue = engine.evaluate(tau);
            sigmaValue = engine.evaluate(sigma);
            weakening = tauValue.toNumber()+sigmaValue.toNumber();
            return weakening;
            break;
        }
    case 2: //He
        {
            tau = "-(0.2154*Math.pow(10,-2))*X+(0.1473)*Math.pow(X,-1)-(0.3322*10)*Math.pow(X,-2)+(0.4893*100)*Math.pow(X,-3)-(-0.1576*100)*Math.pow(X,-4)";
            sigma = "0.4005*(1/4.003)*Math.pow(1+2*X/511,-2)*(1+2*X/511+0.3*Math.pow(2*X/511,2)-0.0625*Math.pow(2*X/511,3))";
            tau.replace("X", QString::number(x));
            sigma.replace("X", QString::number(x));
            tauValue = engine.evaluate(tau);
            sigmaValue = engine.evaluate(sigma);
            weakening = tauValue.toNumber()+sigmaValue.toNumber();
            return weakening;
            break;
        }
    case 3: //Li
        {
            tau = "-(0.3411*Math.pow(10,-2))*Math.pow(X,0)+(0.3088*Math.pow(10,0))*Math.pow(X,-1)-(0.1009*Math.pow(10,2))*Math.pow(X,-2)+(0.2076*Math.pow(10,3))*Math.pow(X,-3)-(-0.4091*Math.pow(10,2))*Math.pow(X,-4)";
            sigmaK = "(1+9.326*Math.pow(10,-2)*X)*Math.pow((1.781*Math.pow(10,0)+8.725*Math.pow(10,-1)*X+7.963*Math.pow(10,-2)*Math.pow(X,2)+8.225*Math.pow(10,-3)*Math.pow(X,3)),-1)";
            sigmaNK = "Math.pow((29.94*Math.pow(X,-1)+4.533+0.03637*X),-1)";
            tau.replace("X", QString::number(x));
            sigmaK.replace("X", QString::number(x));
            sigmaNK.replace("X", QString::number(x));
            tauValue = engine.evaluate(tau);
            sigmaKValue = engine.evaluate(sigmaK);
            sigmaNKValue = engine.evaluate(sigmaNK);
            weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
            return weakening;
            break;
        }
    case 4: //Be
    {
        tau = "-(0.3142*Math.pow(10,-2))*Math.pow(X,0)+(0.4216*Math.pow(10,0))*Math.pow(X,-1)-(0.2014*Math.pow(10,2))*Math.pow(X,-2)+(0.5918*Math.pow(10,3))*Math.pow(X,-3)-(-0.4857*Math.pow(10,2))*Math.pow(X,-4)";
        sigmaK = "(1-3.178*Math.pow(10,-2)*X)*Math.pow((1.267*Math.pow(10,0)+4.619*Math.pow(10,-1)*X+3.102*Math.pow(10,-2)*Math.pow(X,2)-1.493*Math.pow(10,-3)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((25.93*Math.pow(X,-1)+5.067+0.02420*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 5: //B
    {
        tau = "-(0.3267*Math.pow(10,-2))*X+(0.5682)*Math.pow(X,-1)-(0.3483*100)*Math.pow(X,-2)+(0.1326*10000)*Math.pow(X,-3)-(0.3243*100)*Math.pow(X,-4)";
        sigmaK = "(1+1.544*X)*Math.pow((1.010+1.561*X+6.978*0.1*Math.pow(X,2)+5.025*0.01*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((25.58*Math.pow(X,-1)+4.945+0.02191*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 6: //C
    {
        tau = "-(0.3172*Math.pow(10,-2))*X+(0.6921)*Math.pow(X,-1)-(0.5340*100)*Math.pow(X,-2)+(0.2610*10000)*Math.pow(X,-3)-(0.2941*1000)*Math.pow(X,-4)";
        sigmaK = "(1+1.108*X)*Math.pow((8.093*0.1+7.378*0.1*X+3.958*0.1*Math.pow(X,2)+2.532*0.01*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((25.91*Math.pow(X,-1)+4.644+0.01878*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 7: //N
    {
        tau = "-(0.1991*Math.pow(10,-2))*X+(0.6169)*Math.pow(X,-1)-(0.6514*100)*Math.pow(X,-2)+(0.4259*10000)*Math.pow(X,-3)-(0.8060*1000)*Math.pow(X,-4)";
        sigmaK = "(1+5.723*0.1*X)*Math.pow((7.260*0.1+2.910*0.1*X+1.824*0.1*Math.pow(X,2)+1.018*0.1*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((26.44*Math.pow(X,-1)+4.724+0.01809*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 8: //O
    {
        tau = "-(0.2663*Math.pow(10,-2))*X+(0.8397)*Math.pow(X,-1)-(0.9179*100)*Math.pow(X,-2)+(0.6634*10000)*Math.pow(X,-3)-(0.1906*10000)*Math.pow(X,-4)";
        sigmaK = "(1+3.537*0.1*X)*Math.pow((6.396*0.1+1.543*0.1*X+9.948*0.01*Math.pow(X,2)+4.981*0.001*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((29.88*Math.pow(X,-1)+4.656+0.01857*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 9: //F
    {
        tau = "-(0.3038*Math.pow(10,-2))*X+(0.9836)*Math.pow(X,-1)-(0.1128*1000)*Math.pow(X,-2)+(0.9171*10000)*Math.pow(X,-3)-(0.3414*10000)*Math.pow(X,-4)";
        sigmaK = "(1+2.437*0.1*X)*Math.pow((5.970*0.1+9.989*0.01*X+6.409*0.01*Math.pow(X,2)+2.930*0.001*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((33.40*Math.pow(X,-1)+4.961+0.01913*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 10: //Ne
    {
        tau = "-(0.1806*Math.pow(10,-2))*X+(0.7942)*Math.pow(X,-1)-(0.1218*1000)*Math.pow(X,-2)+(0.1307*100000)*Math.pow(X,-3)-(0.5600*10000)*Math.pow(X,-4)";
        sigmaK = "(1+1.820*0.1*X)*Math.pow((5.118*0.1+6.431*0.01*X+4.065*0.01*Math.pow(X,2)+1.715*0.001*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((37.30*Math.pow(X,-1)+4.629+0.01905*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 11: //Na
    {
        if(x>1.072)
        {
          tau = "-(0.3963*Math.pow(10,-2))*X+(0.1359*10)*Math.pow(X,-1)-(0.1720*1000)*Math.pow(X,-2)+(0.1766*100000)*Math.pow(X,-3)-(0.1019*100000)*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.8694*1000)*Math.pow(X,-3)-(0.2170*1000)*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.029*0.1*X)*Math.pow((4.721*0.1+6.884*0.01*X+2.528*0.01*Math.pow(X,2)+8.577*0.0001*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((38.80*Math.pow(X,-1)+4.901+0.01889*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 12: //Mg
    {
        if(x>1.305)
        {
          tau = "-(0.3447*Math.pow(10,-2))*X+(0.1309*Math.pow(10,1))*Math.pow(X,-1)-(0.1895*Math.pow(10,3))*Math.pow(X,-2)+(0.2327*Math.pow(10,5))*Math.pow(X,-3)-(0.1439*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1268*Math.pow(10,4))*Math.pow(X,-3)-(0.3474*Math.pow(10,3))*Math.pow(X,-4)";
        }
        sigmaK = "(1+8.484*Math.pow(10,-2)*X)*Math.pow((4.132*Math.pow(10,-1)+7.566*Math.pow(10,-2)*X+1.836*Math.pow(10,-2)*Math.pow(X,2)+6.034*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((39.81*Math.pow(X,-1)+4.746+0.01832*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 13: //Al
    {
        if(x>1.559)
        {
          tau = "-(0.3471*Math.pow(10,-2))*Math.pow(X,0)+(0.1384*Math.pow(10,1))*Math.pow(X,-1)-(0.2137*Math.pow(10,3))*Math.pow(X,-2)+(0.2950*Math.pow(10,5))*Math.pow(X,-3)-(0.2217*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1688*Math.pow(10,4)*Math.pow(X,-3))-(0.5046*Math.pow(10,3)*Math.pow(X,-4))";
        }
        sigmaK = "(1+8.580*Math.pow(10,-2)*X)*Math.pow((3.905*Math.pow(10,-1)+8.506*Math.pow(10,-2)*X+1.611*Math.pow(10,-2)*Math.pow(X,2)+5.524*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((42.58*Math.pow(X,-1)+4.873+0.01871*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 14: //Si
    {
        if(x>1.838)
        {
          tau = "-(0.2219*Math.pow(10,-2))*X+(0.1141*Math.pow(10,1))*Math.pow(X,-1)-(0.2202*Math.pow(10,3))*Math.pow(X,-2)+(0.3864*Math.pow(10,5))*Math.pow(X,-3)-(0.3319*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1640*Math.pow(10,3))*Math.pow(X,-2)+(0.1853*Math.pow(10,4))*Math.pow(X,-3)-(0.4500*Math.pow(10,3))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.970*X)*Math.pow((3.667*Math.pow(10,-1)+6.375*Math.pow(10,-1)*X+1.589*Math.pow(10,-1)*Math.pow(X,2)+1.114*Math.pow(10,-2)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((42.56*Math.pow(X,-1)+4.729+0.01796*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 15: //P
    {
        if(x>2.145)
        {
          tau = "(0.6962)*Math.pow(X,-1)-(0.2154*Math.pow(10,3))*Math.pow(X,-2)+(0.4590*Math.pow(10,5))*Math.pow(X,-3)-(0.4530*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1908*Math.pow(10,3))*Math.pow(X,-2)+(0.2308*Math.pow(10,4))*Math.pow(X,-3)-(0.5886*Math.pow(10,3))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.169*X)*Math.pow((3.420*Math.pow(10,-1)+6.676*Math.pow(10,-1)*X+1.740*Math.pow(10,-1)*Math.pow(X,2)+1.124*Math.pow(10,-2)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((45.17*Math.pow(X,-1)+4.889+0.01835*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 16: //S
    {
        if(x>2.472)
        {
          tau = "(0.7582)*Math.pow(X,-1)-(0.2447*Math.pow(10,3))*Math.pow(X,-2)+(0.5785*Math.pow(10,5))*Math.pow(X,-3)-(0.6419*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.2282*Math.pow(10,3))*Math.pow(X,-2)+(0.3007*Math.pow(10,4))*Math.pow(X,-3)-(0.8095*Math.pow(10,3))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.786*X)*Math.pow((3.095*Math.pow(10,-1)+4.495*Math.pow(10,-1)*X+1.360*Math.pow(10,-1)*Math.pow(X,2)+7.918*Math.pow(10,-2)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((41.69*Math.pow(X,-1)+4.900+0.01671*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 17: //Cl
    {
        if(x>2.822)
        {
          tau = "(0.7858)*Math.pow(X,-1)-(0.2734*Math.pow(10,3))*Math.pow(X,-2)+(0.7529*Math.pow(10,5))*Math.pow(X,-3)-(0.9287*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.2783*Math.pow(10,3))*Math.pow(X,-2)+(0.4027*Math.pow(10,4))*Math.pow(X,-3)-(0.1144*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.391*X)*Math.pow((3.181*Math.pow(10,-1)+3.659*Math.pow(10,-1)*X+1.077*Math.pow(10,-1)*Math.pow(X,2)+5.874*Math.pow(10,-2)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((44.68*Math.pow(X,-1)+5.115+0.01732*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 18: //Ar
    {
        if(x>3.206)
        {
          tau = "(0.7518)*Math.pow(X,-1)-(0.2633*Math.pow(10,3))*Math.pow(X,-2)+(0.7533*Math.pow(10,5))*Math.pow(X,-3)-(0.1050*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.2629*Math.pow(10,3))*Math.pow(X,-2)+(0.4167*Math.pow(10,4))*Math.pow(X,-3)-(0.1249*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.080*X)*Math.pow((3.078*Math.pow(10,-1)+2.799*Math.pow(10,-1)*X+8.688*Math.pow(10,-2)*Math.pow(X,2)+4.380*Math.pow(10,-3)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((49.09*Math.pow(X,-1)+5.452+0.01840*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 19: //K
    {
        if(x>3.607)
        {
          tau = "(0.6731)*Math.pow(X,-1)-(0.2717*Math.pow(10,3))*Math.pow(X,-2)+(0.9468*Math.pow(10,5))*Math.pow(X,-3)-(0.1384*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.3242*Math.pow(10,3))*Math.pow(X,-2)+(0.5459*Math.pow(10,4))*Math.pow(X,-3)-(0.1733*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+9.832*Math.pow(10,-1)*X)*Math.pow((2.733*Math.pow(10,-1)+2.506*Math.pow(10,-1)*X+6.843*Math.pow(10,-2)*Math.pow(X,2)+3.356*Math.pow(10,-3)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((46.28*Math.pow(X,-1)+5.080+0.01694*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 20: //Ca
    {
        if(x>4.038)
        {
          tau = "(0.7622)*Math.pow(X,-1)-(0.3106*Math.pow(10,3))*Math.pow(X,-2)+(0.1148*Math.pow(10,6))*Math.pow(X,-3)-(0.1902*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.3699*Math.pow(10,3))*Math.pow(X,-2)+(0.6774*Math.pow(10,4))*Math.pow(X,-3)-(0.2287*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.265*X)*Math.pow((2.507*Math.pow(10,-1)+2.991*Math.pow(10,-1)*X+8*Math.pow(10,-2)*Math.pow(X,2)+3.915*Math.pow(10,-3)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((45.51*Math.pow(X,-1)+4.977+0.01634*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 21: //Sc
    {
        if(x>4.492)
        {
          tau = "(0.7710)*Math.pow(X,-1)-(0.3377*Math.pow(10,3))*Math.pow(X,-2)+(0.1397*Math.pow(10,6))*Math.pow(X,-3)-(0.2524*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.3933*Math.pow(10,3))*Math.pow(X,-2)+(0.7473*Math.pow(10,4))*Math.pow(X,-3)-(0.2642*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+9.784*Math.pow(10,-1)*X)*Math.pow((2.502*Math.pow(10,-1)+2.318*Math.pow(10,-1)*X+6.513*Math.pow(10,-2)*Math.pow(X,2)+2.995*Math.pow(10,-3)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((47.76*Math.pow(X,-1)+5.397+0.01697*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 22: //Ti
    {
        if(x>4.966)
        {
          tau = "(0.6170)*Math.pow(X,-1)-(0.2987*Math.pow(10,3))*Math.pow(X,-2)+(0.1409*Math.pow(10,6))*Math.pow(X,-3)-(0.2757*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.4341*Math.pow(10,3))*Math.pow(X,-2)+(0.8580*Math.pow(10,4))*Math.pow(X,-3)-(0.3171*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+7.497*Math.pow(10,-1)*X)*Math.pow((2.462*Math.pow(10,-1)+1.731*Math.pow(10,-1)*X+4.960*Math.pow(10,-2)*Math.pow(X,2)+2.157*Math.pow(10,-3)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((50.03*Math.pow(X,-1)+5.490+0.01728*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 23: //V
    {
        if(x>5.465)
        {
          tau = "-(0.2018*Math.pow(10,3))*Math.pow(X,-2)+(0.1560*Math.pow(10,6))*Math.pow(X,-3)-(0.3252*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.4601*Math.pow(10,3))*Math.pow(X,-2)+(0.9829*Math.pow(10,4))*Math.pow(X,-3)-(0.3826*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.775*Math.pow(10,-1)*X)*Math.pow((2.367*Math.pow(10,-1)+1.307*Math.pow(10,-1)*X+3.807*Math.pow(10,-2)*Math.pow(X,2)+1.566*Math.pow(10,-3)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((52.62*Math.pow(X,-1)+5.580+0.01763*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 24: //Cr
    {
        if(x>5.989)
        {
          tau = "-(0.2213*Math.pow(10,3))*Math.pow(X,-2)+(0.1827*Math.pow(10,6))*Math.pow(X,-3)-(0.4226*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.5149*Math.pow(10,3))*Math.pow(X,-2)+(0.1154*Math.pow(10,5))*Math.pow(X,-3)-(0.4693*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+4.190*Math.pow(10,-1)*X)*Math.pow((2.292*Math.pow(10,-1)+8.642*Math.pow(10,-2)*X+2.697*Math.pow(10,-2)*Math.pow(X,2)+1.028*Math.pow(10,-3)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((54.13*Math.pow(X,-1)+5.430+0.01746*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 25: //Mn
    {
        if(x>6.539)
        {
          tau = "-(0.2245*Math.pow(10,3))*Math.pow(X,-2)+(0.2042*Math.pow(10,6))*Math.pow(X,-3)-(0.5055*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.5484*Math.pow(10,3))*Math.pow(X,-2)+(0.1312*Math.pow(10,5))*Math.pow(X,-3)-(0.5638*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+3.707*Math.pow(10,-1)*X)*Math.pow((2.167*Math.pow(10,-1)+8.029*Math.pow(10,-2)*X+2.324*Math.pow(10,-2)*Math.pow(X,2)+8.595*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((56.25*Math.pow(X,-1)+5.517+0.01764*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 26: //Fe
    {
        if(x>7.112)
        {
          tau = "-(0.2371*Math.pow(10,3))*Math.pow(X,-2)+(0.2359*Math.pow(10,6))*Math.pow(X,-3)-(0.6309*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.6176*Math.pow(10,3))*Math.pow(X,-2)+(0.1530*Math.pow(10,5))*Math.pow(X,-3)-(0.6905*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+3.075*Math.pow(10,-1)*X)*Math.pow((2.062*Math.pow(10,-1)+6.305*Math.pow(10,-2)*X+1.845*Math.pow(10,-2)*Math.pow(X,2)+6.535*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((56.92*Math.pow(X,-1)+5.382+0.01732*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 27: //Co
    {
        if(x>7.705)
        {
          tau = "-(0.2397*Math.pow(10,3))*Math.pow(X,-2)+(0.2612*Math.pow(10,6))*Math.pow(X,-3)-(0.7533*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.6579*Math.pow(10,3))*Math.pow(X,-2)+(0.1696*Math.pow(10,5))*Math.pow(X,-3)-(0.7918*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.591*Math.pow(10,-1)*X)*Math.pow((2.020*Math.pow(10,-1)+5.267*Math.pow(10,-2)*X+1.548*Math.pow(10,-2)*Math.pow(X,2)+5.231*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((59.76*Math.pow(X,-1)+5.460+0.01767*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 28: //Ni
    {
        if(x>8.332)
        {
          tau = "-(0.2531*Math.pow(10,3))*Math.pow(X,-2)+(0.3041*Math.pow(10,6))*Math.pow(X,-3)-(0.9428*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.7147*Math.pow(10,3))*Math.pow(X,-2)+(0.1993*Math.pow(10,5))*Math.pow(X,-3)-(0.9624*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.214*Math.pow(10,-1)*X)*Math.pow((1.870*Math.pow(10,-1)+4.225*Math.pow(10,-2)*X+1.244*Math.pow(10,-2)*Math.pow(X,2)+3.980*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((59.81*Math.pow(X,-1)+5.222+0.01711*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 29: //Cu
    {
        if(x>8.978)
        {
          tau = "-(0.2491*Math.pow(10,3))*Math.pow(X,-2)+(0.3252*Math.pow(10,6))*Math.pow(X,-3)-(0.1097*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(x<8.978 && x>1.096)
        {
          tau = "(0.7031*Math.pow(10,3))*Math.pow(X,-2)+(0.2165*Math.pow(10,5))*Math.pow(X,-3)-(0.1127*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1357*Math.pow(10,5))*Math.pow(X,-3)-(0.3001*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.902*Math.pow(10,-1)*X)*Math.pow((1.874*Math.pow(10,-1)+3.418*Math.pow(10,-2)*X+1.103*Math.pow(10,-2)*Math.pow(X,2)+3.367*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((65.59*Math.pow(X,-1)+5.428+0.01804*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 30: //Zn
    {
        if(x>9.658)
        {
          tau = "-(0.2426*Math.pow(10,3))*Math.pow(X,-2)+(0.3619*Math.pow(10,6))*Math.pow(X,-3)-(0.1289*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(9.658>x && x>1.193)
        {
          tau = "(0.7031*Math.pow(10,3))*Math.pow(X,-2)+(0.2165*Math.pow(10,5))*Math.pow(X,-3)-(0.1127*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.193>=x && x>=1.042)
        {
          tau = "(0.2889*Math.pow(10,5)*Math.pow(X,-3))-(0.1942*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(1.042>x && x>1.019)
        {
          tau = "(-0.1590*Math.pow(10,5)*Math.pow(X,-3))-(-0.2482*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.2572*Math.pow(10,4))*Math.pow(X,-3)-(0.1023*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.690*Math.pow(10,-1)*X)*Math.pow((1.786*Math.pow(10,-1)+3.177*Math.pow(10,-2)*X+9.452*Math.pow(10,-3)*Math.pow(X,2)+2.811*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((67.69*Math.pow(X,-1)+5.377+0.01810*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 31: //Ga
    {
        if(x>10.367)
        {
          tau = "-(0.2328*Math.pow(10,3))*Math.pow(X,-2)+(0.3881*Math.pow(10,6))*Math.pow(X,-3)-(0.1475*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(10.367>x && x>1.297)
        {
          tau = "(0.6933*Math.pow(10,3))*Math.pow(X,-2)+(0.2734*Math.pow(10,5))*Math.pow(X,-3)-(0.1657*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.297>x && x>1.142)
        {
          tau = "(0.3217*Math.pow(10,5)*Math.pow(X,-3))-(0.2363*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(1.142>x && x>1.115)
        {
          tau = "(-0.1315*Math.pow(10,5)*Math.pow(X,-3))-(-0.2503*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.2873*Math.pow(10,4))*Math.pow(X,-3)-(0.1181*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.458*Math.pow(10,-1)*X)*Math.pow((1.799*Math.pow(10,-1)+3.125*Math.pow(10,-2)*X+8.248*Math.pow(10,-3)*Math.pow(X,2)+2.369*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((65.9*Math.pow(X,-1)+5.722+0.01764*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 32: //Ge
    {
        if(x>11.103)
        {
          tau = "-(0.2231*Math.pow(10,3))*Math.pow(X,-2)+(0.4236*Math.pow(10,6))*Math.pow(X,-3)-(0.1708*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(11.103>x && x>1.414)
        {
          tau = "(0.7231*Math.pow(10,3))*Math.pow(X,-2)+(0.3038*Math.pow(10,5))*Math.pow(X,-3)-(0.1954*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.414>x && x>1.247)
        {
          tau = "(0.3416*Math.pow(10,5)*Math.pow(X,-3))-(0.2594*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(1.247>x && x>1.216)
        {
          tau = "(-0.1079*Math.pow(10,5)*Math.pow(X,-3))-(-0.2594*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.3266*Math.pow(10,4))*Math.pow(X,-3)-(0.1378*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.390*Math.pow(10,-1)*X)*Math.pow((1.756*Math.pow(10,-1)+3.140*Math.pow(10,-2)*X+7.573*Math.pow(10,-3)*Math.pow(X,2)+2.114*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((68.74*Math.pow(X,-1)+5.759+0.01789*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 33: //As
    {
        if(x>11.866)
        {
          tau = "-(0.2135*Math.pow(10,3))*Math.pow(X,-2)+(0.4644*Math.pow(10,6))*Math.pow(X,-3)-(0.1982*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(11.866>x && x>1.526)
        {
          tau = "(0.7490*Math.pow(10,3))*Math.pow(X,-2)+(0.3399*Math.pow(10,5))*Math.pow(X,-3)-(0.2339*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.526>x && x>1.358)
        {
          tau = "(0.3594*Math.pow(10,5)*Math.pow(X,-3))-(0.2762*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(1.358>x && x>1.323)
        {
          tau = "(-0.9541*Math.pow(10,4)*Math.pow(X,-3))-(-0.2846*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.3765*Math.pow(10,4))*Math.pow(X,-3)-(0.1650*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.465*Math.pow(10,-1)*X)*Math.pow((1.7*Math.pow(10,-1)+3.174*Math.pow(10,-2)*X+7.414*Math.pow(10,-3)*Math.pow(X,2)+2.197*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((70.84*Math.pow(X,-1)+5.755+0.01797*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 34: //Se
    {
        if(x>12.657)
        {
          tau = "-(0.1963*Math.pow(10,3))*Math.pow(X,-2)+(0.4978*Math.pow(10,6))*Math.pow(X,-3)-(0.2250*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(12.657>x && x>1.653)
        {
          tau = "(0.7661*Math.pow(10,3))*Math.pow(X,-2)+(0.3724*Math.pow(10,5))*Math.pow(X,-3)-(0.2740*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.653>x && x>1.476)
        {
          tau = "(0.3556*Math.pow(10,5)*Math.pow(X,-3))-(0.2627*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(1.476>x && x>1.435)
        {
          tau = "(-0.3092*Math.pow(10,4)*Math.pow(X,-3))-(-0.2333*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.4214*Math.pow(10,4))*Math.pow(X,-3)-(0.1902*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.665*Math.pow(10,-1)*X)*Math.pow((1.688*Math.pow(10,-1)+3.453*Math.pow(10,-2)*X+7.860*Math.pow(10,-3)*Math.pow(X,2)+2.404*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((74.43*Math.pow(X,-1)+5.881+0.01844*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 35: //Br
    {
        if(x>13.473)
        {
          tau = "-(0.1788*Math.pow(10,3))*Math.pow(X,-2)+(0.5528*Math.pow(10,6))*Math.pow(X,-3)-(0.2646*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(13.473>x && x>1.782)
        {
          tau = "(0.7869*Math.pow(10,3))*Math.pow(X,-2)+(0.4214*Math.pow(10,5))*Math.pow(X,-3)-(0.3277*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.782>x && x>1.596)
        {
          tau = "(0.3588*Math.pow(10,5)*Math.pow(X,-3))-(0.2389*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(1.596>x && x>1.549)
        {
          tau = "(-0.7092*Math.pow(10,4)*Math.pow(X,-3))-(-0.3479*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.4876*Math.pow(10,4))*Math.pow(X,-3)-(0.2258*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.040*Math.pow(10,-1)*X)*Math.pow((1.614*Math.pow(10,-1)+3.768*Math.pow(10,-2)*X+8.521*Math.pow(10,-3)*Math.pow(X,2)+2.778*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((74.84*Math.pow(X,-1)+5.785+0.01808*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 36: //Kr
    {
        if(x>14.325)
        {
          tau = "(0.5738*Math.pow(10,6))*Math.pow(X,-3)-(0.2753*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(14.325>x && x>1.921)
        {
          tau = "(0.8114*Math.pow(10,3))*Math.pow(X,-2)+(0.4562*Math.pow(10,5))*Math.pow(X,-3)-(0.3722*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.921>x && x>1.727)
        {
          tau = "(0.3388*Math.pow(10,5)*Math.pow(X,-3))-(0.1775*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(1.727>x && x>1.671)
        {
          tau = "(-0.3331*Math.pow(10,5)*Math.pow(X,-3))-(-0.8565*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.5401*Math.pow(10,4))*Math.pow(X,-3)-(0.2556*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.563*Math.pow(10,-1)*X)*Math.pow((1.605*Math.pow(10,-1)+4.357*Math.pow(10,-2)*X+9.829*Math.pow(10,-3)*Math.pow(X,2)+3.412*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((78.38*Math.pow(X,-1)+5.888+0.01853*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 37: //Rb
    {
        if(x>15.199)
        {
          tau = "(0.6321*Math.pow(10,6))*Math.pow(X,-3)-(0.3222*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(15.199>x && x>2.065)
        {
          tau = "(0.8061*Math.pow(10,3))*Math.pow(X,-2)+(0.5107*Math.pow(10,5))*Math.pow(X,-3)-(0.4410*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.065>x && x>1.863)
        {
          tau = "(0.4800*Math.pow(10,5)*Math.pow(X,-3))-(0.4165*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(1.863>x && x>1.804)
        {
          tau = "(0.2975*Math.pow(10,5)*Math.pow(X,-3))-(-0.2098*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.6124*Math.pow(10,4))*Math.pow(X,-3)-(0.2965*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.660*Math.pow(10,-1)*X)*Math.pow((1.539*Math.pow(10,-1)+3.449*Math.pow(10,-2)*X+6.750*Math.pow(10,-3)*Math.pow(X,2)+2.079*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((79.11*Math.pow(X,-1)+5.851+0.01835*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 38: //Sr
    {
        if(x>16.104)
        {
          tau = "(0.6915*Math.pow(10,6))*Math.pow(X,-3)-(0.3763*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(16.104>x && x>2.216)
        {
          tau = "(0.8448*Math.pow(10,3))*Math.pow(X,-2)+(0.5659*Math.pow(10,5))*Math.pow(X,-3)-(0.5234*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.216>x && x>2.006)
        {
          tau = "(0.5024*Math.pow(10,5)*Math.pow(X,-3))-(0.4284*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(2.006>x && x>1.939)
        {
          tau = "(0.2310*Math.pow(10,5)*Math.pow(X,-3))-(-0.4765*Math.pow(10,4)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1040*Math.pow(10,4))*Math.pow(X,-2)+(0.4001*Math.pow(10,4))*Math.pow(X,-3)-(0.1553*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.310*Math.pow(10,-1)*X)*Math.pow((1.488*Math.pow(10,-1)+3.124*Math.pow(10,-2)*X+5.429*Math.pow(10,-3)*Math.pow(X,2)+1.580*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((79.79*Math.pow(X,-1)+5.863+0.01821*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 39: //Y
    {
        if(x>17.038)
        {
          tau = "(0.7615*Math.pow(10,6))*Math.pow(X,-3)-(0.4412*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(17.038>x && x>2.372)
        {
          tau = "(0.8931*Math.pow(10,3))*Math.pow(X,-2)+(0.6297*Math.pow(10,5))*Math.pow(X,-3)-(0.6171*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.372>x && x>2.155)
        {
          tau = "(0.5458*Math.pow(10,5)*Math.pow(X,-3))-(0.4732*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(2.155>x && x>2.080)
        {
          tau = "(0.2326*Math.pow(10,5)*Math.pow(X,-3))-(-0.3694*Math.pow(10,4)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1148*Math.pow(10,4))*Math.pow(X,-2)+(0.4487*Math.pow(10,4))*Math.pow(X,-3)-(0.1768*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.794*Math.pow(10,-1)*X)*Math.pow((1.435*Math.pow(10,-1)+3.612*Math.pow(10,-2)*X+6.434*Math.pow(10,-3)*Math.pow(X,2)+2.056*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((80.83*Math.pow(X,-1)+5.8+0.01798*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 40: //Zr
    {
        if(x>17.997)
        {
          tau = "(0.8263*Math.pow(10,6))*Math.pow(X,-3)-(0.5088*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(17.997>x && x>2.531)
        {
          tau = "(0.9174*Math.pow(10,3))*Math.pow(X,-2)+(0.6902*Math.pow(10,5))*Math.pow(X,-3)-(0.7135*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.531>x && x>2.306)
        {
          tau = "(0.5865*Math.pow(10,5)*Math.pow(X,-3))-(0.5184*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(2.306>x && x>2.222)
        {
          tau = "(0.2511*Math.pow(10,5)*Math.pow(X,-3))-(-0.2008*Math.pow(10,4)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1245*Math.pow(10,4))*Math.pow(X,-2)+(0.4942*Math.pow(10,4))*Math.pow(X,-3)-(0.1983*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+3.040*Math.pow(10,-1)*X)*Math.pow((1.421*Math.pow(10,-1)+4.878*Math.pow(10,-2)*X+9.372*Math.pow(10,-3)*Math.pow(X,2)+3.381*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((81.87*Math.pow(X,-1)+5.790+0.01805*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 41: //Nb
    {
        if(x>18.985)
        {
          tau = "(0.9001*Math.pow(10,6))*Math.pow(X,-3)-(0.5848*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(18.985>x && x>2.697)
        {
          tau = "(0.9588*Math.pow(10,3))*Math.pow(X,-2)+(0.7570*Math.pow(10,5))*Math.pow(X,-3)-(0.8170*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.697>x && x>2.464)
        {
          tau = "(0.6456*Math.pow(10,5)*Math.pow(X,-3))-(0.6001*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(2.464>x && x>2.370)
        {
          tau = "(0.2869*Math.pow(10,5)*Math.pow(X,-3))-(-0.4479*Math.pow(10,4)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1320*Math.pow(10,4))*Math.pow(X,-2)+(0.5559*Math.pow(10,4))*Math.pow(X,-3)-(0.2278*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+4.815*Math.pow(10,-1)*X)*Math.pow((1.391*Math.pow(10,-1)+6.505*Math.pow(10,-2)*X+1.343*Math.pow(10,-2)*Math.pow(X,2)+5.147*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((77.98*Math.pow(X,-1)+5.895+0.01722*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 42: //Mo
    {
        if(x>19.999)
        {
          tau = "(0.9649*Math.pow(10,6))*Math.pow(X,-3)-(0.6635*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(19.999>x && x>2.865)
        {
          tau = "(0.9956*Math.pow(10,3))*Math.pow(X,-2)+(0.8166*Math.pow(10,5))*Math.pow(X,-3)-(0.9212*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.865>x && x>2.625)
        {
          tau = "(0.6946*Math.pow(10,5)*Math.pow(X,-3))-(0.6720*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(2.625>x && x>2.520)
        {
          tau = "(0.3382*Math.pow(10,5)*Math.pow(X,-3))-(0.5891*Math.pow(10,4)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1371*Math.pow(10,4))*Math.pow(X,-2)+(0.6170*Math.pow(10,4))*Math.pow(X,-3)-(0.2607*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.411*Math.pow(10,-1)*X)*Math.pow((1.345*Math.pow(10,-1)+7.060*Math.pow(10,-2)*X+1.463*Math.pow(10,-2)*Math.pow(X,2)+5.588*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((80.67*Math.pow(X,-1)+5.926+0.01748*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 43: //Tc
    {
        if(x>21.044)
        {
          tau = "(0.1034*Math.pow(10,7))*Math.pow(X,-3)-(0.7517*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(21.044>x && x>3.042)
        {
          tau = "(0.9760*Math.pow(10,3))*Math.pow(X,-2)+(0.8887*Math.pow(10,5))*Math.pow(X,-3)-(0.1071*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(3.042>x && x>2.793)
        {
          tau = "(0.7487*Math.pow(10,5)*Math.pow(X,-3))-(0.7579*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(2.793>x && x>2.676)
        {
          tau = "(0.4015*Math.pow(10,5)*Math.pow(X,-3))-(0.1605*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1452*Math.pow(10,4))*Math.pow(X,-2)+(0.6728*Math.pow(10,4))*Math.pow(X,-3)-(0.2892*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.743*Math.pow(10,-1)*X)*Math.pow((1.311*Math.pow(10,-1)+7.273*Math.pow(10,-2)*X+1.496*Math.pow(10,-2)*Math.pow(X,2)+5.695*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((81.49*Math.pow(X,-1)+5.927+0.01735*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 44: //Ru
    {
        if(x>22.117)
        {
          tau = "(0.1116*Math.pow(10,7))*Math.pow(X,-3)-(0.8554*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(22.117>x && x>3.224)
        {
          tau = "(0.1018*Math.pow(10,4))*Math.pow(X,-2)+(0.9670*Math.pow(10,5))*Math.pow(X,-3)-(0.1222*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(3.224>x && x>2.966)
        {
          tau = "(0.8078*Math.pow(10,5)*Math.pow(X,-3))-(0.8411*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(2.966>x && x>2.837)
        {
          tau = "(0.4695*Math.pow(10,5)*Math.pow(X,-3))-(0.2717*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1537*Math.pow(10,4))*Math.pow(X,-2)+(0.7433*Math.pow(10,4))*Math.pow(X,-3)-(0.3262*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.771*Math.pow(10,-1)*X)*Math.pow((1.297*Math.pow(10,-1)+7.016*Math.pow(10,-2)*X+1.493*Math.pow(10,-2)*Math.pow(X,2)+5.478*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((83.16*Math.pow(X,-1)+5.987+0.01743*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 45: //Rh
    {
        if(x>23.219)
        {
          tau = "(0.1205*Math.pow(10,7))*Math.pow(X,-3)-(0.9730*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(23.219>x && x>3.411)
        {
          tau = "(0.1050*Math.pow(10,4))*Math.pow(X,-2)+(0.1054*Math.pow(10,6))*Math.pow(X,-3)-(0.1398*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(3.411>x && x>3.146)
        {
          tau = "(0.9003*Math.pow(10,5)*Math.pow(X,-3))-(0.1030*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(3.146>x && x>3.003)
        {
          tau = "(0.5655*Math.pow(10,5)*Math.pow(X,-3))-(0.4734*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1612*Math.pow(10,4))*Math.pow(X,-2)+(0.8250*Math.pow(10,4))*Math.pow(X,-3)-(0.3703*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.623*Math.pow(10,-1)*X)*Math.pow((1.281*Math.pow(10,-1)+6.536*Math.pow(10,-2)*X+1.422*Math.pow(10,-2)*Math.pow(X,2)+5.096*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((90.02*Math.pow(X,-1)+5.803+0.01826*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 46: //Pd
    {
        if(x>24.350)
        {
          tau = "(0.1279*Math.pow(10,7))*Math.pow(X,-3)-(0.1088*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(24.350>x && x>3.604)
        {
          tau = "(0.1085*Math.pow(10,4))*Math.pow(X,-2)+(0.1123*Math.pow(10,6))*Math.pow(X,-3)-(0.1547*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(3.604>x && x>3.330)
        {
          tau = "(0.1024*Math.pow(10,6)*Math.pow(X,-3))-(0.1371*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(3.330>x && x>3.173)
        {
          tau = "(0.7496*Math.pow(10,5)*Math.pow(X,-3))-(0.1008*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1693*Math.pow(10,4))*Math.pow(X,-2)+(0.8872*Math.pow(10,4))*Math.pow(X,-3)-(0.4040*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.415*Math.pow(10,-1)*X)*Math.pow((1.272*Math.pow(10,-1)+6.081*Math.pow(10,-2)*X+1.374*Math.pow(10,-2)*Math.pow(X,2)+4.715*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((92.23*Math.pow(X,-1)+5.883+0.01841*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 47: //Ag
    {
        if(x>25.514)
        {
          tau = "(0.1384*Math.pow(10,7))*Math.pow(X,-3)-(0.1238*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(25.514>x && x>3.805)
        {
          tau = "(0.1135*Math.pow(10,4))*Math.pow(X,-2)+(0.1223*Math.pow(10,6))*Math.pow(X,-3)-(0.1765*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(3.805>x && x>3.523)
        {
          tau = "(0.1075*Math.pow(10,6)*Math.pow(X,-3))-(0.1415*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(3.523>x && x>3.351)
        {
          tau = "(0.7408*Math.pow(10,5)*Math.pow(X,-3))-(0.8834*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1818*Math.pow(10,4))*Math.pow(X,-2)+(0.9723*Math.pow(10,4))*Math.pow(X,-3)-(0.4517*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.082*Math.pow(10,-1)*X)*Math.pow((1.214*Math.pow(10,-1)+5.642*Math.pow(10,-2)*X+1.250*Math.pow(10,-2)*Math.pow(X,2)+4.238*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((92.27*Math.pow(X,-1)+5.857+0.01817*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 48: //Cd
    {
        if(x>26.711)
        {
          tau = "(0.1453*Math.pow(10,7))*Math.pow(X,-3)-(0.1366*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(26.711>x && x>4.018)
        {
          tau = "(0.1102*Math.pow(10,4))*Math.pow(X,-2)+(0.1305*Math.pow(10,6))*Math.pow(X,-3)-(0.2009*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(4.018>x && x>3.727)
        {
          tau = "(0.1142*Math.pow(10,6)*Math.pow(X,-3))-(0.1588*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(3.727>x && x>3.537)
        {
          tau = "(0.7830*Math.pow(10,5)*Math.pow(X,-3))-(0.9738*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1874*Math.pow(10,4))*Math.pow(X,-2)+(0.1040*Math.pow(10,5))*Math.pow(X,-3)-(0.4943*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+4.921*Math.pow(10,-1)*X)*Math.pow((1.230*Math.pow(10,-1)+5.532*Math.pow(10,-2)*X+1.208*Math.pow(10,-2)*Math.pow(X,2)+4.035*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((94.65*Math.pow(X,-1)+6.003+0.01841*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 49: //In
    {
        if(x>27.939)
        {
          tau = "(0.1551*Math.pow(10,7))*Math.pow(X,-3)-(0.1519*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(27.939>x && x>4.237)
        {
          tau = "(0.1130*Math.pow(10,4))*Math.pow(X,-2)+(0.1406*Math.pow(10,6))*Math.pow(X,-3)-(0.2257*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(4.237>x && x>3.938)
        {
          tau = "(0.1270*Math.pow(10,6)*Math.pow(X,-3))-(0.1957*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(3.938>x && x>3.730)
        {
          tau = "(0.1006*Math.pow(10,6)*Math.pow(X,-3))-(0.1736*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1980*Math.pow(10,4))*Math.pow(X,-2)+(0.1131*Math.pow(10,5))*Math.pow(X,-3)-(0.5499*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+4.741*Math.pow(10,-1)*X)*Math.pow((1.199*Math.pow(10,-1)+5.382*Math.pow(10,-2)*X+1.136*Math.pow(10,-2)*Math.pow(X,2)+3.757*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((95.15*Math.pow(X,-1)+6.032+0.01830*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 50: //Sn
    {
        if(x>29.200)
        {
          tau = "(0.1630*Math.pow(10,7))*Math.pow(X,-3)-(0.1643*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(29.200>x && x>4.464)
        {
          tau = "(0.1152*Math.pow(10,4))*Math.pow(X,-2)+(0.1492*Math.pow(10,6))*Math.pow(X,-3)-(0.2495*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(4.464>x && x>4.156)
        {
          tau = "(0.1428*Math.pow(10,6)*Math.pow(X,-3))-(0.2532*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(4.156>x && x>3.928)
        {
          tau = "(0.1097*Math.pow(10,6)*Math.pow(X,-3))-(0.2033*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.2051*Math.pow(10,4))*Math.pow(X,-2)+(0.1216*Math.pow(10,5))*Math.pow(X,-3)-(0.6037*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+4.821*Math.pow(10,-1)*X)*Math.pow((1.200*Math.pow(10,-1)+5.513*Math.pow(10,-2)*X+1.138*Math.pow(10,-2)*Math.pow(X,2)+3.740*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((96.63*Math.pow(X,-1)+6.142+0.01838*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 51: //Sb
    {
        if(x>30.491)
        {
          tau = "(0.1734*Math.pow(10,7))*Math.pow(X,-3)-(0.1865*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(30.491>x && x>4.698)
        {
          tau = "(0.1139*Math.pow(10,4))*Math.pow(X,-2)+(0.1604*Math.pow(10,6))*Math.pow(X,-3)-(0.2815*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(4.698>x && x>4.380)
        {
          tau = "(0.1527*Math.pow(10,6)*Math.pow(X,-3))-(0.2838*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(4.380>x && x>4.132)
        {
          tau = "(0.1276*Math.pow(10,6)*Math.pow(X,-3))-(0.2745*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.2136*Math.pow(10,4))*Math.pow(X,-2)+(0.1315*Math.pow(10,5))*Math.pow(X,-3)-(0.6735*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.118*Math.pow(10,-1)*X)*Math.pow((1.184*Math.pow(10,-1)+5.761*Math.pow(10,-2)*X+1.178*Math.pow(10,-2)*Math.pow(X,2)+3.863*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((97.76*Math.pow(X,-1)+6.193+0.01844*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 52: //Te
    {
        if(x>31.813)
        {
          tau = "(0.1797*Math.pow(10,7))*Math.pow(X,-3)-(0.2018*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(31.813>x && x>4.939)
        {
          tau = "(0.1161*Math.pow(10,4))*Math.pow(X,-2)+(0.1668*Math.pow(10,6))*Math.pow(X,-3)-(0.3041*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(4.939>x && x>4.612)
        {
          tau = "(0.1589*Math.pow(10,6)*Math.pow(X,-3))-(0.3071*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(4.612>x && x>4.341)
        {
          tau = "(0.1299*Math.pow(10,6)*Math.pow(X,-3))-(0.2849*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.2199*Math.pow(10,4))*Math.pow(X,-2)+(0.1381*Math.pow(10,5))*Math.pow(X,-3)-(0.7426*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.467*Math.pow(10,-1)*X)*Math.pow((1.186*Math.pow(10,-1)+6.083*Math.pow(10,-2)*X+1.258*Math.pow(10,-2)*Math.pow(X,2)+4.122*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((100.7*Math.pow(X,-1)+6.399+0.01879*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 53: //I
    {
        if(x>33.169)
        {
          tau = "(0.1960*Math.pow(10,7))*Math.pow(X,-3)-(0.2296*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(33.169>x && x>5.188)
        {
          tau = "(0.1219*Math.pow(10,4))*Math.pow(X,-2)+(0.1834*Math.pow(10,6))*Math.pow(X,-3)-(0.3505*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(5.188>x && x>4.852)
        {
          tau = "(0.1703*Math.pow(10,6)*Math.pow(X,-3))-(0.3295*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(4.852>x && x>4.557)
        {
          tau = "(0.1382*Math.pow(10,6)*Math.pow(X,-3))-(0.3053*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(4.557>x && x>1.072)
        {
          tau = "(0.2328*Math.pow(10,4))*Math.pow(X,-2)+(0.1552*Math.pow(10,5))*Math.pow(X,-3)-(0.8511*Math.pow(10,4))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1807*Math.pow(10,5))*Math.pow(X,-3)-(0.8981*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.921*Math.pow(10,-1)*X)*Math.pow((1.144*Math.pow(10,-1)+6.142*Math.pow(10,-2)*X+1.292*Math.pow(10,-2)*Math.pow(X,2)+4.238*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((98.33*Math.pow(X,-1)+6.276+0.01819*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 54: //Xe
    {
        if(x>34.564)
        {
          tau = "(0.2049*Math.pow(10,7))*Math.pow(X,-3)-(0.2505*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(34.564>x && x>5.452)
        {
          tau = "(0.1243*Math.pow(10,4))*Math.pow(X,-2)+(0.1933*Math.pow(10,6))*Math.pow(X,-3)-(0.3852*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(5.452>x && x>5.103)
        {
          tau = "(0.1787*Math.pow(10,6)*Math.pow(X,-3))-(0.3597*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.103>x && x>4.782)
        {
          tau = "(0.1715*Math.pow(10,6)*Math.pow(X,-3))-(0.3859*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(4.782>x && x>1.148)
        {
          tau = "(0.2349*Math.pow(10,4))*Math.pow(X,-2)+(0.1681*Math.pow(10,5))*Math.pow(X,-3)-(0.9660*Math.pow(10,4))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1898*Math.pow(10,5))*Math.pow(X,-3)-(0.9576*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+6.399*Math.pow(10,-1)*X)*Math.pow((1.131*Math.pow(10,-1)+6.545*Math.pow(10,-2)*X+1.382*Math.pow(10,-2)*Math.pow(X,2)+4.502*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((100.3*Math.pow(X,-1)+6.394+0.01839*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 55: //Cs
    {
        if(x>35.984)
        {
          tau = "(0.2187*Math.pow(10,7))*Math.pow(X,-3)-(0.2781*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(35.984>x && x>5.714)
        {
          tau = "(0.1285*Math.pow(10,4))*Math.pow(X,-2)+(0.2082*Math.pow(10,6))*Math.pow(X,-3)-(0.4344*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(5.714>x && x>5.359)
        {
          tau = "(0.1796*Math.pow(10,6)*Math.pow(X,-3))-(0.3318*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.359>x && x>5.011)
        {
          tau = "(0.1253*Math.pow(10,6)*Math.pow(X,-3))-(0.2100*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.011>x && x>1.217)
        {
          tau = "(0.2368*Math.pow(10,4))*Math.pow(X,-2)+(0.1871*Math.pow(10,5))*Math.pow(X,-3)-(0.1126*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.217>x && x>1.065)
        {
          tau = "(0.2142*Math.pow(10,5))*Math.pow(X,-3)-(0.1166*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1854*Math.pow(10,5))*Math.pow(X,-3)-(0.9182*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.245*Math.pow(10,-1)*X)*Math.pow((1.108*Math.pow(10,-1)+5.572*Math.pow(10,-2)*X+1.119*Math.pow(10,-2)*Math.pow(X,2)+3.552*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((99.67*Math.pow(X,-1)+6.385+0.01814*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 56: //Ba
    {
        if(x>37.440)
        {
          tau = "(0.2281*Math.pow(10,7))*Math.pow(X,-3)-(0.2993*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(37.440>x && x>5.988)
        {
          tau = "(0.1297*Math.pow(10,4))*Math.pow(X,-2)+(0.2196*Math.pow(10,6))*Math.pow(X,-3)-(0.4812*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(5.988>x && x>5.623)
        {
          tau = "(0.1930*Math.pow(10,6)*Math.pow(X,-3))-(0.3876*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.623>x && x>5.247)
        {
          tau = "(0.1411*Math.pow(10,6)*Math.pow(X,-3))-(0.2814*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.247>x && x>1.292)
        {
          tau = "(0.2397*Math.pow(10,4))*Math.pow(X,-2)+(0.2017*Math.pow(10,5))*Math.pow(X,-3)-(0.1272*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.292>x && x>1.136)
        {
          tau = "(0.2323*Math.pow(10,5))*Math.pow(X,-3)-(0.1334*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.136>x && x>1.062)
        {
          tau = "(0.1982*Math.pow(10,5))*Math.pow(X,-3)-(0.1019*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1647*Math.pow(10,5))*Math.pow(X,-3)-(0.7933*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+4.967*Math.pow(10,-1)*X)*Math.pow((1.101*Math.pow(10,-1)+5.450*Math.pow(10,-2)*X+1.055*Math.pow(10,-2)*Math.pow(X,2)+3.312*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((101.1*Math.pow(X,-1)+6.513+0.01826*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 57: //La
    {
        if(x>38.924)
        {
          tau = "(0.2430*Math.pow(10,7))*Math.pow(X,-3)-(0.3295*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(38.924>x && x>6.266)
        {
          tau = "(0.1337*Math.pow(10,4))*Math.pow(X,-2)+(0.2358*Math.pow(10,6))*Math.pow(X,-3)-(0.5375*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(6.266>x && x>5.890)
        {
          tau = "(0.2049*Math.pow(10,6)*Math.pow(X,-3))-(0.4180*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.890>x && x>5.482)
        {
          tau = "(0.1487*Math.pow(10,6)*Math.pow(X,-3))-(0.2982*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.482>x && x>1.361)
        {
          tau = "(0.2447*Math.pow(10,4))*Math.pow(X,-2)+(0.2229*Math.pow(10,5))*Math.pow(X,-3)-(0.1474*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.361>x && x>1.204)
        {
          tau = "(0.2551*Math.pow(10,5))*Math.pow(X,-3)-(0.1536*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.204>x && x>1.123)
        {
          tau = "(0.2165*Math.pow(10,5))*Math.pow(X,-3)-(0.1155*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1674*Math.pow(10,5))*Math.pow(X,-3)-(0.7655*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.818*Math.pow(10,-1)*X)*Math.pow((1.076*Math.pow(10,-1)+6.162*Math.pow(10,-2)*X+1.187*Math.pow(10,-2)*Math.pow(X,2)+3.746*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((101.4*Math.pow(X,-1)+6.499+0.01804*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 58: //Ce
    {
        if(x>40.443)
        {
          tau = "(0.2601*Math.pow(10,7))*Math.pow(X,-3)-(0.3722*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(40.443>x && x>6.548)
        {
          tau = "(0.1342*Math.pow(10,4))*Math.pow(X,-2)+(0.2540*Math.pow(10,6))*Math.pow(X,-3)-(0.6049*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(6.548>x && x>6.164)
        {
          tau = "(0.2237*Math.pow(10,6)*Math.pow(X,-3))-(0.4912*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(6.164>x && x>5.723)
        {
          tau = "(0.1597*Math.pow(10,6)*Math.pow(X,-3))-(0.3306*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.723>x && x>1.434)
        {
          tau = "(0.2476*Math.pow(10,4))*Math.pow(X,-2)+(0.2440*Math.pow(10,5))*Math.pow(X,-3)-(0.1673*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.434>x && x>1.272)
        {
          tau = "(0.2770*Math.pow(10,5))*Math.pow(X,-3)-(0.1729*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.272>x && x>1.185)
        {
          tau = "(0.2335*Math.pow(10,5))*Math.pow(X,-3)-(0.1278*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1737*Math.pow(10,5))*Math.pow(X,-3)-(0.7655*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.099*Math.pow(10,-1)*X)*Math.pow((1.048*Math.pow(10,-1)+5.311*Math.pow(10,-2)*X+1.027*Math.pow(10,-2)*Math.pow(X,2)+3.148*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((99.48*Math.pow(X,-1)+6.484+0.01768*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 59: //Pr
    {
        if(x>41.990)
        {
          tau = "(0.2783*Math.pow(10,7))*Math.pow(X,-3)-(0.4146*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(41.990>x && x>6.834)
        {
          tau = "(0.1414*Math.pow(10,4))*Math.pow(X,-2)+(0.2730*Math.pow(10,6))*Math.pow(X,-3)-(0.6747*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(6.834>x && x>6.440)
        {
          tau = "(0.2411*Math.pow(10,6)*Math.pow(X,-3))-(0.5525*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(6.440>x && x>5.964)
        {
          tau = "(0.1669*Math.pow(10,6)*Math.pow(X,-3))-(0.4316*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.964>x && x>1.511)
        {
          tau = "(0.2604*Math.pow(10,4))*Math.pow(X,-2)+(0.2636*Math.pow(10,5))*Math.pow(X,-3)-(0.1858*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.511>x && x>1.337)
        {
          tau = "(0.3020*Math.pow(10,5))*Math.pow(X,-3)-(0.1959*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.337>x && x>1.242)
        {
          tau = "(0.2562*Math.pow(10,5))*Math.pow(X,-3)-(0.1463*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1784*Math.pow(10,5))*Math.pow(X,-3)-(0.7266*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+3.925*Math.pow(10,-1)*X)*Math.pow((1.008*Math.pow(10,-1)+4.051*Math.pow(10,-2)*X+7.917*Math.pow(10,-3)*Math.pow(X,2)+2.310*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((100.1*Math.pow(X,-1)+6.405+0.01752*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 60: //Nd
    {
        if(x>43.568)
        {
          tau = "(0.2920*Math.pow(10,7))*Math.pow(X,-3)-(0.4524*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(43.568>x && x>7.126)
        {
          tau = "(0.1465*Math.pow(10,4))*Math.pow(X,-2)+(0.2880*Math.pow(10,6))*Math.pow(X,-3)-(0.7393*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(7.126>x && x>6.721)
        {
          tau = "(0.2525*Math.pow(10,6)*Math.pow(X,-3))-(0.5923*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(6.721>x && x>6.207)
        {
          tau = "(0.1750*Math.pow(10,6)*Math.pow(X,-3))-(0.3669*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(6.207>x && x>1.575)
        {
          tau = "(0.2581*Math.pow(10,4))*Math.pow(X,-2)+(0.2857*Math.pow(10,5))*Math.pow(X,-3)-(0.2112*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.575>x && x>1.402)
        {
          tau = "(0.3222*Math.pow(10,5))*Math.pow(X,-3)-(0.2164*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.402>x && x>1.297)
        {
          tau = "(0.2732*Math.pow(10,5))*Math.pow(X,-3)-(0.1612*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.2017*Math.pow(10,5))*Math.pow(X,-3)-(0.9345*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+3.511*Math.pow(10,-1)*X)*Math.pow((1.004*Math.pow(10,-1)+3.634*Math.pow(10,-2)*X+7.104*Math.pow(10,-3)*Math.pow(X,2)+2.012*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((101.9*Math.pow(X,-1)+6.457+0.01757*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 61: //Pm
    {
        if(x>45.184)
        {
          tau = "(0.3115*Math.pow(10,7))*Math.pow(X,-3)-(0.5004*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(45.184>x && x>7.427)
        {
          tau = "(0.1538*Math.pow(10,4))*Math.pow(X,-2)+(0.3092*Math.pow(10,6))*Math.pow(X,-3)-(0.8237*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(7.427>x && x>7.012)
        {
          tau = "(0.2740*Math.pow(10,6)*Math.pow(X,-3))-(0.6806*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(7.012>x && x>6.459)
        {
          tau = "(0.1852*Math.pow(10,6)*Math.pow(X,-3))-(0.3906*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(6.459>x && x>1.650)
        {
          tau = "(0.2665*Math.pow(10,4))*Math.pow(X,-2)+(0.3111*Math.pow(10,5))*Math.pow(X,-3)-(0.2388*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.650>x && x>1.471)
        {
          tau = "(0.3519*Math.pow(10,5))*Math.pow(X,-3)-(0.2471*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.471>x && x>1.356)
        {
          tau = "(0.2907*Math.pow(10,5))*Math.pow(X,-3)-(0.1728*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.356>x && x>1.051)
        {
          tau = "(0.2165*Math.pow(10,5))*Math.pow(X,-3)-(0.1022*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.051>x && x>1.026)
        {
          tau = "(0.1631*Math.pow(10,5))*Math.pow(X,-3)-(0.8906*Math.pow(10,4))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.4450*Math.pow(10,4))*Math.pow(X,-3)-(0.2405*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+3.211*Math.pow(10,-1)*X)*Math.pow((9.763*Math.pow(10,-2)+3.251*Math.pow(10,-2)*X+6.383*Math.pow(10,-3)*Math.pow(X,2)+1.763*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((105.4*Math.pow(X,-1)+6.380+0.01743*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 62: //Sm
    {
        if(x>46.834)
        {
          tau = "(0.3211*Math.pow(10,7))*Math.pow(X,-3)-(0.5229*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(46.834>x && x>7.736)
        {
          tau = "(0.1577*Math.pow(10,4))*Math.pow(X,-2)+(0.3207*Math.pow(10,6))*Math.pow(X,-3)-(0.8830*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(7.736>x && x>7.311)
        {
          tau = "(0.2858*Math.pow(10,6)*Math.pow(X,-3))-(0.7415*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(7.311>x && x>6.716)
        {
          tau = "(0.1911*Math.pow(10,6)*Math.pow(X,-3))-(0.4109*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(6.716>x && x>1.722)
        {
          tau = "(0.2669*Math.pow(10,4))*Math.pow(X,-2)+(0.3281*Math.pow(10,5))*Math.pow(X,-3)-(0.2613*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.722>x && x>1.540)
        {
          tau = "(0.3683*Math.pow(10,5))*Math.pow(X,-3)-(0.2662*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.540>x && x>1.419)
        {
          tau = "(0.3069*Math.pow(10,5))*Math.pow(X,-3)-(0.1892*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.419>x && x>1.106)
        {
          tau = "(0.2294*Math.pow(10,5))*Math.pow(X,-3)-(0.1111*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.106>x && x>1.080)
        {
          tau = "(0.2482*Math.pow(10,5))*Math.pow(X,-3)-(0.1799*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.4747*Math.pow(10,4))*Math.pow(X,-3)-(0.2650*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.933*Math.pow(10,-1)*X)*Math.pow((9.671*Math.pow(10,-2)+3.015*Math.pow(10,-2)*X+5.914*Math.pow(10,-3)*Math.pow(X,2)+1.593*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((106.4*Math.pow(X,-1)+6.500+0.01785*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 63: //Eu
    {
        if(x>48.519)
        {
          tau = "(0.3395*Math.pow(10,7))*Math.pow(X,-3)-(0.5788*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(48.519>x && x>8.052)
        {
          tau = "(0.1624*Math.pow(10,4))*Math.pow(X,-2)+(0.3418*Math.pow(10,6))*Math.pow(X,-3)-(0.9797*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(8.052>x && x>7.617)
        {
          tau = "(0.3033*Math.pow(10,6)*Math.pow(X,-3))-(0.8102*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(7.617>x && x>6.976)
        {
          tau = "(0.2032*Math.pow(10,6)*Math.pow(X,-3))-(0.4513*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(6.976>x && x>1.800)
        {
          tau = "(0.2738*Math.pow(10,4))*Math.pow(X,-2)+(0.3541*Math.pow(10,5))*Math.pow(X,-3)-(0.2922*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.800>x && x>1.613)
        {
          tau = "(0.3963*Math.pow(10,5))*Math.pow(X,-3)-(0.2965*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.613>x && x>1.480)
        {
          tau = "(0.3349*Math.pow(10,5))*Math.pow(X,-3)-(0.2175*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.480>x && x>1.160)
        {
          tau = "(0.2465*Math.pow(10,5))*Math.pow(X,-3)-(0.1228*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.160>x && x>1.130)
        {
          tau = "(0.2457*Math.pow(10,5))*Math.pow(X,-3)-(0.1767*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.5084*Math.pow(10,4))*Math.pow(X,-3)-(0.2878*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.696*Math.pow(10,-1)*X)*Math.pow((9.526*Math.pow(10,-2)+2.778*Math.pow(10,-2)*X+5.372*Math.pow(10,-3)*Math.pow(X,2)+1.403*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((107.6*Math.pow(X,-1)+6.459+0.01777*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 64: //Gd
    {
        if(x>50.239)
        {
          tau = "(0.3516*Math.pow(10,7))*Math.pow(X,-3)-(0.6283*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(50.239>x && x>8.375)
        {
          tau = "(0.1604*Math.pow(10,4))*Math.pow(X,-2)+(0.3565*Math.pow(10,6))*Math.pow(X,-3)-(0.1065*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(8.375>x && x>7.930)
        {
          tau = "(0.3168*Math.pow(10,6)*Math.pow(X,-3))-(0.8859*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(7.930>x && x>7.242)
        {
          tau = "(0.2087*Math.pow(10,6)*Math.pow(X,-3))-(0.4667*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(7.242>x && x>1.880)
        {
          tau = "(0.2739*Math.pow(10,4))*Math.pow(X,-2)+(0.3729*Math.pow(10,5))*Math.pow(X,-3)-(0.3185*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.880>x && x>1.688)
        {
          tau = "(0.4162*Math.pow(10,5))*Math.pow(X,-3)-(0.3223*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.688>x && x>1.544)
        {
          tau = "(0.3488*Math.pow(10,5))*Math.pow(X,-3)-(0.2303*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.544>x && x>1.217)
        {
          tau = "(0.2554*Math.pow(10,5))*Math.pow(X,-3)-(0.1278*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.217>x && x>1.185)
        {
          tau = "(0.2413*Math.pow(10,5))*Math.pow(X,-3)-(0.1727*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.5397*Math.pow(10,4))*Math.pow(X,-3)-(0.3116*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.652*Math.pow(10,-1)*X)*Math.pow((9.606*Math.pow(10,-2)+2.706*Math.pow(10,-2)*X+5.275*Math.pow(10,-3)*Math.pow(X,2)+1.377*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((110.9*Math.pow(X,-1)+6.581+0.01813*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 65: //Tb
    {
        if(x>51.995)
        {
          tau = "(0.3722*Math.pow(10,7))*Math.pow(X,-3)-(0.6917*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(51.995>x && x>8.708)
        {
          tau = "(0.1689*Math.pow(10,4))*Math.pow(X,-2)+(0.3777*Math.pow(10,6))*Math.pow(X,-3)-(0.1166*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(8.708>x && x>8.252)
        {
          tau = "(0.3376*Math.pow(10,6)*Math.pow(X,-3))-(0.9837*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(8.252>x && x>7.514)
        {
          tau = "(0.2228*Math.pow(10,6)*Math.pow(X,-3))-(0.5205*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(7.514>x && x>1.967)
        {
          tau = "(0.2817*Math.pow(10,4))*Math.pow(X,-2)+(0.3977*Math.pow(10,5))*Math.pow(X,-3)-(0.3486*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.967>x && x>1.767)
        {
          tau = "(0.4480*Math.pow(10,5))*Math.pow(X,-3)-(0.3608*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.767>x && x>1.611)
        {
          tau = "(0.3706*Math.pow(10,5))*Math.pow(X,-3)-(0.2495*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.611>x && x>1.275)
        {
          tau = "(0.2716*Math.pow(10,5))*Math.pow(X,-3)-(0.1378*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.275>x && x>1.241)
        {
          tau = "(0.3316*Math.pow(10,5))*Math.pow(X,-3)-(0.2830*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.5667*Math.pow(10,4))*Math.pow(X,-3)-(0.3281*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.296*Math.pow(10,-1)*X)*Math.pow((9.451*Math.pow(10,-2)+2.306*Math.pow(10,-2)*X+4.584*Math.pow(10,-3)*Math.pow(X,2)+1.147*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((111.8*Math.pow(X,-1)+6.552+0.01804*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 66: //Dy
    {
        if(x>53.788)
        {
          tau = "(0.3884*Math.pow(10,7))*Math.pow(X,-3)-(0.7467*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(53.788>x && x>9.045)
        {
          tau = "(0.1751*Math.pow(10,4))*Math.pow(X,-2)+(0.3955*Math.pow(10,6))*Math.pow(X,-3)-(0.1264*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(9.045>x && x>8.580)
        {
          tau = "(0.3553*Math.pow(10,6)*Math.pow(X,-3))-(0.1079*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(8.580>x && x>7.790)
        {
          tau = "(0.2253*Math.pow(10,6)*Math.pow(X,-3))-(0.4925*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(7.790>x && x>2.046)
        {
          tau = "(0.2845*Math.pow(10,4))*Math.pow(X,-2)+(0.4216*Math.pow(10,5))*Math.pow(X,-3)-(0.3822*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.046>x && x>1.841)
        {
          tau = "(0.4729*Math.pow(10,5))*Math.pow(X,-3)-(0.3925*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.841>x && x>1.675)
        {
          tau = "(0.3926*Math.pow(10,5))*Math.pow(X,-3)-(0.2725*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.675>x && x>1.333)
        {
          tau = "(0.2916*Math.pow(10,5))*Math.pow(X,-3)-(0.1554*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.333>x && x>1.294)
        {
          tau = "(0.3557*Math.pow(10,5))*Math.pow(X,-3)-(0.3166*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.5996*Math.pow(10,4))*Math.pow(X,-3)-(0.3512*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.124*Math.pow(10,-1)*X)*Math.pow((9.341*Math.pow(10,-2)+2.171*Math.pow(10,-2)*X+4.244*Math.pow(10,-3)*Math.pow(X,2)+1.030*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((114.9*Math.pow(X,-1)+6.580+0.01825*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 67: //Ho
    {
        if(x>55.617)
        {
          tau = "(0.4073*Math.pow(10,7))*Math.pow(X,-3)-(0.8046*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(55.617>x && x>9.394)
        {
          tau = "(0.1829*Math.pow(10,4))*Math.pow(X,-2)+(0.4165*Math.pow(10,6))*Math.pow(X,-3)-(0.1375*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(9.394>x && x>8.917)
        {
          tau = "(0.3764*Math.pow(10,6)*Math.pow(X,-3))-(0.1189*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(8.917>x && x>8.071)
        {
          tau = "(0.2431*Math.pow(10,6)*Math.pow(X,-3))-(0.5884*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(8.071>x && x>2.128)
        {
          tau = "(0.2850*Math.pow(10,4))*Math.pow(X,-2)+(0.4527*Math.pow(10,5))*Math.pow(X,-3)-(0.4260*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.128>x && x>1.923)
        {
          tau = "(0.5025*Math.pow(10,5))*Math.pow(X,-3)-(0.4307*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.923>x && x>1.741)
        {
          tau = "(0.4157*Math.pow(10,5))*Math.pow(X,-3)-(0.2945*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.741>x && x>1.391)
        {
          tau = "(0.3164*Math.pow(10,5))*Math.pow(X,-3)-(0.1788*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.391>x && x>1.351)
        {
          tau = "(0.3209*Math.pow(10,5))*Math.pow(X,-3)-(0.2708*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.6385*Math.pow(10,4))*Math.pow(X,-3)-(0.3779*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.069*Math.pow(10,-1)*X)*Math.pow((9.129*Math.pow(10,-2)+2.144*Math.pow(10,-2)*X+4.057*Math.pow(10,-3)*Math.pow(X,2)+9.736*Math.pow(10,-5)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((117*Math.pow(X,-1)+6.569+0.01831*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 68: //Er
    {
        if(x>57.185)
        {
          tau = "(0.4267*Math.pow(10,7))*Math.pow(X,-3)-(0.8622*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(57.185>x && x>9.751)
        {
          tau = "(0.1910*Math.pow(10,4))*Math.pow(X,-2)+(0.4384*Math.pow(10,6))*Math.pow(X,-3)-(0.1492*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(9.751>x && x>9.264)
        {
          tau = "(0.3982*Math.pow(10,6)*Math.pow(X,-3))-(0.1307*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(9.264>x && x>8.357)
        {
          tau = "(0.2558*Math.pow(10,6)*Math.pow(X,-3))-(0.6354*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(8.357>x && x>2.206)
        {
          tau = "(0.2931*Math.pow(10,4))*Math.pow(X,-2)+(0.4801*Math.pow(10,5))*Math.pow(X,-3)-(0.4657*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.206>x && x>2.005)
        {
          tau = "(0.5830*Math.pow(10,5))*Math.pow(X,-3)-(0.4791*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.005>x && x>1.811)
        {
          tau = "(0.4770*Math.pow(10,5))*Math.pow(X,-3)-(0.3852*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.811>x && x>1.453)
        {
          tau = "(0.3413*Math.pow(10,5))*Math.pow(X,-3)-(0.2030*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.453>x && x>1.409)
        {
          tau = "(0.3585*Math.pow(10,5))*Math.pow(X,-3)-(0.3235*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.6797*Math.pow(10,4))*Math.pow(X,-3)-(0.4059*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.858*Math.pow(10,-1)*X)*Math.pow((9.058*Math.pow(10,-2)+1.863*Math.pow(10,-2)*X+3.656*Math.pow(10,-3)*Math.pow(X,2)+8.480*Math.pow(10,-5)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((118.9*Math.pow(X,-1)+6.556+0.01835*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 69: //Tm
    {
        if(x>59.389)
        {
          tau = "(0.4493*Math.pow(10,7))*Math.pow(X,-3)-(0.9392*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(59.389>x && x>10.115)
        {
          tau = "(0.1980*Math.pow(10,4))*Math.pow(X,-2)+(0.4654*Math.pow(10,6))*Math.pow(X,-3)-(0.1653*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(10.115>x && x>9.616)
        {
          tau = "(0.4232*Math.pow(10,6)*Math.pow(X,-3))-(0.1447*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(9.616>x && x>8.648)
        {
          tau = "(0.2705*Math.pow(10,6)*Math.pow(X,-3))-(0.6928*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(8.648>x && x>2.306)
        {
          tau = "(0.3024*Math.pow(10,4))*Math.pow(X,-2)+(0.5105*Math.pow(10,5))*Math.pow(X,-3)-(0.5103*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.306>x && x>2.089)
        {
          tau = "(0.5721*Math.pow(10,5))*Math.pow(X,-3)-(0.5235*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.089>x && x>1.884)
        {
          tau = "(0.4704*Math.pow(10,5))*Math.pow(X,-3)-(0.3516*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.884>x && x>1.514)
        {
          tau = "(0.3588*Math.pow(10,5))*Math.pow(X,-3)-(0.2150*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.514>x && x>1.467)
        {
          tau = "(0.4367*Math.pow(10,5))*Math.pow(X,-3)-(0.4388*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.7237*Math.pow(10,4))*Math.pow(X,-3)-(0.4349*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.732*Math.pow(10,-1)*X)*Math.pow((8.885*Math.pow(10,-2)+1.718*Math.pow(10,-2)*X+3.374*Math.pow(10,-3)*Math.pow(X,2)+7.638*Math.pow(10,-5)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((120.9*Math.pow(X,-1)+6.504+0.01839*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 70: //Yb
    {
        if(x>61.332)
        {
          tau = "(0.4675*Math.pow(10,7))*Math.pow(X,-3)-(0.1022*Math.pow(10,9))*Math.pow(X,-4)";
        }
        else if(61.332>x && x>10.486)
        {
          tau = "(0.1995*Math.pow(10,4))*Math.pow(X,-2)+(0.4866*Math.pow(10,6))*Math.pow(X,-3)-(0.1796*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(10.486>x && x>9.978)
        {
          tau = "(0.4552*Math.pow(10,6)*Math.pow(X,-3))-(0.1700*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(9.978>x && x>8.943)
        {
          tau = "(0.2817*Math.pow(10,6)*Math.pow(X,-3))-(0.7422*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(8.943>x && x>2.398)
        {
          tau = "(0.3077*Math.pow(10,4))*Math.pow(X,-2)+(0.5341*Math.pow(10,5))*Math.pow(X,-3)-(0.5486*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.398>x && x>2.173)
        {
          tau = "(0.6041*Math.pow(10,5))*Math.pow(X,-3)-(0.5743*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.173>x && x>1.949)
        {
          tau = "(0.4972*Math.pow(10,5))*Math.pow(X,-3)-(0.3864*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.949>x && x>1.576)
        {
          tau = "(0.3790*Math.pow(10,5))*Math.pow(X,-3)-(0.2362*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.576>x && x>1.527)
        {
          tau = "(0.4881*Math.pow(10,5))*Math.pow(X,-3)-(0.5234*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.7585*Math.pow(10,4))*Math.pow(X,-3)-(0.4579*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.621*Math.pow(10,-1)*X)*Math.pow((8.842*Math.pow(10,-2)+1.614*Math.pow(10,-2)*X+3.170*Math.pow(10,-3)*Math.pow(X,2)+7.012*Math.pow(10,-5)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((124.5*Math.pow(X,-1)+6.552+0.01864*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 71: //Lu
    {
        if(x>63.313)
        {
          tau = "(0.4915*Math.pow(10,7))*Math.pow(X,-3)-(0.1111*Math.pow(10,9))*Math.pow(X,-4)";
        }
        else if(63.313>x && x>10.870)
        {
          tau = "(0.2072*Math.pow(10,4))*Math.pow(X,-2)+(0.5147*Math.pow(10,6))*Math.pow(X,-3)-(0.1973*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(10.870>x && x>10.348)
        {
          tau = "(0.4696*Math.pow(10,6)*Math.pow(X,-3))-(0.1735*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(10.348>x && x>9.244)
        {
          tau = "(0.2954*Math.pow(10,6)*Math.pow(X,-3))-(0.7884*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(9.244>x && x>2.491)
        {
          tau = "(0.3152*Math.pow(10,4))*Math.pow(X,-2)+(0.5693*Math.pow(10,5))*Math.pow(X,-3)-(0.6051*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.491>x && x>2.263)
        {
          tau = "(0.6398*Math.pow(10,5))*Math.pow(X,-3)-(0.6231*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.263>x && x>2.023)
        {
          tau = "(0.5265*Math.pow(10,5))*Math.pow(X,-3)-(0.4152*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.023>x && x>1.639)
        {
          tau = "(0.4112*Math.pow(10,5))*Math.pow(X,-3)-(0.2732*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.639>x && x>1.588)
        {
          tau = "(0.3893*Math.pow(10,5))*Math.pow(X,-3)-(0.3686*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.8083*Math.pow(10,4))*Math.pow(X,-3)-(0.4909*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.530*Math.pow(10,-1)*X)*Math.pow((8.663*Math.pow(10,-2)+1.546*Math.pow(10,-2)*X+2.948*Math.pow(10,-3)*Math.pow(X,2)+6.442*Math.pow(10,-5)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((125.9*Math.pow(X,-1)+6.529+0.01861*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 72: //Hf
    {
        if(x>65.350)
        {
          tau = "(0.5124*Math.pow(10,7))*Math.pow(X,-3)-(0.1202*Math.pow(10,9))*Math.pow(X,-4)";
        }
        else if(65.350>x && x>11.270)
        {
          tau = "(0.2144*Math.pow(10,4))*Math.pow(X,-2)+(0.5374*Math.pow(10,6))*Math.pow(X,-3)-(0.2122*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(11.270>x && x>10.739)
        {
          tau = "(0.4976*Math.pow(10,6)*Math.pow(X,-3))-(0.1946*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(10.739>x && x>9.560)
        {
          tau = "(0.3042*Math.pow(10,6)*Math.pow(X,-3))-(0.7954*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(9.560>x && x>2.6)
        {
          tau = "(0.3188*Math.pow(10,4))*Math.pow(X,-2)+(0.6025*Math.pow(10,5))*Math.pow(X,-3)-(0.6630*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.6>x && x>2.365)
        {
          tau = "(0.6774*Math.pow(10,5))*Math.pow(X,-3)-(0.6845*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.365>x && x>2.107)
        {
          tau = "(0.5555*Math.pow(10,5))*Math.pow(X,-3)-(0.4507*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.107>x && x>1.716)
        {
          tau = "(0.4482*Math.pow(10,5))*Math.pow(X,-3)-(0.3245*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.716>x && x>1.661)
        {
          tau = "(0.4261*Math.pow(10,5))*Math.pow(X,-3)-(0.4287*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.8547*Math.pow(10,4))*Math.pow(X,-3)-(0.5228*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.510*Math.pow(10,-1)*X)*Math.pow((8.543*Math.pow(10,-2)+1.524*Math.pow(10,-2)*X+2.866*Math.pow(10,-3)*Math.pow(X,2)+6.275*Math.pow(10,-5)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((128.4*Math.pow(X,-1)+6.563+0.01876*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 73: //Ta
    {
        if(x>67.416)
        {
          tau = "(0.5366*Math.pow(10,7))*Math.pow(X,-3)-(0.1303*Math.pow(10,9))*Math.pow(X,-4)";
        }
        else if(67.416>x && x>11.681)
        {
          tau = "(0.2209*Math.pow(10,4))*Math.pow(X,-2)+(0.5658*Math.pow(10,6))*Math.pow(X,-3)-(0.2321*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(11.681>x && x>11.136)
        {
          tau = "(0.5262*Math.pow(10,6)*Math.pow(X,-3))-(0.2149*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(11.136>x && x>9.881)
        {
          tau = "(0.3165*Math.pow(10,6)*Math.pow(X,-3))-(0.8344*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(9.881>x && x>2.708)
        {
          tau = "(0.3243*Math.pow(10,4))*Math.pow(X,-2)+(0.6393*Math.pow(10,5))*Math.pow(X,-3)-(0.7252*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.708>x && x>2.468)
        {
          tau = "(0.7168*Math.pow(10,5))*Math.pow(X,-3)-(0.7459*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.468>x && x>2.194)
        {
          tau = "(0.6294*Math.pow(10,5))*Math.pow(X,-3)-(0.5814*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.194>x && x>1.793)
        {
          tau = "(0.4761*Math.pow(10,5))*Math.pow(X,-3)-(0.3533*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.793>x && x>1.735)
        {
          tau = "(0.5234*Math.pow(10,5))*Math.pow(X,-3)-(0.5955*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.2539*Math.pow(10,4))*Math.pow(X,-2)+(0.2405*Math.pow(10,4))*Math.pow(X,-3)-(0.1445*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.529*Math.pow(10,-1)*X)*Math.pow((8.421*Math.pow(10,-2)+1.531*Math.pow(10,-2)*X+2.825*Math.pow(10,-3)*Math.pow(X,2)+6.203*Math.pow(10,-5)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((130.3*Math.pow(X,-1)+6.552+0.01884*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 74: //W
    {
        if(x>69.525)
        {
          tau = "(0.5609*Math.pow(10,7))*Math.pow(X,-3)-(0.1409*Math.pow(10,9))*Math.pow(X,-4)";
        }
        else if(69.525>x && x>12.099)
        {
          tau = "(0.2293*Math.pow(10,4))*Math.pow(X,-2)+(0.5922*Math.pow(10,6))*Math.pow(X,-3)-(0.2501*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(12.099>x && x>11.544)
        {
          tau = "(0.5357*Math.pow(10,6)*Math.pow(X,-3))-(0.2132*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(11.544>x && x>10.206)
        {
          tau = "(0.3312*Math.pow(10,6)*Math.pow(X,-3))-(0.8942*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(10.206>x && x>2.819)
        {
          tau = "(0.3533*Math.pow(10,4))*Math.pow(X,-2)+(0.6433*Math.pow(10,5))*Math.pow(X,-3)-(0.6929*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.819>x && x>2.574)
        {
          tau = "(0.7600*Math.pow(10,5))*Math.pow(X,-3)-(0.8179*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.574>x && x>2.281)
        {
          tau = "(0.6324*Math.pow(10,5))*Math.pow(X,-3)-(0.5576*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.281>x && x>1.871)
        {
          tau = "(0.6520*Math.pow(10,5))*Math.pow(X,-3)-(0.7006*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.871>x && x>1.809)
        {
          tau = "(0.4760*Math.pow(10,5))*Math.pow(X,-3)-(0.5122*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.2594*Math.pow(10,4))*Math.pow(X,-2)+(0.2704*Math.pow(10,4))*Math.pow(X,-3)-(0.1624*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.571*Math.pow(10,-1)*X)*Math.pow((8.324*Math.pow(10,-2)+1.526*Math.pow(10,-2)*X+2.825*Math.pow(10,-3)*Math.pow(X,2)+6.277*Math.pow(10,-5)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((126.1*Math.pow(X,-1)+6.724+0.01805*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }




    }
}

double MainWindow::oslableniyeFirst(double x)
{
    QString tau;

    QString sigma;
    QString sigmaK;
    QString sigmaNK;

    QScriptEngine engine;

    QScriptValue tauValue;
    QScriptValue sigmaValue;
    QScriptValue sigmaKValue;
    QScriptValue sigmaNKValue;

    switch(ui->materialFirst->currentIndex())
    {
    case 0: //Air
        {
            if(x<3.203)
            {
                tau = "-(0.1991*Math.pow(10,-2))*X+(0.1169)*Math.pow(X,-1)-(0.1514*100)*Math.pow(X,-2)+(0.4759*10000)*Math.pow(X,-3)-(0.9060*1000)*Math.pow(X,-4)";
            }
            else
            {
               tau = "-(0.3514*Math.pow(10,2))*Math.pow(X,-2)+(0.5059*10000)*Math.pow(X,-3)-(0.1060*1000)*Math.pow(X,-4)+Math.pow((26.44*Math.pow(X,-1)+4.724+0.01809*X),-1)";
            }
            tau.replace("X", QString::number(x));
            tauValue = engine.evaluate(tau);
            weakening = tauValue.toNumber();
            return weakening;
            break;
        }
    case 1: //H
        {
            tau = "-(0.2645*Math.pow(10,-2))*X+(0.7086*0.1)*Math.pow(X,-1)-(0.7487)*Math.pow(X,-2)+(0.5575*10)*Math.pow(X,-3)-(-0.1936*10)*Math.pow(X,-4)";
            sigma = "0.4005*(1/1.008)*Math.pow((1+2*X/511),-2)*(1+2*X/511+0.3*Math.pow(2*X/511,2)-0.0625*Math.pow(2*X/511,3))";
            tau.replace("X", QString::number(x));
            sigma.replace("X", QString::number(x));
            tauValue = engine.evaluate(tau);
            sigmaValue = engine.evaluate(sigma);
            weakening = tauValue.toNumber()+sigmaValue.toNumber();
            return weakening;
            break;
        }
    case 2: //He
        {
            tau = "-(0.2154*Math.pow(10,-2))*X+(0.1473)*Math.pow(X,-1)-(0.3322*10)*Math.pow(X,-2)+(0.4893*100)*Math.pow(X,-3)-(-0.1576*100)*Math.pow(X,-4)";
            sigma = "0.4005*(1/4.003)*Math.pow(1+2*X/511,-2)*(1+2*X/511+0.3*Math.pow(2*X/511,2)-0.0625*Math.pow(2*X/511,3))";
            tau.replace("X", QString::number(x));
            sigma.replace("X", QString::number(x));
            tauValue = engine.evaluate(tau);
            sigmaValue = engine.evaluate(sigma);
            weakening = tauValue.toNumber()+sigmaValue.toNumber();
            return weakening;
            break;
        }
    case 3: //Li
        {
            tau = "-(0.3411*Math.pow(10,-2))*X+(0.3088)*Math.pow(X,-1)-(0.1009*100)*Math.pow(X,-2)+(0.2076*1000)*Math.pow(X,-3)-(-0.4091*100)*Math.pow(X,-4)";
            sigmaK = "(1+9.326*Math.pow(10,-2)*X)*Math.pow((1.781+8.725*0.1*X+7.963*0.01*Math.pow(X,2)+8.225*0.001*Math.pow(X,3)),-1)";
            sigmaNK = "Math.pow((29.94*Math.pow(X,-1)+4.533+0.03637*X),-1)";
            tau.replace("X", QString::number(x));
            sigmaK.replace("X", QString::number(x));
            sigmaNK.replace("X", QString::number(x));
            tauValue = engine.evaluate(tau);
            sigmaKValue = engine.evaluate(sigmaK);
            sigmaNKValue = engine.evaluate(sigmaNK);
            weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
            return weakening;
            break;
        }
    case 4: //Be
    {
        tau = "-(0.3142*Math.pow(10,-2))*Math.pow(X,0)+(0.4216*Math.pow(10,0))*Math.pow(X,-1)-(0.2014*Math.pow(10,2))*Math.pow(X,-2)+(0.5918*Math.pow(10,3))*Math.pow(X,-3)-(-0.4857*Math.pow(10,2))*Math.pow(X,-4)";
        sigmaK = "(1-3.178*Math.pow(10,-2)*X)*Math.pow((1.267*Math.pow(10,0)+4.619*Math.pow(10,-1)*X+3.102*Math.pow(10,-2)*Math.pow(X,2)+1.493*Math.pow(10,-3)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((25.93*Math.pow(X,-1)+5.067+0.02420*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 5: //B
    {
        tau = "-(0.3267*Math.pow(10,-2))*X+(0.5682)*Math.pow(X,-1)-(0.3483*100)*Math.pow(X,-2)+(0.1326*10000)*Math.pow(X,-3)-(0.3243*100)*Math.pow(X,-4)";
        sigmaK = "(1+1.544*X)*Math.pow((1.010+1.561*X+6.978*0.1*Math.pow(X,2)+5.025*0.01*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((25.58*Math.pow(X,-1)+4.945+0.02191*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 6: //C
    {
        tau = "-(0.3172*Math.pow(10,-2))*X+(0.6921)*Math.pow(X,-1)-(0.5340*100)*Math.pow(X,-2)+(0.2610*10000)*Math.pow(X,-3)-(0.2941*1000)*Math.pow(X,-4)";
        sigmaK = "(1+1.108*X)*Math.pow((8.093*0.1+7.378*0.1*X+3.958*0.1*Math.pow(X,2)+2.532*0.01*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((25.91*Math.pow(X,-1)+4.644+0.01878*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 7: //N
    {
        tau = "-(0.1991*Math.pow(10,-2))*X+(0.6169)*Math.pow(X,-1)-(0.6514*100)*Math.pow(X,-2)+(0.4259*10000)*Math.pow(X,-3)-(0.8060*1000)*Math.pow(X,-4)";
        sigmaK = "(1+5.723*0.1*X)*Math.pow((7.260*0.1+2.910*0.1*X+1.824*0.1*Math.pow(X,2)+1.018*0.1*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((26.44*Math.pow(X,-1)+4.724+0.01809*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 8: //O
    {
        tau = "-(0.2663*Math.pow(10,-2))*X+(0.8397)*Math.pow(X,-1)-(0.9179*100)*Math.pow(X,-2)+(0.6634*10000)*Math.pow(X,-3)-(0.1906*10000)*Math.pow(X,-4)";
        sigmaK = "(1+3.537*0.1*X)*Math.pow((6.396*0.1+1.543*0.1*X+9.948*0.01*Math.pow(X,2)+4.981*0.001*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((29.88*Math.pow(X,-1)+4.656+0.01857*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 9: //F
    {
        tau = "-(0.3038*Math.pow(10,-2))*X+(0.9836)*Math.pow(X,-1)-(0.1128*1000)*Math.pow(X,-2)+(0.9171*10000)*Math.pow(X,-3)-(0.3414*10000)*Math.pow(X,-4)";
        sigmaK = "(1+2.437*0.1*X)*Math.pow((5.970*0.1+9.989*0.01*X+6.409*0.01*Math.pow(X,2)+2.930*0.001*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((33.40*Math.pow(X,-1)+4.961+0.01913*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 10: //Ne
    {
        tau = "-(0.1806*Math.pow(10,-2))*X+(0.7942)*Math.pow(X,-1)-(0.1218*1000)*Math.pow(X,-2)+(0.1307*100000)*Math.pow(X,-3)-(0.5600*10000)*Math.pow(X,-4)";
        sigmaK = "(1+1.820*0.1*X)*Math.pow((5.118*0.1+6.431*0.01*X+4.065*0.01*Math.pow(X,2)+1.715*0.001*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((37.30*Math.pow(X,-1)+4.629+0.01905*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 11: //Na
    {
        if(x>1.072)
        {
          tau = "-(0.3963*Math.pow(10,-2))*X+(0.1359*10)*Math.pow(X,-1)-(0.1720*1000)*Math.pow(X,-2)+(0.1766*100000)*Math.pow(X,-3)-(0.1019*100000)*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.8694*1000)*Math.pow(X,-3)-(0.2170*1000)*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.029*0.1*X)*Math.pow((4.721*0.1+6.884*0.01*X+2.528*0.01*Math.pow(X,2)+8.577*0.0001*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((38.80*Math.pow(X,-1)+4.901+0.01889*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 12: //Mg
    {
        if(x>1.305)
        {
          tau = "-(0.3447*Math.pow(10,-2))*X+(0.1309*Math.pow(10,1))*Math.pow(X,-1)-(0.1895*Math.pow(10,3))*Math.pow(X,-2)+(0.2327*Math.pow(10,5))*Math.pow(X,-3)-(0.1439*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1268*Math.pow(10,4))*Math.pow(X,-3)-(0.3474*Math.pow(10,3))*Math.pow(X,-4)";
        }
        sigmaK = "(1+8.484*Math.pow(10,-2)*X)*Math.pow((4.132*Math.pow(10,-1)+7.566*Math.pow(10,-2)*X+1.836*Math.pow(10,-2)*Math.pow(X,2)+6.034*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((39.81*Math.pow(X,-1)+4.746+0.01832*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 13: //Al
    {
        if(x>1.559)
        {
          tau = "-(0.3471*Math.pow(10,-2))*Math.pow(X,0)+(0.1384*Math.pow(10,1))*Math.pow(X,-1)-(0.2137*Math.pow(10,3))*Math.pow(X,-2)+(0.2950*Math.pow(10,5))*Math.pow(X,-3)-(0.2217*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1688*Math.pow(10,4)*Math.pow(X,-3))-(0.5046*Math.pow(10,3)*Math.pow(X,-4))";
        }
        sigmaK = "(1+8.580*Math.pow(10,-2)*X)*Math.pow((3.905*Math.pow(10,-1)+8.506*Math.pow(10,-2)*X+1.611*Math.pow(10,-2)*Math.pow(X,2)+5.524*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((42.58*Math.pow(X,-1)+4.873+0.01871*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 14: //Si
    {
        if(x>1.838)
        {
          tau = "-(0.2219*Math.pow(10,-2))*X+(0.1141*Math.pow(10,1))*Math.pow(X,-1)-(0.2202*Math.pow(10,3))*Math.pow(X,-2)+(0.3864*Math.pow(10,5))*Math.pow(X,-3)-(0.3319*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1640*Math.pow(10,3))*Math.pow(X,-2)+(0.1853*Math.pow(10,4))*Math.pow(X,-3)-(0.4500*Math.pow(10,3))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.970*X)*Math.pow((3.667*Math.pow(10,-1)+6.375*Math.pow(10,-1)*X+1.589*Math.pow(10,-1)*Math.pow(X,2)+1.114*Math.pow(10,-2)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((42.56*Math.pow(X,-1)+4.729+0.01796*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 15: //P
    {
        if(x>2.145)
        {
          tau = "(0.6962)*Math.pow(X,-1)-(0.2154*Math.pow(10,3))*Math.pow(X,-2)+(0.4590*Math.pow(10,5))*Math.pow(X,-3)-(0.4530*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1908*Math.pow(10,3))*Math.pow(X,-2)+(0.2308*Math.pow(10,4))*Math.pow(X,-3)-(0.5886*Math.pow(10,3))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.169*X)*Math.pow((3.420*Math.pow(10,-1)+6.676*Math.pow(10,-1)*X+1.740*Math.pow(10,-1)*Math.pow(X,2)+1.124*Math.pow(10,-2)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((45.17*Math.pow(X,-1)+4.889+0.01835*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 16: //S
    {
        if(x>2.472)
        {
          tau = "(0.7582)*Math.pow(X,-1)-(0.2447*Math.pow(10,3))*Math.pow(X,-2)+(0.5785*Math.pow(10,5))*Math.pow(X,-3)-(0.6419*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.2282*Math.pow(10,3))*Math.pow(X,-2)+(0.3007*Math.pow(10,4))*Math.pow(X,-3)-(0.8095*Math.pow(10,3))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.786*X)*Math.pow((3.095*Math.pow(10,-1)+4.495*Math.pow(10,-1)*X+1.360*Math.pow(10,-1)*Math.pow(X,2)+7.918*Math.pow(10,-2)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((41.69*Math.pow(X,-1)+4.900+0.01671*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 17: //Cl
    {
        if(x>2.822)
        {
          tau = "(0.7858)*Math.pow(X,-1)-(0.2734*Math.pow(10,3))*Math.pow(X,-2)+(0.7529*Math.pow(10,5))*Math.pow(X,-3)-(0.9287*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.2783*Math.pow(10,3))*Math.pow(X,-2)+(0.4027*Math.pow(10,4))*Math.pow(X,-3)-(0.1144*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.391*X)*Math.pow((3.181*Math.pow(10,-1)+3.659*Math.pow(10,-1)*X+1.077*Math.pow(10,-1)*Math.pow(X,2)+5.874*Math.pow(10,-2)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((44.68*Math.pow(X,-1)+5.115+0.01732*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 18: //Ar
    {
        if(x>3.206)
        {
          tau = "(0.7518)*Math.pow(X,-1)-(0.2633*Math.pow(10,3))*Math.pow(X,-2)+(0.7533*Math.pow(10,5))*Math.pow(X,-3)-(0.1050*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.2629*Math.pow(10,3))*Math.pow(X,-2)+(0.4167*Math.pow(10,4))*Math.pow(X,-3)-(0.1249*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.080*X)*Math.pow((3.078*Math.pow(10,-1)+2.799*Math.pow(10,-1)*X+8.688*Math.pow(10,-2)*Math.pow(X,2)+4.380*Math.pow(10,-3)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((49.09*Math.pow(X,-1)+5.452+0.01840*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 19: //K
    {
        if(x>3.607)
        {
          tau = "(0.6731)*Math.pow(X,-1)-(0.2717*Math.pow(10,3))*Math.pow(X,-2)+(0.9468*Math.pow(10,5))*Math.pow(X,-3)-(0.1384*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.3242*Math.pow(10,3))*Math.pow(X,-2)+(0.5459*Math.pow(10,4))*Math.pow(X,-3)-(0.1733*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+9.832*Math.pow(10,-1)*X)*Math.pow((2.733*Math.pow(10,-1)+2.506*Math.pow(10,-1)*X+6.843*Math.pow(10,-2)*Math.pow(X,2)+3.356*Math.pow(10,-3)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((46.28*Math.pow(X,-1)+5.080+0.01694*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 20: //Ca
    {
        if(x>4.038)
        {
          tau = "(0.7622)*Math.pow(X,-1)-(0.3106*Math.pow(10,3))*Math.pow(X,-2)+(0.1148*Math.pow(10,6))*Math.pow(X,-3)-(0.1902*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.3699*Math.pow(10,3))*Math.pow(X,-2)+(0.6774*Math.pow(10,4))*Math.pow(X,-3)-(0.2287*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.265*X)*Math.pow((2.507*Math.pow(10,-1)+2.991*Math.pow(10,-1)*X+8*Math.pow(10,-2)*Math.pow(X,2)+3.915*Math.pow(10,-3)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((45.51*Math.pow(X,-1)+4.977+0.01634*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 21: //Sc
    {
        if(x>4.492)
        {
          tau = "(0.7710)*Math.pow(X,-1)-(0.3377*Math.pow(10,3))*Math.pow(X,-2)+(0.1397*Math.pow(10,6))*Math.pow(X,-3)-(0.2524*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.3933*Math.pow(10,3))*Math.pow(X,-2)+(0.7473*Math.pow(10,4))*Math.pow(X,-3)-(0.2642*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+9.784*Math.pow(10,-1)*X)*Math.pow((2.502*Math.pow(10,-1)+2.318*Math.pow(10,-1)*X+6.513*Math.pow(10,-2)*Math.pow(X,2)+2.995*Math.pow(10,-3)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((47.76*Math.pow(X,-1)+5.397+0.01697*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 22: //Ti
    {
        if(x>4.966)
        {
          tau = "(0.6170)*Math.pow(X,-1)-(0.2987*Math.pow(10,3))*Math.pow(X,-2)+(0.1409*Math.pow(10,6))*Math.pow(X,-3)-(0.2757*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.4341*Math.pow(10,3))*Math.pow(X,-2)+(0.8580*Math.pow(10,4))*Math.pow(X,-3)-(0.3171*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+7.497*Math.pow(10,-1)*X)*Math.pow((2.462*Math.pow(10,-1)+1.731*Math.pow(10,-1)*X+4.960*Math.pow(10,-2)*Math.pow(X,2)+2.157*Math.pow(10,-3)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((50.03*Math.pow(X,-1)+5.490+0.01728*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 23: //V
    {
        if(x>5.465)
        {
          tau = "-(0.2018*Math.pow(10,3))*Math.pow(X,-2)+(0.1560*Math.pow(10,6))*Math.pow(X,-3)-(0.3252*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.4601*Math.pow(10,3))*Math.pow(X,-2)+(0.9829*Math.pow(10,4))*Math.pow(X,-3)-(0.3826*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.775*Math.pow(10,-1)*X)*Math.pow((2.367*Math.pow(10,-1)+1.307*Math.pow(10,-1)*X+3.807*Math.pow(10,-2)*Math.pow(X,2)+1.566*Math.pow(10,-3)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((52.62*Math.pow(X,-1)+5.580+0.01763*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 24: //Cr
    {
        if(x>5.989)
        {
          tau = "-(0.2213*Math.pow(10,3))*Math.pow(X,-2)+(0.1827*Math.pow(10,6))*Math.pow(X,-3)-(0.4226*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.5149*Math.pow(10,3))*Math.pow(X,-2)+(0.1154*Math.pow(10,5))*Math.pow(X,-3)-(0.4693*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+4.190*Math.pow(10,-1)*X)*Math.pow((2.292*Math.pow(10,-1)+8.642*Math.pow(10,-2)*X+2.697*Math.pow(10,-2)*Math.pow(X,2)+1.028*Math.pow(10,-3)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((54.13*Math.pow(X,-1)+5.430+0.01746*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 25: //Mn
    {
        if(x>6.539)
        {
          tau = "-(0.2245*Math.pow(10,3))*Math.pow(X,-2)+(0.2042*Math.pow(10,6))*Math.pow(X,-3)-(0.5055*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.5484*Math.pow(10,3))*Math.pow(X,-2)+(0.1312*Math.pow(10,5))*Math.pow(X,-3)-(0.5638*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+3.707*Math.pow(10,-1)*X)*Math.pow((2.167*Math.pow(10,-1)+8.029*Math.pow(10,-2)*X+2.324*Math.pow(10,-2)*Math.pow(X,2)+8.595*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((56.25*Math.pow(X,-1)+5.517+0.01764*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 26: //Fe
    {
        if(x>7.112)
        {
          tau = "-(0.2371*Math.pow(10,3))*Math.pow(X,-2)+(0.2359*Math.pow(10,6))*Math.pow(X,-3)-(0.6309*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.6176*Math.pow(10,3))*Math.pow(X,-2)+(0.1530*Math.pow(10,5))*Math.pow(X,-3)-(0.6905*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+3.075*Math.pow(10,-1)*X)*Math.pow((2.062*Math.pow(10,-1)+6.305*Math.pow(10,-2)*X+1.845*Math.pow(10,-2)*Math.pow(X,2)+6.535*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((56.92*Math.pow(X,-1)+5.382+0.01732*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 27: //Co
    {
        if(x>7.705)
        {
          tau = "-(0.2397*Math.pow(10,3))*Math.pow(X,-2)+(0.2612*Math.pow(10,6))*Math.pow(X,-3)-(0.7533*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.6579*Math.pow(10,3))*Math.pow(X,-2)+(0.1696*Math.pow(10,5))*Math.pow(X,-3)-(0.7918*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.591*Math.pow(10,-1)*X)*Math.pow((2.020*Math.pow(10,-1)+5.267*Math.pow(10,-2)*X+1.548*Math.pow(10,-2)*Math.pow(X,2)+5.231*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((59.76*Math.pow(X,-1)+5.460+0.01767*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 28: //Ni
    {
        if(x>8.332)
        {
          tau = "-(0.2531*Math.pow(10,3))*Math.pow(X,-2)+(0.3041*Math.pow(10,6))*Math.pow(X,-3)-(0.9428*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.7147*Math.pow(10,3))*Math.pow(X,-2)+(0.1993*Math.pow(10,5))*Math.pow(X,-3)-(0.9624*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.214*Math.pow(10,-1)*X)*Math.pow((1.870*Math.pow(10,-1)+4.225*Math.pow(10,-2)*X+1.244*Math.pow(10,-2)*Math.pow(X,2)+3.980*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((59.81*Math.pow(X,-1)+5.222+0.01711*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 29: //Cu
    {
        if(x>8.978)
        {
          tau = "-(0.2491*Math.pow(10,3))*Math.pow(X,-2)+(0.3252*Math.pow(10,6))*Math.pow(X,-3)-(0.1097*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(x<8.978 && x>1.096)
        {
          tau = "(0.7031*Math.pow(10,3))*Math.pow(X,-2)+(0.2165*Math.pow(10,5))*Math.pow(X,-3)-(0.1127*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1357*Math.pow(10,5))*Math.pow(X,-3)-(0.3001*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.902*Math.pow(10,-1)*X)*Math.pow((1.874*Math.pow(10,-1)+3.418*Math.pow(10,-2)*X+1.103*Math.pow(10,-2)*Math.pow(X,2)+3.367*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((65.59*Math.pow(X,-1)+5.428+0.01804*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 30: //Zn
    {
        if(x>9.658)
        {
          tau = "-(0.2426*Math.pow(10,3))*Math.pow(X,-2)+(0.3619*Math.pow(10,6))*Math.pow(X,-3)-(0.1289*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(9.658>x && x>1.193)
        {
          tau = "(0.7031*Math.pow(10,3))*Math.pow(X,-2)+(0.2165*Math.pow(10,5))*Math.pow(X,-3)-(0.1127*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.193>=x && x>=1.042)
        {
          tau = "(0.2889*Math.pow(10,5)*Math.pow(X,-3))-(0.1942*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(1.042>x && x>1.019)
        {
          tau = "(-0.1590*Math.pow(10,5)*Math.pow(X,-3))-(-0.2482*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.2572*Math.pow(10,4))*Math.pow(X,-3)-(0.1023*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.690*Math.pow(10,-1)*X)*Math.pow((1.786*Math.pow(10,-1)+3.177*Math.pow(10,-2)*X+9.452*Math.pow(10,-3)*Math.pow(X,2)+2.811*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((67.69*Math.pow(X,-1)+5.377+0.01810*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 31: //Ga
    {
        if(x>10.367)
        {
          tau = "-(0.2328*Math.pow(10,3))*Math.pow(X,-2)+(0.3881*Math.pow(10,6))*Math.pow(X,-3)-(0.1475*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(10.367>x && x>1.297)
        {
          tau = "(0.6933*Math.pow(10,3))*Math.pow(X,-2)+(0.2734*Math.pow(10,5))*Math.pow(X,-3)-(0.1657*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.297>x && x>1.142)
        {
          tau = "(0.3217*Math.pow(10,5)*Math.pow(X,-3))-(0.2363*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(1.142>x && x>1.115)
        {
          tau = "(-0.1315*Math.pow(10,5)*Math.pow(X,-3))-(-0.2503*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.2873*Math.pow(10,4))*Math.pow(X,-3)-(0.1181*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.458*Math.pow(10,-1)*X)*Math.pow((1.799*Math.pow(10,-1)+3.125*Math.pow(10,-2)*X+8.248*Math.pow(10,-3)*Math.pow(X,2)+2.369*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((65.9*Math.pow(X,-1)+5.722+0.01764*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 32: //Ge
    {
        if(x>11.103)
        {
          tau = "-(0.2231*Math.pow(10,3))*Math.pow(X,-2)+(0.4236*Math.pow(10,6))*Math.pow(X,-3)-(0.1708*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(11.103>x && x>1.414)
        {
          tau = "(0.7231*Math.pow(10,3))*Math.pow(X,-2)+(0.3038*Math.pow(10,5))*Math.pow(X,-3)-(0.1954*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.414>x && x>1.247)
        {
          tau = "(0.3416*Math.pow(10,5)*Math.pow(X,-3))-(0.2594*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(1.247>x && x>1.216)
        {
          tau = "(-0.1079*Math.pow(10,5)*Math.pow(X,-3))-(-0.2594*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.3266*Math.pow(10,4))*Math.pow(X,-3)-(0.1378*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.390*Math.pow(10,-1)*X)*Math.pow((1.756*Math.pow(10,-1)+3.140*Math.pow(10,-2)*X+7.573*Math.pow(10,-3)*Math.pow(X,2)+2.114*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((68.74*Math.pow(X,-1)+5.759+0.01789*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 33: //As
    {
        if(x>11.866)
        {
          tau = "-(0.2135*Math.pow(10,3))*Math.pow(X,-2)+(0.4644*Math.pow(10,6))*Math.pow(X,-3)-(0.1982*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(11.866>x && x>1.526)
        {
          tau = "(0.7490*Math.pow(10,3))*Math.pow(X,-2)+(0.3399*Math.pow(10,5))*Math.pow(X,-3)-(0.2339*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.526>x && x>1.358)
        {
          tau = "(0.3594*Math.pow(10,5)*Math.pow(X,-3))-(0.2762*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(1.358>x && x>1.323)
        {
          tau = "(-0.9541*Math.pow(10,4)*Math.pow(X,-3))-(-0.2846*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.3765*Math.pow(10,4))*Math.pow(X,-3)-(0.1650*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.465*Math.pow(10,-1)*X)*Math.pow((1.7*Math.pow(10,-1)+3.174*Math.pow(10,-2)*X+7.414*Math.pow(10,-3)*Math.pow(X,2)+2.197*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((70.84*Math.pow(X,-1)+5.755+0.01797*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 34: //Se
    {
        if(x>12.657)
        {
          tau = "-(0.1963*Math.pow(10,3))*Math.pow(X,-2)+(0.4978*Math.pow(10,6))*Math.pow(X,-3)-(0.2250*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(12.657>x && x>1.653)
        {
          tau = "(0.7661*Math.pow(10,3))*Math.pow(X,-2)+(0.3724*Math.pow(10,5))*Math.pow(X,-3)-(0.2740*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.653>x && x>1.476)
        {
          tau = "(0.3556*Math.pow(10,5)*Math.pow(X,-3))-(0.2627*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(1.476>x && x>1.435)
        {
          tau = "(-0.3092*Math.pow(10,4)*Math.pow(X,-3))-(-0.2333*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.4214*Math.pow(10,4))*Math.pow(X,-3)-(0.1902*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.665*Math.pow(10,-1)*X)*Math.pow((1.688*Math.pow(10,-1)+3.453*Math.pow(10,-2)*X+7.860*Math.pow(10,-3)*Math.pow(X,2)+2.404*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((74.43*Math.pow(X,-1)+5.881+0.01844*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 35: //Br
    {
        if(x>13.473)
        {
          tau = "-(0.1788*Math.pow(10,3))*Math.pow(X,-2)+(0.5528*Math.pow(10,6))*Math.pow(X,-3)-(0.2646*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(13.473>x && x>1.782)
        {
          tau = "(0.7869*Math.pow(10,3))*Math.pow(X,-2)+(0.4214*Math.pow(10,5))*Math.pow(X,-3)-(0.3277*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.782>x && x>1.596)
        {
          tau = "(0.3588*Math.pow(10,5)*Math.pow(X,-3))-(0.2389*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(1.596>x && x>1.549)
        {
          tau = "(-0.7092*Math.pow(10,4)*Math.pow(X,-3))-(-0.3479*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.4876*Math.pow(10,4))*Math.pow(X,-3)-(0.2258*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.040*Math.pow(10,-1)*X)*Math.pow((1.614*Math.pow(10,-1)+3.768*Math.pow(10,-2)*X+8.521*Math.pow(10,-3)*Math.pow(X,2)+2.778*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((74.84*Math.pow(X,-1)+5.785+0.01808*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 36: //Kr
    {
        if(x>14.325)
        {
          tau = "(0.5738*Math.pow(10,6))*Math.pow(X,-3)-(0.2753*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(14.325>x && x>1.921)
        {
          tau = "(0.8114*Math.pow(10,3))*Math.pow(X,-2)+(0.4562*Math.pow(10,5))*Math.pow(X,-3)-(0.3722*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.921>x && x>1.727)
        {
          tau = "(0.3388*Math.pow(10,5)*Math.pow(X,-3))-(0.1775*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(1.727>x && x>1.671)
        {
          tau = "(-0.3331*Math.pow(10,5)*Math.pow(X,-3))-(-0.8565*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.5401*Math.pow(10,4))*Math.pow(X,-3)-(0.2556*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.563*Math.pow(10,-1)*X)*Math.pow((1.605*Math.pow(10,-1)+4.357*Math.pow(10,-2)*X+9.829*Math.pow(10,-3)*Math.pow(X,2)+3.412*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((78.38*Math.pow(X,-1)+5.888+0.01853*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 37: //Rb
    {
        if(x>15.199)
        {
          tau = "(0.6321*Math.pow(10,6))*Math.pow(X,-3)-(0.3222*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(15.199>x && x>2.065)
        {
          tau = "(0.8061*Math.pow(10,3))*Math.pow(X,-2)+(0.5107*Math.pow(10,5))*Math.pow(X,-3)-(0.4410*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.065>x && x>1.863)
        {
          tau = "(0.4800*Math.pow(10,5)*Math.pow(X,-3))-(0.4165*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(1.863>x && x>1.804)
        {
          tau = "(0.2975*Math.pow(10,5)*Math.pow(X,-3))-(-0.2098*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.6124*Math.pow(10,4))*Math.pow(X,-3)-(0.2965*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.660*Math.pow(10,-1)*X)*Math.pow((1.539*Math.pow(10,-1)+3.449*Math.pow(10,-2)*X+6.750*Math.pow(10,-3)*Math.pow(X,2)+2.079*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((79.11*Math.pow(X,-1)+5.851+0.01835*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 38: //Sr
    {
        if(x>16.104)
        {
          tau = "(0.6915*Math.pow(10,6))*Math.pow(X,-3)-(0.3763*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(16.104>x && x>2.216)
        {
          tau = "(0.8448*Math.pow(10,3))*Math.pow(X,-2)+(0.5659*Math.pow(10,5))*Math.pow(X,-3)-(0.5234*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.216>x && x>2.006)
        {
          tau = "(0.5024*Math.pow(10,5)*Math.pow(X,-3))-(0.4284*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(2.006>x && x>1.939)
        {
          tau = "(0.2310*Math.pow(10,5)*Math.pow(X,-3))-(-0.4765*Math.pow(10,4)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1040*Math.pow(10,4))*Math.pow(X,-2)+(0.4001*Math.pow(10,4))*Math.pow(X,-3)-(0.1553*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.310*Math.pow(10,-1)*X)*Math.pow((1.488*Math.pow(10,-1)+3.124*Math.pow(10,-2)*X+5.429*Math.pow(10,-3)*Math.pow(X,2)+1.580*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((79.79*Math.pow(X,-1)+5.863+0.01821*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 39: //Y
    {
        if(x>17.038)
        {
          tau = "(0.7615*Math.pow(10,6))*Math.pow(X,-3)-(0.4412*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(17.038>x && x>2.372)
        {
          tau = "(0.8931*Math.pow(10,3))*Math.pow(X,-2)+(0.6297*Math.pow(10,5))*Math.pow(X,-3)-(0.6171*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.372>x && x>2.155)
        {
          tau = "(0.5458*Math.pow(10,5)*Math.pow(X,-3))-(0.4732*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(2.155>x && x>2.080)
        {
          tau = "(0.2326*Math.pow(10,5)*Math.pow(X,-3))-(-0.3694*Math.pow(10,4)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1148*Math.pow(10,4))*Math.pow(X,-2)+(0.4487*Math.pow(10,4))*Math.pow(X,-3)-(0.1768*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.794*Math.pow(10,-1)*X)*Math.pow((1.435*Math.pow(10,-1)+3.612*Math.pow(10,-2)*X+6.434*Math.pow(10,-3)*Math.pow(X,2)+2.056*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((80.83*Math.pow(X,-1)+5.8+0.01798*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 40: //Zr
    {
        if(x>17.997)
        {
          tau = "(0.8263*Math.pow(10,6))*Math.pow(X,-3)-(0.5088*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(17.997>x && x>2.531)
        {
          tau = "(0.9174*Math.pow(10,3))*Math.pow(X,-2)+(0.6902*Math.pow(10,5))*Math.pow(X,-3)-(0.7135*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.531>x && x>2.306)
        {
          tau = "(0.5865*Math.pow(10,5)*Math.pow(X,-3))-(0.5184*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(2.306>x && x>2.222)
        {
          tau = "(0.2511*Math.pow(10,5)*Math.pow(X,-3))-(-0.2008*Math.pow(10,4)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1245*Math.pow(10,4))*Math.pow(X,-2)+(0.4942*Math.pow(10,4))*Math.pow(X,-3)-(0.1983*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+3.040*Math.pow(10,-1)*X)*Math.pow((1.421*Math.pow(10,-1)+4.878*Math.pow(10,-2)*X+9.372*Math.pow(10,-3)*Math.pow(X,2)+3.381*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((81.87*Math.pow(X,-1)+5.790+0.01805*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 41: //Nb
    {
        if(x>18.985)
        {
          tau = "(0.9001*Math.pow(10,6))*Math.pow(X,-3)-(0.5848*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(18.985>x && x>2.697)
        {
          tau = "(0.9588*Math.pow(10,3))*Math.pow(X,-2)+(0.7570*Math.pow(10,5))*Math.pow(X,-3)-(0.8170*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.697>x && x>2.464)
        {
          tau = "(0.6456*Math.pow(10,5)*Math.pow(X,-3))-(0.6001*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(2.464>x && x>2.370)
        {
          tau = "(0.2869*Math.pow(10,5)*Math.pow(X,-3))-(-0.4479*Math.pow(10,4)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1320*Math.pow(10,4))*Math.pow(X,-2)+(0.5559*Math.pow(10,4))*Math.pow(X,-3)-(0.2278*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+4.815*Math.pow(10,-1)*X)*Math.pow((1.391*Math.pow(10,-1)+6.505*Math.pow(10,-2)*X+1.343*Math.pow(10,-2)*Math.pow(X,2)+5.147*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((77.98*Math.pow(X,-1)+5.895+0.01722*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 42: //Mo
    {
        if(x>19.999)
        {
          tau = "(0.9649*Math.pow(10,6))*Math.pow(X,-3)-(0.6635*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(19.999>x && x>2.865)
        {
          tau = "(0.9956*Math.pow(10,3))*Math.pow(X,-2)+(0.8166*Math.pow(10,5))*Math.pow(X,-3)-(0.9212*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.865>x && x>2.625)
        {
          tau = "(0.6946*Math.pow(10,5)*Math.pow(X,-3))-(0.6720*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(2.625>x && x>2.520)
        {
          tau = "(0.3382*Math.pow(10,5)*Math.pow(X,-3))-(0.5891*Math.pow(10,4)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1371*Math.pow(10,4))*Math.pow(X,-2)+(0.6170*Math.pow(10,4))*Math.pow(X,-3)-(0.2607*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.411*Math.pow(10,-1)*X)*Math.pow((1.345*Math.pow(10,-1)+7.060*Math.pow(10,-2)*X+1.463*Math.pow(10,-2)*Math.pow(X,2)+5.588*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((80.67*Math.pow(X,-1)+5.926+0.01748*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 43: //Tc
    {
        if(x>21.044)
        {
          tau = "(0.1034*Math.pow(10,7))*Math.pow(X,-3)-(0.7517*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(21.044>x && x>3.042)
        {
          tau = "(0.9760*Math.pow(10,3))*Math.pow(X,-2)+(0.8887*Math.pow(10,5))*Math.pow(X,-3)-(0.1071*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(3.042>x && x>2.793)
        {
          tau = "(0.7487*Math.pow(10,5)*Math.pow(X,-3))-(0.7579*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(2.793>x && x>2.676)
        {
          tau = "(0.4015*Math.pow(10,5)*Math.pow(X,-3))-(0.1605*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1452*Math.pow(10,4))*Math.pow(X,-2)+(0.6728*Math.pow(10,4))*Math.pow(X,-3)-(0.2892*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.743*Math.pow(10,-1)*X)*Math.pow((1.311*Math.pow(10,-1)+7.273*Math.pow(10,-2)*X+1.496*Math.pow(10,-2)*Math.pow(X,2)+5.695*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((81.49*Math.pow(X,-1)+5.927+0.01735*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 44: //Ru
    {
        if(x>22.117)
        {
          tau = "(0.1116*Math.pow(10,7))*Math.pow(X,-3)-(0.8554*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(22.117>x && x>3.224)
        {
          tau = "(0.1018*Math.pow(10,4))*Math.pow(X,-2)+(0.9670*Math.pow(10,5))*Math.pow(X,-3)-(0.1222*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(3.224>x && x>2.966)
        {
          tau = "(0.8078*Math.pow(10,5)*Math.pow(X,-3))-(0.8411*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(2.966>x && x>2.837)
        {
          tau = "(0.4695*Math.pow(10,5)*Math.pow(X,-3))-(0.2717*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1537*Math.pow(10,4))*Math.pow(X,-2)+(0.7433*Math.pow(10,4))*Math.pow(X,-3)-(0.3262*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.771*Math.pow(10,-1)*X)*Math.pow((1.297*Math.pow(10,-1)+7.016*Math.pow(10,-2)*X+1.493*Math.pow(10,-2)*Math.pow(X,2)+5.478*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((83.16*Math.pow(X,-1)+5.987+0.01743*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 45: //Rh
    {
        if(x>23.219)
        {
          tau = "(0.1205*Math.pow(10,7))*Math.pow(X,-3)-(0.9730*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(23.219>x && x>3.411)
        {
          tau = "(0.1050*Math.pow(10,4))*Math.pow(X,-2)+(0.1054*Math.pow(10,6))*Math.pow(X,-3)-(0.1398*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(3.411>x && x>3.146)
        {
          tau = "(0.9003*Math.pow(10,5)*Math.pow(X,-3))-(0.1030*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(3.146>x && x>3.003)
        {
          tau = "(0.5655*Math.pow(10,5)*Math.pow(X,-3))-(0.4734*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1612*Math.pow(10,4))*Math.pow(X,-2)+(0.8250*Math.pow(10,4))*Math.pow(X,-3)-(0.3703*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.623*Math.pow(10,-1)*X)*Math.pow((1.281*Math.pow(10,-1)+6.536*Math.pow(10,-2)*X+1.422*Math.pow(10,-2)*Math.pow(X,2)+5.096*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((90.02*Math.pow(X,-1)+5.803+0.01826*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 46: //Pd
    {
        if(x>24.350)
        {
          tau = "(0.1279*Math.pow(10,7))*Math.pow(X,-3)-(0.1088*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(24.350>x && x>3.604)
        {
          tau = "(0.1085*Math.pow(10,4))*Math.pow(X,-2)+(0.1123*Math.pow(10,6))*Math.pow(X,-3)-(0.1547*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(3.604>x && x>3.330)
        {
          tau = "(0.1024*Math.pow(10,6)*Math.pow(X,-3))-(0.1371*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(3.330>x && x>3.173)
        {
          tau = "(0.7496*Math.pow(10,5)*Math.pow(X,-3))-(0.1008*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1693*Math.pow(10,4))*Math.pow(X,-2)+(0.8872*Math.pow(10,4))*Math.pow(X,-3)-(0.4040*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.415*Math.pow(10,-1)*X)*Math.pow((1.272*Math.pow(10,-1)+6.081*Math.pow(10,-2)*X+1.374*Math.pow(10,-2)*Math.pow(X,2)+4.715*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((92.23*Math.pow(X,-1)+5.883+0.01841*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 47: //Ag
    {
        if(x>25.514)
        {
          tau = "(0.1384*Math.pow(10,7))*Math.pow(X,-3)-(0.1238*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(25.514>x && x>3.805)
        {
          tau = "(0.1135*Math.pow(10,4))*Math.pow(X,-2)+(0.1223*Math.pow(10,6))*Math.pow(X,-3)-(0.1765*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(3.805>x && x>3.523)
        {
          tau = "(0.1075*Math.pow(10,6)*Math.pow(X,-3))-(0.1415*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(3.523>x && x>3.351)
        {
          tau = "(0.7408*Math.pow(10,5)*Math.pow(X,-3))-(0.8834*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1818*Math.pow(10,4))*Math.pow(X,-2)+(0.9723*Math.pow(10,4))*Math.pow(X,-3)-(0.4517*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.082*Math.pow(10,-1)*X)*Math.pow((1.214*Math.pow(10,-1)+5.642*Math.pow(10,-2)*X+1.250*Math.pow(10,-2)*Math.pow(X,2)+4.238*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((92.27*Math.pow(X,-1)+5.857+0.01817*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 48: //Cd
    {
        if(x>26.711)
        {
          tau = "(0.1453*Math.pow(10,7))*Math.pow(X,-3)-(0.1366*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(26.711>x && x>4.018)
        {
          tau = "(0.1102*Math.pow(10,4))*Math.pow(X,-2)+(0.1305*Math.pow(10,6))*Math.pow(X,-3)-(0.2009*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(4.018>x && x>3.727)
        {
          tau = "(0.1142*Math.pow(10,6)*Math.pow(X,-3))-(0.1588*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(3.727>x && x>3.537)
        {
          tau = "(0.7830*Math.pow(10,5)*Math.pow(X,-3))-(0.9738*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1874*Math.pow(10,4))*Math.pow(X,-2)+(0.1040*Math.pow(10,5))*Math.pow(X,-3)-(0.4943*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+4.921*Math.pow(10,-1)*X)*Math.pow((1.230*Math.pow(10,-1)+5.532*Math.pow(10,-2)*X+1.208*Math.pow(10,-2)*Math.pow(X,2)+4.035*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((94.65*Math.pow(X,-1)+6.003+0.01841*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 49: //In
    {
        if(x>27.939)
        {
          tau = "(0.1551*Math.pow(10,7))*Math.pow(X,-3)-(0.1519*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(27.939>x && x>4.237)
        {
          tau = "(0.1130*Math.pow(10,4))*Math.pow(X,-2)+(0.1406*Math.pow(10,6))*Math.pow(X,-3)-(0.2257*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(4.237>x && x>3.938)
        {
          tau = "(0.1270*Math.pow(10,6)*Math.pow(X,-3))-(0.1957*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(3.938>x && x>3.730)
        {
          tau = "(0.1006*Math.pow(10,6)*Math.pow(X,-3))-(0.1736*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1980*Math.pow(10,4))*Math.pow(X,-2)+(0.1131*Math.pow(10,5))*Math.pow(X,-3)-(0.5499*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+4.741*Math.pow(10,-1)*X)*Math.pow((1.199*Math.pow(10,-1)+5.382*Math.pow(10,-2)*X+1.136*Math.pow(10,-2)*Math.pow(X,2)+3.757*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((95.15*Math.pow(X,-1)+6.032+0.01830*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 50: //Sn
    {
        if(x>29.200)
        {
          tau = "(0.1630*Math.pow(10,7))*Math.pow(X,-3)-(0.1643*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(29.200>x && x>4.464)
        {
          tau = "(0.1152*Math.pow(10,4))*Math.pow(X,-2)+(0.1492*Math.pow(10,6))*Math.pow(X,-3)-(0.2495*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(4.464>x && x>4.156)
        {
          tau = "(0.1428*Math.pow(10,6)*Math.pow(X,-3))-(0.2532*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(4.156>x && x>3.928)
        {
          tau = "(0.1097*Math.pow(10,6)*Math.pow(X,-3))-(0.2033*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.2051*Math.pow(10,4))*Math.pow(X,-2)+(0.1216*Math.pow(10,5))*Math.pow(X,-3)-(0.6037*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+4.821*Math.pow(10,-1)*X)*Math.pow((1.200*Math.pow(10,-1)+5.513*Math.pow(10,-2)*X+1.138*Math.pow(10,-2)*Math.pow(X,2)+3.740*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((96.63*Math.pow(X,-1)+6.142+0.01838*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 51: //Sb
    {
        if(x>30.491)
        {
          tau = "(0.1734*Math.pow(10,7))*Math.pow(X,-3)-(0.1865*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(30.491>x && x>4.698)
        {
          tau = "(0.1139*Math.pow(10,4))*Math.pow(X,-2)+(0.1604*Math.pow(10,6))*Math.pow(X,-3)-(0.2815*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(4.698>x && x>4.380)
        {
          tau = "(0.1527*Math.pow(10,6)*Math.pow(X,-3))-(0.2838*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(4.380>x && x>4.132)
        {
          tau = "(0.1276*Math.pow(10,6)*Math.pow(X,-3))-(0.2745*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.2136*Math.pow(10,4))*Math.pow(X,-2)+(0.1315*Math.pow(10,5))*Math.pow(X,-3)-(0.6735*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.118*Math.pow(10,-1)*X)*Math.pow((1.184*Math.pow(10,-1)+5.761*Math.pow(10,-2)*X+1.178*Math.pow(10,-2)*Math.pow(X,2)+3.863*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((97.76*Math.pow(X,-1)+6.193+0.01844*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 52: //Te
    {
        if(x>31.813)
        {
          tau = "(0.1797*Math.pow(10,7))*Math.pow(X,-3)-(0.2018*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(31.813>x && x>4.939)
        {
          tau = "(0.1161*Math.pow(10,4))*Math.pow(X,-2)+(0.1668*Math.pow(10,6))*Math.pow(X,-3)-(0.3041*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(4.939>x && x>4.612)
        {
          tau = "(0.1589*Math.pow(10,6)*Math.pow(X,-3))-(0.3071*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(4.612>x && x>4.341)
        {
          tau = "(0.1299*Math.pow(10,6)*Math.pow(X,-3))-(0.2849*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.2199*Math.pow(10,4))*Math.pow(X,-2)+(0.1381*Math.pow(10,5))*Math.pow(X,-3)-(0.7426*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.467*Math.pow(10,-1)*X)*Math.pow((1.186*Math.pow(10,-1)+6.083*Math.pow(10,-2)*X+1.258*Math.pow(10,-2)*Math.pow(X,2)+4.122*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((100.7*Math.pow(X,-1)+6.399+0.01879*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 53: //I
    {
        if(x>33.169)
        {
          tau = "(0.1960*Math.pow(10,7))*Math.pow(X,-3)-(0.2296*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(33.169>x && x>5.188)
        {
          tau = "(0.1219*Math.pow(10,4))*Math.pow(X,-2)+(0.1834*Math.pow(10,6))*Math.pow(X,-3)-(0.3505*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(5.188>x && x>4.852)
        {
          tau = "(0.1703*Math.pow(10,6)*Math.pow(X,-3))-(0.3295*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(4.852>x && x>4.557)
        {
          tau = "(0.1382*Math.pow(10,6)*Math.pow(X,-3))-(0.3053*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(4.557>x && x>1.072)
        {
          tau = "(0.2328*Math.pow(10,4))*Math.pow(X,-2)+(0.1552*Math.pow(10,5))*Math.pow(X,-3)-(0.8511*Math.pow(10,4))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1807*Math.pow(10,5))*Math.pow(X,-3)-(0.8981*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.921*Math.pow(10,-1)*X)*Math.pow((1.144*Math.pow(10,-1)+6.142*Math.pow(10,-2)*X+1.292*Math.pow(10,-2)*Math.pow(X,2)+4.238*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((98.33*Math.pow(X,-1)+6.276+0.01819*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 54: //Xe
    {
        if(x>34.564)
        {
          tau = "(0.2049*Math.pow(10,7))*Math.pow(X,-3)-(0.2505*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(34.564>x && x>5.452)
        {
          tau = "(0.1243*Math.pow(10,4))*Math.pow(X,-2)+(0.1933*Math.pow(10,6))*Math.pow(X,-3)-(0.3852*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(5.452>x && x>5.103)
        {
          tau = "(0.1787*Math.pow(10,6)*Math.pow(X,-3))-(0.3597*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.103>x && x>4.782)
        {
          tau = "(0.1715*Math.pow(10,6)*Math.pow(X,-3))-(0.3859*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(4.782>x && x>1.148)
        {
          tau = "(0.2349*Math.pow(10,4))*Math.pow(X,-2)+(0.1681*Math.pow(10,5))*Math.pow(X,-3)-(0.9660*Math.pow(10,4))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1898*Math.pow(10,5))*Math.pow(X,-3)-(0.9576*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+6.399*Math.pow(10,-1)*X)*Math.pow((1.131*Math.pow(10,-1)+6.545*Math.pow(10,-2)*X+1.382*Math.pow(10,-2)*Math.pow(X,2)+4.502*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((100.3*Math.pow(X,-1)+6.394+0.01839*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 55: //Cs
    {
        if(x>35.984)
        {
          tau = "(0.2187*Math.pow(10,7))*Math.pow(X,-3)-(0.2781*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(35.984>x && x>5.714)
        {
          tau = "(0.1285*Math.pow(10,4))*Math.pow(X,-2)+(0.2082*Math.pow(10,6))*Math.pow(X,-3)-(0.4344*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(5.714>x && x>5.359)
        {
          tau = "(0.1796*Math.pow(10,6)*Math.pow(X,-3))-(0.3318*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.359>x && x>5.011)
        {
          tau = "(0.1253*Math.pow(10,6)*Math.pow(X,-3))-(0.2100*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.011>x && x>1.217)
        {
          tau = "(0.2368*Math.pow(10,4))*Math.pow(X,-2)+(0.1871*Math.pow(10,5))*Math.pow(X,-3)-(0.1126*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.217>x && x>1.065)
        {
          tau = "(0.2142*Math.pow(10,5))*Math.pow(X,-3)-(0.1166*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1854*Math.pow(10,5))*Math.pow(X,-3)-(0.9182*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.245*Math.pow(10,-1)*X)*Math.pow((1.108*Math.pow(10,-1)+5.572*Math.pow(10,-2)*X+1.119*Math.pow(10,-2)*Math.pow(X,2)+3.552*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((99.67*Math.pow(X,-1)+6.385+0.01814*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 56: //Ba
    {
        if(x>37.440)
        {
          tau = "(0.2281*Math.pow(10,7))*Math.pow(X,-3)-(0.2993*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(37.440>x && x>5.988)
        {
          tau = "(0.1297*Math.pow(10,4))*Math.pow(X,-2)+(0.2196*Math.pow(10,6))*Math.pow(X,-3)-(0.4812*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(5.988>x && x>5.623)
        {
          tau = "(0.1930*Math.pow(10,6)*Math.pow(X,-3))-(0.3876*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.623>x && x>5.247)
        {
          tau = "(0.1411*Math.pow(10,6)*Math.pow(X,-3))-(0.2814*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.247>x && x>1.292)
        {
          tau = "(0.2397*Math.pow(10,4))*Math.pow(X,-2)+(0.2017*Math.pow(10,5))*Math.pow(X,-3)-(0.1272*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.292>x && x>1.136)
        {
          tau = "(0.2323*Math.pow(10,5))*Math.pow(X,-3)-(0.1334*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.136>x && x>1.062)
        {
          tau = "(0.1982*Math.pow(10,5))*Math.pow(X,-3)-(0.1019*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1647*Math.pow(10,5))*Math.pow(X,-3)-(0.7933*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+4.967*Math.pow(10,-1)*X)*Math.pow((1.101*Math.pow(10,-1)+5.450*Math.pow(10,-2)*X+1.055*Math.pow(10,-2)*Math.pow(X,2)+3.312*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((101.1*Math.pow(X,-1)+6.513+0.01826*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 57: //La
    {
        if(x>38.924)
        {
          tau = "(0.2430*Math.pow(10,7))*Math.pow(X,-3)-(0.3295*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(38.924>x && x>6.266)
        {
          tau = "(0.1337*Math.pow(10,4))*Math.pow(X,-2)+(0.2358*Math.pow(10,6))*Math.pow(X,-3)-(0.5375*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(6.266>x && x>5.890)
        {
          tau = "(0.2049*Math.pow(10,6)*Math.pow(X,-3))-(0.4180*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.890>x && x>5.482)
        {
          tau = "(0.1487*Math.pow(10,6)*Math.pow(X,-3))-(0.2982*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.482>x && x>1.361)
        {
          tau = "(0.2447*Math.pow(10,4))*Math.pow(X,-2)+(0.2229*Math.pow(10,5))*Math.pow(X,-3)-(0.1474*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.361>x && x>1.204)
        {
          tau = "(0.2551*Math.pow(10,5))*Math.pow(X,-3)-(0.1536*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.204>x && x>1.123)
        {
          tau = "(0.2165*Math.pow(10,5))*Math.pow(X,-3)-(0.1155*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1674*Math.pow(10,5))*Math.pow(X,-3)-(0.7655*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.818*Math.pow(10,-1)*X)*Math.pow((1.076*Math.pow(10,-1)+6.162*Math.pow(10,-2)*X+1.187*Math.pow(10,-2)*Math.pow(X,2)+3.746*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((101.4*Math.pow(X,-1)+6.499+0.01804*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 58: //Ce
    {
        if(x>40.443)
        {
          tau = "(0.2601*Math.pow(10,7))*Math.pow(X,-3)-(0.3722*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(40.443>x && x>6.548)
        {
          tau = "(0.1342*Math.pow(10,4))*Math.pow(X,-2)+(0.2540*Math.pow(10,6))*Math.pow(X,-3)-(0.6049*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(6.548>x && x>6.164)
        {
          tau = "(0.2237*Math.pow(10,6)*Math.pow(X,-3))-(0.4912*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(6.164>x && x>5.723)
        {
          tau = "(0.1597*Math.pow(10,6)*Math.pow(X,-3))-(0.3306*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.723>x && x>1.434)
        {
          tau = "(0.2476*Math.pow(10,4))*Math.pow(X,-2)+(0.2440*Math.pow(10,5))*Math.pow(X,-3)-(0.1673*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.434>x && x>1.272)
        {
          tau = "(0.2770*Math.pow(10,5))*Math.pow(X,-3)-(0.1729*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.272>x && x>1.185)
        {
          tau = "(0.2335*Math.pow(10,5))*Math.pow(X,-3)-(0.1278*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1737*Math.pow(10,5))*Math.pow(X,-3)-(0.7655*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.099*Math.pow(10,-1)*X)*Math.pow((1.048*Math.pow(10,-1)+5.311*Math.pow(10,-2)*X+1.027*Math.pow(10,-2)*Math.pow(X,2)+3.148*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((99.48*Math.pow(X,-1)+6.484+0.01768*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 59: //Pr
    {
        if(x>41.990)
        {
          tau = "(0.2783*Math.pow(10,7))*Math.pow(X,-3)-(0.4146*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(41.990>x && x>6.834)
        {
          tau = "(0.1414*Math.pow(10,4))*Math.pow(X,-2)+(0.2730*Math.pow(10,6))*Math.pow(X,-3)-(0.6747*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(6.834>x && x>6.440)
        {
          tau = "(0.2411*Math.pow(10,6)*Math.pow(X,-3))-(0.5525*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(6.440>x && x>5.964)
        {
          tau = "(0.1669*Math.pow(10,6)*Math.pow(X,-3))-(0.4316*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.964>x && x>1.511)
        {
          tau = "(0.2604*Math.pow(10,4))*Math.pow(X,-2)+(0.2636*Math.pow(10,5))*Math.pow(X,-3)-(0.1858*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.511>x && x>1.337)
        {
          tau = "(0.3020*Math.pow(10,5))*Math.pow(X,-3)-(0.1959*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.337>x && x>1.242)
        {
          tau = "(0.2562*Math.pow(10,5))*Math.pow(X,-3)-(0.1463*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1784*Math.pow(10,5))*Math.pow(X,-3)-(0.7266*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+3.925*Math.pow(10,-1)*X)*Math.pow((1.008*Math.pow(10,-1)+4.051*Math.pow(10,-2)*X+7.917*Math.pow(10,-3)*Math.pow(X,2)+2.310*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((100.1*Math.pow(X,-1)+6.405+0.01752*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 60: //Nd
    {
        if(x>43.568)
        {
          tau = "(0.2920*Math.pow(10,7))*Math.pow(X,-3)-(0.4524*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(43.568>x && x>7.126)
        {
          tau = "(0.1465*Math.pow(10,4))*Math.pow(X,-2)+(0.2880*Math.pow(10,6))*Math.pow(X,-3)-(0.7393*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(7.126>x && x>6.721)
        {
          tau = "(0.2525*Math.pow(10,6)*Math.pow(X,-3))-(0.5923*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(6.721>x && x>6.207)
        {
          tau = "(0.1750*Math.pow(10,6)*Math.pow(X,-3))-(0.3669*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(6.207>x && x>1.575)
        {
          tau = "(0.2581*Math.pow(10,4))*Math.pow(X,-2)+(0.2857*Math.pow(10,5))*Math.pow(X,-3)-(0.2112*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.575>x && x>1.402)
        {
          tau = "(0.3222*Math.pow(10,5))*Math.pow(X,-3)-(0.2164*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.402>x && x>1.297)
        {
          tau = "(0.2732*Math.pow(10,5))*Math.pow(X,-3)-(0.1612*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.2017*Math.pow(10,5))*Math.pow(X,-3)-(0.9345*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+3.511*Math.pow(10,-1)*X)*Math.pow((1.004*Math.pow(10,-1)+3.634*Math.pow(10,-2)*X+7.104*Math.pow(10,-3)*Math.pow(X,2)+2.012*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((101.9*Math.pow(X,-1)+6.457+0.01757*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 61: //Pm
    {
        if(x>45.184)
        {
          tau = "(0.3115*Math.pow(10,7))*Math.pow(X,-3)-(0.5004*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(45.184>x && x>7.427)
        {
          tau = "(0.1538*Math.pow(10,4))*Math.pow(X,-2)+(0.3092*Math.pow(10,6))*Math.pow(X,-3)-(0.8237*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(7.427>x && x>7.012)
        {
          tau = "(0.2740*Math.pow(10,6)*Math.pow(X,-3))-(0.6806*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(7.012>x && x>6.459)
        {
          tau = "(0.1852*Math.pow(10,6)*Math.pow(X,-3))-(0.3906*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(6.459>x && x>1.650)
        {
          tau = "(0.2665*Math.pow(10,4))*Math.pow(X,-2)+(0.3111*Math.pow(10,5))*Math.pow(X,-3)-(0.2388*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.650>x && x>1.471)
        {
          tau = "(0.3519*Math.pow(10,5))*Math.pow(X,-3)-(0.2471*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.471>x && x>1.356)
        {
          tau = "(0.2907*Math.pow(10,5))*Math.pow(X,-3)-(0.1728*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.356>x && x>1.051)
        {
          tau = "(0.2165*Math.pow(10,5))*Math.pow(X,-3)-(0.1022*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.051>x && x>1.026)
        {
          tau = "(0.1631*Math.pow(10,5))*Math.pow(X,-3)-(0.8906*Math.pow(10,4))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.4450*Math.pow(10,4))*Math.pow(X,-3)-(0.2405*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+3.211*Math.pow(10,-1)*X)*Math.pow((9.763*Math.pow(10,-2)+3.251*Math.pow(10,-2)*X+6.383*Math.pow(10,-3)*Math.pow(X,2)+1.763*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((105.4*Math.pow(X,-1)+6.380+0.01743*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 62: //Sm
    {
        if(x>46.834)
        {
          tau = "(0.3211*Math.pow(10,7))*Math.pow(X,-3)-(0.5229*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(46.834>x && x>7.736)
        {
          tau = "(0.1577*Math.pow(10,4))*Math.pow(X,-2)+(0.3207*Math.pow(10,6))*Math.pow(X,-3)-(0.8830*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(7.736>x && x>7.311)
        {
          tau = "(0.2858*Math.pow(10,6)*Math.pow(X,-3))-(0.7415*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(7.311>x && x>6.716)
        {
          tau = "(0.1911*Math.pow(10,6)*Math.pow(X,-3))-(0.4109*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(6.716>x && x>1.722)
        {
          tau = "(0.2669*Math.pow(10,4))*Math.pow(X,-2)+(0.3281*Math.pow(10,5))*Math.pow(X,-3)-(0.2613*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.722>x && x>1.540)
        {
          tau = "(0.3683*Math.pow(10,5))*Math.pow(X,-3)-(0.2662*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.540>x && x>1.419)
        {
          tau = "(0.3069*Math.pow(10,5))*Math.pow(X,-3)-(0.1892*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.419>x && x>1.106)
        {
          tau = "(0.2294*Math.pow(10,5))*Math.pow(X,-3)-(0.1111*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.106>x && x>1.080)
        {
          tau = "(0.2482*Math.pow(10,5))*Math.pow(X,-3)-(0.1799*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.4747*Math.pow(10,4))*Math.pow(X,-3)-(0.2650*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.933*Math.pow(10,-1)*X)*Math.pow((9.671*Math.pow(10,-2)+3.015*Math.pow(10,-2)*X+5.914*Math.pow(10,-3)*Math.pow(X,2)+1.593*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((106.4*Math.pow(X,-1)+6.500+0.01785*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 63: //Eu
    {
        if(x>48.519)
        {
          tau = "(0.3395*Math.pow(10,7))*Math.pow(X,-3)-(0.5788*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(48.519>x && x>8.052)
        {
          tau = "(0.1624*Math.pow(10,4))*Math.pow(X,-2)+(0.3418*Math.pow(10,6))*Math.pow(X,-3)-(0.9797*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(8.052>x && x>7.617)
        {
          tau = "(0.3033*Math.pow(10,6)*Math.pow(X,-3))-(0.8102*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(7.617>x && x>6.976)
        {
          tau = "(0.2032*Math.pow(10,6)*Math.pow(X,-3))-(0.4513*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(6.976>x && x>1.800)
        {
          tau = "(0.2738*Math.pow(10,4))*Math.pow(X,-2)+(0.3541*Math.pow(10,5))*Math.pow(X,-3)-(0.2922*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.800>x && x>1.613)
        {
          tau = "(0.3963*Math.pow(10,5))*Math.pow(X,-3)-(0.2965*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.613>x && x>1.480)
        {
          tau = "(0.3349*Math.pow(10,5))*Math.pow(X,-3)-(0.2175*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.480>x && x>1.160)
        {
          tau = "(0.2465*Math.pow(10,5))*Math.pow(X,-3)-(0.1228*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.160>x && x>1.130)
        {
          tau = "(0.2457*Math.pow(10,5))*Math.pow(X,-3)-(0.1767*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.5084*Math.pow(10,4))*Math.pow(X,-3)-(0.2878*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.696*Math.pow(10,-1)*X)*Math.pow((9.526*Math.pow(10,-2)+2.778*Math.pow(10,-2)*X+5.372*Math.pow(10,-3)*Math.pow(X,2)+1.403*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((107.6*Math.pow(X,-1)+6.459+0.01777*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 64: //Gd
    {
        if(x>50.239)
        {
          tau = "(0.3516*Math.pow(10,7))*Math.pow(X,-3)-(0.6283*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(50.239>x && x>8.375)
        {
          tau = "(0.1604*Math.pow(10,4))*Math.pow(X,-2)+(0.3565*Math.pow(10,6))*Math.pow(X,-3)-(0.1065*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(8.375>x && x>7.930)
        {
          tau = "(0.3168*Math.pow(10,6)*Math.pow(X,-3))-(0.8859*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(7.930>x && x>7.242)
        {
          tau = "(0.2087*Math.pow(10,6)*Math.pow(X,-3))-(0.4667*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(7.242>x && x>1.880)
        {
          tau = "(0.2739*Math.pow(10,4))*Math.pow(X,-2)+(0.3729*Math.pow(10,5))*Math.pow(X,-3)-(0.3185*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.880>x && x>1.688)
        {
          tau = "(0.4162*Math.pow(10,5))*Math.pow(X,-3)-(0.3223*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.688>x && x>1.544)
        {
          tau = "(0.3488*Math.pow(10,5))*Math.pow(X,-3)-(0.2303*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.544>x && x>1.217)
        {
          tau = "(0.2554*Math.pow(10,5))*Math.pow(X,-3)-(0.1278*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.217>x && x>1.185)
        {
          tau = "(0.2413*Math.pow(10,5))*Math.pow(X,-3)-(0.1727*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.5397*Math.pow(10,4))*Math.pow(X,-3)-(0.3116*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.652*Math.pow(10,-1)*X)*Math.pow((9.606*Math.pow(10,-2)+2.706*Math.pow(10,-2)*X+5.275*Math.pow(10,-3)*Math.pow(X,2)+1.377*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((110.9*Math.pow(X,-1)+6.581+0.01813*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 65: //Tb
    {
        if(x>51.995)
        {
          tau = "(0.3722*Math.pow(10,7))*Math.pow(X,-3)-(0.6917*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(51.995>x && x>8.708)
        {
          tau = "(0.1689*Math.pow(10,4))*Math.pow(X,-2)+(0.3777*Math.pow(10,6))*Math.pow(X,-3)-(0.1166*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(8.708>x && x>8.252)
        {
          tau = "(0.3376*Math.pow(10,6)*Math.pow(X,-3))-(0.9837*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(8.252>x && x>7.514)
        {
          tau = "(0.2228*Math.pow(10,6)*Math.pow(X,-3))-(0.5205*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(7.514>x && x>1.967)
        {
          tau = "(0.2817*Math.pow(10,4))*Math.pow(X,-2)+(0.3977*Math.pow(10,5))*Math.pow(X,-3)-(0.3486*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.967>x && x>1.767)
        {
          tau = "(0.4480*Math.pow(10,5))*Math.pow(X,-3)-(0.3608*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.767>x && x>1.611)
        {
          tau = "(0.3706*Math.pow(10,5))*Math.pow(X,-3)-(0.2495*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.611>x && x>1.275)
        {
          tau = "(0.2716*Math.pow(10,5))*Math.pow(X,-3)-(0.1378*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.275>x && x>1.241)
        {
          tau = "(0.3316*Math.pow(10,5))*Math.pow(X,-3)-(0.2830*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.5667*Math.pow(10,4))*Math.pow(X,-3)-(0.3281*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.296*Math.pow(10,-1)*X)*Math.pow((9.451*Math.pow(10,-2)+2.306*Math.pow(10,-2)*X+4.584*Math.pow(10,-3)*Math.pow(X,2)+1.147*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((111.8*Math.pow(X,-1)+6.552+0.01804*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 66: //Dy
    {
        if(x>53.788)
        {
          tau = "(0.3884*Math.pow(10,7))*Math.pow(X,-3)-(0.7467*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(53.788>x && x>9.045)
        {
          tau = "(0.1751*Math.pow(10,4))*Math.pow(X,-2)+(0.3955*Math.pow(10,6))*Math.pow(X,-3)-(0.1264*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(9.045>x && x>8.580)
        {
          tau = "(0.3553*Math.pow(10,6)*Math.pow(X,-3))-(0.1079*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(8.580>x && x>7.790)
        {
          tau = "(0.2253*Math.pow(10,6)*Math.pow(X,-3))-(0.4925*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(7.790>x && x>2.046)
        {
          tau = "(0.2845*Math.pow(10,4))*Math.pow(X,-2)+(0.4216*Math.pow(10,5))*Math.pow(X,-3)-(0.3822*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.046>x && x>1.841)
        {
          tau = "(0.4729*Math.pow(10,5))*Math.pow(X,-3)-(0.3925*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.841>x && x>1.675)
        {
          tau = "(0.3926*Math.pow(10,5))*Math.pow(X,-3)-(0.2725*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.675>x && x>1.333)
        {
          tau = "(0.2916*Math.pow(10,5))*Math.pow(X,-3)-(0.1554*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.333>x && x>1.294)
        {
          tau = "(0.3557*Math.pow(10,5))*Math.pow(X,-3)-(0.3166*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.5996*Math.pow(10,4))*Math.pow(X,-3)-(0.3512*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.124*Math.pow(10,-1)*X)*Math.pow((9.341*Math.pow(10,-2)+2.171*Math.pow(10,-2)*X+4.244*Math.pow(10,-3)*Math.pow(X,2)+1.030*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((114.9*Math.pow(X,-1)+6.580+0.01825*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 67: //Ho
    {
        if(x>55.617)
        {
          tau = "(0.4073*Math.pow(10,7))*Math.pow(X,-3)-(0.8046*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(55.617>x && x>9.394)
        {
          tau = "(0.1829*Math.pow(10,4))*Math.pow(X,-2)+(0.4165*Math.pow(10,6))*Math.pow(X,-3)-(0.1375*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(9.394>x && x>8.917)
        {
          tau = "(0.3764*Math.pow(10,6)*Math.pow(X,-3))-(0.1189*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(8.917>x && x>8.071)
        {
          tau = "(0.2431*Math.pow(10,6)*Math.pow(X,-3))-(0.5884*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(8.071>x && x>2.128)
        {
          tau = "(0.2850*Math.pow(10,4))*Math.pow(X,-2)+(0.4527*Math.pow(10,5))*Math.pow(X,-3)-(0.4260*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.128>x && x>1.923)
        {
          tau = "(0.5025*Math.pow(10,5))*Math.pow(X,-3)-(0.4307*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.923>x && x>1.741)
        {
          tau = "(0.4157*Math.pow(10,5))*Math.pow(X,-3)-(0.2945*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.741>x && x>1.391)
        {
          tau = "(0.3164*Math.pow(10,5))*Math.pow(X,-3)-(0.1788*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.391>x && x>1.351)
        {
          tau = "(0.3209*Math.pow(10,5))*Math.pow(X,-3)-(0.2708*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.6385*Math.pow(10,4))*Math.pow(X,-3)-(0.3779*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.069*Math.pow(10,-1)*X)*Math.pow((9.129*Math.pow(10,-2)+2.144*Math.pow(10,-2)*X+4.057*Math.pow(10,-3)*Math.pow(X,2)+9.736*Math.pow(10,-5)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((117*Math.pow(X,-1)+6.569+0.01831*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 68: //Er
    {
        if(x>57.185)
        {
          tau = "(0.4267*Math.pow(10,7))*Math.pow(X,-3)-(0.8622*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(57.185>x && x>9.751)
        {
          tau = "(0.1910*Math.pow(10,4))*Math.pow(X,-2)+(0.4384*Math.pow(10,6))*Math.pow(X,-3)-(0.1492*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(9.751>x && x>9.264)
        {
          tau = "(0.3982*Math.pow(10,6)*Math.pow(X,-3))-(0.1307*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(9.264>x && x>8.357)
        {
          tau = "(0.2558*Math.pow(10,6)*Math.pow(X,-3))-(0.6354*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(8.357>x && x>2.206)
        {
          tau = "(0.2931*Math.pow(10,4))*Math.pow(X,-2)+(0.4801*Math.pow(10,5))*Math.pow(X,-3)-(0.4657*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.206>x && x>2.005)
        {
          tau = "(0.5830*Math.pow(10,5))*Math.pow(X,-3)-(0.4791*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.005>x && x>1.811)
        {
          tau = "(0.4770*Math.pow(10,5))*Math.pow(X,-3)-(0.3852*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.811>x && x>1.453)
        {
          tau = "(0.3413*Math.pow(10,5))*Math.pow(X,-3)-(0.2030*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.453>x && x>1.409)
        {
          tau = "(0.3585*Math.pow(10,5))*Math.pow(X,-3)-(0.3235*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.6797*Math.pow(10,4))*Math.pow(X,-3)-(0.4059*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.858*Math.pow(10,-1)*X)*Math.pow((9.058*Math.pow(10,-2)+1.863*Math.pow(10,-2)*X+3.656*Math.pow(10,-3)*Math.pow(X,2)+8.480*Math.pow(10,-5)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((118.9*Math.pow(X,-1)+6.556+0.01835*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 69: //Tm
    {
        if(x>59.389)
        {
          tau = "(0.4493*Math.pow(10,7))*Math.pow(X,-3)-(0.9392*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(59.389>x && x>10.115)
        {
          tau = "(0.1980*Math.pow(10,4))*Math.pow(X,-2)+(0.4654*Math.pow(10,6))*Math.pow(X,-3)-(0.1653*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(10.115>x && x>9.616)
        {
          tau = "(0.4232*Math.pow(10,6)*Math.pow(X,-3))-(0.1447*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(9.616>x && x>8.648)
        {
          tau = "(0.2705*Math.pow(10,6)*Math.pow(X,-3))-(0.6928*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(8.648>x && x>2.306)
        {
          tau = "(0.3024*Math.pow(10,4))*Math.pow(X,-2)+(0.5105*Math.pow(10,5))*Math.pow(X,-3)-(0.5103*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.306>x && x>2.089)
        {
          tau = "(0.5721*Math.pow(10,5))*Math.pow(X,-3)-(0.5235*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.089>x && x>1.884)
        {
          tau = "(0.4704*Math.pow(10,5))*Math.pow(X,-3)-(0.3516*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.884>x && x>1.514)
        {
          tau = "(0.3588*Math.pow(10,5))*Math.pow(X,-3)-(0.2150*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.514>x && x>1.467)
        {
          tau = "(0.4367*Math.pow(10,5))*Math.pow(X,-3)-(0.4388*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.7237*Math.pow(10,4))*Math.pow(X,-3)-(0.4349*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.732*Math.pow(10,-1)*X)*Math.pow((8.885*Math.pow(10,-2)+1.718*Math.pow(10,-2)*X+3.374*Math.pow(10,-3)*Math.pow(X,2)+7.638*Math.pow(10,-5)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((120.9*Math.pow(X,-1)+6.504+0.01839*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 70: //Yb
    {
        if(x>61.332)
        {
          tau = "(0.4675*Math.pow(10,7))*Math.pow(X,-3)-(0.1022*Math.pow(10,9))*Math.pow(X,-4)";
        }
        else if(61.332>x && x>10.486)
        {
          tau = "(0.1995*Math.pow(10,4))*Math.pow(X,-2)+(0.4866*Math.pow(10,6))*Math.pow(X,-3)-(0.1796*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(10.486>x && x>9.978)
        {
          tau = "(0.4552*Math.pow(10,6)*Math.pow(X,-3))-(0.1700*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(9.978>x && x>8.943)
        {
          tau = "(0.2817*Math.pow(10,6)*Math.pow(X,-3))-(0.7422*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(8.943>x && x>2.398)
        {
          tau = "(0.3077*Math.pow(10,4))*Math.pow(X,-2)+(0.5341*Math.pow(10,5))*Math.pow(X,-3)-(0.5486*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.398>x && x>2.173)
        {
          tau = "(0.6041*Math.pow(10,5))*Math.pow(X,-3)-(0.5743*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.173>x && x>1.949)
        {
          tau = "(0.4972*Math.pow(10,5))*Math.pow(X,-3)-(0.3864*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.949>x && x>1.576)
        {
          tau = "(0.3790*Math.pow(10,5))*Math.pow(X,-3)-(0.2362*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.576>x && x>1.527)
        {
          tau = "(0.4881*Math.pow(10,5))*Math.pow(X,-3)-(0.5234*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.7585*Math.pow(10,4))*Math.pow(X,-3)-(0.4579*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.621*Math.pow(10,-1)*X)*Math.pow((8.842*Math.pow(10,-2)+1.614*Math.pow(10,-2)*X+3.170*Math.pow(10,-3)*Math.pow(X,2)+7.012*Math.pow(10,-5)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((124.5*Math.pow(X,-1)+6.552+0.01864*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 71: //Lu
    {
        if(x>63.313)
        {
          tau = "(0.4915*Math.pow(10,7))*Math.pow(X,-3)-(0.1111*Math.pow(10,9))*Math.pow(X,-4)";
        }
        else if(63.313>x && x>10.870)
        {
          tau = "(0.2072*Math.pow(10,4))*Math.pow(X,-2)+(0.5147*Math.pow(10,6))*Math.pow(X,-3)-(0.1973*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(10.870>x && x>10.348)
        {
          tau = "(0.4696*Math.pow(10,6)*Math.pow(X,-3))-(0.1735*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(10.348>x && x>9.244)
        {
          tau = "(0.2954*Math.pow(10,6)*Math.pow(X,-3))-(0.7884*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(9.244>x && x>2.491)
        {
          tau = "(0.3152*Math.pow(10,4))*Math.pow(X,-2)+(0.5693*Math.pow(10,5))*Math.pow(X,-3)-(0.6051*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.491>x && x>2.263)
        {
          tau = "(0.6398*Math.pow(10,5))*Math.pow(X,-3)-(0.6231*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.263>x && x>2.023)
        {
          tau = "(0.5265*Math.pow(10,5))*Math.pow(X,-3)-(0.4152*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.023>x && x>1.639)
        {
          tau = "(0.4112*Math.pow(10,5))*Math.pow(X,-3)-(0.2732*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.639>x && x>1.588)
        {
          tau = "(0.3893*Math.pow(10,5))*Math.pow(X,-3)-(0.3686*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.8083*Math.pow(10,4))*Math.pow(X,-3)-(0.4909*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.530*Math.pow(10,-1)*X)*Math.pow((8.663*Math.pow(10,-2)+1.546*Math.pow(10,-2)*X+2.948*Math.pow(10,-3)*Math.pow(X,2)+6.442*Math.pow(10,-5)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((125.9*Math.pow(X,-1)+6.529+0.01861*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 72: //Hf
    {
        if(x>65.350)
        {
          tau = "(0.5124*Math.pow(10,7))*Math.pow(X,-3)-(0.1202*Math.pow(10,9))*Math.pow(X,-4)";
        }
        else if(65.350>x && x>11.270)
        {
          tau = "(0.2144*Math.pow(10,4))*Math.pow(X,-2)+(0.5374*Math.pow(10,6))*Math.pow(X,-3)-(0.2122*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(11.270>x && x>10.739)
        {
          tau = "(0.4976*Math.pow(10,6)*Math.pow(X,-3))-(0.1946*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(10.739>x && x>9.560)
        {
          tau = "(0.3042*Math.pow(10,6)*Math.pow(X,-3))-(0.7954*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(9.560>x && x>2.6)
        {
          tau = "(0.3188*Math.pow(10,4))*Math.pow(X,-2)+(0.6025*Math.pow(10,5))*Math.pow(X,-3)-(0.6630*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.6>x && x>2.365)
        {
          tau = "(0.6774*Math.pow(10,5))*Math.pow(X,-3)-(0.6845*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.365>x && x>2.107)
        {
          tau = "(0.5555*Math.pow(10,5))*Math.pow(X,-3)-(0.4507*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.107>x && x>1.716)
        {
          tau = "(0.4482*Math.pow(10,5))*Math.pow(X,-3)-(0.3245*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.716>x && x>1.661)
        {
          tau = "(0.4261*Math.pow(10,5))*Math.pow(X,-3)-(0.4287*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.8547*Math.pow(10,4))*Math.pow(X,-3)-(0.5228*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.510*Math.pow(10,-1)*X)*Math.pow((8.543*Math.pow(10,-2)+1.524*Math.pow(10,-2)*X+2.866*Math.pow(10,-3)*Math.pow(X,2)+6.275*Math.pow(10,-5)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((128.4*Math.pow(X,-1)+6.563+0.01876*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 73: //Ta
    {
        if(x>67.416)
        {
          tau = "(0.5366*Math.pow(10,7))*Math.pow(X,-3)-(0.1303*Math.pow(10,9))*Math.pow(X,-4)";
        }
        else if(67.416>x && x>11.681)
        {
          tau = "(0.2209*Math.pow(10,4))*Math.pow(X,-2)+(0.5658*Math.pow(10,6))*Math.pow(X,-3)-(0.2321*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(11.681>x && x>11.136)
        {
          tau = "(0.5262*Math.pow(10,6)*Math.pow(X,-3))-(0.2149*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(11.136>x && x>9.881)
        {
          tau = "(0.3165*Math.pow(10,6)*Math.pow(X,-3))-(0.8344*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(9.881>x && x>2.708)
        {
          tau = "(0.3243*Math.pow(10,4))*Math.pow(X,-2)+(0.6393*Math.pow(10,5))*Math.pow(X,-3)-(0.7252*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.708>x && x>2.468)
        {
          tau = "(0.7168*Math.pow(10,5))*Math.pow(X,-3)-(0.7459*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.468>x && x>2.194)
        {
          tau = "(0.6294*Math.pow(10,5))*Math.pow(X,-3)-(0.5814*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.194>x && x>1.793)
        {
          tau = "(0.4761*Math.pow(10,5))*Math.pow(X,-3)-(0.3533*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.793>x && x>1.735)
        {
          tau = "(0.5234*Math.pow(10,5))*Math.pow(X,-3)-(0.5955*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.2539*Math.pow(10,4))*Math.pow(X,-2)+(0.2405*Math.pow(10,4))*Math.pow(X,-3)-(0.1445*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.529*Math.pow(10,-1)*X)*Math.pow((8.421*Math.pow(10,-2)+1.531*Math.pow(10,-2)*X+2.825*Math.pow(10,-3)*Math.pow(X,2)+6.203*Math.pow(10,-5)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((130.3*Math.pow(X,-1)+6.552+0.01884*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 74: //W
    {
        if(x>69.525)
        {
          tau = "(0.5609*Math.pow(10,7))*Math.pow(X,-3)-(0.1409*Math.pow(10,9))*Math.pow(X,-4)";
        }
        else if(69.525>x && x>12.099)
        {
          tau = "(0.2293*Math.pow(10,4))*Math.pow(X,-2)+(0.5922*Math.pow(10,6))*Math.pow(X,-3)-(0.2501*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(12.099>x && x>11.544)
        {
          tau = "(0.5357*Math.pow(10,6)*Math.pow(X,-3))-(0.2132*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(11.544>x && x>10.206)
        {
          tau = "(0.3312*Math.pow(10,6)*Math.pow(X,-3))-(0.8942*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(10.206>x && x>2.819)
        {
          tau = "(0.3533*Math.pow(10,4))*Math.pow(X,-2)+(0.6433*Math.pow(10,5))*Math.pow(X,-3)-(0.6929*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.819>x && x>2.574)
        {
          tau = "(0.7600*Math.pow(10,5))*Math.pow(X,-3)-(0.8179*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.574>x && x>2.281)
        {
          tau = "(0.6324*Math.pow(10,5))*Math.pow(X,-3)-(0.5576*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.281>x && x>1.871)
        {
          tau = "(0.6520*Math.pow(10,5))*Math.pow(X,-3)-(0.7006*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.871>x && x>1.809)
        {
          tau = "(0.4760*Math.pow(10,5))*Math.pow(X,-3)-(0.5122*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.2594*Math.pow(10,4))*Math.pow(X,-2)+(0.2704*Math.pow(10,4))*Math.pow(X,-3)-(0.1624*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.571*Math.pow(10,-1)*X)*Math.pow((8.324*Math.pow(10,-2)+1.526*Math.pow(10,-2)*X+2.825*Math.pow(10,-3)*Math.pow(X,2)+6.277*Math.pow(10,-5)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((126.1*Math.pow(X,-1)+6.724+0.01805*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }


    }
}

double MainWindow::oslableniyeSecond(double x)
{
    QString tau;
    //QString tau1;
    //QString tau2;

    QString sigma;
    QString sigmaK;
    QString sigmaNK;

    QScriptEngine engine;

    QScriptValue tauValue;
    QScriptValue sigmaValue;
    QScriptValue sigmaKValue;
    QScriptValue sigmaNKValue;

    switch(ui->materialSecond->currentIndex())
    {
    case 0: //Air
        {
            if(x<3.203)
            {
                tau = "-(0.1991*Math.pow(10,-2))*X+(0.1169)*Math.pow(X,-1)-(0.1514*100)*Math.pow(X,-2)+(0.4759*10000)*Math.pow(X,-3)-(0.9060*1000)*Math.pow(X,-4)";
            }
            else
            {
               tau = "-(0.3514*Math.pow(10,2))*Math.pow(X,-2)+(0.5059*10000)*Math.pow(X,-3)-(0.1060*1000)*Math.pow(X,-4)+Math.pow((26.44*Math.pow(X,-1)+4.724+0.01809*X),-1)";
            }
            tau.replace("X", QString::number(x));
            tauValue = engine.evaluate(tau);
            weakening = tauValue.toNumber();
            return weakening;
            break;
        }
    case 1: //H
        {
            tau = "-(0.2645*Math.pow(10,-2))*X+(0.7086*0.1)*Math.pow(X,-1)-(0.7487)*Math.pow(X,-2)+(0.5575*10)*Math.pow(X,-3)-(-0.1936*10)*Math.pow(X,-4)";
            sigma = "0.4005*(1/1.008)*Math.pow((1+2*X/511),-2)*(1+2*X/511+0.3*Math.pow(2*X/511,2)-0.0625*Math.pow(2*X/511,3))";
            tau.replace("X", QString::number(x));
            sigma.replace("X", QString::number(x));
            tauValue = engine.evaluate(tau);
            sigmaValue = engine.evaluate(sigma);
            weakening = tauValue.toNumber()+sigmaValue.toNumber();
            return weakening;
            break;
        }
    case 2: //He
        {
            tau = "-(0.2154*Math.pow(10,-2))*X+(0.1473)*Math.pow(X,-1)-(0.3322*10)*Math.pow(X,-2)+(0.4893*100)*Math.pow(X,-3)-(-0.1576*100)*Math.pow(X,-4)";
            sigma = "0.4005*(1/4.003)*Math.pow(1+2*X/511,-2)*(1+2*X/511+0.3*Math.pow(2*X/511,2)-0.0625*Math.pow(2*X/511,3))";
            tau.replace("X", QString::number(x));
            sigma.replace("X", QString::number(x));
            tauValue = engine.evaluate(tau);
            sigmaValue = engine.evaluate(sigma);
            weakening = tauValue.toNumber()+sigmaValue.toNumber();
            return weakening;
            break;
        }
    case 3: //Li
        {
            tau = "-(0.3411*Math.pow(10,-2))*Math.pow(X,0)+(0.3088*Math.pow(10,0))*Math.pow(X,-1)-(0.1009*Math.pow(10,2))*Math.pow(X,-2)+(0.2076*Math.pow(10,3))*Math.pow(X,-3)-(-0.4091*Math.pow(10,2))*Math.pow(X,-4)";
            sigmaK = "(1+9.326*Math.pow(10,-2)*X)*Math.pow((1.781*Math.pow(10,0)+8.725*Math.pow(10,-1)*X+7.963*Math.pow(10,-2)*Math.pow(X,2)+8.225*Math.pow(10,-3)*Math.pow(X,3)),-1)";
            sigmaNK = "Math.pow((29.94*Math.pow(X,-1)+4.533+0.03637*X),-1)";
            tau.replace("X", QString::number(x));
            sigmaK.replace("X", QString::number(x));
            sigmaNK.replace("X", QString::number(x));
            tauValue = engine.evaluate(tau);
            sigmaKValue = engine.evaluate(sigmaK);
            sigmaNKValue = engine.evaluate(sigmaNK);
            weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
            return weakening;
            break;
        }
    case 4: //Be
    {
        tau = "-(0.3142*Math.pow(10,-2))*Math.pow(X,0)+(0.4216*Math.pow(10,0))*Math.pow(X,-1)-(0.2014*Math.pow(10,2))*Math.pow(X,-2)+(0.5918*Math.pow(10,3))*Math.pow(X,-3)-(-0.4857*Math.pow(10,2))*Math.pow(X,-4)";
        sigmaK = "(1-3.178*Math.pow(10,-2)*X)*Math.pow((1.267*Math.pow(10,0)+4.619*Math.pow(10,-1)*X+3.102*Math.pow(10,-2)*Math.pow(X,2)-1.493*Math.pow(10,-3)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((25.93*Math.pow(X,-1)+5.067+0.02420*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 5: //B
    {
        tau = "-(0.3267*Math.pow(10,-2))*X+(0.5682)*Math.pow(X,-1)-(0.3483*100)*Math.pow(X,-2)+(0.1326*10000)*Math.pow(X,-3)-(0.3243*100)*Math.pow(X,-4)";
        sigmaK = "(1+1.544*X)*Math.pow((1.010+1.561*X+6.978*0.1*Math.pow(X,2)+5.025*0.01*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((25.58*Math.pow(X,-1)+4.945+0.02191*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 6: //C
    {
        tau = "-(0.3172*Math.pow(10,-2))*X+(0.6921)*Math.pow(X,-1)-(0.5340*100)*Math.pow(X,-2)+(0.2610*10000)*Math.pow(X,-3)-(0.2941*1000)*Math.pow(X,-4)";
        sigmaK = "(1+1.108*X)*Math.pow((8.093*0.1+7.378*0.1*X+3.958*0.1*Math.pow(X,2)+2.532*0.01*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((25.91*Math.pow(X,-1)+4.644+0.01878*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 7: //N
    {
        tau = "-(0.1991*Math.pow(10,-2))*X+(0.6169)*Math.pow(X,-1)-(0.6514*100)*Math.pow(X,-2)+(0.4259*10000)*Math.pow(X,-3)-(0.8060*1000)*Math.pow(X,-4)";
        sigmaK = "(1+5.723*0.1*X)*Math.pow((7.260*0.1+2.910*0.1*X+1.824*0.1*Math.pow(X,2)+1.018*0.1*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((26.44*Math.pow(X,-1)+4.724+0.01809*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 8: //O
    {
        tau = "-(0.2663*Math.pow(10,-2))*X+(0.8397)*Math.pow(X,-1)-(0.9179*100)*Math.pow(X,-2)+(0.6634*10000)*Math.pow(X,-3)-(0.1906*10000)*Math.pow(X,-4)";
        sigmaK = "(1+3.537*0.1*X)*Math.pow((6.396*0.1+1.543*0.1*X+9.948*0.01*Math.pow(X,2)+4.981*0.001*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((29.88*Math.pow(X,-1)+4.656+0.01857*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 9: //F
    {
        tau = "-(0.3038*Math.pow(10,-2))*X+(0.9836)*Math.pow(X,-1)-(0.1128*1000)*Math.pow(X,-2)+(0.9171*10000)*Math.pow(X,-3)-(0.3414*10000)*Math.pow(X,-4)";
        sigmaK = "(1+2.437*0.1*X)*Math.pow((5.970*0.1+9.989*0.01*X+6.409*0.01*Math.pow(X,2)+2.930*0.001*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((33.40*Math.pow(X,-1)+4.961+0.01913*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 10: //Ne
    {
        tau = "-(0.1806*Math.pow(10,-2))*X+(0.7942)*Math.pow(X,-1)-(0.1218*1000)*Math.pow(X,-2)+(0.1307*100000)*Math.pow(X,-3)-(0.5600*10000)*Math.pow(X,-4)";
        sigmaK = "(1+1.820*0.1*X)*Math.pow((5.118*0.1+6.431*0.01*X+4.065*0.01*Math.pow(X,2)+1.715*0.001*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((37.30*Math.pow(X,-1)+4.629+0.01905*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 11: //Na
    {
        if(x>1.072)
        {
          tau = "-(0.3963*Math.pow(10,-2))*X+(0.1359*10)*Math.pow(X,-1)-(0.1720*1000)*Math.pow(X,-2)+(0.1766*100000)*Math.pow(X,-3)-(0.1019*100000)*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.8694*1000)*Math.pow(X,-3)-(0.2170*1000)*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.029*0.1*X)*Math.pow((4.721*0.1+6.884*0.01*X+2.528*0.01*Math.pow(X,2)+8.577*0.0001*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((38.80*Math.pow(X,-1)+4.901+0.01889*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 12: //Mg
    {
        if(x>1.305)
        {
          tau = "-(0.3447*Math.pow(10,-2))*X+(0.1309*Math.pow(10,1))*Math.pow(X,-1)-(0.1895*Math.pow(10,3))*Math.pow(X,-2)+(0.2327*Math.pow(10,5))*Math.pow(X,-3)-(0.1439*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1268*Math.pow(10,4))*Math.pow(X,-3)-(0.3474*Math.pow(10,3))*Math.pow(X,-4)";
        }
        sigmaK = "(1+8.484*Math.pow(10,-2)*X)*Math.pow((4.132*Math.pow(10,-1)+7.566*Math.pow(10,-2)*X+1.836*Math.pow(10,-2)*Math.pow(X,2)+6.034*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((39.81*Math.pow(X,-1)+4.746+0.01832*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 13: //Al
    {
        if(x>1.559)
        {
          tau = "-(0.3471*Math.pow(10,-2))*Math.pow(X,0)+(0.1384*Math.pow(10,1))*Math.pow(X,-1)-(0.2137*Math.pow(10,3))*Math.pow(X,-2)+(0.2950*Math.pow(10,5))*Math.pow(X,-3)-(0.2217*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1688*Math.pow(10,4)*Math.pow(X,-3))-(0.5046*Math.pow(10,3)*Math.pow(X,-4))";
        }
        sigmaK = "(1+8.580*Math.pow(10,-2)*X)*Math.pow((3.905*Math.pow(10,-1)+8.506*Math.pow(10,-2)*X+1.611*Math.pow(10,-2)*Math.pow(X,2)+5.524*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((42.58*Math.pow(X,-1)+4.873+0.01871*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 14: //Si
    {
        if(x>1.838)
        {
          tau = "-(0.2219*Math.pow(10,-2))*X+(0.1141*Math.pow(10,1))*Math.pow(X,-1)-(0.2202*Math.pow(10,3))*Math.pow(X,-2)+(0.3864*Math.pow(10,5))*Math.pow(X,-3)-(0.3319*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1640*Math.pow(10,3))*Math.pow(X,-2)+(0.1853*Math.pow(10,4))*Math.pow(X,-3)-(0.4500*Math.pow(10,3))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.970*X)*Math.pow((3.667*Math.pow(10,-1)+6.375*Math.pow(10,-1)*X+1.589*Math.pow(10,-1)*Math.pow(X,2)+1.114*Math.pow(10,-2)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((42.56*Math.pow(X,-1)+4.729+0.01796*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 15: //P
    {
        if(x>2.145)
        {
          tau = "(0.6962)*Math.pow(X,-1)-(0.2154*Math.pow(10,3))*Math.pow(X,-2)+(0.4590*Math.pow(10,5))*Math.pow(X,-3)-(0.4530*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1908*Math.pow(10,3))*Math.pow(X,-2)+(0.2308*Math.pow(10,4))*Math.pow(X,-3)-(0.5886*Math.pow(10,3))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.169*X)*Math.pow((3.420*Math.pow(10,-1)+6.676*Math.pow(10,-1)*X+1.740*Math.pow(10,-1)*Math.pow(X,2)+1.124*Math.pow(10,-2)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((45.17*Math.pow(X,-1)+4.889+0.01835*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 16: //S
    {
        if(x>2.472)
        {
          tau = "(0.7582)*Math.pow(X,-1)-(0.2447*Math.pow(10,3))*Math.pow(X,-2)+(0.5785*Math.pow(10,5))*Math.pow(X,-3)-(0.6419*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.2282*Math.pow(10,3))*Math.pow(X,-2)+(0.3007*Math.pow(10,4))*Math.pow(X,-3)-(0.8095*Math.pow(10,3))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.786*X)*Math.pow((3.095*Math.pow(10,-1)+4.495*Math.pow(10,-1)*X+1.360*Math.pow(10,-1)*Math.pow(X,2)+7.918*Math.pow(10,-2)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((41.69*Math.pow(X,-1)+4.900+0.01671*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 17: //Cl
    {
        if(x>2.822)
        {
          tau = "(0.7858)*Math.pow(X,-1)-(0.2734*Math.pow(10,3))*Math.pow(X,-2)+(0.7529*Math.pow(10,5))*Math.pow(X,-3)-(0.9287*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.2783*Math.pow(10,3))*Math.pow(X,-2)+(0.4027*Math.pow(10,4))*Math.pow(X,-3)-(0.1144*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.391*X)*Math.pow((3.181*Math.pow(10,-1)+3.659*Math.pow(10,-1)*X+1.077*Math.pow(10,-1)*Math.pow(X,2)+5.874*Math.pow(10,-2)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((44.68*Math.pow(X,-1)+5.115+0.01732*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 18: //Ar
    {
        if(x>3.206)
        {
          tau = "(0.7518)*Math.pow(X,-1)-(0.2633*Math.pow(10,3))*Math.pow(X,-2)+(0.7533*Math.pow(10,5))*Math.pow(X,-3)-(0.1050*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.2629*Math.pow(10,3))*Math.pow(X,-2)+(0.4167*Math.pow(10,4))*Math.pow(X,-3)-(0.1249*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.080*X)*Math.pow((3.078*Math.pow(10,-1)+2.799*Math.pow(10,-1)*X+8.688*Math.pow(10,-2)*Math.pow(X,2)+4.380*Math.pow(10,-3)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((49.09*Math.pow(X,-1)+5.452+0.01840*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 19: //K
    {
        if(x>3.607)
        {
          tau = "(0.6731)*Math.pow(X,-1)-(0.2717*Math.pow(10,3))*Math.pow(X,-2)+(0.9468*Math.pow(10,5))*Math.pow(X,-3)-(0.1384*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.3242*Math.pow(10,3))*Math.pow(X,-2)+(0.5459*Math.pow(10,4))*Math.pow(X,-3)-(0.1733*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+9.832*Math.pow(10,-1)*X)*Math.pow((2.733*Math.pow(10,-1)+2.506*Math.pow(10,-1)*X+6.843*Math.pow(10,-2)*Math.pow(X,2)+3.356*Math.pow(10,-3)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((46.28*Math.pow(X,-1)+5.080+0.01694*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 20: //Ca
    {
        if(x>4.038)
        {
          tau = "(0.7622)*Math.pow(X,-1)-(0.3106*Math.pow(10,3))*Math.pow(X,-2)+(0.1148*Math.pow(10,6))*Math.pow(X,-3)-(0.1902*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.3699*Math.pow(10,3))*Math.pow(X,-2)+(0.6774*Math.pow(10,4))*Math.pow(X,-3)-(0.2287*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.265*X)*Math.pow((2.507*Math.pow(10,-1)+2.991*Math.pow(10,-1)*X+8*Math.pow(10,-2)*Math.pow(X,2)+3.915*Math.pow(10,-3)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((45.51*Math.pow(X,-1)+4.977+0.01634*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 21: //Sc
    {
        if(x>4.492)
        {
          tau = "(0.7710)*Math.pow(X,-1)-(0.3377*Math.pow(10,3))*Math.pow(X,-2)+(0.1397*Math.pow(10,6))*Math.pow(X,-3)-(0.2524*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.3933*Math.pow(10,3))*Math.pow(X,-2)+(0.7473*Math.pow(10,4))*Math.pow(X,-3)-(0.2642*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+9.784*Math.pow(10,-1)*X)*Math.pow((2.502*Math.pow(10,-1)+2.318*Math.pow(10,-1)*X+6.513*Math.pow(10,-2)*Math.pow(X,2)+2.995*Math.pow(10,-3)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((47.76*Math.pow(X,-1)+5.397+0.01697*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 22: //Ti
    {
        if(x>4.966)
        {
          tau = "(0.6170)*Math.pow(X,-1)-(0.2987*Math.pow(10,3))*Math.pow(X,-2)+(0.1409*Math.pow(10,6))*Math.pow(X,-3)-(0.2757*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.4341*Math.pow(10,3))*Math.pow(X,-2)+(0.8580*Math.pow(10,4))*Math.pow(X,-3)-(0.3171*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+7.497*Math.pow(10,-1)*X)*Math.pow((2.462*Math.pow(10,-1)+1.731*Math.pow(10,-1)*X+4.960*Math.pow(10,-2)*Math.pow(X,2)+2.157*Math.pow(10,-3)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((50.03*Math.pow(X,-1)+5.490+0.01728*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 23: //V
    {
        if(x>5.465)
        {
          tau = "-(0.2018*Math.pow(10,3))*Math.pow(X,-2)+(0.1560*Math.pow(10,6))*Math.pow(X,-3)-(0.3252*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.4601*Math.pow(10,3))*Math.pow(X,-2)+(0.9829*Math.pow(10,4))*Math.pow(X,-3)-(0.3826*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.775*Math.pow(10,-1)*X)*Math.pow((2.367*Math.pow(10,-1)+1.307*Math.pow(10,-1)*X+3.807*Math.pow(10,-2)*Math.pow(X,2)+1.566*Math.pow(10,-3)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((52.62*Math.pow(X,-1)+5.580+0.01763*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 24: //Cr
    {
        if(x>5.989)
        {
          tau = "-(0.2213*Math.pow(10,3))*Math.pow(X,-2)+(0.1827*Math.pow(10,6))*Math.pow(X,-3)-(0.4226*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.5149*Math.pow(10,3))*Math.pow(X,-2)+(0.1154*Math.pow(10,5))*Math.pow(X,-3)-(0.4693*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+4.190*Math.pow(10,-1)*X)*Math.pow((2.292*Math.pow(10,-1)+8.642*Math.pow(10,-2)*X+2.697*Math.pow(10,-2)*Math.pow(X,2)+1.028*Math.pow(10,-3)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((54.13*Math.pow(X,-1)+5.430+0.01746*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 25: //Mn
    {
        if(x>6.539)
        {
          tau = "-(0.2245*Math.pow(10,3))*Math.pow(X,-2)+(0.2042*Math.pow(10,6))*Math.pow(X,-3)-(0.5055*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.5484*Math.pow(10,3))*Math.pow(X,-2)+(0.1312*Math.pow(10,5))*Math.pow(X,-3)-(0.5638*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+3.707*Math.pow(10,-1)*X)*Math.pow((2.167*Math.pow(10,-1)+8.029*Math.pow(10,-2)*X+2.324*Math.pow(10,-2)*Math.pow(X,2)+8.595*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((56.25*Math.pow(X,-1)+5.517+0.01764*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 26: //Fe
    {
        if(x>7.112)
        {
          tau = "-(0.2371*Math.pow(10,3))*Math.pow(X,-2)+(0.2359*Math.pow(10,6))*Math.pow(X,-3)-(0.6309*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.6176*Math.pow(10,3))*Math.pow(X,-2)+(0.1530*Math.pow(10,5))*Math.pow(X,-3)-(0.6905*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+3.075*Math.pow(10,-1)*X)*Math.pow((2.062*Math.pow(10,-1)+6.305*Math.pow(10,-2)*X+1.845*Math.pow(10,-2)*Math.pow(X,2)+6.535*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((56.92*Math.pow(X,-1)+5.382+0.01732*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 27: //Co
    {
        if(x>7.705)
        {
          tau = "-(0.2397*Math.pow(10,3))*Math.pow(X,-2)+(0.2612*Math.pow(10,6))*Math.pow(X,-3)-(0.7533*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.6579*Math.pow(10,3))*Math.pow(X,-2)+(0.1696*Math.pow(10,5))*Math.pow(X,-3)-(0.7918*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.591*Math.pow(10,-1)*X)*Math.pow((2.020*Math.pow(10,-1)+5.267*Math.pow(10,-2)*X+1.548*Math.pow(10,-2)*Math.pow(X,2)+5.231*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((59.76*Math.pow(X,-1)+5.460+0.01767*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 28: //Ni
    {
        if(x>8.332)
        {
          tau = "-(0.2531*Math.pow(10,3))*Math.pow(X,-2)+(0.3041*Math.pow(10,6))*Math.pow(X,-3)-(0.9428*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.7147*Math.pow(10,3))*Math.pow(X,-2)+(0.1993*Math.pow(10,5))*Math.pow(X,-3)-(0.9624*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.214*Math.pow(10,-1)*X)*Math.pow((1.870*Math.pow(10,-1)+4.225*Math.pow(10,-2)*X+1.244*Math.pow(10,-2)*Math.pow(X,2)+3.980*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((59.81*Math.pow(X,-1)+5.222+0.01711*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 29: //Cu
    {
        if(x>8.978)
        {
          tau = "-(0.2491*Math.pow(10,3))*Math.pow(X,-2)+(0.3252*Math.pow(10,6))*Math.pow(X,-3)-(0.1097*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(x<8.978 && x>1.096)
        {
          tau = "(0.7031*Math.pow(10,3))*Math.pow(X,-2)+(0.2165*Math.pow(10,5))*Math.pow(X,-3)-(0.1127*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1357*Math.pow(10,5))*Math.pow(X,-3)-(0.3001*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.902*Math.pow(10,-1)*X)*Math.pow((1.874*Math.pow(10,-1)+3.418*Math.pow(10,-2)*X+1.103*Math.pow(10,-2)*Math.pow(X,2)+3.367*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((65.59*Math.pow(X,-1)+5.428+0.01804*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 30: //Zn
    {
        if(x>9.658)
        {
          tau = "-(0.2426*Math.pow(10,3))*Math.pow(X,-2)+(0.3619*Math.pow(10,6))*Math.pow(X,-3)-(0.1289*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(9.658>x && x>1.193)
        {
          tau = "(0.7031*Math.pow(10,3))*Math.pow(X,-2)+(0.2165*Math.pow(10,5))*Math.pow(X,-3)-(0.1127*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.193>=x && x>=1.042)
        {
          tau = "(0.2889*Math.pow(10,5)*Math.pow(X,-3))-(0.1942*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(1.042>x && x>1.019)
        {
          tau = "(-0.1590*Math.pow(10,5)*Math.pow(X,-3))-(-0.2482*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.2572*Math.pow(10,4))*Math.pow(X,-3)-(0.1023*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.690*Math.pow(10,-1)*X)*Math.pow((1.786*Math.pow(10,-1)+3.177*Math.pow(10,-2)*X+9.452*Math.pow(10,-3)*Math.pow(X,2)+2.811*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((67.69*Math.pow(X,-1)+5.377+0.01810*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 31: //Ga
    {
        if(x>10.367)
        {
          tau = "-(0.2328*Math.pow(10,3))*Math.pow(X,-2)+(0.3881*Math.pow(10,6))*Math.pow(X,-3)-(0.1475*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(10.367>x && x>1.297)
        {
          tau = "(0.6933*Math.pow(10,3))*Math.pow(X,-2)+(0.2734*Math.pow(10,5))*Math.pow(X,-3)-(0.1657*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.297>x && x>1.142)
        {
          tau = "(0.3217*Math.pow(10,5)*Math.pow(X,-3))-(0.2363*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(1.142>x && x>1.115)
        {
          tau = "(-0.1315*Math.pow(10,5)*Math.pow(X,-3))-(-0.2503*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.2873*Math.pow(10,4))*Math.pow(X,-3)-(0.1181*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.458*Math.pow(10,-1)*X)*Math.pow((1.799*Math.pow(10,-1)+3.125*Math.pow(10,-2)*X+8.248*Math.pow(10,-3)*Math.pow(X,2)+2.369*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((65.9*Math.pow(X,-1)+5.722+0.01764*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 32: //Ge
    {
        if(x>11.103)
        {
          tau = "-(0.2231*Math.pow(10,3))*Math.pow(X,-2)+(0.4236*Math.pow(10,6))*Math.pow(X,-3)-(0.1708*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(11.103>x && x>1.414)
        {
          tau = "(0.7231*Math.pow(10,3))*Math.pow(X,-2)+(0.3038*Math.pow(10,5))*Math.pow(X,-3)-(0.1954*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.414>x && x>1.247)
        {
          tau = "(0.3416*Math.pow(10,5)*Math.pow(X,-3))-(0.2594*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(1.247>x && x>1.216)
        {
          tau = "(-0.1079*Math.pow(10,5)*Math.pow(X,-3))-(-0.2594*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.3266*Math.pow(10,4))*Math.pow(X,-3)-(0.1378*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.390*Math.pow(10,-1)*X)*Math.pow((1.756*Math.pow(10,-1)+3.140*Math.pow(10,-2)*X+7.573*Math.pow(10,-3)*Math.pow(X,2)+2.114*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((68.74*Math.pow(X,-1)+5.759+0.01789*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 33: //As
    {
        if(x>11.866)
        {
          tau = "-(0.2135*Math.pow(10,3))*Math.pow(X,-2)+(0.4644*Math.pow(10,6))*Math.pow(X,-3)-(0.1982*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(11.866>x && x>1.526)
        {
          tau = "(0.7490*Math.pow(10,3))*Math.pow(X,-2)+(0.3399*Math.pow(10,5))*Math.pow(X,-3)-(0.2339*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.526>x && x>1.358)
        {
          tau = "(0.3594*Math.pow(10,5)*Math.pow(X,-3))-(0.2762*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(1.358>x && x>1.323)
        {
          tau = "(-0.9541*Math.pow(10,4)*Math.pow(X,-3))-(-0.2846*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.3765*Math.pow(10,4))*Math.pow(X,-3)-(0.1650*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.465*Math.pow(10,-1)*X)*Math.pow((1.7*Math.pow(10,-1)+3.174*Math.pow(10,-2)*X+7.414*Math.pow(10,-3)*Math.pow(X,2)+2.197*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((70.84*Math.pow(X,-1)+5.755+0.01797*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 34: //Se
    {
        if(x>12.657)
        {
          tau = "-(0.1963*Math.pow(10,3))*Math.pow(X,-2)+(0.4978*Math.pow(10,6))*Math.pow(X,-3)-(0.2250*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(12.657>x && x>1.653)
        {
          tau = "(0.7661*Math.pow(10,3))*Math.pow(X,-2)+(0.3724*Math.pow(10,5))*Math.pow(X,-3)-(0.2740*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.653>x && x>1.476)
        {
          tau = "(0.3556*Math.pow(10,5)*Math.pow(X,-3))-(0.2627*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(1.476>x && x>1.435)
        {
          tau = "(-0.3092*Math.pow(10,4)*Math.pow(X,-3))-(-0.2333*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.4214*Math.pow(10,4))*Math.pow(X,-3)-(0.1902*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.665*Math.pow(10,-1)*X)*Math.pow((1.688*Math.pow(10,-1)+3.453*Math.pow(10,-2)*X+7.860*Math.pow(10,-3)*Math.pow(X,2)+2.404*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((74.43*Math.pow(X,-1)+5.881+0.01844*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 35: //Br
    {
        if(x>13.473)
        {
          tau = "-(0.1788*Math.pow(10,3))*Math.pow(X,-2)+(0.5528*Math.pow(10,6))*Math.pow(X,-3)-(0.2646*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(13.473>x && x>1.782)
        {
          tau = "(0.7869*Math.pow(10,3))*Math.pow(X,-2)+(0.4214*Math.pow(10,5))*Math.pow(X,-3)-(0.3277*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.782>x && x>1.596)
        {
          tau = "(0.3588*Math.pow(10,5)*Math.pow(X,-3))-(0.2389*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(1.596>x && x>1.549)
        {
          tau = "(-0.7092*Math.pow(10,4)*Math.pow(X,-3))-(-0.3479*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.4876*Math.pow(10,4))*Math.pow(X,-3)-(0.2258*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.040*Math.pow(10,-1)*X)*Math.pow((1.614*Math.pow(10,-1)+3.768*Math.pow(10,-2)*X+8.521*Math.pow(10,-3)*Math.pow(X,2)+2.778*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((74.84*Math.pow(X,-1)+5.785+0.01808*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 36: //Kr
    {
        if(x>14.325)
        {
          tau = "(0.5738*Math.pow(10,6))*Math.pow(X,-3)-(0.2753*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(14.325>x && x>1.921)
        {
          tau = "(0.8114*Math.pow(10,3))*Math.pow(X,-2)+(0.4562*Math.pow(10,5))*Math.pow(X,-3)-(0.3722*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.921>x && x>1.727)
        {
          tau = "(0.3388*Math.pow(10,5)*Math.pow(X,-3))-(0.1775*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(1.727>x && x>1.671)
        {
          tau = "(-0.3331*Math.pow(10,5)*Math.pow(X,-3))-(-0.8565*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.5401*Math.pow(10,4))*Math.pow(X,-3)-(0.2556*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.563*Math.pow(10,-1)*X)*Math.pow((1.605*Math.pow(10,-1)+4.357*Math.pow(10,-2)*X+9.829*Math.pow(10,-3)*Math.pow(X,2)+3.412*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((78.38*Math.pow(X,-1)+5.888+0.01853*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 37: //Rb
    {
        if(x>15.199)
        {
          tau = "(0.6321*Math.pow(10,6))*Math.pow(X,-3)-(0.3222*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(15.199>x && x>2.065)
        {
          tau = "(0.8061*Math.pow(10,3))*Math.pow(X,-2)+(0.5107*Math.pow(10,5))*Math.pow(X,-3)-(0.4410*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.065>x && x>1.863)
        {
          tau = "(0.4800*Math.pow(10,5)*Math.pow(X,-3))-(0.4165*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(1.863>x && x>1.804)
        {
          tau = "(0.2975*Math.pow(10,5)*Math.pow(X,-3))-(-0.2098*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.6124*Math.pow(10,4))*Math.pow(X,-3)-(0.2965*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.660*Math.pow(10,-1)*X)*Math.pow((1.539*Math.pow(10,-1)+3.449*Math.pow(10,-2)*X+6.750*Math.pow(10,-3)*Math.pow(X,2)+2.079*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((79.11*Math.pow(X,-1)+5.851+0.01835*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 38: //Sr
    {
        if(x>16.104)
        {
          tau = "(0.6915*Math.pow(10,6))*Math.pow(X,-3)-(0.3763*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(16.104>x && x>2.216)
        {
          tau = "(0.8448*Math.pow(10,3))*Math.pow(X,-2)+(0.5659*Math.pow(10,5))*Math.pow(X,-3)-(0.5234*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.216>x && x>2.006)
        {
          tau = "(0.5024*Math.pow(10,5)*Math.pow(X,-3))-(0.4284*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(2.006>x && x>1.939)
        {
          tau = "(0.2310*Math.pow(10,5)*Math.pow(X,-3))-(-0.4765*Math.pow(10,4)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1040*Math.pow(10,4))*Math.pow(X,-2)+(0.4001*Math.pow(10,4))*Math.pow(X,-3)-(0.1553*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.310*Math.pow(10,-1)*X)*Math.pow((1.488*Math.pow(10,-1)+3.124*Math.pow(10,-2)*X+5.429*Math.pow(10,-3)*Math.pow(X,2)+1.580*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((79.79*Math.pow(X,-1)+5.863+0.01821*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 39: //Y
    {
        if(x>17.038)
        {
          tau = "(0.7615*Math.pow(10,6))*Math.pow(X,-3)-(0.4412*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(17.038>x && x>2.372)
        {
          tau = "(0.8931*Math.pow(10,3))*Math.pow(X,-2)+(0.6297*Math.pow(10,5))*Math.pow(X,-3)-(0.6171*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.372>x && x>2.155)
        {
          tau = "(0.5458*Math.pow(10,5)*Math.pow(X,-3))-(0.4732*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(2.155>x && x>2.080)
        {
          tau = "(0.2326*Math.pow(10,5)*Math.pow(X,-3))-(-0.3694*Math.pow(10,4)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1148*Math.pow(10,4))*Math.pow(X,-2)+(0.4487*Math.pow(10,4))*Math.pow(X,-3)-(0.1768*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.794*Math.pow(10,-1)*X)*Math.pow((1.435*Math.pow(10,-1)+3.612*Math.pow(10,-2)*X+6.434*Math.pow(10,-3)*Math.pow(X,2)+2.056*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((80.83*Math.pow(X,-1)+5.8+0.01798*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 40: //Zr
    {
        if(x>17.997)
        {
          tau = "(0.8263*Math.pow(10,6))*Math.pow(X,-3)-(0.5088*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(17.997>x && x>2.531)
        {
          tau = "(0.9174*Math.pow(10,3))*Math.pow(X,-2)+(0.6902*Math.pow(10,5))*Math.pow(X,-3)-(0.7135*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.531>x && x>2.306)
        {
          tau = "(0.5865*Math.pow(10,5)*Math.pow(X,-3))-(0.5184*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(2.306>x && x>2.222)
        {
          tau = "(0.2511*Math.pow(10,5)*Math.pow(X,-3))-(-0.2008*Math.pow(10,4)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1245*Math.pow(10,4))*Math.pow(X,-2)+(0.4942*Math.pow(10,4))*Math.pow(X,-3)-(0.1983*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+3.040*Math.pow(10,-1)*X)*Math.pow((1.421*Math.pow(10,-1)+4.878*Math.pow(10,-2)*X+9.372*Math.pow(10,-3)*Math.pow(X,2)+3.381*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((81.87*Math.pow(X,-1)+5.790+0.01805*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 41: //Nb
    {
        if(x>18.985)
        {
          tau = "(0.9001*Math.pow(10,6))*Math.pow(X,-3)-(0.5848*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(18.985>x && x>2.697)
        {
          tau = "(0.9588*Math.pow(10,3))*Math.pow(X,-2)+(0.7570*Math.pow(10,5))*Math.pow(X,-3)-(0.8170*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.697>x && x>2.464)
        {
          tau = "(0.6456*Math.pow(10,5)*Math.pow(X,-3))-(0.6001*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(2.464>x && x>2.370)
        {
          tau = "(0.2869*Math.pow(10,5)*Math.pow(X,-3))-(-0.4479*Math.pow(10,4)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1320*Math.pow(10,4))*Math.pow(X,-2)+(0.5559*Math.pow(10,4))*Math.pow(X,-3)-(0.2278*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+4.815*Math.pow(10,-1)*X)*Math.pow((1.391*Math.pow(10,-1)+6.505*Math.pow(10,-2)*X+1.343*Math.pow(10,-2)*Math.pow(X,2)+5.147*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((77.98*Math.pow(X,-1)+5.895+0.01722*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 42: //Mo
    {
        if(x>19.999)
        {
          tau = "(0.9649*Math.pow(10,6))*Math.pow(X,-3)-(0.6635*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(19.999>x && x>2.865)
        {
          tau = "(0.9956*Math.pow(10,3))*Math.pow(X,-2)+(0.8166*Math.pow(10,5))*Math.pow(X,-3)-(0.9212*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.865>x && x>2.625)
        {
          tau = "(0.6946*Math.pow(10,5)*Math.pow(X,-3))-(0.6720*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(2.625>x && x>2.520)
        {
          tau = "(0.3382*Math.pow(10,5)*Math.pow(X,-3))-(0.5891*Math.pow(10,4)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1371*Math.pow(10,4))*Math.pow(X,-2)+(0.6170*Math.pow(10,4))*Math.pow(X,-3)-(0.2607*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.411*Math.pow(10,-1)*X)*Math.pow((1.345*Math.pow(10,-1)+7.060*Math.pow(10,-2)*X+1.463*Math.pow(10,-2)*Math.pow(X,2)+5.588*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((80.67*Math.pow(X,-1)+5.926+0.01748*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 43: //Tc
    {
        if(x>21.044)
        {
          tau = "(0.1034*Math.pow(10,7))*Math.pow(X,-3)-(0.7517*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(21.044>x && x>3.042)
        {
          tau = "(0.9760*Math.pow(10,3))*Math.pow(X,-2)+(0.8887*Math.pow(10,5))*Math.pow(X,-3)-(0.1071*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(3.042>x && x>2.793)
        {
          tau = "(0.7487*Math.pow(10,5)*Math.pow(X,-3))-(0.7579*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(2.793>x && x>2.676)
        {
          tau = "(0.4015*Math.pow(10,5)*Math.pow(X,-3))-(0.1605*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1452*Math.pow(10,4))*Math.pow(X,-2)+(0.6728*Math.pow(10,4))*Math.pow(X,-3)-(0.2892*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.743*Math.pow(10,-1)*X)*Math.pow((1.311*Math.pow(10,-1)+7.273*Math.pow(10,-2)*X+1.496*Math.pow(10,-2)*Math.pow(X,2)+5.695*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((81.49*Math.pow(X,-1)+5.927+0.01735*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 44: //Ru
    {
        if(x>22.117)
        {
          tau = "(0.1116*Math.pow(10,7))*Math.pow(X,-3)-(0.8554*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(22.117>x && x>3.224)
        {
          tau = "(0.1018*Math.pow(10,4))*Math.pow(X,-2)+(0.9670*Math.pow(10,5))*Math.pow(X,-3)-(0.1222*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(3.224>x && x>2.966)
        {
          tau = "(0.8078*Math.pow(10,5)*Math.pow(X,-3))-(0.8411*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(2.966>x && x>2.837)
        {
          tau = "(0.4695*Math.pow(10,5)*Math.pow(X,-3))-(0.2717*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1537*Math.pow(10,4))*Math.pow(X,-2)+(0.7433*Math.pow(10,4))*Math.pow(X,-3)-(0.3262*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.771*Math.pow(10,-1)*X)*Math.pow((1.297*Math.pow(10,-1)+7.016*Math.pow(10,-2)*X+1.493*Math.pow(10,-2)*Math.pow(X,2)+5.478*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((83.16*Math.pow(X,-1)+5.987+0.01743*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 45: //Rh
    {
        if(x>23.219)
        {
          tau = "(0.1205*Math.pow(10,7))*Math.pow(X,-3)-(0.9730*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(23.219>x && x>3.411)
        {
          tau = "(0.1050*Math.pow(10,4))*Math.pow(X,-2)+(0.1054*Math.pow(10,6))*Math.pow(X,-3)-(0.1398*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(3.411>x && x>3.146)
        {
          tau = "(0.9003*Math.pow(10,5)*Math.pow(X,-3))-(0.1030*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(3.146>x && x>3.003)
        {
          tau = "(0.5655*Math.pow(10,5)*Math.pow(X,-3))-(0.4734*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1612*Math.pow(10,4))*Math.pow(X,-2)+(0.8250*Math.pow(10,4))*Math.pow(X,-3)-(0.3703*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.623*Math.pow(10,-1)*X)*Math.pow((1.281*Math.pow(10,-1)+6.536*Math.pow(10,-2)*X+1.422*Math.pow(10,-2)*Math.pow(X,2)+5.096*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((90.02*Math.pow(X,-1)+5.803+0.01826*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 46: //Pd
    {
        if(x>24.350)
        {
          tau = "(0.1279*Math.pow(10,7))*Math.pow(X,-3)-(0.1088*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(24.350>x && x>3.604)
        {
          tau = "(0.1085*Math.pow(10,4))*Math.pow(X,-2)+(0.1123*Math.pow(10,6))*Math.pow(X,-3)-(0.1547*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(3.604>x && x>3.330)
        {
          tau = "(0.1024*Math.pow(10,6)*Math.pow(X,-3))-(0.1371*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(3.330>x && x>3.173)
        {
          tau = "(0.7496*Math.pow(10,5)*Math.pow(X,-3))-(0.1008*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1693*Math.pow(10,4))*Math.pow(X,-2)+(0.8872*Math.pow(10,4))*Math.pow(X,-3)-(0.4040*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.415*Math.pow(10,-1)*X)*Math.pow((1.272*Math.pow(10,-1)+6.081*Math.pow(10,-2)*X+1.374*Math.pow(10,-2)*Math.pow(X,2)+4.715*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((92.23*Math.pow(X,-1)+5.883+0.01841*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 47: //Ag
    {
        if(x>25.514)
        {
          tau = "(0.1384*Math.pow(10,7))*Math.pow(X,-3)-(0.1238*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(25.514>x && x>3.805)
        {
          tau = "(0.1135*Math.pow(10,4))*Math.pow(X,-2)+(0.1223*Math.pow(10,6))*Math.pow(X,-3)-(0.1765*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(3.805>x && x>3.523)
        {
          tau = "(0.1075*Math.pow(10,6)*Math.pow(X,-3))-(0.1415*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(3.523>x && x>3.351)
        {
          tau = "(0.7408*Math.pow(10,5)*Math.pow(X,-3))-(0.8834*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1818*Math.pow(10,4))*Math.pow(X,-2)+(0.9723*Math.pow(10,4))*Math.pow(X,-3)-(0.4517*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.082*Math.pow(10,-1)*X)*Math.pow((1.214*Math.pow(10,-1)+5.642*Math.pow(10,-2)*X+1.250*Math.pow(10,-2)*Math.pow(X,2)+4.238*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((92.27*Math.pow(X,-1)+5.857+0.01817*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 48: //Cd
    {
        if(x>26.711)
        {
          tau = "(0.1453*Math.pow(10,7))*Math.pow(X,-3)-(0.1366*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(26.711>x && x>4.018)
        {
          tau = "(0.1102*Math.pow(10,4))*Math.pow(X,-2)+(0.1305*Math.pow(10,6))*Math.pow(X,-3)-(0.2009*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(4.018>x && x>3.727)
        {
          tau = "(0.1142*Math.pow(10,6)*Math.pow(X,-3))-(0.1588*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(3.727>x && x>3.537)
        {
          tau = "(0.7830*Math.pow(10,5)*Math.pow(X,-3))-(0.9738*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1874*Math.pow(10,4))*Math.pow(X,-2)+(0.1040*Math.pow(10,5))*Math.pow(X,-3)-(0.4943*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+4.921*Math.pow(10,-1)*X)*Math.pow((1.230*Math.pow(10,-1)+5.532*Math.pow(10,-2)*X+1.208*Math.pow(10,-2)*Math.pow(X,2)+4.035*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((94.65*Math.pow(X,-1)+6.003+0.01841*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 49: //In
    {
        if(x>27.939)
        {
          tau = "(0.1551*Math.pow(10,7))*Math.pow(X,-3)-(0.1519*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(27.939>x && x>4.237)
        {
          tau = "(0.1130*Math.pow(10,4))*Math.pow(X,-2)+(0.1406*Math.pow(10,6))*Math.pow(X,-3)-(0.2257*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(4.237>x && x>3.938)
        {
          tau = "(0.1270*Math.pow(10,6)*Math.pow(X,-3))-(0.1957*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(3.938>x && x>3.730)
        {
          tau = "(0.1006*Math.pow(10,6)*Math.pow(X,-3))-(0.1736*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1980*Math.pow(10,4))*Math.pow(X,-2)+(0.1131*Math.pow(10,5))*Math.pow(X,-3)-(0.5499*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+4.741*Math.pow(10,-1)*X)*Math.pow((1.199*Math.pow(10,-1)+5.382*Math.pow(10,-2)*X+1.136*Math.pow(10,-2)*Math.pow(X,2)+3.757*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((95.15*Math.pow(X,-1)+6.032+0.01830*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 50: //Sn
    {
        if(x>29.200)
        {
          tau = "(0.1630*Math.pow(10,7))*Math.pow(X,-3)-(0.1643*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(29.200>x && x>4.464)
        {
          tau = "(0.1152*Math.pow(10,4))*Math.pow(X,-2)+(0.1492*Math.pow(10,6))*Math.pow(X,-3)-(0.2495*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(4.464>x && x>4.156)
        {
          tau = "(0.1428*Math.pow(10,6)*Math.pow(X,-3))-(0.2532*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(4.156>x && x>3.928)
        {
          tau = "(0.1097*Math.pow(10,6)*Math.pow(X,-3))-(0.2033*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.2051*Math.pow(10,4))*Math.pow(X,-2)+(0.1216*Math.pow(10,5))*Math.pow(X,-3)-(0.6037*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+4.821*Math.pow(10,-1)*X)*Math.pow((1.200*Math.pow(10,-1)+5.513*Math.pow(10,-2)*X+1.138*Math.pow(10,-2)*Math.pow(X,2)+3.740*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((96.63*Math.pow(X,-1)+6.142+0.01838*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 51: //Sb
    {
        if(x>30.491)
        {
          tau = "(0.1734*Math.pow(10,7))*Math.pow(X,-3)-(0.1865*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(30.491>x && x>4.698)
        {
          tau = "(0.1139*Math.pow(10,4))*Math.pow(X,-2)+(0.1604*Math.pow(10,6))*Math.pow(X,-3)-(0.2815*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(4.698>x && x>4.380)
        {
          tau = "(0.1527*Math.pow(10,6)*Math.pow(X,-3))-(0.2838*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(4.380>x && x>4.132)
        {
          tau = "(0.1276*Math.pow(10,6)*Math.pow(X,-3))-(0.2745*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.2136*Math.pow(10,4))*Math.pow(X,-2)+(0.1315*Math.pow(10,5))*Math.pow(X,-3)-(0.6735*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.118*Math.pow(10,-1)*X)*Math.pow((1.184*Math.pow(10,-1)+5.761*Math.pow(10,-2)*X+1.178*Math.pow(10,-2)*Math.pow(X,2)+3.863*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((97.76*Math.pow(X,-1)+6.193+0.01844*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 52: //Te
    {
        if(x>31.813)
        {
          tau = "(0.1797*Math.pow(10,7))*Math.pow(X,-3)-(0.2018*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(31.813>x && x>4.939)
        {
          tau = "(0.1161*Math.pow(10,4))*Math.pow(X,-2)+(0.1668*Math.pow(10,6))*Math.pow(X,-3)-(0.3041*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(4.939>x && x>4.612)
        {
          tau = "(0.1589*Math.pow(10,6)*Math.pow(X,-3))-(0.3071*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(4.612>x && x>4.341)
        {
          tau = "(0.1299*Math.pow(10,6)*Math.pow(X,-3))-(0.2849*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.2199*Math.pow(10,4))*Math.pow(X,-2)+(0.1381*Math.pow(10,5))*Math.pow(X,-3)-(0.7426*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.467*Math.pow(10,-1)*X)*Math.pow((1.186*Math.pow(10,-1)+6.083*Math.pow(10,-2)*X+1.258*Math.pow(10,-2)*Math.pow(X,2)+4.122*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((100.7*Math.pow(X,-1)+6.399+0.01879*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 53: //I
    {
        if(x>33.169)
        {
          tau = "(0.1960*Math.pow(10,7))*Math.pow(X,-3)-(0.2296*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(33.169>x && x>5.188)
        {
          tau = "(0.1219*Math.pow(10,4))*Math.pow(X,-2)+(0.1834*Math.pow(10,6))*Math.pow(X,-3)-(0.3505*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(5.188>x && x>4.852)
        {
          tau = "(0.1703*Math.pow(10,6)*Math.pow(X,-3))-(0.3295*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(4.852>x && x>4.557)
        {
          tau = "(0.1382*Math.pow(10,6)*Math.pow(X,-3))-(0.3053*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(4.557>x && x>1.072)
        {
          tau = "(0.2328*Math.pow(10,4))*Math.pow(X,-2)+(0.1552*Math.pow(10,5))*Math.pow(X,-3)-(0.8511*Math.pow(10,4))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1807*Math.pow(10,5))*Math.pow(X,-3)-(0.8981*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.921*Math.pow(10,-1)*X)*Math.pow((1.144*Math.pow(10,-1)+6.142*Math.pow(10,-2)*X+1.292*Math.pow(10,-2)*Math.pow(X,2)+4.238*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((98.33*Math.pow(X,-1)+6.276+0.01819*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 54: //Xe
    {
        if(x>34.564)
        {
          tau = "(0.2049*Math.pow(10,7))*Math.pow(X,-3)-(0.2505*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(34.564>x && x>5.452)
        {
          tau = "(0.1243*Math.pow(10,4))*Math.pow(X,-2)+(0.1933*Math.pow(10,6))*Math.pow(X,-3)-(0.3852*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(5.452>x && x>5.103)
        {
          tau = "(0.1787*Math.pow(10,6)*Math.pow(X,-3))-(0.3597*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.103>x && x>4.782)
        {
          tau = "(0.1715*Math.pow(10,6)*Math.pow(X,-3))-(0.3859*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(4.782>x && x>1.148)
        {
          tau = "(0.2349*Math.pow(10,4))*Math.pow(X,-2)+(0.1681*Math.pow(10,5))*Math.pow(X,-3)-(0.9660*Math.pow(10,4))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1898*Math.pow(10,5))*Math.pow(X,-3)-(0.9576*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+6.399*Math.pow(10,-1)*X)*Math.pow((1.131*Math.pow(10,-1)+6.545*Math.pow(10,-2)*X+1.382*Math.pow(10,-2)*Math.pow(X,2)+4.502*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((100.3*Math.pow(X,-1)+6.394+0.01839*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 55: //Cs
    {
        if(x>35.984)
        {
          tau = "(0.2187*Math.pow(10,7))*Math.pow(X,-3)-(0.2781*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(35.984>x && x>5.714)
        {
          tau = "(0.1285*Math.pow(10,4))*Math.pow(X,-2)+(0.2082*Math.pow(10,6))*Math.pow(X,-3)-(0.4344*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(5.714>x && x>5.359)
        {
          tau = "(0.1796*Math.pow(10,6)*Math.pow(X,-3))-(0.3318*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.359>x && x>5.011)
        {
          tau = "(0.1253*Math.pow(10,6)*Math.pow(X,-3))-(0.2100*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.011>x && x>1.217)
        {
          tau = "(0.2368*Math.pow(10,4))*Math.pow(X,-2)+(0.1871*Math.pow(10,5))*Math.pow(X,-3)-(0.1126*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.217>x && x>1.065)
        {
          tau = "(0.2142*Math.pow(10,5))*Math.pow(X,-3)-(0.1166*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1854*Math.pow(10,5))*Math.pow(X,-3)-(0.9182*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.245*Math.pow(10,-1)*X)*Math.pow((1.108*Math.pow(10,-1)+5.572*Math.pow(10,-2)*X+1.119*Math.pow(10,-2)*Math.pow(X,2)+3.552*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((99.67*Math.pow(X,-1)+6.385+0.01814*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 56: //Ba
    {
        if(x>37.440)
        {
          tau = "(0.2281*Math.pow(10,7))*Math.pow(X,-3)-(0.2993*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(37.440>x && x>5.988)
        {
          tau = "(0.1297*Math.pow(10,4))*Math.pow(X,-2)+(0.2196*Math.pow(10,6))*Math.pow(X,-3)-(0.4812*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(5.988>x && x>5.623)
        {
          tau = "(0.1930*Math.pow(10,6)*Math.pow(X,-3))-(0.3876*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.623>x && x>5.247)
        {
          tau = "(0.1411*Math.pow(10,6)*Math.pow(X,-3))-(0.2814*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.247>x && x>1.292)
        {
          tau = "(0.2397*Math.pow(10,4))*Math.pow(X,-2)+(0.2017*Math.pow(10,5))*Math.pow(X,-3)-(0.1272*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.292>x && x>1.136)
        {
          tau = "(0.2323*Math.pow(10,5))*Math.pow(X,-3)-(0.1334*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.136>x && x>1.062)
        {
          tau = "(0.1982*Math.pow(10,5))*Math.pow(X,-3)-(0.1019*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1647*Math.pow(10,5))*Math.pow(X,-3)-(0.7933*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+4.967*Math.pow(10,-1)*X)*Math.pow((1.101*Math.pow(10,-1)+5.450*Math.pow(10,-2)*X+1.055*Math.pow(10,-2)*Math.pow(X,2)+3.312*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((101.1*Math.pow(X,-1)+6.513+0.01826*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 57: //La
    {
        if(x>38.924)
        {
          tau = "(0.2430*Math.pow(10,7))*Math.pow(X,-3)-(0.3295*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(38.924>x && x>6.266)
        {
          tau = "(0.1337*Math.pow(10,4))*Math.pow(X,-2)+(0.2358*Math.pow(10,6))*Math.pow(X,-3)-(0.5375*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(6.266>x && x>5.890)
        {
          tau = "(0.2049*Math.pow(10,6)*Math.pow(X,-3))-(0.4180*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.890>x && x>5.482)
        {
          tau = "(0.1487*Math.pow(10,6)*Math.pow(X,-3))-(0.2982*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.482>x && x>1.361)
        {
          tau = "(0.2447*Math.pow(10,4))*Math.pow(X,-2)+(0.2229*Math.pow(10,5))*Math.pow(X,-3)-(0.1474*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.361>x && x>1.204)
        {
          tau = "(0.2551*Math.pow(10,5))*Math.pow(X,-3)-(0.1536*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.204>x && x>1.123)
        {
          tau = "(0.2165*Math.pow(10,5))*Math.pow(X,-3)-(0.1155*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1674*Math.pow(10,5))*Math.pow(X,-3)-(0.7655*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.818*Math.pow(10,-1)*X)*Math.pow((1.076*Math.pow(10,-1)+6.162*Math.pow(10,-2)*X+1.187*Math.pow(10,-2)*Math.pow(X,2)+3.746*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((101.4*Math.pow(X,-1)+6.499+0.01804*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 58: //Ce
    {
        if(x>40.443)
        {
          tau = "(0.2601*Math.pow(10,7))*Math.pow(X,-3)-(0.3722*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(40.443>x && x>6.548)
        {
          tau = "(0.1342*Math.pow(10,4))*Math.pow(X,-2)+(0.2540*Math.pow(10,6))*Math.pow(X,-3)-(0.6049*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(6.548>x && x>6.164)
        {
          tau = "(0.2237*Math.pow(10,6)*Math.pow(X,-3))-(0.4912*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(6.164>x && x>5.723)
        {
          tau = "(0.1597*Math.pow(10,6)*Math.pow(X,-3))-(0.3306*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.723>x && x>1.434)
        {
          tau = "(0.2476*Math.pow(10,4))*Math.pow(X,-2)+(0.2440*Math.pow(10,5))*Math.pow(X,-3)-(0.1673*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.434>x && x>1.272)
        {
          tau = "(0.2770*Math.pow(10,5))*Math.pow(X,-3)-(0.1729*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.272>x && x>1.185)
        {
          tau = "(0.2335*Math.pow(10,5))*Math.pow(X,-3)-(0.1278*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1737*Math.pow(10,5))*Math.pow(X,-3)-(0.7655*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.099*Math.pow(10,-1)*X)*Math.pow((1.048*Math.pow(10,-1)+5.311*Math.pow(10,-2)*X+1.027*Math.pow(10,-2)*Math.pow(X,2)+3.148*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((99.48*Math.pow(X,-1)+6.484+0.01768*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 59: //Pr
    {
        if(x>41.990)
        {
          tau = "(0.2783*Math.pow(10,7))*Math.pow(X,-3)-(0.4146*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(41.990>x && x>6.834)
        {
          tau = "(0.1414*Math.pow(10,4))*Math.pow(X,-2)+(0.2730*Math.pow(10,6))*Math.pow(X,-3)-(0.6747*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(6.834>x && x>6.440)
        {
          tau = "(0.2411*Math.pow(10,6)*Math.pow(X,-3))-(0.5525*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(6.440>x && x>5.964)
        {
          tau = "(0.1669*Math.pow(10,6)*Math.pow(X,-3))-(0.4316*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.964>x && x>1.511)
        {
          tau = "(0.2604*Math.pow(10,4))*Math.pow(X,-2)+(0.2636*Math.pow(10,5))*Math.pow(X,-3)-(0.1858*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.511>x && x>1.337)
        {
          tau = "(0.3020*Math.pow(10,5))*Math.pow(X,-3)-(0.1959*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.337>x && x>1.242)
        {
          tau = "(0.2562*Math.pow(10,5))*Math.pow(X,-3)-(0.1463*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1784*Math.pow(10,5))*Math.pow(X,-3)-(0.7266*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+3.925*Math.pow(10,-1)*X)*Math.pow((1.008*Math.pow(10,-1)+4.051*Math.pow(10,-2)*X+7.917*Math.pow(10,-3)*Math.pow(X,2)+2.310*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((100.1*Math.pow(X,-1)+6.405+0.01752*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 60: //Nd
    {
        if(x>43.568)
        {
          tau = "(0.2920*Math.pow(10,7))*Math.pow(X,-3)-(0.4524*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(43.568>x && x>7.126)
        {
          tau = "(0.1465*Math.pow(10,4))*Math.pow(X,-2)+(0.2880*Math.pow(10,6))*Math.pow(X,-3)-(0.7393*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(7.126>x && x>6.721)
        {
          tau = "(0.2525*Math.pow(10,6)*Math.pow(X,-3))-(0.5923*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(6.721>x && x>6.207)
        {
          tau = "(0.1750*Math.pow(10,6)*Math.pow(X,-3))-(0.3669*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(6.207>x && x>1.575)
        {
          tau = "(0.2581*Math.pow(10,4))*Math.pow(X,-2)+(0.2857*Math.pow(10,5))*Math.pow(X,-3)-(0.2112*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.575>x && x>1.402)
        {
          tau = "(0.3222*Math.pow(10,5))*Math.pow(X,-3)-(0.2164*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.402>x && x>1.297)
        {
          tau = "(0.2732*Math.pow(10,5))*Math.pow(X,-3)-(0.1612*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.2017*Math.pow(10,5))*Math.pow(X,-3)-(0.9345*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+3.511*Math.pow(10,-1)*X)*Math.pow((1.004*Math.pow(10,-1)+3.634*Math.pow(10,-2)*X+7.104*Math.pow(10,-3)*Math.pow(X,2)+2.012*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((101.9*Math.pow(X,-1)+6.457+0.01757*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 61: //Pm
    {
        if(x>45.184)
        {
          tau = "(0.3115*Math.pow(10,7))*Math.pow(X,-3)-(0.5004*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(45.184>x && x>7.427)
        {
          tau = "(0.1538*Math.pow(10,4))*Math.pow(X,-2)+(0.3092*Math.pow(10,6))*Math.pow(X,-3)-(0.8237*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(7.427>x && x>7.012)
        {
          tau = "(0.2740*Math.pow(10,6)*Math.pow(X,-3))-(0.6806*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(7.012>x && x>6.459)
        {
          tau = "(0.1852*Math.pow(10,6)*Math.pow(X,-3))-(0.3906*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(6.459>x && x>1.650)
        {
          tau = "(0.2665*Math.pow(10,4))*Math.pow(X,-2)+(0.3111*Math.pow(10,5))*Math.pow(X,-3)-(0.2388*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.650>x && x>1.471)
        {
          tau = "(0.3519*Math.pow(10,5))*Math.pow(X,-3)-(0.2471*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.471>x && x>1.356)
        {
          tau = "(0.2907*Math.pow(10,5))*Math.pow(X,-3)-(0.1728*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.356>x && x>1.051)
        {
          tau = "(0.2165*Math.pow(10,5))*Math.pow(X,-3)-(0.1022*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.051>x && x>1.026)
        {
          tau = "(0.1631*Math.pow(10,5))*Math.pow(X,-3)-(0.8906*Math.pow(10,4))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.4450*Math.pow(10,4))*Math.pow(X,-3)-(0.2405*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+3.211*Math.pow(10,-1)*X)*Math.pow((9.763*Math.pow(10,-2)+3.251*Math.pow(10,-2)*X+6.383*Math.pow(10,-3)*Math.pow(X,2)+1.763*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((105.4*Math.pow(X,-1)+6.380+0.01743*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 62: //Sm
    {
        if(x>46.834)
        {
          tau = "(0.3211*Math.pow(10,7))*Math.pow(X,-3)-(0.5229*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(46.834>x && x>7.736)
        {
          tau = "(0.1577*Math.pow(10,4))*Math.pow(X,-2)+(0.3207*Math.pow(10,6))*Math.pow(X,-3)-(0.8830*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(7.736>x && x>7.311)
        {
          tau = "(0.2858*Math.pow(10,6)*Math.pow(X,-3))-(0.7415*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(7.311>x && x>6.716)
        {
          tau = "(0.1911*Math.pow(10,6)*Math.pow(X,-3))-(0.4109*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(6.716>x && x>1.722)
        {
          tau = "(0.2669*Math.pow(10,4))*Math.pow(X,-2)+(0.3281*Math.pow(10,5))*Math.pow(X,-3)-(0.2613*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.722>x && x>1.540)
        {
          tau = "(0.3683*Math.pow(10,5))*Math.pow(X,-3)-(0.2662*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.540>x && x>1.419)
        {
          tau = "(0.3069*Math.pow(10,5))*Math.pow(X,-3)-(0.1892*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.419>x && x>1.106)
        {
          tau = "(0.2294*Math.pow(10,5))*Math.pow(X,-3)-(0.1111*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.106>x && x>1.080)
        {
          tau = "(0.2482*Math.pow(10,5))*Math.pow(X,-3)-(0.1799*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.4747*Math.pow(10,4))*Math.pow(X,-3)-(0.2650*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.933*Math.pow(10,-1)*X)*Math.pow((9.671*Math.pow(10,-2)+3.015*Math.pow(10,-2)*X+5.914*Math.pow(10,-3)*Math.pow(X,2)+1.593*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((106.4*Math.pow(X,-1)+6.500+0.01785*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 63: //Eu
    {
        if(x>48.519)
        {
          tau = "(0.3395*Math.pow(10,7))*Math.pow(X,-3)-(0.5788*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(48.519>x && x>8.052)
        {
          tau = "(0.1624*Math.pow(10,4))*Math.pow(X,-2)+(0.3418*Math.pow(10,6))*Math.pow(X,-3)-(0.9797*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(8.052>x && x>7.617)
        {
          tau = "(0.3033*Math.pow(10,6)*Math.pow(X,-3))-(0.8102*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(7.617>x && x>6.976)
        {
          tau = "(0.2032*Math.pow(10,6)*Math.pow(X,-3))-(0.4513*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(6.976>x && x>1.800)
        {
          tau = "(0.2738*Math.pow(10,4))*Math.pow(X,-2)+(0.3541*Math.pow(10,5))*Math.pow(X,-3)-(0.2922*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.800>x && x>1.613)
        {
          tau = "(0.3963*Math.pow(10,5))*Math.pow(X,-3)-(0.2965*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.613>x && x>1.480)
        {
          tau = "(0.3349*Math.pow(10,5))*Math.pow(X,-3)-(0.2175*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.480>x && x>1.160)
        {
          tau = "(0.2465*Math.pow(10,5))*Math.pow(X,-3)-(0.1228*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.160>x && x>1.130)
        {
          tau = "(0.2457*Math.pow(10,5))*Math.pow(X,-3)-(0.1767*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.5084*Math.pow(10,4))*Math.pow(X,-3)-(0.2878*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.696*Math.pow(10,-1)*X)*Math.pow((9.526*Math.pow(10,-2)+2.778*Math.pow(10,-2)*X+5.372*Math.pow(10,-3)*Math.pow(X,2)+1.403*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((107.6*Math.pow(X,-1)+6.459+0.01777*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 64: //Gd
    {
        if(x>50.239)
        {
          tau = "(0.3516*Math.pow(10,7))*Math.pow(X,-3)-(0.6283*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(50.239>x && x>8.375)
        {
          tau = "(0.1604*Math.pow(10,4))*Math.pow(X,-2)+(0.3565*Math.pow(10,6))*Math.pow(X,-3)-(0.1065*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(8.375>x && x>7.930)
        {
          tau = "(0.3168*Math.pow(10,6)*Math.pow(X,-3))-(0.8859*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(7.930>x && x>7.242)
        {
          tau = "(0.2087*Math.pow(10,6)*Math.pow(X,-3))-(0.4667*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(7.242>x && x>1.880)
        {
          tau = "(0.2739*Math.pow(10,4))*Math.pow(X,-2)+(0.3729*Math.pow(10,5))*Math.pow(X,-3)-(0.3185*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.880>x && x>1.688)
        {
          tau = "(0.4162*Math.pow(10,5))*Math.pow(X,-3)-(0.3223*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.688>x && x>1.544)
        {
          tau = "(0.3488*Math.pow(10,5))*Math.pow(X,-3)-(0.2303*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.544>x && x>1.217)
        {
          tau = "(0.2554*Math.pow(10,5))*Math.pow(X,-3)-(0.1278*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.217>x && x>1.185)
        {
          tau = "(0.2413*Math.pow(10,5))*Math.pow(X,-3)-(0.1727*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.5397*Math.pow(10,4))*Math.pow(X,-3)-(0.3116*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.652*Math.pow(10,-1)*X)*Math.pow((9.606*Math.pow(10,-2)+2.706*Math.pow(10,-2)*X+5.275*Math.pow(10,-3)*Math.pow(X,2)+1.377*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((110.9*Math.pow(X,-1)+6.581+0.01813*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 65: //Tb
    {
        if(x>51.995)
        {
          tau = "(0.3722*Math.pow(10,7))*Math.pow(X,-3)-(0.6917*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(51.995>x && x>8.708)
        {
          tau = "(0.1689*Math.pow(10,4))*Math.pow(X,-2)+(0.3777*Math.pow(10,6))*Math.pow(X,-3)-(0.1166*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(8.708>x && x>8.252)
        {
          tau = "(0.3376*Math.pow(10,6)*Math.pow(X,-3))-(0.9837*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(8.252>x && x>7.514)
        {
          tau = "(0.2228*Math.pow(10,6)*Math.pow(X,-3))-(0.5205*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(7.514>x && x>1.967)
        {
          tau = "(0.2817*Math.pow(10,4))*Math.pow(X,-2)+(0.3977*Math.pow(10,5))*Math.pow(X,-3)-(0.3486*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.967>x && x>1.767)
        {
          tau = "(0.4480*Math.pow(10,5))*Math.pow(X,-3)-(0.3608*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.767>x && x>1.611)
        {
          tau = "(0.3706*Math.pow(10,5))*Math.pow(X,-3)-(0.2495*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.611>x && x>1.275)
        {
          tau = "(0.2716*Math.pow(10,5))*Math.pow(X,-3)-(0.1378*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.275>x && x>1.241)
        {
          tau = "(0.3316*Math.pow(10,5))*Math.pow(X,-3)-(0.2830*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.5667*Math.pow(10,4))*Math.pow(X,-3)-(0.3281*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.296*Math.pow(10,-1)*X)*Math.pow((9.451*Math.pow(10,-2)+2.306*Math.pow(10,-2)*X+4.584*Math.pow(10,-3)*Math.pow(X,2)+1.147*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((111.8*Math.pow(X,-1)+6.552+0.01804*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 66: //Dy
    {
        if(x>53.788)
        {
          tau = "(0.3884*Math.pow(10,7))*Math.pow(X,-3)-(0.7467*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(53.788>x && x>9.045)
        {
          tau = "(0.1751*Math.pow(10,4))*Math.pow(X,-2)+(0.3955*Math.pow(10,6))*Math.pow(X,-3)-(0.1264*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(9.045>x && x>8.580)
        {
          tau = "(0.3553*Math.pow(10,6)*Math.pow(X,-3))-(0.1079*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(8.580>x && x>7.790)
        {
          tau = "(0.2253*Math.pow(10,6)*Math.pow(X,-3))-(0.4925*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(7.790>x && x>2.046)
        {
          tau = "(0.2845*Math.pow(10,4))*Math.pow(X,-2)+(0.4216*Math.pow(10,5))*Math.pow(X,-3)-(0.3822*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.046>x && x>1.841)
        {
          tau = "(0.4729*Math.pow(10,5))*Math.pow(X,-3)-(0.3925*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.841>x && x>1.675)
        {
          tau = "(0.3926*Math.pow(10,5))*Math.pow(X,-3)-(0.2725*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.675>x && x>1.333)
        {
          tau = "(0.2916*Math.pow(10,5))*Math.pow(X,-3)-(0.1554*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.333>x && x>1.294)
        {
          tau = "(0.3557*Math.pow(10,5))*Math.pow(X,-3)-(0.3166*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.5996*Math.pow(10,4))*Math.pow(X,-3)-(0.3512*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.124*Math.pow(10,-1)*X)*Math.pow((9.341*Math.pow(10,-2)+2.171*Math.pow(10,-2)*X+4.244*Math.pow(10,-3)*Math.pow(X,2)+1.030*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((114.9*Math.pow(X,-1)+6.580+0.01825*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 67: //Ho
    {
        if(x>55.617)
        {
          tau = "(0.4073*Math.pow(10,7))*Math.pow(X,-3)-(0.8046*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(55.617>x && x>9.394)
        {
          tau = "(0.1829*Math.pow(10,4))*Math.pow(X,-2)+(0.4165*Math.pow(10,6))*Math.pow(X,-3)-(0.1375*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(9.394>x && x>8.917)
        {
          tau = "(0.3764*Math.pow(10,6)*Math.pow(X,-3))-(0.1189*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(8.917>x && x>8.071)
        {
          tau = "(0.2431*Math.pow(10,6)*Math.pow(X,-3))-(0.5884*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(8.071>x && x>2.128)
        {
          tau = "(0.2850*Math.pow(10,4))*Math.pow(X,-2)+(0.4527*Math.pow(10,5))*Math.pow(X,-3)-(0.4260*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.128>x && x>1.923)
        {
          tau = "(0.5025*Math.pow(10,5))*Math.pow(X,-3)-(0.4307*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.923>x && x>1.741)
        {
          tau = "(0.4157*Math.pow(10,5))*Math.pow(X,-3)-(0.2945*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.741>x && x>1.391)
        {
          tau = "(0.3164*Math.pow(10,5))*Math.pow(X,-3)-(0.1788*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.391>x && x>1.351)
        {
          tau = "(0.3209*Math.pow(10,5))*Math.pow(X,-3)-(0.2708*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.6385*Math.pow(10,4))*Math.pow(X,-3)-(0.3779*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.069*Math.pow(10,-1)*X)*Math.pow((9.129*Math.pow(10,-2)+2.144*Math.pow(10,-2)*X+4.057*Math.pow(10,-3)*Math.pow(X,2)+9.736*Math.pow(10,-5)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((117*Math.pow(X,-1)+6.569+0.01831*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 68: //Er
    {
        if(x>57.185)
        {
          tau = "(0.4267*Math.pow(10,7))*Math.pow(X,-3)-(0.8622*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(57.185>x && x>9.751)
        {
          tau = "(0.1910*Math.pow(10,4))*Math.pow(X,-2)+(0.4384*Math.pow(10,6))*Math.pow(X,-3)-(0.1492*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(9.751>x && x>9.264)
        {
          tau = "(0.3982*Math.pow(10,6)*Math.pow(X,-3))-(0.1307*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(9.264>x && x>8.357)
        {
          tau = "(0.2558*Math.pow(10,6)*Math.pow(X,-3))-(0.6354*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(8.357>x && x>2.206)
        {
          tau = "(0.2931*Math.pow(10,4))*Math.pow(X,-2)+(0.4801*Math.pow(10,5))*Math.pow(X,-3)-(0.4657*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.206>x && x>2.005)
        {
          tau = "(0.5830*Math.pow(10,5))*Math.pow(X,-3)-(0.4791*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.005>x && x>1.811)
        {
          tau = "(0.4770*Math.pow(10,5))*Math.pow(X,-3)-(0.3852*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.811>x && x>1.453)
        {
          tau = "(0.3413*Math.pow(10,5))*Math.pow(X,-3)-(0.2030*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.453>x && x>1.409)
        {
          tau = "(0.3585*Math.pow(10,5))*Math.pow(X,-3)-(0.3235*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.6797*Math.pow(10,4))*Math.pow(X,-3)-(0.4059*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.858*Math.pow(10,-1)*X)*Math.pow((9.058*Math.pow(10,-2)+1.863*Math.pow(10,-2)*X+3.656*Math.pow(10,-3)*Math.pow(X,2)+8.480*Math.pow(10,-5)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((118.9*Math.pow(X,-1)+6.556+0.01835*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 69: //Tm
    {
        if(x>59.389)
        {
          tau = "(0.4493*Math.pow(10,7))*Math.pow(X,-3)-(0.9392*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(59.389>x && x>10.115)
        {
          tau = "(0.1980*Math.pow(10,4))*Math.pow(X,-2)+(0.4654*Math.pow(10,6))*Math.pow(X,-3)-(0.1653*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(10.115>x && x>9.616)
        {
          tau = "(0.4232*Math.pow(10,6)*Math.pow(X,-3))-(0.1447*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(9.616>x && x>8.648)
        {
          tau = "(0.2705*Math.pow(10,6)*Math.pow(X,-3))-(0.6928*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(8.648>x && x>2.306)
        {
          tau = "(0.3024*Math.pow(10,4))*Math.pow(X,-2)+(0.5105*Math.pow(10,5))*Math.pow(X,-3)-(0.5103*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.306>x && x>2.089)
        {
          tau = "(0.5721*Math.pow(10,5))*Math.pow(X,-3)-(0.5235*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.089>x && x>1.884)
        {
          tau = "(0.4704*Math.pow(10,5))*Math.pow(X,-3)-(0.3516*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.884>x && x>1.514)
        {
          tau = "(0.3588*Math.pow(10,5))*Math.pow(X,-3)-(0.2150*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.514>x && x>1.467)
        {
          tau = "(0.4367*Math.pow(10,5))*Math.pow(X,-3)-(0.4388*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.7237*Math.pow(10,4))*Math.pow(X,-3)-(0.4349*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.732*Math.pow(10,-1)*X)*Math.pow((8.885*Math.pow(10,-2)+1.718*Math.pow(10,-2)*X+3.374*Math.pow(10,-3)*Math.pow(X,2)+7.638*Math.pow(10,-5)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((120.9*Math.pow(X,-1)+6.504+0.01839*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 70: //Yb
    {
        if(x>61.332)
        {
          tau = "(0.4675*Math.pow(10,7))*Math.pow(X,-3)-(0.1022*Math.pow(10,9))*Math.pow(X,-4)";
        }
        else if(61.332>x && x>10.486)
        {
          tau = "(0.1995*Math.pow(10,4))*Math.pow(X,-2)+(0.4866*Math.pow(10,6))*Math.pow(X,-3)-(0.1796*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(10.486>x && x>9.978)
        {
          tau = "(0.4552*Math.pow(10,6)*Math.pow(X,-3))-(0.1700*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(9.978>x && x>8.943)
        {
          tau = "(0.2817*Math.pow(10,6)*Math.pow(X,-3))-(0.7422*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(8.943>x && x>2.398)
        {
          tau = "(0.3077*Math.pow(10,4))*Math.pow(X,-2)+(0.5341*Math.pow(10,5))*Math.pow(X,-3)-(0.5486*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.398>x && x>2.173)
        {
          tau = "(0.6041*Math.pow(10,5))*Math.pow(X,-3)-(0.5743*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.173>x && x>1.949)
        {
          tau = "(0.4972*Math.pow(10,5))*Math.pow(X,-3)-(0.3864*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.949>x && x>1.576)
        {
          tau = "(0.3790*Math.pow(10,5))*Math.pow(X,-3)-(0.2362*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.576>x && x>1.527)
        {
          tau = "(0.4881*Math.pow(10,5))*Math.pow(X,-3)-(0.5234*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.7585*Math.pow(10,4))*Math.pow(X,-3)-(0.4579*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.621*Math.pow(10,-1)*X)*Math.pow((8.842*Math.pow(10,-2)+1.614*Math.pow(10,-2)*X+3.170*Math.pow(10,-3)*Math.pow(X,2)+7.012*Math.pow(10,-5)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((124.5*Math.pow(X,-1)+6.552+0.01864*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 71: //Lu
    {
        if(x>63.313)
        {
          tau = "(0.4915*Math.pow(10,7))*Math.pow(X,-3)-(0.1111*Math.pow(10,9))*Math.pow(X,-4)";
        }
        else if(63.313>x && x>10.870)
        {
          tau = "(0.2072*Math.pow(10,4))*Math.pow(X,-2)+(0.5147*Math.pow(10,6))*Math.pow(X,-3)-(0.1973*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(10.870>x && x>10.348)
        {
          tau = "(0.4696*Math.pow(10,6)*Math.pow(X,-3))-(0.1735*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(10.348>x && x>9.244)
        {
          tau = "(0.2954*Math.pow(10,6)*Math.pow(X,-3))-(0.7884*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(9.244>x && x>2.491)
        {
          tau = "(0.3152*Math.pow(10,4))*Math.pow(X,-2)+(0.5693*Math.pow(10,5))*Math.pow(X,-3)-(0.6051*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.491>x && x>2.263)
        {
          tau = "(0.6398*Math.pow(10,5))*Math.pow(X,-3)-(0.6231*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.263>x && x>2.023)
        {
          tau = "(0.5265*Math.pow(10,5))*Math.pow(X,-3)-(0.4152*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.023>x && x>1.639)
        {
          tau = "(0.4112*Math.pow(10,5))*Math.pow(X,-3)-(0.2732*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.639>x && x>1.588)
        {
          tau = "(0.3893*Math.pow(10,5))*Math.pow(X,-3)-(0.3686*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.8083*Math.pow(10,4))*Math.pow(X,-3)-(0.4909*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.530*Math.pow(10,-1)*X)*Math.pow((8.663*Math.pow(10,-2)+1.546*Math.pow(10,-2)*X+2.948*Math.pow(10,-3)*Math.pow(X,2)+6.442*Math.pow(10,-5)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((125.9*Math.pow(X,-1)+6.529+0.01861*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 72: //Hf
    {
        if(x>65.350)
        {
          tau = "(0.5124*Math.pow(10,7))*Math.pow(X,-3)-(0.1202*Math.pow(10,9))*Math.pow(X,-4)";
        }
        else if(65.350>x && x>11.270)
        {
          tau = "(0.2144*Math.pow(10,4))*Math.pow(X,-2)+(0.5374*Math.pow(10,6))*Math.pow(X,-3)-(0.2122*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(11.270>x && x>10.739)
        {
          tau = "(0.4976*Math.pow(10,6)*Math.pow(X,-3))-(0.1946*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(10.739>x && x>9.560)
        {
          tau = "(0.3042*Math.pow(10,6)*Math.pow(X,-3))-(0.7954*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(9.560>x && x>2.6)
        {
          tau = "(0.3188*Math.pow(10,4))*Math.pow(X,-2)+(0.6025*Math.pow(10,5))*Math.pow(X,-3)-(0.6630*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.6>x && x>2.365)
        {
          tau = "(0.6774*Math.pow(10,5))*Math.pow(X,-3)-(0.6845*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.365>x && x>2.107)
        {
          tau = "(0.5555*Math.pow(10,5))*Math.pow(X,-3)-(0.4507*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.107>x && x>1.716)
        {
          tau = "(0.4482*Math.pow(10,5))*Math.pow(X,-3)-(0.3245*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.716>x && x>1.661)
        {
          tau = "(0.4261*Math.pow(10,5))*Math.pow(X,-3)-(0.4287*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.8547*Math.pow(10,4))*Math.pow(X,-3)-(0.5228*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.510*Math.pow(10,-1)*X)*Math.pow((8.543*Math.pow(10,-2)+1.524*Math.pow(10,-2)*X+2.866*Math.pow(10,-3)*Math.pow(X,2)+6.275*Math.pow(10,-5)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((128.4*Math.pow(X,-1)+6.563+0.01876*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 73: //Ta
    {
        if(x>67.416)
        {
          tau = "(0.5366*Math.pow(10,7))*Math.pow(X,-3)-(0.1303*Math.pow(10,9))*Math.pow(X,-4)";
        }
        else if(67.416>x && x>11.681)
        {
          tau = "(0.2209*Math.pow(10,4))*Math.pow(X,-2)+(0.5658*Math.pow(10,6))*Math.pow(X,-3)-(0.2321*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(11.681>x && x>11.136)
        {
          tau = "(0.5262*Math.pow(10,6)*Math.pow(X,-3))-(0.2149*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(11.136>x && x>9.881)
        {
          tau = "(0.3165*Math.pow(10,6)*Math.pow(X,-3))-(0.8344*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(9.881>x && x>2.708)
        {
          tau = "(0.3243*Math.pow(10,4))*Math.pow(X,-2)+(0.6393*Math.pow(10,5))*Math.pow(X,-3)-(0.7252*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.708>x && x>2.468)
        {
          tau = "(0.7168*Math.pow(10,5))*Math.pow(X,-3)-(0.7459*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.468>x && x>2.194)
        {
          tau = "(0.6294*Math.pow(10,5))*Math.pow(X,-3)-(0.5814*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.194>x && x>1.793)
        {
          tau = "(0.4761*Math.pow(10,5))*Math.pow(X,-3)-(0.3533*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.793>x && x>1.735)
        {
          tau = "(0.5234*Math.pow(10,5))*Math.pow(X,-3)-(0.5955*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.2539*Math.pow(10,4))*Math.pow(X,-2)+(0.2405*Math.pow(10,4))*Math.pow(X,-3)-(0.1445*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.529*Math.pow(10,-1)*X)*Math.pow((8.421*Math.pow(10,-2)+1.531*Math.pow(10,-2)*X+2.825*Math.pow(10,-3)*Math.pow(X,2)+6.203*Math.pow(10,-5)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((130.3*Math.pow(X,-1)+6.552+0.01884*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 74: //W
    {
        if(x>69.525)
        {
          tau = "(0.5609*Math.pow(10,7))*Math.pow(X,-3)-(0.1409*Math.pow(10,9))*Math.pow(X,-4)";
        }
        else if(69.525>x && x>12.099)
        {
          tau = "(0.2293*Math.pow(10,4))*Math.pow(X,-2)+(0.5922*Math.pow(10,6))*Math.pow(X,-3)-(0.2501*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(12.099>x && x>11.544)
        {
          tau = "(0.5357*Math.pow(10,6)*Math.pow(X,-3))-(0.2132*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(11.544>x && x>10.206)
        {
          tau = "(0.3312*Math.pow(10,6)*Math.pow(X,-3))-(0.8942*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(10.206>x && x>2.819)
        {
          tau = "(0.3533*Math.pow(10,4))*Math.pow(X,-2)+(0.6433*Math.pow(10,5))*Math.pow(X,-3)-(0.6929*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.819>x && x>2.574)
        {
          tau = "(0.7600*Math.pow(10,5))*Math.pow(X,-3)-(0.8179*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.574>x && x>2.281)
        {
          tau = "(0.6324*Math.pow(10,5))*Math.pow(X,-3)-(0.5576*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.281>x && x>1.871)
        {
          tau = "(0.6520*Math.pow(10,5))*Math.pow(X,-3)-(0.7006*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.871>x && x>1.809)
        {
          tau = "(0.4760*Math.pow(10,5))*Math.pow(X,-3)-(0.5122*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.2594*Math.pow(10,4))*Math.pow(X,-2)+(0.2704*Math.pow(10,4))*Math.pow(X,-3)-(0.1624*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.571*Math.pow(10,-1)*X)*Math.pow((8.324*Math.pow(10,-2)+1.526*Math.pow(10,-2)*X+2.825*Math.pow(10,-3)*Math.pow(X,2)+6.277*Math.pow(10,-5)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((126.1*Math.pow(X,-1)+6.724+0.01805*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }




    }
}

double MainWindow::oslableniyeThird(double x)
{
    QString tau;

    QString sigma;
    QString sigmaK;
    QString sigmaNK;

    QScriptEngine engine;

    QScriptValue tauValue;
    QScriptValue sigmaValue;
    QScriptValue sigmaKValue;
    QScriptValue sigmaNKValue;

    switch(ui->materialThird->currentIndex())
    {
    case 0: //Air
        {
            if(x<3.203)
            {
                tau = "-(0.1991*Math.pow(10,-2))*X+(0.1169)*Math.pow(X,-1)-(0.1514*100)*Math.pow(X,-2)+(0.4759*10000)*Math.pow(X,-3)-(0.9060*1000)*Math.pow(X,-4)";
            }
            else
            {
               tau = "-(0.3514*Math.pow(10,2))*Math.pow(X,-2)+(0.5059*10000)*Math.pow(X,-3)-(0.1060*1000)*Math.pow(X,-4)+Math.pow((26.44*Math.pow(X,-1)+4.724+0.01809*X),-1)";
            }
            tau.replace("X", QString::number(x));
            tauValue = engine.evaluate(tau);
            weakening = tauValue.toNumber();
            return weakening;
            break;
        }
    case 1: //H
        {
            tau = "-(0.2645*Math.pow(10,-2))*X+(0.7086*0.1)*Math.pow(X,-1)-(0.7487)*Math.pow(X,-2)+(0.5575*10)*Math.pow(X,-3)-(-0.1936*10)*Math.pow(X,-4)";
            sigma = "0.4005*(1/1.008)*Math.pow((1+2*X/511),-2)*(1+2*X/511+0.3*Math.pow(2*X/511,2)-0.0625*Math.pow(2*X/511,3))";
            tau.replace("X", QString::number(x));
            sigma.replace("X", QString::number(x));
            tauValue = engine.evaluate(tau);
            sigmaValue = engine.evaluate(sigma);
            weakening = tauValue.toNumber()+sigmaValue.toNumber();
            return weakening;
            break;
        }
    case 2: //He
        {
            tau = "-(0.2154*Math.pow(10,-2))*X+(0.1473)*Math.pow(X,-1)-(0.3322*10)*Math.pow(X,-2)+(0.4893*100)*Math.pow(X,-3)-(-0.1576*100)*Math.pow(X,-4)";
            sigma = "0.4005*(1/4.003)*Math.pow(1+2*X/511,-2)*(1+2*X/511+0.3*Math.pow(2*X/511,2)-0.0625*Math.pow(2*X/511,3))";
            tau.replace("X", QString::number(x));
            sigma.replace("X", QString::number(x));
            tauValue = engine.evaluate(tau);
            sigmaValue = engine.evaluate(sigma);
            weakening = tauValue.toNumber()+sigmaValue.toNumber();
            return weakening;
            break;
        }
    case 3: //Li
        {
            tau = "-(0.3411*Math.pow(10,-2))*Math.pow(X,0)+(0.3088*Math.pow(10,0))*Math.pow(X,-1)-(0.1009*Math.pow(10,2))*Math.pow(X,-2)+(0.2076*Math.pow(10,3))*Math.pow(X,-3)-(-0.4091*Math.pow(10,2))*Math.pow(X,-4)";
            sigmaK = "(1+9.326*Math.pow(10,-2)*X)*Math.pow((1.781*Math.pow(10,0)+8.725*Math.pow(10,-1)*X+7.963*Math.pow(10,-2)*Math.pow(X,2)+8.225*Math.pow(10,-3)*Math.pow(X,3)),-1)";
            sigmaNK = "Math.pow((29.94*Math.pow(X,-1)+4.533+0.03637*X),-1)";
            tau.replace("X", QString::number(x));
            sigmaK.replace("X", QString::number(x));
            sigmaNK.replace("X", QString::number(x));
            tauValue = engine.evaluate(tau);
            sigmaKValue = engine.evaluate(sigmaK);
            sigmaNKValue = engine.evaluate(sigmaNK);
            weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
            return weakening;
            break;
        }
    case 4: //Be
    {
        tau = "-(0.3142*Math.pow(10,-2))*Math.pow(X,0)+(0.4216*Math.pow(10,0))*Math.pow(X,-1)-(0.2014*Math.pow(10,2))*Math.pow(X,-2)+(0.5918*Math.pow(10,3))*Math.pow(X,-3)-(-0.4857*Math.pow(10,2))*Math.pow(X,-4)";
        sigmaK = "(1-3.178*Math.pow(10,-2)*X)*Math.pow((1.267*Math.pow(10,0)+4.619*Math.pow(10,-1)*X+3.102*Math.pow(10,-2)*Math.pow(X,2)-1.493*Math.pow(10,-3)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((25.93*Math.pow(X,-1)+5.067+0.02420*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 5: //B
    {
        tau = "-(0.3267*Math.pow(10,-2))*X+(0.5682)*Math.pow(X,-1)-(0.3483*100)*Math.pow(X,-2)+(0.1326*10000)*Math.pow(X,-3)-(0.3243*100)*Math.pow(X,-4)";
        sigmaK = "(1+1.544*X)*Math.pow((1.010+1.561*X+6.978*0.1*Math.pow(X,2)+5.025*0.01*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((25.58*Math.pow(X,-1)+4.945+0.02191*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 6: //C
    {
        tau = "-(0.3172*Math.pow(10,-2))*X+(0.6921)*Math.pow(X,-1)-(0.5340*100)*Math.pow(X,-2)+(0.2610*10000)*Math.pow(X,-3)-(0.2941*1000)*Math.pow(X,-4)";
        sigmaK = "(1+1.108*X)*Math.pow((8.093*0.1+7.378*0.1*X+3.958*0.1*Math.pow(X,2)+2.532*0.01*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((25.91*Math.pow(X,-1)+4.644+0.01878*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 7: //N
    {
        tau = "-(0.1991*Math.pow(10,-2))*X+(0.6169)*Math.pow(X,-1)-(0.6514*100)*Math.pow(X,-2)+(0.4259*10000)*Math.pow(X,-3)-(0.8060*1000)*Math.pow(X,-4)";
        sigmaK = "(1+5.723*0.1*X)*Math.pow((7.260*0.1+2.910*0.1*X+1.824*0.1*Math.pow(X,2)+1.018*0.1*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((26.44*Math.pow(X,-1)+4.724+0.01809*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 8: //O
    {
        tau = "-(0.2663*Math.pow(10,-2))*X+(0.8397)*Math.pow(X,-1)-(0.9179*100)*Math.pow(X,-2)+(0.6634*10000)*Math.pow(X,-3)-(0.1906*10000)*Math.pow(X,-4)";
        sigmaK = "(1+3.537*0.1*X)*Math.pow((6.396*0.1+1.543*0.1*X+9.948*0.01*Math.pow(X,2)+4.981*0.001*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((29.88*Math.pow(X,-1)+4.656+0.01857*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 9: //F
    {
        tau = "-(0.3038*Math.pow(10,-2))*X+(0.9836)*Math.pow(X,-1)-(0.1128*1000)*Math.pow(X,-2)+(0.9171*10000)*Math.pow(X,-3)-(0.3414*10000)*Math.pow(X,-4)";
        sigmaK = "(1+2.437*0.1*X)*Math.pow((5.970*0.1+9.989*0.01*X+6.409*0.01*Math.pow(X,2)+2.930*0.001*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((33.40*Math.pow(X,-1)+4.961+0.01913*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 10: //Ne
    {
        tau = "-(0.1806*Math.pow(10,-2))*X+(0.7942)*Math.pow(X,-1)-(0.1218*1000)*Math.pow(X,-2)+(0.1307*100000)*Math.pow(X,-3)-(0.5600*10000)*Math.pow(X,-4)";
        sigmaK = "(1+1.820*0.1*X)*Math.pow((5.118*0.1+6.431*0.01*X+4.065*0.01*Math.pow(X,2)+1.715*0.001*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((37.30*Math.pow(X,-1)+4.629+0.01905*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 11: //Na
    {
        if(x>1.072)
        {
          tau = "-(0.3963*Math.pow(10,-2))*X+(0.1359*10)*Math.pow(X,-1)-(0.1720*1000)*Math.pow(X,-2)+(0.1766*100000)*Math.pow(X,-3)-(0.1019*100000)*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.8694*1000)*Math.pow(X,-3)-(0.2170*1000)*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.029*0.1*X)*Math.pow((4.721*0.1+6.884*0.01*X+2.528*0.01*Math.pow(X,2)+8.577*0.0001*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((38.80*Math.pow(X,-1)+4.901+0.01889*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 12: //Mg
    {
        if(x>1.305)
        {
          tau = "-(0.3447*Math.pow(10,-2))*X+(0.1309*Math.pow(10,1))*Math.pow(X,-1)-(0.1895*Math.pow(10,3))*Math.pow(X,-2)+(0.2327*Math.pow(10,5))*Math.pow(X,-3)-(0.1439*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1268*Math.pow(10,4))*Math.pow(X,-3)-(0.3474*Math.pow(10,3))*Math.pow(X,-4)";
        }
        sigmaK = "(1+8.484*Math.pow(10,-2)*X)*Math.pow((4.132*Math.pow(10,-1)+7.566*Math.pow(10,-2)*X+1.836*Math.pow(10,-2)*Math.pow(X,2)+6.034*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((39.81*Math.pow(X,-1)+4.746+0.01832*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 13: //Al
    {
        if(x>1.559)
        {
          tau = "-(0.3471*Math.pow(10,-2))*Math.pow(X,0)+(0.1384*Math.pow(10,1))*Math.pow(X,-1)-(0.2137*Math.pow(10,3))*Math.pow(X,-2)+(0.2950*Math.pow(10,5))*Math.pow(X,-3)-(0.2217*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1688*Math.pow(10,4)*Math.pow(X,-3))-(0.5046*Math.pow(10,3)*Math.pow(X,-4))";
        }
        sigmaK = "(1+8.580*Math.pow(10,-2)*X)*Math.pow((3.905*Math.pow(10,-1)+8.506*Math.pow(10,-2)*X+1.611*Math.pow(10,-2)*Math.pow(X,2)+5.524*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((42.58*Math.pow(X,-1)+4.873+0.01871*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 14: //Si
    {
        if(x>1.838)
        {
          tau = "-(0.2219*Math.pow(10,-2))*X+(0.1141*Math.pow(10,1))*Math.pow(X,-1)-(0.2202*Math.pow(10,3))*Math.pow(X,-2)+(0.3864*Math.pow(10,5))*Math.pow(X,-3)-(0.3319*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1640*Math.pow(10,3))*Math.pow(X,-2)+(0.1853*Math.pow(10,4))*Math.pow(X,-3)-(0.4500*Math.pow(10,3))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.970*X)*Math.pow((3.667*Math.pow(10,-1)+6.375*Math.pow(10,-1)*X+1.589*Math.pow(10,-1)*Math.pow(X,2)+1.114*Math.pow(10,-2)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((42.56*Math.pow(X,-1)+4.729+0.01796*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 15: //P
    {
        if(x>2.145)
        {
          tau = "(0.6962)*Math.pow(X,-1)-(0.2154*Math.pow(10,3))*Math.pow(X,-2)+(0.4590*Math.pow(10,5))*Math.pow(X,-3)-(0.4530*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1908*Math.pow(10,3))*Math.pow(X,-2)+(0.2308*Math.pow(10,4))*Math.pow(X,-3)-(0.5886*Math.pow(10,3))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.169*X)*Math.pow((3.420*Math.pow(10,-1)+6.676*Math.pow(10,-1)*X+1.740*Math.pow(10,-1)*Math.pow(X,2)+1.124*Math.pow(10,-2)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((45.17*Math.pow(X,-1)+4.889+0.01835*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 16: //S
    {
        if(x>2.472)
        {
          tau = "(0.7582)*Math.pow(X,-1)-(0.2447*Math.pow(10,3))*Math.pow(X,-2)+(0.5785*Math.pow(10,5))*Math.pow(X,-3)-(0.6419*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.2282*Math.pow(10,3))*Math.pow(X,-2)+(0.3007*Math.pow(10,4))*Math.pow(X,-3)-(0.8095*Math.pow(10,3))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.786*X)*Math.pow((3.095*Math.pow(10,-1)+4.495*Math.pow(10,-1)*X+1.360*Math.pow(10,-1)*Math.pow(X,2)+7.918*Math.pow(10,-2)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((41.69*Math.pow(X,-1)+4.900+0.01671*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 17: //Cl
    {
        if(x>2.822)
        {
          tau = "(0.7858)*Math.pow(X,-1)-(0.2734*Math.pow(10,3))*Math.pow(X,-2)+(0.7529*Math.pow(10,5))*Math.pow(X,-3)-(0.9287*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.2783*Math.pow(10,3))*Math.pow(X,-2)+(0.4027*Math.pow(10,4))*Math.pow(X,-3)-(0.1144*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.391*X)*Math.pow((3.181*Math.pow(10,-1)+3.659*Math.pow(10,-1)*X+1.077*Math.pow(10,-1)*Math.pow(X,2)+5.874*Math.pow(10,-2)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((44.68*Math.pow(X,-1)+5.115+0.01732*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 18: //Ar
    {
        if(x>3.206)
        {
          tau = "(0.7518)*Math.pow(X,-1)-(0.2633*Math.pow(10,3))*Math.pow(X,-2)+(0.7533*Math.pow(10,5))*Math.pow(X,-3)-(0.1050*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.2629*Math.pow(10,3))*Math.pow(X,-2)+(0.4167*Math.pow(10,4))*Math.pow(X,-3)-(0.1249*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.080*X)*Math.pow((3.078*Math.pow(10,-1)+2.799*Math.pow(10,-1)*X+8.688*Math.pow(10,-2)*Math.pow(X,2)+4.380*Math.pow(10,-3)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((49.09*Math.pow(X,-1)+5.452+0.01840*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 19: //K
    {
        if(x>3.607)
        {
          tau = "(0.6731)*Math.pow(X,-1)-(0.2717*Math.pow(10,3))*Math.pow(X,-2)+(0.9468*Math.pow(10,5))*Math.pow(X,-3)-(0.1384*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.3242*Math.pow(10,3))*Math.pow(X,-2)+(0.5459*Math.pow(10,4))*Math.pow(X,-3)-(0.1733*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+9.832*Math.pow(10,-1)*X)*Math.pow((2.733*Math.pow(10,-1)+2.506*Math.pow(10,-1)*X+6.843*Math.pow(10,-2)*Math.pow(X,2)+3.356*Math.pow(10,-3)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((46.28*Math.pow(X,-1)+5.080+0.01694*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 20: //Ca
    {
        if(x>4.038)
        {
          tau = "(0.7622)*Math.pow(X,-1)-(0.3106*Math.pow(10,3))*Math.pow(X,-2)+(0.1148*Math.pow(10,6))*Math.pow(X,-3)-(0.1902*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.3699*Math.pow(10,3))*Math.pow(X,-2)+(0.6774*Math.pow(10,4))*Math.pow(X,-3)-(0.2287*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.265*X)*Math.pow((2.507*Math.pow(10,-1)+2.991*Math.pow(10,-1)*X+8*Math.pow(10,-2)*Math.pow(X,2)+3.915*Math.pow(10,-3)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((45.51*Math.pow(X,-1)+4.977+0.01634*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 21: //Sc
    {
        if(x>4.492)
        {
          tau = "(0.7710)*Math.pow(X,-1)-(0.3377*Math.pow(10,3))*Math.pow(X,-2)+(0.1397*Math.pow(10,6))*Math.pow(X,-3)-(0.2524*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.3933*Math.pow(10,3))*Math.pow(X,-2)+(0.7473*Math.pow(10,4))*Math.pow(X,-3)-(0.2642*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+9.784*Math.pow(10,-1)*X)*Math.pow((2.502*Math.pow(10,-1)+2.318*Math.pow(10,-1)*X+6.513*Math.pow(10,-2)*Math.pow(X,2)+2.995*Math.pow(10,-3)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((47.76*Math.pow(X,-1)+5.397+0.01697*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 22: //Ti
    {
        if(x>4.966)
        {
          tau = "(0.6170)*Math.pow(X,-1)-(0.2987*Math.pow(10,3))*Math.pow(X,-2)+(0.1409*Math.pow(10,6))*Math.pow(X,-3)-(0.2757*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.4341*Math.pow(10,3))*Math.pow(X,-2)+(0.8580*Math.pow(10,4))*Math.pow(X,-3)-(0.3171*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+7.497*Math.pow(10,-1)*X)*Math.pow((2.462*Math.pow(10,-1)+1.731*Math.pow(10,-1)*X+4.960*Math.pow(10,-2)*Math.pow(X,2)+2.157*Math.pow(10,-3)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((50.03*Math.pow(X,-1)+5.490+0.01728*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 23: //V
    {
        if(x>5.465)
        {
          tau = "-(0.2018*Math.pow(10,3))*Math.pow(X,-2)+(0.1560*Math.pow(10,6))*Math.pow(X,-3)-(0.3252*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.4601*Math.pow(10,3))*Math.pow(X,-2)+(0.9829*Math.pow(10,4))*Math.pow(X,-3)-(0.3826*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.775*Math.pow(10,-1)*X)*Math.pow((2.367*Math.pow(10,-1)+1.307*Math.pow(10,-1)*X+3.807*Math.pow(10,-2)*Math.pow(X,2)+1.566*Math.pow(10,-3)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((52.62*Math.pow(X,-1)+5.580+0.01763*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 24: //Cr
    {
        if(x>5.989)
        {
          tau = "-(0.2213*Math.pow(10,3))*Math.pow(X,-2)+(0.1827*Math.pow(10,6))*Math.pow(X,-3)-(0.4226*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.5149*Math.pow(10,3))*Math.pow(X,-2)+(0.1154*Math.pow(10,5))*Math.pow(X,-3)-(0.4693*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+4.190*Math.pow(10,-1)*X)*Math.pow((2.292*Math.pow(10,-1)+8.642*Math.pow(10,-2)*X+2.697*Math.pow(10,-2)*Math.pow(X,2)+1.028*Math.pow(10,-3)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((54.13*Math.pow(X,-1)+5.430+0.01746*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 25: //Mn
    {
        if(x>6.539)
        {
          tau = "-(0.2245*Math.pow(10,3))*Math.pow(X,-2)+(0.2042*Math.pow(10,6))*Math.pow(X,-3)-(0.5055*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.5484*Math.pow(10,3))*Math.pow(X,-2)+(0.1312*Math.pow(10,5))*Math.pow(X,-3)-(0.5638*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+3.707*Math.pow(10,-1)*X)*Math.pow((2.167*Math.pow(10,-1)+8.029*Math.pow(10,-2)*X+2.324*Math.pow(10,-2)*Math.pow(X,2)+8.595*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((56.25*Math.pow(X,-1)+5.517+0.01764*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 26: //Fe
    {
        if(x>7.112)
        {
          tau = "-(0.2371*Math.pow(10,3))*Math.pow(X,-2)+(0.2359*Math.pow(10,6))*Math.pow(X,-3)-(0.6309*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.6176*Math.pow(10,3))*Math.pow(X,-2)+(0.1530*Math.pow(10,5))*Math.pow(X,-3)-(0.6905*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+3.075*Math.pow(10,-1)*X)*Math.pow((2.062*Math.pow(10,-1)+6.305*Math.pow(10,-2)*X+1.845*Math.pow(10,-2)*Math.pow(X,2)+6.535*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((56.92*Math.pow(X,-1)+5.382+0.01732*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 27: //Co
    {
        if(x>7.705)
        {
          tau = "-(0.2397*Math.pow(10,3))*Math.pow(X,-2)+(0.2612*Math.pow(10,6))*Math.pow(X,-3)-(0.7533*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.6579*Math.pow(10,3))*Math.pow(X,-2)+(0.1696*Math.pow(10,5))*Math.pow(X,-3)-(0.7918*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.591*Math.pow(10,-1)*X)*Math.pow((2.020*Math.pow(10,-1)+5.267*Math.pow(10,-2)*X+1.548*Math.pow(10,-2)*Math.pow(X,2)+5.231*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((59.76*Math.pow(X,-1)+5.460+0.01767*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 28: //Ni
    {
        if(x>8.332)
        {
          tau = "-(0.2531*Math.pow(10,3))*Math.pow(X,-2)+(0.3041*Math.pow(10,6))*Math.pow(X,-3)-(0.9428*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.7147*Math.pow(10,3))*Math.pow(X,-2)+(0.1993*Math.pow(10,5))*Math.pow(X,-3)-(0.9624*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.214*Math.pow(10,-1)*X)*Math.pow((1.870*Math.pow(10,-1)+4.225*Math.pow(10,-2)*X+1.244*Math.pow(10,-2)*Math.pow(X,2)+3.980*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((59.81*Math.pow(X,-1)+5.222+0.01711*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 29: //Cu
    {
        if(x>8.978)
        {
          tau = "-(0.2491*Math.pow(10,3))*Math.pow(X,-2)+(0.3252*Math.pow(10,6))*Math.pow(X,-3)-(0.1097*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(x<8.978 && x>1.096)
        {
          tau = "(0.7031*Math.pow(10,3))*Math.pow(X,-2)+(0.2165*Math.pow(10,5))*Math.pow(X,-3)-(0.1127*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1357*Math.pow(10,5))*Math.pow(X,-3)-(0.3001*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.902*Math.pow(10,-1)*X)*Math.pow((1.874*Math.pow(10,-1)+3.418*Math.pow(10,-2)*X+1.103*Math.pow(10,-2)*Math.pow(X,2)+3.367*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((65.59*Math.pow(X,-1)+5.428+0.01804*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 30: //Zn
    {
        if(x>9.658)
        {
          tau = "-(0.2426*Math.pow(10,3))*Math.pow(X,-2)+(0.3619*Math.pow(10,6))*Math.pow(X,-3)-(0.1289*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(9.658>x && x>1.193)
        {
          tau = "(0.7031*Math.pow(10,3))*Math.pow(X,-2)+(0.2165*Math.pow(10,5))*Math.pow(X,-3)-(0.1127*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.193>=x && x>=1.042)
        {
          tau = "(0.2889*Math.pow(10,5)*Math.pow(X,-3))-(0.1942*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(1.042>x && x>1.019)
        {
          tau = "(-0.1590*Math.pow(10,5)*Math.pow(X,-3))-(-0.2482*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.2572*Math.pow(10,4))*Math.pow(X,-3)-(0.1023*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.690*Math.pow(10,-1)*X)*Math.pow((1.786*Math.pow(10,-1)+3.177*Math.pow(10,-2)*X+9.452*Math.pow(10,-3)*Math.pow(X,2)+2.811*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((67.69*Math.pow(X,-1)+5.377+0.01810*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 31: //Ga
    {
        if(x>10.367)
        {
          tau = "-(0.2328*Math.pow(10,3))*Math.pow(X,-2)+(0.3881*Math.pow(10,6))*Math.pow(X,-3)-(0.1475*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(10.367>x && x>1.297)
        {
          tau = "(0.6933*Math.pow(10,3))*Math.pow(X,-2)+(0.2734*Math.pow(10,5))*Math.pow(X,-3)-(0.1657*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.297>x && x>1.142)
        {
          tau = "(0.3217*Math.pow(10,5)*Math.pow(X,-3))-(0.2363*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(1.142>x && x>1.115)
        {
          tau = "(-0.1315*Math.pow(10,5)*Math.pow(X,-3))-(-0.2503*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.2873*Math.pow(10,4))*Math.pow(X,-3)-(0.1181*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.458*Math.pow(10,-1)*X)*Math.pow((1.799*Math.pow(10,-1)+3.125*Math.pow(10,-2)*X+8.248*Math.pow(10,-3)*Math.pow(X,2)+2.369*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((65.9*Math.pow(X,-1)+5.722+0.01764*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 32: //Ge
    {
        if(x>11.103)
        {
          tau = "-(0.2231*Math.pow(10,3))*Math.pow(X,-2)+(0.4236*Math.pow(10,6))*Math.pow(X,-3)-(0.1708*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(11.103>x && x>1.414)
        {
          tau = "(0.7231*Math.pow(10,3))*Math.pow(X,-2)+(0.3038*Math.pow(10,5))*Math.pow(X,-3)-(0.1954*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.414>x && x>1.247)
        {
          tau = "(0.3416*Math.pow(10,5)*Math.pow(X,-3))-(0.2594*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(1.247>x && x>1.216)
        {
          tau = "(-0.1079*Math.pow(10,5)*Math.pow(X,-3))-(-0.2594*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.3266*Math.pow(10,4))*Math.pow(X,-3)-(0.1378*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.390*Math.pow(10,-1)*X)*Math.pow((1.756*Math.pow(10,-1)+3.140*Math.pow(10,-2)*X+7.573*Math.pow(10,-3)*Math.pow(X,2)+2.114*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((68.74*Math.pow(X,-1)+5.759+0.01789*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 33: //As
    {
        if(x>11.866)
        {
          tau = "-(0.2135*Math.pow(10,3))*Math.pow(X,-2)+(0.4644*Math.pow(10,6))*Math.pow(X,-3)-(0.1982*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(11.866>x && x>1.526)
        {
          tau = "(0.7490*Math.pow(10,3))*Math.pow(X,-2)+(0.3399*Math.pow(10,5))*Math.pow(X,-3)-(0.2339*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.526>x && x>1.358)
        {
          tau = "(0.3594*Math.pow(10,5)*Math.pow(X,-3))-(0.2762*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(1.358>x && x>1.323)
        {
          tau = "(-0.9541*Math.pow(10,4)*Math.pow(X,-3))-(-0.2846*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.3765*Math.pow(10,4))*Math.pow(X,-3)-(0.1650*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.465*Math.pow(10,-1)*X)*Math.pow((1.7*Math.pow(10,-1)+3.174*Math.pow(10,-2)*X+7.414*Math.pow(10,-3)*Math.pow(X,2)+2.197*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((70.84*Math.pow(X,-1)+5.755+0.01797*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 34: //Se
    {
        if(x>12.657)
        {
          tau = "-(0.1963*Math.pow(10,3))*Math.pow(X,-2)+(0.4978*Math.pow(10,6))*Math.pow(X,-3)-(0.2250*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(12.657>x && x>1.653)
        {
          tau = "(0.7661*Math.pow(10,3))*Math.pow(X,-2)+(0.3724*Math.pow(10,5))*Math.pow(X,-3)-(0.2740*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.653>x && x>1.476)
        {
          tau = "(0.3556*Math.pow(10,5)*Math.pow(X,-3))-(0.2627*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(1.476>x && x>1.435)
        {
          tau = "(-0.3092*Math.pow(10,4)*Math.pow(X,-3))-(-0.2333*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.4214*Math.pow(10,4))*Math.pow(X,-3)-(0.1902*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.665*Math.pow(10,-1)*X)*Math.pow((1.688*Math.pow(10,-1)+3.453*Math.pow(10,-2)*X+7.860*Math.pow(10,-3)*Math.pow(X,2)+2.404*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((74.43*Math.pow(X,-1)+5.881+0.01844*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 35: //Br
    {
        if(x>13.473)
        {
          tau = "-(0.1788*Math.pow(10,3))*Math.pow(X,-2)+(0.5528*Math.pow(10,6))*Math.pow(X,-3)-(0.2646*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(13.473>x && x>1.782)
        {
          tau = "(0.7869*Math.pow(10,3))*Math.pow(X,-2)+(0.4214*Math.pow(10,5))*Math.pow(X,-3)-(0.3277*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.782>x && x>1.596)
        {
          tau = "(0.3588*Math.pow(10,5)*Math.pow(X,-3))-(0.2389*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(1.596>x && x>1.549)
        {
          tau = "(-0.7092*Math.pow(10,4)*Math.pow(X,-3))-(-0.3479*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.4876*Math.pow(10,4))*Math.pow(X,-3)-(0.2258*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.040*Math.pow(10,-1)*X)*Math.pow((1.614*Math.pow(10,-1)+3.768*Math.pow(10,-2)*X+8.521*Math.pow(10,-3)*Math.pow(X,2)+2.778*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((74.84*Math.pow(X,-1)+5.785+0.01808*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 36: //Kr
    {
        if(x>14.325)
        {
          tau = "(0.5738*Math.pow(10,6))*Math.pow(X,-3)-(0.2753*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(14.325>x && x>1.921)
        {
          tau = "(0.8114*Math.pow(10,3))*Math.pow(X,-2)+(0.4562*Math.pow(10,5))*Math.pow(X,-3)-(0.3722*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.921>x && x>1.727)
        {
          tau = "(0.3388*Math.pow(10,5)*Math.pow(X,-3))-(0.1775*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(1.727>x && x>1.671)
        {
          tau = "(-0.3331*Math.pow(10,5)*Math.pow(X,-3))-(-0.8565*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.5401*Math.pow(10,4))*Math.pow(X,-3)-(0.2556*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.563*Math.pow(10,-1)*X)*Math.pow((1.605*Math.pow(10,-1)+4.357*Math.pow(10,-2)*X+9.829*Math.pow(10,-3)*Math.pow(X,2)+3.412*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((78.38*Math.pow(X,-1)+5.888+0.01853*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 37: //Rb
    {
        if(x>15.199)
        {
          tau = "(0.6321*Math.pow(10,6))*Math.pow(X,-3)-(0.3222*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(15.199>x && x>2.065)
        {
          tau = "(0.8061*Math.pow(10,3))*Math.pow(X,-2)+(0.5107*Math.pow(10,5))*Math.pow(X,-3)-(0.4410*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.065>x && x>1.863)
        {
          tau = "(0.4800*Math.pow(10,5)*Math.pow(X,-3))-(0.4165*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(1.863>x && x>1.804)
        {
          tau = "(0.2975*Math.pow(10,5)*Math.pow(X,-3))-(-0.2098*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.6124*Math.pow(10,4))*Math.pow(X,-3)-(0.2965*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.660*Math.pow(10,-1)*X)*Math.pow((1.539*Math.pow(10,-1)+3.449*Math.pow(10,-2)*X+6.750*Math.pow(10,-3)*Math.pow(X,2)+2.079*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((79.11*Math.pow(X,-1)+5.851+0.01835*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 38: //Sr
    {
        if(x>16.104)
        {
          tau = "(0.6915*Math.pow(10,6))*Math.pow(X,-3)-(0.3763*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(16.104>x && x>2.216)
        {
          tau = "(0.8448*Math.pow(10,3))*Math.pow(X,-2)+(0.5659*Math.pow(10,5))*Math.pow(X,-3)-(0.5234*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.216>x && x>2.006)
        {
          tau = "(0.5024*Math.pow(10,5)*Math.pow(X,-3))-(0.4284*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(2.006>x && x>1.939)
        {
          tau = "(0.2310*Math.pow(10,5)*Math.pow(X,-3))-(-0.4765*Math.pow(10,4)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1040*Math.pow(10,4))*Math.pow(X,-2)+(0.4001*Math.pow(10,4))*Math.pow(X,-3)-(0.1553*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.310*Math.pow(10,-1)*X)*Math.pow((1.488*Math.pow(10,-1)+3.124*Math.pow(10,-2)*X+5.429*Math.pow(10,-3)*Math.pow(X,2)+1.580*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((79.79*Math.pow(X,-1)+5.863+0.01821*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 39: //Y
    {
        if(x>17.038)
        {
          tau = "(0.7615*Math.pow(10,6))*Math.pow(X,-3)-(0.4412*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(17.038>x && x>2.372)
        {
          tau = "(0.8931*Math.pow(10,3))*Math.pow(X,-2)+(0.6297*Math.pow(10,5))*Math.pow(X,-3)-(0.6171*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.372>x && x>2.155)
        {
          tau = "(0.5458*Math.pow(10,5)*Math.pow(X,-3))-(0.4732*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(2.155>x && x>2.080)
        {
          tau = "(0.2326*Math.pow(10,5)*Math.pow(X,-3))-(-0.3694*Math.pow(10,4)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1148*Math.pow(10,4))*Math.pow(X,-2)+(0.4487*Math.pow(10,4))*Math.pow(X,-3)-(0.1768*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.794*Math.pow(10,-1)*X)*Math.pow((1.435*Math.pow(10,-1)+3.612*Math.pow(10,-2)*X+6.434*Math.pow(10,-3)*Math.pow(X,2)+2.056*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((80.83*Math.pow(X,-1)+5.8+0.01798*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 40: //Zr
    {
        if(x>17.997)
        {
          tau = "(0.8263*Math.pow(10,6))*Math.pow(X,-3)-(0.5088*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(17.997>x && x>2.531)
        {
          tau = "(0.9174*Math.pow(10,3))*Math.pow(X,-2)+(0.6902*Math.pow(10,5))*Math.pow(X,-3)-(0.7135*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.531>x && x>2.306)
        {
          tau = "(0.5865*Math.pow(10,5)*Math.pow(X,-3))-(0.5184*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(2.306>x && x>2.222)
        {
          tau = "(0.2511*Math.pow(10,5)*Math.pow(X,-3))-(-0.2008*Math.pow(10,4)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1245*Math.pow(10,4))*Math.pow(X,-2)+(0.4942*Math.pow(10,4))*Math.pow(X,-3)-(0.1983*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+3.040*Math.pow(10,-1)*X)*Math.pow((1.421*Math.pow(10,-1)+4.878*Math.pow(10,-2)*X+9.372*Math.pow(10,-3)*Math.pow(X,2)+3.381*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((81.87*Math.pow(X,-1)+5.790+0.01805*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 41: //Nb
    {
        if(x>18.985)
        {
          tau = "(0.9001*Math.pow(10,6))*Math.pow(X,-3)-(0.5848*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(18.985>x && x>2.697)
        {
          tau = "(0.9588*Math.pow(10,3))*Math.pow(X,-2)+(0.7570*Math.pow(10,5))*Math.pow(X,-3)-(0.8170*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.697>x && x>2.464)
        {
          tau = "(0.6456*Math.pow(10,5)*Math.pow(X,-3))-(0.6001*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(2.464>x && x>2.370)
        {
          tau = "(0.2869*Math.pow(10,5)*Math.pow(X,-3))-(-0.4479*Math.pow(10,4)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1320*Math.pow(10,4))*Math.pow(X,-2)+(0.5559*Math.pow(10,4))*Math.pow(X,-3)-(0.2278*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+4.815*Math.pow(10,-1)*X)*Math.pow((1.391*Math.pow(10,-1)+6.505*Math.pow(10,-2)*X+1.343*Math.pow(10,-2)*Math.pow(X,2)+5.147*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((77.98*Math.pow(X,-1)+5.895+0.01722*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 42: //Mo
    {
        if(x>19.999)
        {
          tau = "(0.9649*Math.pow(10,6))*Math.pow(X,-3)-(0.6635*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(19.999>x && x>2.865)
        {
          tau = "(0.9956*Math.pow(10,3))*Math.pow(X,-2)+(0.8166*Math.pow(10,5))*Math.pow(X,-3)-(0.9212*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.865>x && x>2.625)
        {
          tau = "(0.6946*Math.pow(10,5)*Math.pow(X,-3))-(0.6720*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(2.625>x && x>2.520)
        {
          tau = "(0.3382*Math.pow(10,5)*Math.pow(X,-3))-(0.5891*Math.pow(10,4)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1371*Math.pow(10,4))*Math.pow(X,-2)+(0.6170*Math.pow(10,4))*Math.pow(X,-3)-(0.2607*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.411*Math.pow(10,-1)*X)*Math.pow((1.345*Math.pow(10,-1)+7.060*Math.pow(10,-2)*X+1.463*Math.pow(10,-2)*Math.pow(X,2)+5.588*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((80.67*Math.pow(X,-1)+5.926+0.01748*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 43: //Tc
    {
        if(x>21.044)
        {
          tau = "(0.1034*Math.pow(10,7))*Math.pow(X,-3)-(0.7517*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(21.044>x && x>3.042)
        {
          tau = "(0.9760*Math.pow(10,3))*Math.pow(X,-2)+(0.8887*Math.pow(10,5))*Math.pow(X,-3)-(0.1071*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(3.042>x && x>2.793)
        {
          tau = "(0.7487*Math.pow(10,5)*Math.pow(X,-3))-(0.7579*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(2.793>x && x>2.676)
        {
          tau = "(0.4015*Math.pow(10,5)*Math.pow(X,-3))-(0.1605*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1452*Math.pow(10,4))*Math.pow(X,-2)+(0.6728*Math.pow(10,4))*Math.pow(X,-3)-(0.2892*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.743*Math.pow(10,-1)*X)*Math.pow((1.311*Math.pow(10,-1)+7.273*Math.pow(10,-2)*X+1.496*Math.pow(10,-2)*Math.pow(X,2)+5.695*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((81.49*Math.pow(X,-1)+5.927+0.01735*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 44: //Ru
    {
        if(x>22.117)
        {
          tau = "(0.1116*Math.pow(10,7))*Math.pow(X,-3)-(0.8554*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(22.117>x && x>3.224)
        {
          tau = "(0.1018*Math.pow(10,4))*Math.pow(X,-2)+(0.9670*Math.pow(10,5))*Math.pow(X,-3)-(0.1222*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(3.224>x && x>2.966)
        {
          tau = "(0.8078*Math.pow(10,5)*Math.pow(X,-3))-(0.8411*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else if(2.966>x && x>2.837)
        {
          tau = "(0.4695*Math.pow(10,5)*Math.pow(X,-3))-(0.2717*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1537*Math.pow(10,4))*Math.pow(X,-2)+(0.7433*Math.pow(10,4))*Math.pow(X,-3)-(0.3262*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.771*Math.pow(10,-1)*X)*Math.pow((1.297*Math.pow(10,-1)+7.016*Math.pow(10,-2)*X+1.493*Math.pow(10,-2)*Math.pow(X,2)+5.478*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((83.16*Math.pow(X,-1)+5.987+0.01743*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 45: //Rh
    {
        if(x>23.219)
        {
          tau = "(0.1205*Math.pow(10,7))*Math.pow(X,-3)-(0.9730*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(23.219>x && x>3.411)
        {
          tau = "(0.1050*Math.pow(10,4))*Math.pow(X,-2)+(0.1054*Math.pow(10,6))*Math.pow(X,-3)-(0.1398*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(3.411>x && x>3.146)
        {
          tau = "(0.9003*Math.pow(10,5)*Math.pow(X,-3))-(0.1030*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(3.146>x && x>3.003)
        {
          tau = "(0.5655*Math.pow(10,5)*Math.pow(X,-3))-(0.4734*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1612*Math.pow(10,4))*Math.pow(X,-2)+(0.8250*Math.pow(10,4))*Math.pow(X,-3)-(0.3703*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.623*Math.pow(10,-1)*X)*Math.pow((1.281*Math.pow(10,-1)+6.536*Math.pow(10,-2)*X+1.422*Math.pow(10,-2)*Math.pow(X,2)+5.096*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((90.02*Math.pow(X,-1)+5.803+0.01826*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 46: //Pd
    {
        if(x>24.350)
        {
          tau = "(0.1279*Math.pow(10,7))*Math.pow(X,-3)-(0.1088*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(24.350>x && x>3.604)
        {
          tau = "(0.1085*Math.pow(10,4))*Math.pow(X,-2)+(0.1123*Math.pow(10,6))*Math.pow(X,-3)-(0.1547*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(3.604>x && x>3.330)
        {
          tau = "(0.1024*Math.pow(10,6)*Math.pow(X,-3))-(0.1371*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(3.330>x && x>3.173)
        {
          tau = "(0.7496*Math.pow(10,5)*Math.pow(X,-3))-(0.1008*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1693*Math.pow(10,4))*Math.pow(X,-2)+(0.8872*Math.pow(10,4))*Math.pow(X,-3)-(0.4040*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.415*Math.pow(10,-1)*X)*Math.pow((1.272*Math.pow(10,-1)+6.081*Math.pow(10,-2)*X+1.374*Math.pow(10,-2)*Math.pow(X,2)+4.715*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((92.23*Math.pow(X,-1)+5.883+0.01841*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 47: //Ag
    {
        if(x>25.514)
        {
          tau = "(0.1384*Math.pow(10,7))*Math.pow(X,-3)-(0.1238*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(25.514>x && x>3.805)
        {
          tau = "(0.1135*Math.pow(10,4))*Math.pow(X,-2)+(0.1223*Math.pow(10,6))*Math.pow(X,-3)-(0.1765*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(3.805>x && x>3.523)
        {
          tau = "(0.1075*Math.pow(10,6)*Math.pow(X,-3))-(0.1415*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(3.523>x && x>3.351)
        {
          tau = "(0.7408*Math.pow(10,5)*Math.pow(X,-3))-(0.8834*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1818*Math.pow(10,4))*Math.pow(X,-2)+(0.9723*Math.pow(10,4))*Math.pow(X,-3)-(0.4517*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.082*Math.pow(10,-1)*X)*Math.pow((1.214*Math.pow(10,-1)+5.642*Math.pow(10,-2)*X+1.250*Math.pow(10,-2)*Math.pow(X,2)+4.238*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((92.27*Math.pow(X,-1)+5.857+0.01817*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 48: //Cd
    {
        if(x>26.711)
        {
          tau = "(0.1453*Math.pow(10,7))*Math.pow(X,-3)-(0.1366*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(26.711>x && x>4.018)
        {
          tau = "(0.1102*Math.pow(10,4))*Math.pow(X,-2)+(0.1305*Math.pow(10,6))*Math.pow(X,-3)-(0.2009*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(4.018>x && x>3.727)
        {
          tau = "(0.1142*Math.pow(10,6)*Math.pow(X,-3))-(0.1588*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(3.727>x && x>3.537)
        {
          tau = "(0.7830*Math.pow(10,5)*Math.pow(X,-3))-(0.9738*Math.pow(10,5)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1874*Math.pow(10,4))*Math.pow(X,-2)+(0.1040*Math.pow(10,5))*Math.pow(X,-3)-(0.4943*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+4.921*Math.pow(10,-1)*X)*Math.pow((1.230*Math.pow(10,-1)+5.532*Math.pow(10,-2)*X+1.208*Math.pow(10,-2)*Math.pow(X,2)+4.035*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((94.65*Math.pow(X,-1)+6.003+0.01841*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 49: //In
    {
        if(x>27.939)
        {
          tau = "(0.1551*Math.pow(10,7))*Math.pow(X,-3)-(0.1519*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(27.939>x && x>4.237)
        {
          tau = "(0.1130*Math.pow(10,4))*Math.pow(X,-2)+(0.1406*Math.pow(10,6))*Math.pow(X,-3)-(0.2257*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(4.237>x && x>3.938)
        {
          tau = "(0.1270*Math.pow(10,6)*Math.pow(X,-3))-(0.1957*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(3.938>x && x>3.730)
        {
          tau = "(0.1006*Math.pow(10,6)*Math.pow(X,-3))-(0.1736*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.1980*Math.pow(10,4))*Math.pow(X,-2)+(0.1131*Math.pow(10,5))*Math.pow(X,-3)-(0.5499*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+4.741*Math.pow(10,-1)*X)*Math.pow((1.199*Math.pow(10,-1)+5.382*Math.pow(10,-2)*X+1.136*Math.pow(10,-2)*Math.pow(X,2)+3.757*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((95.15*Math.pow(X,-1)+6.032+0.01830*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 50: //Sn
    {
        if(x>29.200)
        {
          tau = "(0.1630*Math.pow(10,7))*Math.pow(X,-3)-(0.1643*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(29.200>x && x>4.464)
        {
          tau = "(0.1152*Math.pow(10,4))*Math.pow(X,-2)+(0.1492*Math.pow(10,6))*Math.pow(X,-3)-(0.2495*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(4.464>x && x>4.156)
        {
          tau = "(0.1428*Math.pow(10,6)*Math.pow(X,-3))-(0.2532*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(4.156>x && x>3.928)
        {
          tau = "(0.1097*Math.pow(10,6)*Math.pow(X,-3))-(0.2033*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.2051*Math.pow(10,4))*Math.pow(X,-2)+(0.1216*Math.pow(10,5))*Math.pow(X,-3)-(0.6037*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+4.821*Math.pow(10,-1)*X)*Math.pow((1.200*Math.pow(10,-1)+5.513*Math.pow(10,-2)*X+1.138*Math.pow(10,-2)*Math.pow(X,2)+3.740*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((96.63*Math.pow(X,-1)+6.142+0.01838*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 51: //Sb
    {
        if(x>30.491)
        {
          tau = "(0.1734*Math.pow(10,7))*Math.pow(X,-3)-(0.1865*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(30.491>x && x>4.698)
        {
          tau = "(0.1139*Math.pow(10,4))*Math.pow(X,-2)+(0.1604*Math.pow(10,6))*Math.pow(X,-3)-(0.2815*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(4.698>x && x>4.380)
        {
          tau = "(0.1527*Math.pow(10,6)*Math.pow(X,-3))-(0.2838*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(4.380>x && x>4.132)
        {
          tau = "(0.1276*Math.pow(10,6)*Math.pow(X,-3))-(0.2745*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.2136*Math.pow(10,4))*Math.pow(X,-2)+(0.1315*Math.pow(10,5))*Math.pow(X,-3)-(0.6735*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.118*Math.pow(10,-1)*X)*Math.pow((1.184*Math.pow(10,-1)+5.761*Math.pow(10,-2)*X+1.178*Math.pow(10,-2)*Math.pow(X,2)+3.863*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((97.76*Math.pow(X,-1)+6.193+0.01844*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 52: //Te
    {
        if(x>31.813)
        {
          tau = "(0.1797*Math.pow(10,7))*Math.pow(X,-3)-(0.2018*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(31.813>x && x>4.939)
        {
          tau = "(0.1161*Math.pow(10,4))*Math.pow(X,-2)+(0.1668*Math.pow(10,6))*Math.pow(X,-3)-(0.3041*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(4.939>x && x>4.612)
        {
          tau = "(0.1589*Math.pow(10,6)*Math.pow(X,-3))-(0.3071*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(4.612>x && x>4.341)
        {
          tau = "(0.1299*Math.pow(10,6)*Math.pow(X,-3))-(0.2849*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else
        {
          tau = "(0.2199*Math.pow(10,4))*Math.pow(X,-2)+(0.1381*Math.pow(10,5))*Math.pow(X,-3)-(0.7426*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.467*Math.pow(10,-1)*X)*Math.pow((1.186*Math.pow(10,-1)+6.083*Math.pow(10,-2)*X+1.258*Math.pow(10,-2)*Math.pow(X,2)+4.122*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((100.7*Math.pow(X,-1)+6.399+0.01879*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 53: //I
    {
        if(x>33.169)
        {
          tau = "(0.1960*Math.pow(10,7))*Math.pow(X,-3)-(0.2296*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(33.169>x && x>5.188)
        {
          tau = "(0.1219*Math.pow(10,4))*Math.pow(X,-2)+(0.1834*Math.pow(10,6))*Math.pow(X,-3)-(0.3505*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(5.188>x && x>4.852)
        {
          tau = "(0.1703*Math.pow(10,6)*Math.pow(X,-3))-(0.3295*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(4.852>x && x>4.557)
        {
          tau = "(0.1382*Math.pow(10,6)*Math.pow(X,-3))-(0.3053*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(4.557>x && x>1.072)
        {
          tau = "(0.2328*Math.pow(10,4))*Math.pow(X,-2)+(0.1552*Math.pow(10,5))*Math.pow(X,-3)-(0.8511*Math.pow(10,4))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1807*Math.pow(10,5))*Math.pow(X,-3)-(0.8981*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.921*Math.pow(10,-1)*X)*Math.pow((1.144*Math.pow(10,-1)+6.142*Math.pow(10,-2)*X+1.292*Math.pow(10,-2)*Math.pow(X,2)+4.238*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((98.33*Math.pow(X,-1)+6.276+0.01819*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 54: //Xe
    {
        if(x>34.564)
        {
          tau = "(0.2049*Math.pow(10,7))*Math.pow(X,-3)-(0.2505*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(34.564>x && x>5.452)
        {
          tau = "(0.1243*Math.pow(10,4))*Math.pow(X,-2)+(0.1933*Math.pow(10,6))*Math.pow(X,-3)-(0.3852*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(5.452>x && x>5.103)
        {
          tau = "(0.1787*Math.pow(10,6)*Math.pow(X,-3))-(0.3597*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.103>x && x>4.782)
        {
          tau = "(0.1715*Math.pow(10,6)*Math.pow(X,-3))-(0.3859*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(4.782>x && x>1.148)
        {
          tau = "(0.2349*Math.pow(10,4))*Math.pow(X,-2)+(0.1681*Math.pow(10,5))*Math.pow(X,-3)-(0.9660*Math.pow(10,4))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1898*Math.pow(10,5))*Math.pow(X,-3)-(0.9576*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+6.399*Math.pow(10,-1)*X)*Math.pow((1.131*Math.pow(10,-1)+6.545*Math.pow(10,-2)*X+1.382*Math.pow(10,-2)*Math.pow(X,2)+4.502*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((100.3*Math.pow(X,-1)+6.394+0.01839*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 55: //Cs
    {
        if(x>35.984)
        {
          tau = "(0.2187*Math.pow(10,7))*Math.pow(X,-3)-(0.2781*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(35.984>x && x>5.714)
        {
          tau = "(0.1285*Math.pow(10,4))*Math.pow(X,-2)+(0.2082*Math.pow(10,6))*Math.pow(X,-3)-(0.4344*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(5.714>x && x>5.359)
        {
          tau = "(0.1796*Math.pow(10,6)*Math.pow(X,-3))-(0.3318*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.359>x && x>5.011)
        {
          tau = "(0.1253*Math.pow(10,6)*Math.pow(X,-3))-(0.2100*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.011>x && x>1.217)
        {
          tau = "(0.2368*Math.pow(10,4))*Math.pow(X,-2)+(0.1871*Math.pow(10,5))*Math.pow(X,-3)-(0.1126*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.217>x && x>1.065)
        {
          tau = "(0.2142*Math.pow(10,5))*Math.pow(X,-3)-(0.1166*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1854*Math.pow(10,5))*Math.pow(X,-3)-(0.9182*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.245*Math.pow(10,-1)*X)*Math.pow((1.108*Math.pow(10,-1)+5.572*Math.pow(10,-2)*X+1.119*Math.pow(10,-2)*Math.pow(X,2)+3.552*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((99.67*Math.pow(X,-1)+6.385+0.01814*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 56: //Ba
    {
        if(x>37.440)
        {
          tau = "(0.2281*Math.pow(10,7))*Math.pow(X,-3)-(0.2993*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(37.440>x && x>5.988)
        {
          tau = "(0.1297*Math.pow(10,4))*Math.pow(X,-2)+(0.2196*Math.pow(10,6))*Math.pow(X,-3)-(0.4812*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(5.988>x && x>5.623)
        {
          tau = "(0.1930*Math.pow(10,6)*Math.pow(X,-3))-(0.3876*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.623>x && x>5.247)
        {
          tau = "(0.1411*Math.pow(10,6)*Math.pow(X,-3))-(0.2814*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.247>x && x>1.292)
        {
          tau = "(0.2397*Math.pow(10,4))*Math.pow(X,-2)+(0.2017*Math.pow(10,5))*Math.pow(X,-3)-(0.1272*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.292>x && x>1.136)
        {
          tau = "(0.2323*Math.pow(10,5))*Math.pow(X,-3)-(0.1334*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.136>x && x>1.062)
        {
          tau = "(0.1982*Math.pow(10,5))*Math.pow(X,-3)-(0.1019*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1647*Math.pow(10,5))*Math.pow(X,-3)-(0.7933*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+4.967*Math.pow(10,-1)*X)*Math.pow((1.101*Math.pow(10,-1)+5.450*Math.pow(10,-2)*X+1.055*Math.pow(10,-2)*Math.pow(X,2)+3.312*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((101.1*Math.pow(X,-1)+6.513+0.01826*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 57: //La
    {
        if(x>38.924)
        {
          tau = "(0.2430*Math.pow(10,7))*Math.pow(X,-3)-(0.3295*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(38.924>x && x>6.266)
        {
          tau = "(0.1337*Math.pow(10,4))*Math.pow(X,-2)+(0.2358*Math.pow(10,6))*Math.pow(X,-3)-(0.5375*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(6.266>x && x>5.890)
        {
          tau = "(0.2049*Math.pow(10,6)*Math.pow(X,-3))-(0.4180*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.890>x && x>5.482)
        {
          tau = "(0.1487*Math.pow(10,6)*Math.pow(X,-3))-(0.2982*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.482>x && x>1.361)
        {
          tau = "(0.2447*Math.pow(10,4))*Math.pow(X,-2)+(0.2229*Math.pow(10,5))*Math.pow(X,-3)-(0.1474*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.361>x && x>1.204)
        {
          tau = "(0.2551*Math.pow(10,5))*Math.pow(X,-3)-(0.1536*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.204>x && x>1.123)
        {
          tau = "(0.2165*Math.pow(10,5))*Math.pow(X,-3)-(0.1155*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1674*Math.pow(10,5))*Math.pow(X,-3)-(0.7655*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.818*Math.pow(10,-1)*X)*Math.pow((1.076*Math.pow(10,-1)+6.162*Math.pow(10,-2)*X+1.187*Math.pow(10,-2)*Math.pow(X,2)+3.746*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((101.4*Math.pow(X,-1)+6.499+0.01804*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 58: //Ce
    {
        if(x>40.443)
        {
          tau = "(0.2601*Math.pow(10,7))*Math.pow(X,-3)-(0.3722*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(40.443>x && x>6.548)
        {
          tau = "(0.1342*Math.pow(10,4))*Math.pow(X,-2)+(0.2540*Math.pow(10,6))*Math.pow(X,-3)-(0.6049*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(6.548>x && x>6.164)
        {
          tau = "(0.2237*Math.pow(10,6)*Math.pow(X,-3))-(0.4912*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(6.164>x && x>5.723)
        {
          tau = "(0.1597*Math.pow(10,6)*Math.pow(X,-3))-(0.3306*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.723>x && x>1.434)
        {
          tau = "(0.2476*Math.pow(10,4))*Math.pow(X,-2)+(0.2440*Math.pow(10,5))*Math.pow(X,-3)-(0.1673*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.434>x && x>1.272)
        {
          tau = "(0.2770*Math.pow(10,5))*Math.pow(X,-3)-(0.1729*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.272>x && x>1.185)
        {
          tau = "(0.2335*Math.pow(10,5))*Math.pow(X,-3)-(0.1278*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1737*Math.pow(10,5))*Math.pow(X,-3)-(0.7655*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+5.099*Math.pow(10,-1)*X)*Math.pow((1.048*Math.pow(10,-1)+5.311*Math.pow(10,-2)*X+1.027*Math.pow(10,-2)*Math.pow(X,2)+3.148*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((99.48*Math.pow(X,-1)+6.484+0.01768*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 59: //Pr
    {
        if(x>41.990)
        {
          tau = "(0.2783*Math.pow(10,7))*Math.pow(X,-3)-(0.4146*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(41.990>x && x>6.834)
        {
          tau = "(0.1414*Math.pow(10,4))*Math.pow(X,-2)+(0.2730*Math.pow(10,6))*Math.pow(X,-3)-(0.6747*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(6.834>x && x>6.440)
        {
          tau = "(0.2411*Math.pow(10,6)*Math.pow(X,-3))-(0.5525*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(6.440>x && x>5.964)
        {
          tau = "(0.1669*Math.pow(10,6)*Math.pow(X,-3))-(0.4316*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(5.964>x && x>1.511)
        {
          tau = "(0.2604*Math.pow(10,4))*Math.pow(X,-2)+(0.2636*Math.pow(10,5))*Math.pow(X,-3)-(0.1858*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.511>x && x>1.337)
        {
          tau = "(0.3020*Math.pow(10,5))*Math.pow(X,-3)-(0.1959*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.337>x && x>1.242)
        {
          tau = "(0.2562*Math.pow(10,5))*Math.pow(X,-3)-(0.1463*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.1784*Math.pow(10,5))*Math.pow(X,-3)-(0.7266*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+3.925*Math.pow(10,-1)*X)*Math.pow((1.008*Math.pow(10,-1)+4.051*Math.pow(10,-2)*X+7.917*Math.pow(10,-3)*Math.pow(X,2)+2.310*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((100.1*Math.pow(X,-1)+6.405+0.01752*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 60: //Nd
    {
        if(x>43.568)
        {
          tau = "(0.2920*Math.pow(10,7))*Math.pow(X,-3)-(0.4524*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(43.568>x && x>7.126)
        {
          tau = "(0.1465*Math.pow(10,4))*Math.pow(X,-2)+(0.2880*Math.pow(10,6))*Math.pow(X,-3)-(0.7393*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(7.126>x && x>6.721)
        {
          tau = "(0.2525*Math.pow(10,6)*Math.pow(X,-3))-(0.5923*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(6.721>x && x>6.207)
        {
          tau = "(0.1750*Math.pow(10,6)*Math.pow(X,-3))-(0.3669*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(6.207>x && x>1.575)
        {
          tau = "(0.2581*Math.pow(10,4))*Math.pow(X,-2)+(0.2857*Math.pow(10,5))*Math.pow(X,-3)-(0.2112*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.575>x && x>1.402)
        {
          tau = "(0.3222*Math.pow(10,5))*Math.pow(X,-3)-(0.2164*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.402>x && x>1.297)
        {
          tau = "(0.2732*Math.pow(10,5))*Math.pow(X,-3)-(0.1612*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.2017*Math.pow(10,5))*Math.pow(X,-3)-(0.9345*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+3.511*Math.pow(10,-1)*X)*Math.pow((1.004*Math.pow(10,-1)+3.634*Math.pow(10,-2)*X+7.104*Math.pow(10,-3)*Math.pow(X,2)+2.012*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((101.9*Math.pow(X,-1)+6.457+0.01757*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 61: //Pm
    {
        if(x>45.184)
        {
          tau = "(0.3115*Math.pow(10,7))*Math.pow(X,-3)-(0.5004*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(45.184>x && x>7.427)
        {
          tau = "(0.1538*Math.pow(10,4))*Math.pow(X,-2)+(0.3092*Math.pow(10,6))*Math.pow(X,-3)-(0.8237*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(7.427>x && x>7.012)
        {
          tau = "(0.2740*Math.pow(10,6)*Math.pow(X,-3))-(0.6806*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(7.012>x && x>6.459)
        {
          tau = "(0.1852*Math.pow(10,6)*Math.pow(X,-3))-(0.3906*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(6.459>x && x>1.650)
        {
          tau = "(0.2665*Math.pow(10,4))*Math.pow(X,-2)+(0.3111*Math.pow(10,5))*Math.pow(X,-3)-(0.2388*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.650>x && x>1.471)
        {
          tau = "(0.3519*Math.pow(10,5))*Math.pow(X,-3)-(0.2471*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.471>x && x>1.356)
        {
          tau = "(0.2907*Math.pow(10,5))*Math.pow(X,-3)-(0.1728*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.356>x && x>1.051)
        {
          tau = "(0.2165*Math.pow(10,5))*Math.pow(X,-3)-(0.1022*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.051>x && x>1.026)
        {
          tau = "(0.1631*Math.pow(10,5))*Math.pow(X,-3)-(0.8906*Math.pow(10,4))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.4450*Math.pow(10,4))*Math.pow(X,-3)-(0.2405*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+3.211*Math.pow(10,-1)*X)*Math.pow((9.763*Math.pow(10,-2)+3.251*Math.pow(10,-2)*X+6.383*Math.pow(10,-3)*Math.pow(X,2)+1.763*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((105.4*Math.pow(X,-1)+6.380+0.01743*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 62: //Sm
    {
        if(x>46.834)
        {
          tau = "(0.3211*Math.pow(10,7))*Math.pow(X,-3)-(0.5229*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(46.834>x && x>7.736)
        {
          tau = "(0.1577*Math.pow(10,4))*Math.pow(X,-2)+(0.3207*Math.pow(10,6))*Math.pow(X,-3)-(0.8830*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(7.736>x && x>7.311)
        {
          tau = "(0.2858*Math.pow(10,6)*Math.pow(X,-3))-(0.7415*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(7.311>x && x>6.716)
        {
          tau = "(0.1911*Math.pow(10,6)*Math.pow(X,-3))-(0.4109*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(6.716>x && x>1.722)
        {
          tau = "(0.2669*Math.pow(10,4))*Math.pow(X,-2)+(0.3281*Math.pow(10,5))*Math.pow(X,-3)-(0.2613*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.722>x && x>1.540)
        {
          tau = "(0.3683*Math.pow(10,5))*Math.pow(X,-3)-(0.2662*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.540>x && x>1.419)
        {
          tau = "(0.3069*Math.pow(10,5))*Math.pow(X,-3)-(0.1892*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.419>x && x>1.106)
        {
          tau = "(0.2294*Math.pow(10,5))*Math.pow(X,-3)-(0.1111*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.106>x && x>1.080)
        {
          tau = "(0.2482*Math.pow(10,5))*Math.pow(X,-3)-(0.1799*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.4747*Math.pow(10,4))*Math.pow(X,-3)-(0.2650*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.933*Math.pow(10,-1)*X)*Math.pow((9.671*Math.pow(10,-2)+3.015*Math.pow(10,-2)*X+5.914*Math.pow(10,-3)*Math.pow(X,2)+1.593*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((106.4*Math.pow(X,-1)+6.500+0.01785*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 63: //Eu
    {
        if(x>48.519)
        {
          tau = "(0.3395*Math.pow(10,7))*Math.pow(X,-3)-(0.5788*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(48.519>x && x>8.052)
        {
          tau = "(0.1624*Math.pow(10,4))*Math.pow(X,-2)+(0.3418*Math.pow(10,6))*Math.pow(X,-3)-(0.9797*Math.pow(10,6))*Math.pow(X,-4)";
        }
        else if(8.052>x && x>7.617)
        {
          tau = "(0.3033*Math.pow(10,6)*Math.pow(X,-3))-(0.8102*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(7.617>x && x>6.976)
        {
          tau = "(0.2032*Math.pow(10,6)*Math.pow(X,-3))-(0.4513*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(6.976>x && x>1.800)
        {
          tau = "(0.2738*Math.pow(10,4))*Math.pow(X,-2)+(0.3541*Math.pow(10,5))*Math.pow(X,-3)-(0.2922*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.800>x && x>1.613)
        {
          tau = "(0.3963*Math.pow(10,5))*Math.pow(X,-3)-(0.2965*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.613>x && x>1.480)
        {
          tau = "(0.3349*Math.pow(10,5))*Math.pow(X,-3)-(0.2175*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.480>x && x>1.160)
        {
          tau = "(0.2465*Math.pow(10,5))*Math.pow(X,-3)-(0.1228*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.160>x && x>1.130)
        {
          tau = "(0.2457*Math.pow(10,5))*Math.pow(X,-3)-(0.1767*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.5084*Math.pow(10,4))*Math.pow(X,-3)-(0.2878*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.696*Math.pow(10,-1)*X)*Math.pow((9.526*Math.pow(10,-2)+2.778*Math.pow(10,-2)*X+5.372*Math.pow(10,-3)*Math.pow(X,2)+1.403*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((107.6*Math.pow(X,-1)+6.459+0.01777*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 64: //Gd
    {
        if(x>50.239)
        {
          tau = "(0.3516*Math.pow(10,7))*Math.pow(X,-3)-(0.6283*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(50.239>x && x>8.375)
        {
          tau = "(0.1604*Math.pow(10,4))*Math.pow(X,-2)+(0.3565*Math.pow(10,6))*Math.pow(X,-3)-(0.1065*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(8.375>x && x>7.930)
        {
          tau = "(0.3168*Math.pow(10,6)*Math.pow(X,-3))-(0.8859*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(7.930>x && x>7.242)
        {
          tau = "(0.2087*Math.pow(10,6)*Math.pow(X,-3))-(0.4667*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(7.242>x && x>1.880)
        {
          tau = "(0.2739*Math.pow(10,4))*Math.pow(X,-2)+(0.3729*Math.pow(10,5))*Math.pow(X,-3)-(0.3185*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.880>x && x>1.688)
        {
          tau = "(0.4162*Math.pow(10,5))*Math.pow(X,-3)-(0.3223*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.688>x && x>1.544)
        {
          tau = "(0.3488*Math.pow(10,5))*Math.pow(X,-3)-(0.2303*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.544>x && x>1.217)
        {
          tau = "(0.2554*Math.pow(10,5))*Math.pow(X,-3)-(0.1278*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.217>x && x>1.185)
        {
          tau = "(0.2413*Math.pow(10,5))*Math.pow(X,-3)-(0.1727*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.5397*Math.pow(10,4))*Math.pow(X,-3)-(0.3116*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.652*Math.pow(10,-1)*X)*Math.pow((9.606*Math.pow(10,-2)+2.706*Math.pow(10,-2)*X+5.275*Math.pow(10,-3)*Math.pow(X,2)+1.377*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((110.9*Math.pow(X,-1)+6.581+0.01813*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 65: //Tb
    {
        if(x>51.995)
        {
          tau = "(0.3722*Math.pow(10,7))*Math.pow(X,-3)-(0.6917*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(51.995>x && x>8.708)
        {
          tau = "(0.1689*Math.pow(10,4))*Math.pow(X,-2)+(0.3777*Math.pow(10,6))*Math.pow(X,-3)-(0.1166*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(8.708>x && x>8.252)
        {
          tau = "(0.3376*Math.pow(10,6)*Math.pow(X,-3))-(0.9837*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(8.252>x && x>7.514)
        {
          tau = "(0.2228*Math.pow(10,6)*Math.pow(X,-3))-(0.5205*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(7.514>x && x>1.967)
        {
          tau = "(0.2817*Math.pow(10,4))*Math.pow(X,-2)+(0.3977*Math.pow(10,5))*Math.pow(X,-3)-(0.3486*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.967>x && x>1.767)
        {
          tau = "(0.4480*Math.pow(10,5))*Math.pow(X,-3)-(0.3608*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.767>x && x>1.611)
        {
          tau = "(0.3706*Math.pow(10,5))*Math.pow(X,-3)-(0.2495*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.611>x && x>1.275)
        {
          tau = "(0.2716*Math.pow(10,5))*Math.pow(X,-3)-(0.1378*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.275>x && x>1.241)
        {
          tau = "(0.3316*Math.pow(10,5))*Math.pow(X,-3)-(0.2830*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.5667*Math.pow(10,4))*Math.pow(X,-3)-(0.3281*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.296*Math.pow(10,-1)*X)*Math.pow((9.451*Math.pow(10,-2)+2.306*Math.pow(10,-2)*X+4.584*Math.pow(10,-3)*Math.pow(X,2)+1.147*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((111.8*Math.pow(X,-1)+6.552+0.01804*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 66: //Dy
    {
        if(x>53.788)
        {
          tau = "(0.3884*Math.pow(10,7))*Math.pow(X,-3)-(0.7467*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(53.788>x && x>9.045)
        {
          tau = "(0.1751*Math.pow(10,4))*Math.pow(X,-2)+(0.3955*Math.pow(10,6))*Math.pow(X,-3)-(0.1264*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(9.045>x && x>8.580)
        {
          tau = "(0.3553*Math.pow(10,6)*Math.pow(X,-3))-(0.1079*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(8.580>x && x>7.790)
        {
          tau = "(0.2253*Math.pow(10,6)*Math.pow(X,-3))-(0.4925*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(7.790>x && x>2.046)
        {
          tau = "(0.2845*Math.pow(10,4))*Math.pow(X,-2)+(0.4216*Math.pow(10,5))*Math.pow(X,-3)-(0.3822*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.046>x && x>1.841)
        {
          tau = "(0.4729*Math.pow(10,5))*Math.pow(X,-3)-(0.3925*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.841>x && x>1.675)
        {
          tau = "(0.3926*Math.pow(10,5))*Math.pow(X,-3)-(0.2725*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.675>x && x>1.333)
        {
          tau = "(0.2916*Math.pow(10,5))*Math.pow(X,-3)-(0.1554*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.333>x && x>1.294)
        {
          tau = "(0.3557*Math.pow(10,5))*Math.pow(X,-3)-(0.3166*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.5996*Math.pow(10,4))*Math.pow(X,-3)-(0.3512*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.124*Math.pow(10,-1)*X)*Math.pow((9.341*Math.pow(10,-2)+2.171*Math.pow(10,-2)*X+4.244*Math.pow(10,-3)*Math.pow(X,2)+1.030*Math.pow(10,-4)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((114.9*Math.pow(X,-1)+6.580+0.01825*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 67: //Ho
    {
        if(x>55.617)
        {
          tau = "(0.4073*Math.pow(10,7))*Math.pow(X,-3)-(0.8046*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(55.617>x && x>9.394)
        {
          tau = "(0.1829*Math.pow(10,4))*Math.pow(X,-2)+(0.4165*Math.pow(10,6))*Math.pow(X,-3)-(0.1375*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(9.394>x && x>8.917)
        {
          tau = "(0.3764*Math.pow(10,6)*Math.pow(X,-3))-(0.1189*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(8.917>x && x>8.071)
        {
          tau = "(0.2431*Math.pow(10,6)*Math.pow(X,-3))-(0.5884*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(8.071>x && x>2.128)
        {
          tau = "(0.2850*Math.pow(10,4))*Math.pow(X,-2)+(0.4527*Math.pow(10,5))*Math.pow(X,-3)-(0.4260*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.128>x && x>1.923)
        {
          tau = "(0.5025*Math.pow(10,5))*Math.pow(X,-3)-(0.4307*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.923>x && x>1.741)
        {
          tau = "(0.4157*Math.pow(10,5))*Math.pow(X,-3)-(0.2945*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.741>x && x>1.391)
        {
          tau = "(0.3164*Math.pow(10,5))*Math.pow(X,-3)-(0.1788*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.391>x && x>1.351)
        {
          tau = "(0.3209*Math.pow(10,5))*Math.pow(X,-3)-(0.2708*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.6385*Math.pow(10,4))*Math.pow(X,-3)-(0.3779*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+2.069*Math.pow(10,-1)*X)*Math.pow((9.129*Math.pow(10,-2)+2.144*Math.pow(10,-2)*X+4.057*Math.pow(10,-3)*Math.pow(X,2)+9.736*Math.pow(10,-5)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((117*Math.pow(X,-1)+6.569+0.01831*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 68: //Er
    {
        if(x>57.185)
        {
          tau = "(0.4267*Math.pow(10,7))*Math.pow(X,-3)-(0.8622*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(57.185>x && x>9.751)
        {
          tau = "(0.1910*Math.pow(10,4))*Math.pow(X,-2)+(0.4384*Math.pow(10,6))*Math.pow(X,-3)-(0.1492*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(9.751>x && x>9.264)
        {
          tau = "(0.3982*Math.pow(10,6)*Math.pow(X,-3))-(0.1307*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(9.264>x && x>8.357)
        {
          tau = "(0.2558*Math.pow(10,6)*Math.pow(X,-3))-(0.6354*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(8.357>x && x>2.206)
        {
          tau = "(0.2931*Math.pow(10,4))*Math.pow(X,-2)+(0.4801*Math.pow(10,5))*Math.pow(X,-3)-(0.4657*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.206>x && x>2.005)
        {
          tau = "(0.5830*Math.pow(10,5))*Math.pow(X,-3)-(0.4791*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.005>x && x>1.811)
        {
          tau = "(0.4770*Math.pow(10,5))*Math.pow(X,-3)-(0.3852*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.811>x && x>1.453)
        {
          tau = "(0.3413*Math.pow(10,5))*Math.pow(X,-3)-(0.2030*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.453>x && x>1.409)
        {
          tau = "(0.3585*Math.pow(10,5))*Math.pow(X,-3)-(0.3235*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.6797*Math.pow(10,4))*Math.pow(X,-3)-(0.4059*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.858*Math.pow(10,-1)*X)*Math.pow((9.058*Math.pow(10,-2)+1.863*Math.pow(10,-2)*X+3.656*Math.pow(10,-3)*Math.pow(X,2)+8.480*Math.pow(10,-5)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((118.9*Math.pow(X,-1)+6.556+0.01835*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 69: //Tm
    {
        if(x>59.389)
        {
          tau = "(0.4493*Math.pow(10,7))*Math.pow(X,-3)-(0.9392*Math.pow(10,8))*Math.pow(X,-4)";
        }
        else if(59.389>x && x>10.115)
        {
          tau = "(0.1980*Math.pow(10,4))*Math.pow(X,-2)+(0.4654*Math.pow(10,6))*Math.pow(X,-3)-(0.1653*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(10.115>x && x>9.616)
        {
          tau = "(0.4232*Math.pow(10,6)*Math.pow(X,-3))-(0.1447*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(9.616>x && x>8.648)
        {
          tau = "(0.2705*Math.pow(10,6)*Math.pow(X,-3))-(0.6928*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(8.648>x && x>2.306)
        {
          tau = "(0.3024*Math.pow(10,4))*Math.pow(X,-2)+(0.5105*Math.pow(10,5))*Math.pow(X,-3)-(0.5103*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.306>x && x>2.089)
        {
          tau = "(0.5721*Math.pow(10,5))*Math.pow(X,-3)-(0.5235*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.089>x && x>1.884)
        {
          tau = "(0.4704*Math.pow(10,5))*Math.pow(X,-3)-(0.3516*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.884>x && x>1.514)
        {
          tau = "(0.3588*Math.pow(10,5))*Math.pow(X,-3)-(0.2150*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.514>x && x>1.467)
        {
          tau = "(0.4367*Math.pow(10,5))*Math.pow(X,-3)-(0.4388*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.7237*Math.pow(10,4))*Math.pow(X,-3)-(0.4349*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.732*Math.pow(10,-1)*X)*Math.pow((8.885*Math.pow(10,-2)+1.718*Math.pow(10,-2)*X+3.374*Math.pow(10,-3)*Math.pow(X,2)+7.638*Math.pow(10,-5)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((120.9*Math.pow(X,-1)+6.504+0.01839*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 70: //Yb
    {
        if(x>61.332)
        {
          tau = "(0.4675*Math.pow(10,7))*Math.pow(X,-3)-(0.1022*Math.pow(10,9))*Math.pow(X,-4)";
        }
        else if(61.332>x && x>10.486)
        {
          tau = "(0.1995*Math.pow(10,4))*Math.pow(X,-2)+(0.4866*Math.pow(10,6))*Math.pow(X,-3)-(0.1796*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(10.486>x && x>9.978)
        {
          tau = "(0.4552*Math.pow(10,6)*Math.pow(X,-3))-(0.1700*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(9.978>x && x>8.943)
        {
          tau = "(0.2817*Math.pow(10,6)*Math.pow(X,-3))-(0.7422*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(8.943>x && x>2.398)
        {
          tau = "(0.3077*Math.pow(10,4))*Math.pow(X,-2)+(0.5341*Math.pow(10,5))*Math.pow(X,-3)-(0.5486*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.398>x && x>2.173)
        {
          tau = "(0.6041*Math.pow(10,5))*Math.pow(X,-3)-(0.5743*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.173>x && x>1.949)
        {
          tau = "(0.4972*Math.pow(10,5))*Math.pow(X,-3)-(0.3864*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.949>x && x>1.576)
        {
          tau = "(0.3790*Math.pow(10,5))*Math.pow(X,-3)-(0.2362*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.576>x && x>1.527)
        {
          tau = "(0.4881*Math.pow(10,5))*Math.pow(X,-3)-(0.5234*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.7585*Math.pow(10,4))*Math.pow(X,-3)-(0.4579*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.621*Math.pow(10,-1)*X)*Math.pow((8.842*Math.pow(10,-2)+1.614*Math.pow(10,-2)*X+3.170*Math.pow(10,-3)*Math.pow(X,2)+7.012*Math.pow(10,-5)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((124.5*Math.pow(X,-1)+6.552+0.01864*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 71: //Lu
    {
        if(x>63.313)
        {
          tau = "(0.4915*Math.pow(10,7))*Math.pow(X,-3)-(0.1111*Math.pow(10,9))*Math.pow(X,-4)";
        }
        else if(63.313>x && x>10.870)
        {
          tau = "(0.2072*Math.pow(10,4))*Math.pow(X,-2)+(0.5147*Math.pow(10,6))*Math.pow(X,-3)-(0.1973*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(10.870>x && x>10.348)
        {
          tau = "(0.4696*Math.pow(10,6)*Math.pow(X,-3))-(0.1735*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(10.348>x && x>9.244)
        {
          tau = "(0.2954*Math.pow(10,6)*Math.pow(X,-3))-(0.7884*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(9.244>x && x>2.491)
        {
          tau = "(0.3152*Math.pow(10,4))*Math.pow(X,-2)+(0.5693*Math.pow(10,5))*Math.pow(X,-3)-(0.6051*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.491>x && x>2.263)
        {
          tau = "(0.6398*Math.pow(10,5))*Math.pow(X,-3)-(0.6231*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.263>x && x>2.023)
        {
          tau = "(0.5265*Math.pow(10,5))*Math.pow(X,-3)-(0.4152*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.023>x && x>1.639)
        {
          tau = "(0.4112*Math.pow(10,5))*Math.pow(X,-3)-(0.2732*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.639>x && x>1.588)
        {
          tau = "(0.3893*Math.pow(10,5))*Math.pow(X,-3)-(0.3686*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.8083*Math.pow(10,4))*Math.pow(X,-3)-(0.4909*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.530*Math.pow(10,-1)*X)*Math.pow((8.663*Math.pow(10,-2)+1.546*Math.pow(10,-2)*X+2.948*Math.pow(10,-3)*Math.pow(X,2)+6.442*Math.pow(10,-5)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((125.9*Math.pow(X,-1)+6.529+0.01861*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 72: //Hf
    {
        if(x>65.350)
        {
          tau = "(0.5124*Math.pow(10,7))*Math.pow(X,-3)-(0.1202*Math.pow(10,9))*Math.pow(X,-4)";
        }
        else if(65.350>x && x>11.270)
        {
          tau = "(0.2144*Math.pow(10,4))*Math.pow(X,-2)+(0.5374*Math.pow(10,6))*Math.pow(X,-3)-(0.2122*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(11.270>x && x>10.739)
        {
          tau = "(0.4976*Math.pow(10,6)*Math.pow(X,-3))-(0.1946*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(10.739>x && x>9.560)
        {
          tau = "(0.3042*Math.pow(10,6)*Math.pow(X,-3))-(0.7954*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(9.560>x && x>2.6)
        {
          tau = "(0.3188*Math.pow(10,4))*Math.pow(X,-2)+(0.6025*Math.pow(10,5))*Math.pow(X,-3)-(0.6630*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.6>x && x>2.365)
        {
          tau = "(0.6774*Math.pow(10,5))*Math.pow(X,-3)-(0.6845*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.365>x && x>2.107)
        {
          tau = "(0.5555*Math.pow(10,5))*Math.pow(X,-3)-(0.4507*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.107>x && x>1.716)
        {
          tau = "(0.4482*Math.pow(10,5))*Math.pow(X,-3)-(0.3245*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.716>x && x>1.661)
        {
          tau = "(0.4261*Math.pow(10,5))*Math.pow(X,-3)-(0.4287*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.8547*Math.pow(10,4))*Math.pow(X,-3)-(0.5228*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.510*Math.pow(10,-1)*X)*Math.pow((8.543*Math.pow(10,-2)+1.524*Math.pow(10,-2)*X+2.866*Math.pow(10,-3)*Math.pow(X,2)+6.275*Math.pow(10,-5)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((128.4*Math.pow(X,-1)+6.563+0.01876*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 73: //Ta
    {
        if(x>67.416)
        {
          tau = "(0.5366*Math.pow(10,7))*Math.pow(X,-3)-(0.1303*Math.pow(10,9))*Math.pow(X,-4)";
        }
        else if(67.416>x && x>11.681)
        {
          tau = "(0.2209*Math.pow(10,4))*Math.pow(X,-2)+(0.5658*Math.pow(10,6))*Math.pow(X,-3)-(0.2321*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(11.681>x && x>11.136)
        {
          tau = "(0.5262*Math.pow(10,6)*Math.pow(X,-3))-(0.2149*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(11.136>x && x>9.881)
        {
          tau = "(0.3165*Math.pow(10,6)*Math.pow(X,-3))-(0.8344*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(9.881>x && x>2.708)
        {
          tau = "(0.3243*Math.pow(10,4))*Math.pow(X,-2)+(0.6393*Math.pow(10,5))*Math.pow(X,-3)-(0.7252*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.708>x && x>2.468)
        {
          tau = "(0.7168*Math.pow(10,5))*Math.pow(X,-3)-(0.7459*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.468>x && x>2.194)
        {
          tau = "(0.6294*Math.pow(10,5))*Math.pow(X,-3)-(0.5814*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.194>x && x>1.793)
        {
          tau = "(0.4761*Math.pow(10,5))*Math.pow(X,-3)-(0.3533*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.793>x && x>1.735)
        {
          tau = "(0.5234*Math.pow(10,5))*Math.pow(X,-3)-(0.5955*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.2539*Math.pow(10,4))*Math.pow(X,-2)+(0.2405*Math.pow(10,4))*Math.pow(X,-3)-(0.1445*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.529*Math.pow(10,-1)*X)*Math.pow((8.421*Math.pow(10,-2)+1.531*Math.pow(10,-2)*X+2.825*Math.pow(10,-3)*Math.pow(X,2)+6.203*Math.pow(10,-5)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((130.3*Math.pow(X,-1)+6.552+0.01884*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 74: //W
    {
        if(x>69.525)
        {
          tau = "(0.5609*Math.pow(10,7))*Math.pow(X,-3)-(0.1409*Math.pow(10,9))*Math.pow(X,-4)";
        }
        else if(69.525>x && x>12.099)
        {
          tau = "(0.2293*Math.pow(10,4))*Math.pow(X,-2)+(0.5922*Math.pow(10,6))*Math.pow(X,-3)-(0.2501*Math.pow(10,7))*Math.pow(X,-4)";
        }
        else if(12.099>x && x>11.544)
        {
          tau = "(0.5357*Math.pow(10,6)*Math.pow(X,-3))-(0.2132*Math.pow(10,7)*Math.pow(X,-4))";
        }
        else if(11.544>x && x>10.206)
        {
          tau = "(0.3312*Math.pow(10,6)*Math.pow(X,-3))-(0.8942*Math.pow(10,6)*Math.pow(X,-4))";
        }
        else if(10.206>x && x>2.819)
        {
          tau = "(0.3533*Math.pow(10,4))*Math.pow(X,-2)+(0.6433*Math.pow(10,5))*Math.pow(X,-3)-(0.6929*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.819>x && x>2.574)
        {
          tau = "(0.7600*Math.pow(10,5))*Math.pow(X,-3)-(0.8179*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.574>x && x>2.281)
        {
          tau = "(0.6324*Math.pow(10,5))*Math.pow(X,-3)-(0.5576*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(2.281>x && x>1.871)
        {
          tau = "(0.6520*Math.pow(10,5))*Math.pow(X,-3)-(0.7006*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else if(1.871>x && x>1.809)
        {
          tau = "(0.4760*Math.pow(10,5))*Math.pow(X,-3)-(0.5122*Math.pow(10,5))*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.2594*Math.pow(10,4))*Math.pow(X,-2)+(0.2704*Math.pow(10,4))*Math.pow(X,-3)-(0.1624*Math.pow(10,4))*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.571*Math.pow(10,-1)*X)*Math.pow((8.324*Math.pow(10,-2)+1.526*Math.pow(10,-2)*X+2.825*Math.pow(10,-3)*Math.pow(X,2)+6.277*Math.pow(10,-5)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((126.1*Math.pow(X,-1)+6.724+0.01805*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    }
}

double MainWindow::oslableniyeEnv(double x)
{
    QString tau;

    QString sigma;
    QString sigmaK;
    QString sigmaNK;

    QScriptEngine engine;

    QScriptValue tauValue;
    QScriptValue sigmaValue;
    QScriptValue sigmaKValue;
    QScriptValue sigmaNKValue;

    switch(ui->materialEnv->currentIndex())
    {
    case 0: //Air
        {
            if(x<3.203)
            {
                tau = "-(0.1991*Math.pow(10,-2))*X+(0.1169)*Math.pow(X,-1)-(0.1514*100)*Math.pow(X,-2)+(0.4759*10000)*Math.pow(X,-3)-(0.9060*1000)*Math.pow(X,-4)";
            }
            else
            {
               tau = "-(0.3514*Math.pow(10,2))*Math.pow(X,-2)+(0.5059*10000)*Math.pow(X,-3)-(0.1060*1000)*Math.pow(X,-4)+Math.pow((26.44*Math.pow(X,-1)+4.724+0.01809*X),-1)";
            }
            tau.replace("X", QString::number(x));
            tauValue = engine.evaluate(tau);
            weakening = tauValue.toNumber();
            return weakening;
            break;
        }
    case 1: //H
        {
            tau = "-(0.2645*Math.pow(10,-2))*X+(0.7086*0.1)*Math.pow(X,-1)-(0.7487)*Math.pow(X,-2)+(0.5575*10)*Math.pow(X,-3)-(-0.1936*10)*Math.pow(X,-4)";
            sigma = "0.4005*(1/1.008)*Math.pow((1+2*X/511),-2)*(1+2*X/511+0.3*Math.pow(2*X/511,2)-0.0625*Math.pow(2*X/511,3))";
            tau.replace("X", QString::number(x));
            sigma.replace("X", QString::number(x));
            tauValue = engine.evaluate(tau);
            sigmaValue = engine.evaluate(sigma);
            weakening = tauValue.toNumber()+sigmaValue.toNumber();
            return weakening;
            break;
        }
    case 2: //He
        {
            tau = "-(0.2154*Math.pow(10,-2))*X+(0.1473)*Math.pow(X,-1)-(0.3322*10)*Math.pow(X,-2)+(0.4893*100)*Math.pow(X,-3)-(-0.1576*100)*Math.pow(X,-4)";
            sigma = "0.4005*(1/4.003)*Math.pow(1+2*X/511,-2)*(1+2*X/511+0.3*Math.pow(2*X/511,2)-0.0625*Math.pow(2*X/511,3))";
            tau.replace("X", QString::number(x));
            sigma.replace("X", QString::number(x));
            tauValue = engine.evaluate(tau);
            sigmaValue = engine.evaluate(sigma);
            weakening = tauValue.toNumber()+sigmaValue.toNumber();
            return weakening;
            break;
        }
    case 3: //Li
        {
            tau = "-(0.3411*Math.pow(10,-2))*X+(0.3088)*Math.pow(X,-1)-(0.1009*100)*Math.pow(X,-2)+(0.2076*1000)*Math.pow(X,-3)-(-0.4091*100)*Math.pow(X,-4)";
            sigmaK = "(1+9.326*Math.pow(10,-2)*X)*Math.pow((1.781+8.725*0.1*X+7.963*0.01*Math.pow(X,2)+8.225*0.001*Math.pow(X,3)),-1)";
            sigmaNK = "Math.pow((29.94*Math.pow(X,-1)+4.533+0.03637*X),-1)";
            tau.replace("X", QString::number(x));
            sigmaK.replace("X", QString::number(x));
            sigmaNK.replace("X", QString::number(x));
            tauValue = engine.evaluate(tau);
            sigmaKValue = engine.evaluate(sigmaK);
            sigmaNKValue = engine.evaluate(sigmaNK);
            weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
            return weakening;
            break;
        }
    case 4: //Be
    {
        tau = "-(0.3142*Math.pow(10,-2))*X+(0.4216)*Math.pow(X,-1)-(0.2014*100)*Math.pow(X,-2)+(0.5918*1000)*Math.pow(X,-3)-(-0.4857*100)*Math.pow(X,-4)";
        sigmaK = "(1-3.178*Math.pow(10,-2)*X)*Math.pow((1.267+4.619*0.1*X+3.102*0.01*Math.pow(X,2)-1.493*0.001*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((25.93*Math.pow(X,-1)+5.067+0.02420*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 5: //B
    {
        tau = "-(0.3267*Math.pow(10,-2))*X+(0.5682)*Math.pow(X,-1)-(0.3483*100)*Math.pow(X,-2)+(0.1326*10000)*Math.pow(X,-3)-(0.3243*100)*Math.pow(X,-4)";
        sigmaK = "(1+1.544*X)*Math.pow((1.010+1.561*X+6.978*0.1*Math.pow(X,2)+5.025*0.01*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((25.58*Math.pow(X,-1)+4.945+0.02191*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 6: //C
    {
        tau = "-(0.3172*Math.pow(10,-2))*X+(0.6921)*Math.pow(X,-1)-(0.5340*100)*Math.pow(X,-2)+(0.2610*10000)*Math.pow(X,-3)-(0.2941*1000)*Math.pow(X,-4)";
        sigmaK = "(1+1.108*X)*Math.pow((8.093*0.1+7.378*0.1*X+3.958*0.1*Math.pow(X,2)+2.532*0.01*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((25.91*Math.pow(X,-1)+4.644+0.01878*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 7: //N
    {
        tau = "-(0.1991*Math.pow(10,-2))*X+(0.6169)*Math.pow(X,-1)-(0.6514*100)*Math.pow(X,-2)+(0.4259*10000)*Math.pow(X,-3)-(0.8060*1000)*Math.pow(X,-4)";
        sigmaK = "(1+5.723*0.1*X)*Math.pow((7.260*0.1+2.910*0.1*X+1.824*0.1*Math.pow(X,2)+1.018*0.1*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((26.44*Math.pow(X,-1)+4.724+0.01809*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 8: //O
    {
        tau = "-(0.2663*Math.pow(10,-2))*X+(0.8397)*Math.pow(X,-1)-(0.9179*100)*Math.pow(X,-2)+(0.6634*10000)*Math.pow(X,-3)-(0.1906*10000)*Math.pow(X,-4)";
        sigmaK = "(1+3.537*0.1*X)*Math.pow((6.396*0.1+1.543*0.1*X+9.948*0.01*Math.pow(X,2)+4.981*0.001*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((29.88*Math.pow(X,-1)+4.656+0.01857*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 9: //F
    {
        tau = "-(0.3038*Math.pow(10,-2))*X+(0.9836)*Math.pow(X,-1)-(0.1128*1000)*Math.pow(X,-2)+(0.9171*10000)*Math.pow(X,-3)-(0.3414*10000)*Math.pow(X,-4)";
        sigmaK = "(1+2.437*0.1*X)*Math.pow((5.970*0.1+9.989*0.01*X+6.409*0.01*Math.pow(X,2)+2.930*0.001*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((33.40*Math.pow(X,-1)+4.961+0.01913*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 10: //Ne
    {
        tau = "-(0.1806*Math.pow(10,-2))*X+(0.7942)*Math.pow(X,-1)-(0.1218*1000)*Math.pow(X,-2)+(0.1307*100000)*Math.pow(X,-3)-(0.5600*10000)*Math.pow(X,-4)";
        sigmaK = "(1+1.820*0.1*X)*Math.pow((5.118*0.1+6.431*0.01*X+4.065*0.01*Math.pow(X,2)+1.715*0.001*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((37.30*Math.pow(X,-1)+4.629+0.01905*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    case 11: //Na
    {
        if(x>1.072)
        {
          tau = "-(0.3963*Math.pow(10,-2))*X+(0.1359*10)*Math.pow(X,-1)-(0.1720*1000)*Math.pow(X,-2)+(0.1766*100000)*Math.pow(X,-3)-(0.1019*100000)*Math.pow(X,-4)";
        }
        else
        {
          tau = "(0.8694*1000)*Math.pow(X,-3)-(0.2170*1000)*Math.pow(X,-4)";
        }
        sigmaK = "(1+1.029*0.1*X)*Math.pow((4.721*0.1+6.884*0.01*X+2.528*0.01*Math.pow(X,2)+8.577*0.0001*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((38.80*Math.pow(X,-1)+4.901+0.01889*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
        break;
    }
    }
}

double MainWindow::oslableniyeWindow(double x)
{
    QString tau;

    QString sigma;
    QString sigmaK;
    QString sigmaNK;

    QScriptEngine engine;

    QScriptValue tauValue;
    QScriptValue sigmaValue;
    QScriptValue sigmaKValue;
    QScriptValue sigmaNKValue;

        tau = "-(0.3142*Math.pow(10,-2))*Math.pow(X,0)+(0.4216*Math.pow(10,0))*Math.pow(X,-1)-(0.2014*Math.pow(10,2))*Math.pow(X,-2)+(0.5918*Math.pow(10,3))*Math.pow(X,-3)-(-0.4857*Math.pow(10,2))*Math.pow(X,-4)";
        sigmaK = "(1-3.178*Math.pow(10,-2)*X)*Math.pow((1.267*Math.pow(10,0)+4.619*Math.pow(10,-1)*X+3.102*Math.pow(10,-2)*Math.pow(X,2)-1.493*Math.pow(10,-3)*Math.pow(X,3)),-1)";
        sigmaNK = "Math.pow((25.93*Math.pow(X,-1)+5.067+0.02420*X),-1)";
        tau.replace("X", QString::number(x));
        sigmaK.replace("X", QString::number(x));
        sigmaNK.replace("X", QString::number(x));
        tauValue = engine.evaluate(tau);
        sigmaKValue = engine.evaluate(sigmaK);
        sigmaNKValue = engine.evaluate(sigmaNK);
        weakening = tauValue.toNumber()+sigmaKValue.toNumber()+sigmaNKValue.toNumber();
        return weakening;
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
                character();
                double sum = 0;
                if (N - int(N) != 0)
                wrongBorders();
            //Вычисляем наши данные
                else
                {
                    graph1->setName(ui->materialPipe->currentText());

                    QVector<double> x(N), y0(N);

                    int i=0;

                //Записываем данные в переменную

                    QFile fileOut(QCoreApplication::applicationDirPath() + "/Output/fileOut.txt");
                    if (fileOut.open(QIODevice::WriteOnly| QIODevice::Text))
                    {
                        QTextStream in(&fileOut);
                        for (double X=a; fabs(X - b) >= 0.00000001; X+= h)
                        {
                            x[i] = X;
                            y0[i] = (8.8*pow(10,8)*pipeNumber*current*pow(10,-3)*((voltage/X)-1))*qExp(-oslableniyePipe(X)*pipePlotnost*(targetThick-voltage/20)*pow(10,-4))*qExp(-oslableniyeWindow(X)*1.85*windowThick*pow(10,-4))*qExp(-oslableniyeFirst(X)*firstPlotnost*firstThick*pow(10,-4))*qExp(-oslableniyeSecond(X)*secondPlotnost*secondThick*pow(10,-4))*qExp(-oslableniyeThird(X)*thirdPlotnost*thirdThick*pow(10,-4))*qExp(-oslableniyeEnv(X)*envPlotnost*distance*pow(10,-4));
                            if(voltage>EK && round(X*10)/10 == round(EKa*10)/10)
                            {
                                y0[i] = (8.8*pow(10,8)*pipeNumber*current*pow(10,-3)*((voltage/X)-1))*qExp(-oslableniyePipe(X)*pipePlotnost*(targetThick-voltage/20)*pow(10,-4))*qExp(-oslableniyeWindow(X)*1.85*windowThick*pow(10,-4))*qExp(-oslableniyeFirst(X)*firstPlotnost*firstThick*pow(10,-4))*qExp(-oslableniyeSecond(X)*secondPlotnost*secondThick*pow(10,-4))*qExp(-oslableniyeThird(X)*thirdPlotnost*thirdThick*pow(10,-4))*qExp(-oslableniyeEnv(X)*envPlotnost*distance*pow(10,-4))
                                        +iKa*1/(qSqrt(2*M_PI)*(0.0428265524625))*qExp((-1/(2*qPow((0.0428265524625),2)))*qPow((X-EKa),2))*qExp(-oslableniyePipe(X)*pipePlotnost*(targetThick-voltage/20)*pow(10,-4))*qExp(-oslableniyeWindow(X)*1.85*windowThick*pow(10,-4))*qExp(-oslableniyeFirst(X)*firstPlotnost*firstThick*pow(10,-4))*qExp(-oslableniyeSecond(X)*secondPlotnost*secondThick*pow(10,-4))*qExp(-oslableniyeThird(X)*thirdPlotnost*thirdThick*pow(10,-4))*qExp(-oslableniyeEnv(X)*envPlotnost*distance*pow(10,-4));
                            }

                            else if(voltage>EK && round(X*10)/10 == round(EKb*10)/10)
                            {
                                y0[i] = (8.8*pow(10,8)*pipeNumber*current*pow(10,-3)*((voltage/X)-1))*qExp(-oslableniyePipe(X)*pipePlotnost*(targetThick-voltage/20)*pow(10,-4))*qExp(-oslableniyeWindow(X)*1.85*windowThick*pow(10,-4))*qExp(-oslableniyeFirst(X)*firstPlotnost*firstThick*pow(10,-4))*qExp(-oslableniyeSecond(X)*secondPlotnost*secondThick*pow(10,-4))*qExp(-oslableniyeThird(X)*thirdPlotnost*thirdThick*pow(10,-4))*qExp(-oslableniyeEnv(X)*envPlotnost*distance*pow(10,-4))
                                        +iKb*1/(qSqrt(2*M_PI)*(0.0428265524625))*qExp((-1/(2*qPow((0.0428265524625),2)))*qPow((X-EKb),2))*qExp(-oslableniyePipe(X)*pipePlotnost*(targetThick-voltage/20)*pow(10,-4))*qExp(-oslableniyeWindow(X)*1.85*windowThick*pow(10,-4))*qExp(-oslableniyeFirst(X)*firstPlotnost*firstThick*pow(10,-4))*qExp(-oslableniyeSecond(X)*secondPlotnost*secondThick*pow(10,-4))*qExp(-oslableniyeThird(X)*thirdPlotnost*thirdThick*pow(10,-4))*qExp(-oslableniyeEnv(X)*envPlotnost*distance*pow(10,-4));
                            }
                            else
                            {
                                y0[i] = (8.8*pow(10,8)*pipeNumber*current*pow(10,-3)*((voltage/X)-1))*qExp(-oslableniyePipe(X)*pipePlotnost*(targetThick-voltage/20)*pow(10,-4))*qExp(-oslableniyeWindow(X)*1.85*windowThick*pow(10,-4))*qExp(-oslableniyeFirst(X)*firstPlotnost*firstThick*pow(10,-4))*qExp(-oslableniyeSecond(X)*secondPlotnost*secondThick*pow(10,-4))*qExp(-oslableniyeThird(X)*thirdPlotnost*thirdThick*pow(10,-4))*qExp(-oslableniyeEnv(X)*envPlotnost*distance*pow(10,-4));
                            }
                            qDebug() << x;
                            sum += y0[i];
                            in << x[i] << "\t" << y0[i] << "\n";
                            i++;
                        }
                        x[i]=b;
                        y0[i]=(8.8*pow(10,8)*pipeNumber*current*pow(10,-3)*((voltage/b)-1))*qExp(-oslableniyePipe(b)*pipePlotnost*(targetThick-voltage/20)*pow(10,-4))*qExp(-oslableniyeWindow(b)*1.85*windowThick*pow(10,-4))*qExp(-oslableniyeFirst(b)*firstPlotnost*firstThick*pow(10,-4))*qExp(-oslableniyeSecond(b)*secondPlotnost*secondThick*pow(10,-4))*qExp(-oslableniyeThird(b)*thirdPlotnost*thirdThick*pow(10,-4))*qExp(-oslableniyeEnv(b)*envPlotnost*distance*pow(10,-4));
                        in << x[i] << "\t" << y0[i]  << "\n";

                        fileOut.close();
                    }
                //Отрисовка графика

                    ui->widget->legend->itemWithPlottable(graph1)->setVisible(true);
                    ui->firstSum->setText("Сумма 1: " + QString::number(sum));
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
                }
            break;
            }
            case 1:
            {
            character();
            double sum = 0;
            if (N - int(N) != 0)
            wrongBorders();
        //Вычисляем наши данные
            else
                {
                graph2->setName(ui->materialPipe->currentText());

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
                        y1[i] = (8.8*pow(10,8)*pipeNumber*current*pow(10,-3)*((voltage/X)-1))*qExp(-oslableniyePipe(X)*pipePlotnost*(targetThick-voltage/20)*pow(10,-4))*qExp(-oslableniyeWindow(X)*1.85*windowThick*pow(10,-4))*qExp(-oslableniyeFirst(X)*firstPlotnost*firstThick*pow(10,-4))*qExp(-oslableniyeSecond(X)*secondPlotnost*secondThick*pow(10,-4))*qExp(-oslableniyeThird(X)*thirdPlotnost*thirdThick*pow(10,-4))*qExp(-oslableniyeEnv(X)*envPlotnost*distance*pow(10,-4));
                        if(voltage>EK && round(X*10)/10 == round(EKa*10)/10)
                        {
                            y1[i] = (8.8*pow(10,8)*pipeNumber*current*pow(10,-3)*((voltage/X)-1))*qExp(-oslableniyePipe(X)*pipePlotnost*(targetThick-voltage/20)*pow(10,-4))*qExp(-oslableniyeWindow(X)*1.85*windowThick*pow(10,-4))*qExp(-oslableniyeFirst(X)*firstPlotnost*firstThick*pow(10,-4))*qExp(-oslableniyeSecond(X)*secondPlotnost*secondThick*pow(10,-4))*qExp(-oslableniyeThird(X)*thirdPlotnost*thirdThick*pow(10,-4))*qExp(-oslableniyeEnv(X)*envPlotnost*distance*pow(10,-4))
                                    +iKa*1/(qSqrt(2*M_PI)*(0.0428265524625))*qExp((-1/(2*qPow((0.0428265524625),2)))*qPow((X-EKa),2))*qExp(-oslableniyePipe(X)*pipePlotnost*(targetThick-voltage/20)*pow(10,-4))*qExp(-oslableniyeWindow(X)*1.85*windowThick*pow(10,-4))*qExp(-oslableniyeFirst(X)*firstPlotnost*firstThick*pow(10,-4))*qExp(-oslableniyeSecond(X)*secondPlotnost*secondThick*pow(10,-4))*qExp(-oslableniyeThird(X)*thirdPlotnost*thirdThick*pow(10,-4))*qExp(-oslableniyeEnv(X)*envPlotnost*distance*pow(10,-4));
                        }

                        else if(voltage>EK && round(X*10)/10 == round(EKb*10)/10)
                        {
                            y1[i] = (8.8*pow(10,8)*pipeNumber*current*pow(10,-3)*((voltage/X)-1))*qExp(-oslableniyePipe(X)*pipePlotnost*(targetThick-voltage/20)*pow(10,-4))*qExp(-oslableniyeWindow(X)*1.85*windowThick*pow(10,-4))*qExp(-oslableniyeFirst(X)*firstPlotnost*firstThick*pow(10,-4))*qExp(-oslableniyeSecond(X)*secondPlotnost*secondThick*pow(10,-4))*qExp(-oslableniyeThird(X)*thirdPlotnost*thirdThick*pow(10,-4))*qExp(-oslableniyeEnv(X)*envPlotnost*distance*pow(10,-4))
                                    +iKb*1/(qSqrt(2*M_PI)*(0.0428265524625))*qExp((-1/(2*qPow((0.0428265524625),2)))*qPow((X-EKb),2))*qExp(-oslableniyePipe(X)*pipePlotnost*(targetThick-voltage/20)*pow(10,-4))*qExp(-oslableniyeWindow(X)*1.85*windowThick*pow(10,-4))*qExp(-oslableniyeFirst(X)*firstPlotnost*firstThick*pow(10,-4))*qExp(-oslableniyeSecond(X)*secondPlotnost*secondThick*pow(10,-4))*qExp(-oslableniyeThird(X)*thirdPlotnost*thirdThick*pow(10,-4))*qExp(-oslableniyeEnv(X)*envPlotnost*distance*pow(10,-4));
                        }
                        else
                        {
                            y1[i] = (8.8*pow(10,8)*pipeNumber*current*pow(10,-3)*((voltage/X)-1))*qExp(-oslableniyePipe(X)*pipePlotnost*(targetThick-voltage/20)*pow(10,-4))*qExp(-oslableniyeWindow(X)*1.85*windowThick*pow(10,-4))*qExp(-oslableniyeFirst(X)*firstPlotnost*firstThick*pow(10,-4))*qExp(-oslableniyeSecond(X)*secondPlotnost*secondThick*pow(10,-4))*qExp(-oslableniyeThird(X)*thirdPlotnost*thirdThick*pow(10,-4))*qExp(-oslableniyeEnv(X)*envPlotnost*distance*pow(10,-4));
                        }
                        sum += y1[i];
                        in << x[i] << "\t" << y1[i] << "\n";
                        i++;
                    }
                    x[i]=b;
                    y1[i]=(8.8*pow(10,8)*pipeNumber*current*pow(10,-3)*((voltage/b)-1))*qExp(-oslableniyePipe(b)*pipePlotnost*(targetThick-voltage/20)*pow(10,-4))*qExp(-oslableniyeWindow(b)*1.85*windowThick*pow(10,-4))*qExp(-oslableniyeFirst(b)*firstPlotnost*firstThick*pow(10,-4))*qExp(-oslableniyeSecond(b)*secondPlotnost*secondThick*pow(10,-4))*qExp(-oslableniyeThird(b)*thirdPlotnost*thirdThick*pow(10,-4))*qExp(-oslableniyeEnv(b)*envPlotnost*distance*pow(10,-4));
                    in << x[i] << "\t" << y1[i]  << "\n";

                    fileOut.close();
                }

            //Отрисовка графика
                ui->widget->legend->itemWithPlottable(graph2)->setVisible(true);
                ui->secondSum->setText("Сумма 2: " + QString::number(sum));
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
                character();
                double sum =0;
                if (N - int(N) != 0)
                wrongBorders();
            //Вычисляем наши данные
                else
                {
                graph3->setName(ui->materialPipe->currentText());

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
                        y2[i] = (8.8*pow(10,8)*pipeNumber*current*pow(10,-3)*((voltage/X)-1))*qExp(-oslableniyePipe(X)*pipePlotnost*(targetThick-voltage/20)*pow(10,-4))*qExp(-oslableniyeWindow(X)*1.85*windowThick*pow(10,-4))*qExp(-oslableniyeFirst(X)*firstPlotnost*firstThick*pow(10,-4))*qExp(-oslableniyeSecond(X)*secondPlotnost*secondThick*pow(10,-4))*qExp(-oslableniyeThird(X)*thirdPlotnost*thirdThick*pow(10,-4))*qExp(-oslableniyeEnv(X)*envPlotnost*distance*pow(10,-4));
                        if(voltage>EK && round(X*10)/10 == round(EKa*10)/10)
                        {
                            y2[i] = (8.8*pow(10,8)*pipeNumber*current*pow(10,-3)*((voltage/X)-1))*qExp(-oslableniyePipe(X)*pipePlotnost*(targetThick-voltage/20)*pow(10,-4))*qExp(-oslableniyeWindow(X)*1.85*windowThick*pow(10,-4))*qExp(-oslableniyeFirst(X)*firstPlotnost*firstThick*pow(10,-4))*qExp(-oslableniyeSecond(X)*secondPlotnost*secondThick*pow(10,-4))*qExp(-oslableniyeThird(X)*thirdPlotnost*thirdThick*pow(10,-4))*qExp(-oslableniyeEnv(X)*envPlotnost*distance*pow(10,-4))
                                    +iKa*1/(qSqrt(2*M_PI)*(0.0428265524625))*qExp((-1/(2*qPow((0.0428265524625),2)))*qPow((X-EKa),2))*qExp(-oslableniyePipe(X)*pipePlotnost*(targetThick-voltage/20)*pow(10,-4))*qExp(-oslableniyeWindow(X)*1.85*windowThick*pow(10,-4))*qExp(-oslableniyeFirst(X)*firstPlotnost*firstThick*pow(10,-4))*qExp(-oslableniyeSecond(X)*secondPlotnost*secondThick*pow(10,-4))*qExp(-oslableniyeThird(X)*thirdPlotnost*thirdThick*pow(10,-4))*qExp(-oslableniyeEnv(X)*envPlotnost*distance*pow(10,-4));
                        }

                        else if(voltage>EK && round(X*10)/10 == round(EKb*10)/10)
                        {
                            y2[i] = (8.8*pow(10,8)*pipeNumber*current*pow(10,-3)*((voltage/X)-1))*qExp(-oslableniyePipe(X)*pipePlotnost*(targetThick-voltage/20)*pow(10,-4))*qExp(-oslableniyeWindow(X)*1.85*windowThick*pow(10,-4))*qExp(-oslableniyeFirst(X)*firstPlotnost*firstThick*pow(10,-4))*qExp(-oslableniyeSecond(X)*secondPlotnost*secondThick*pow(10,-4))*qExp(-oslableniyeThird(X)*thirdPlotnost*thirdThick*pow(10,-4))*qExp(-oslableniyeEnv(X)*envPlotnost*distance*pow(10,-4))
                                    +iKb*1/(qSqrt(2*M_PI)*(0.0428265524625))*qExp((-1/(2*qPow((0.0428265524625),2)))*qPow((X-EKb),2))*qExp(-oslableniyePipe(X)*pipePlotnost*(targetThick-voltage/20)*pow(10,-4))*qExp(-oslableniyeWindow(X)*1.85*windowThick*pow(10,-4))*qExp(-oslableniyeFirst(X)*firstPlotnost*firstThick*pow(10,-4))*qExp(-oslableniyeSecond(X)*secondPlotnost*secondThick*pow(10,-4))*qExp(-oslableniyeThird(X)*thirdPlotnost*thirdThick*pow(10,-4))*qExp(-oslableniyeEnv(X)*envPlotnost*distance*pow(10,-4));
                        }
                        else
                        {
                            y2[i] = (8.8*pow(10,8)*pipeNumber*current*pow(10,-3)*((voltage/X)-1))*qExp(-oslableniyePipe(X)*pipePlotnost*(targetThick-voltage/20)*pow(10,-4))*qExp(-oslableniyeWindow(X)*1.85*windowThick*pow(10,-4))*qExp(-oslableniyeFirst(X)*firstPlotnost*firstThick*pow(10,-4))*qExp(-oslableniyeSecond(X)*secondPlotnost*secondThick*pow(10,-4))*qExp(-oslableniyeThird(X)*thirdPlotnost*thirdThick*pow(10,-4))*qExp(-oslableniyeEnv(X)*envPlotnost*distance*pow(10,-4));
                        }
                        sum += y2[i];
                        in << x[i] << "\t" << y2[i] << "\n";
                        i++;
                    }
                    x[i]=b;
                    y2[i]=(8.8*pow(10,8)*pipeNumber*current*pow(10,-3)*((voltage/b)-1))*qExp(-oslableniyePipe(b)*pipePlotnost*(targetThick-voltage/20)*pow(10,-4))*qExp(-oslableniyeWindow(b)*1.85*windowThick*pow(10,-4))*qExp(-oslableniyeFirst(b)*firstPlotnost*firstThick*pow(10,-4))*qExp(-oslableniyeSecond(b)*secondPlotnost*secondThick*pow(10,-4))*qExp(-oslableniyeThird(b)*thirdPlotnost*thirdThick*pow(10,-4))*qExp(-oslableniyeEnv(b)*envPlotnost*distance*pow(10,-4));
                    in << x[i] << "\t" << y2[i]  << "\n";

                    fileOut.close();
                }
            //Отрисовка графика

                ui->widget->legend->itemWithPlottable(graph3)->setVisible(true);
                ui->thirdSum->setText("Сумма 3: " + QString::number(sum));
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
