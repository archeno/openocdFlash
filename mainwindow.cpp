#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QProcess>
#include <QThread>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), myprocess(new QProcess(this))
{
    ui->setupUi(this);
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
    QString outputStr = QString::fromUtf8(output);
    QStringList outputLines = outputStr.split("\n", QString::SkipEmptyParts);
    QString cleanedOutput = outputLines.join("\n");
    qDebug() << "std_out: " << cleanedOutput;
    //    ui->plainTextEdit->appendPlainText("slot standard output: ");
    ui->plainTextEdit->appendPlainText(cleanedOutput);
    output = myprocess->readAllStandardError();
    outputStr = QString::fromUtf8(output);
    outputLines = outputStr.split("\n", QString::SkipEmptyParts);
    cleanedOutput = outputLines.join("\n");
    qDebug() << "std_err: " << cleanedOutput;
    //    ui->plainTextEdit->appendPlainText("slot standard_error output: ");
    ui->plainTextEdit->appendPlainText(cleanedOutput);
}

void MainWindow::on_btnLoadFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open File"),
                                                    "",
                                                    tr("bin文件 (*.bin)"));
    if (!fileName.isEmpty()) {
        ui->labFileInfo->setText(fileName);
        m_fileName = fileName;
    } else {
    }
}

void MainWindow::on_btnDownLoad_clicked()
{
    QString program = "D:/DevelopmentTools/OpenOCD/bin/openocd.exe";
    QStringList arguments;
    //    arguments << "www.baidu.com";
    //    myprocess->setWorkingDirectory("D:\\DevelopmentTools\\OpenOCD\\bin\\openocd.exe");
    arguments << "-f"
              << "cmsis-dap.cfg"
              << "-c" << QString("program %1 0x08000000 verify reset exit").arg(m_fileName);
    qDebug() << "Command:" << program;
    qDebug() << "arguments:" << arguments;
    qDebug() << "Working Directory:" << myprocess->workingDirectory();
    QFileInfo checkFile(program);
    if (!checkFile.exists() || !checkFile.isExecutable()) {
        qDebug() << "The command path is invalid or not executable.";
        return;
    }
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
