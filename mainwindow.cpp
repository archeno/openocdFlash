#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QComboBox>
#include <QDebug>
#include <QFileDialog>
#include <QProcess>
#include <QThread>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), myprocess(new QProcess(this))
{
    ui->setupUi(this);
    m_buggerCofigType = ui->combDebuggerType->currentText().toLower() + ".cfg";
    m_mcuType = ui->combMcuSelect->currentText().toLower() + ".cfg";
    resize(1024, 768);
    connect(myprocess, &QProcess::readyReadStandardOutput, this, &MainWindow::readProcessOutput);
    connect(myprocess, &QProcess::readyReadStandardError, this, &MainWindow::readProcessOutput);
    connect(myprocess, &QProcess::errorOccurred, this, &MainWindow::dealError);
}
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::readProcessOutput()
{
    // 输出 OpenOCD 的输出信息

    QByteArray output = myprocess->readAllStandardOutput();
    if (!output.isEmpty()) {
        QString outputStr = QString::fromUtf8(output);
        ui->plainTextEdit->appendPlainText(outputStr);
    }

    //std_err
    output = myprocess->readAllStandardError();
    if (!output.isEmpty()) {
        QString outputStr = QString::fromUtf8(output);
        ui->plainTextEdit->appendPlainText(outputStr);
    }
}

void MainWindow::on_btnLoadFile_clicked()
{
    QString fileName
        = QFileDialog::getOpenFileName(this,
                                       tr("Open File"),
                                       "",
                                       tr("bin文件 (*.bin);; elf文件(*.elf);;所有文件(*.*)"));
    if (!fileName.isEmpty()) {
        ui->labFileInfo->setText(fileName);
        m_fileName = fileName;
    } else {
    }
}

void MainWindow::on_btnDownLoad_clicked()
{
    QString program = "openocd.exe";
    QString cfgInterfacePath = "OpenOCD/share/openocd/scripts/interface/" + m_buggerCofigType;
    QString TargetPath = "OpenOCD/share/openocd/scripts/target/" + m_mcuType;
    QStringList arguments;
    //    arguments << "www.baidu.com";
    //    myprocess->setWorkingDirectory("D:\\DevelopmentTools\\OpenOCD\\bin\\openocd.exe");
    arguments << "-f" << cfgInterfacePath << "-f" << TargetPath << "-c"
              << QString("program %1 0x08000000 verify reset exit").arg(m_fileName);
    qDebug() << "Command:" << program;
    qDebug() << "arguments:" << arguments;
    qDebug() << "Working Directory:" << myprocess->workingDirectory();
    QFileInfo checkFile(program);
    //    if (!checkFile.exists() || !checkFile.isExecutable()) {
    //        qDebug() << "The command path is invalid or not executable.";
    //        return;
    //    }
    myprocess->start(program, arguments);
    //    myprocess->waitForStarted();
    //    myprocess->waitForFinished();
    //    myprocess->waitForReadyRead();
    //    // 输出 OpenOCD 的输出信息
    //    QByteArray output = myprocess->readAllStandardOutput();
    //    qDebug() << "std_out: " << QString(output);
    //    ui->plainTextEdit->appendPlainText("standard output: ");
    //    ui->plainTextEdit->appendPlainText(output);
    //    output = myprocess->readAllStandardOutput();
    //    qDebug() << "std_err: " << QString(output);
    //    ui->plainTextEdit->appendPlainText("standard_error output: ");
    //    ui->plainTextEdit->appendPlainText(output);
    //    output = myprocess->readAll();
    //    qDebug() << "all: " << QString(output);
    //    ui->plainTextEdit->appendPlainText("all output: ");
    //    ui->plainTextEdit->appendPlainText(output);
}

void MainWindow::dealError(QProcess::ProcessError error)
{
    qDebug() << error << myprocess->errorString();
}

void MainWindow::on_combDebuggerType_currentTextChanged(const QString &arg1)
{
    m_buggerCofigType = arg1.toLower() + ".cfg";
    qDebug() << "m_buggerCofigFileName" << m_buggerCofigType;
}

void MainWindow::on_combMcuSelect_currentTextChanged(const QString &arg1)
{
    m_mcuType = arg1.toLower() + ".cfg";
}

void MainWindow::on_btnClearText_clicked()
{
    ui->plainTextEdit->clear();
}
