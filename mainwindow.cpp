#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "versionConfig.h"
#include <cJSON/cJSON.h>
#include <QComboBox>
#include <QDebug>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QThread>

#define CONFIG_FILE_NAME "./appconfig.json"
void MainWindow::loadJsfile()
{
    QJsonDocument configdoc;
    //读取文件
    QFile jsFile(CONFIG_FILE_NAME);
    if (!jsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "failed to open jsfile" << CONFIG_FILE_NAME
                 << "err_string:" << jsFile.errorString();
        qDebug() << "create default empty appconfig.json file";
        setDefaultConfig();
        saveJsfile();
    } else {
        QJsonParseError jsparseErr;
        //转换为jsondoc
        configdoc = QJsonDocument::fromJson(jsFile.readAll(), &jsparseErr);
        jsFile.close();
        if (jsparseErr.error != QJsonParseError::NoError) {
            qDebug() << "js Parse err:" << jsparseErr.error;
            return;
        }
        //转换为jsonObj
        if (configdoc.isObject()) {
            m_jsObj = configdoc.object();
            if (m_jsObj["version2"].isString()) {
                //read value
                qDebug() << m_jsObj["version"].toString();
            } else {
                qDebug() << 'm_jsObj["version2"]' << m_jsObj["version2"].type();
            }
        } else {
            qDebug() << "appconfig.js is not object";
        }
    }
}

void MainWindow::saveJsfile()
{
    QJsonDocument doc(m_jsObj);
    qDebug() << "doctoJson:" << doc.toJson();
    QFile file(CONFIG_FILE_NAME);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(doc.toJson());
    } else {
        qDebug() << file.errorString();
    }
    file.close();
}

void MainWindow::createJsonObj()
{
    cJSON *root = cJSON_CreateObject();
    QString versionString
        = QString("%1.%2.%3").arg(APP_VERSION_MAJOR).arg(APP_VERSION_MINOR).arg(APP_VERSION_PATCH);

    cJSON_AddStringToObject(root, "Version", versionString.toUtf8().constData());
    cJSON_AddStringToObject(root, "url", "https://github.com/archeno/openocdFlash.git");
    cJSON *DataTimeItem = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "DateTime", DataTimeItem);
    cJSON_AddStringToObject(DataTimeItem, "timeStamp", "2024-05-29 09:22:01");
    cJSON_AddNumberToObject(DataTimeItem, "value", 23.2);
    char *jsonString = cJSON_Print(root);
    ui->plainTextEdit->appendPlainText(jsonString);
    saveJsfile();
    delete jsonString;
    cJSON_Delete(root);

    loadJsfile();
}

void MainWindow::setDefaultConfig()
{
    m_jsObj["version"]
        = QString("%1.%2.%3").arg(APP_VERSION_MAJOR).arg(APP_VERSION_MINOR).arg(APP_VERSION_PATCH);
    m_jsObj["url"] = "https://github.com/archeno/openocdFlash.git";
}
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), myprocess(new QProcess(this)),
      m_binAddrLoayout(nullptr)
{
    ui->setupUi(this);
    m_buggerCofigType = ui->combDebuggerType->currentText().toLower() + ".cfg";
    m_mcuType = ui->combMcuSelect->currentText().toLower() + ".cfg";
    resize(1024, 768);
    connect(myprocess, &QProcess::readyReadStandardOutput, this, &MainWindow::readProcessOutput);
    connect(myprocess, &QProcess::readyReadStandardError, this, &MainWindow::readProcessOutput);
    connect(myprocess, &QProcess::errorOccurred, this, &MainWindow::dealError);
    //    createJsonObj();
}
MainWindow::~MainWindow()
{
    saveJsfile();
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

        //根据文件类型选择是否加载bin起始地址 LineEdit控件
        QFileInfo fileinfo(m_fileName);
        QString bin_type = fileinfo.suffix();
        if (bin_type == "bin") {
            if (m_binAddrLoayout == nullptr) {
                m_binAddrLoayout = new QHBoxLayout();
                QLabel *labBinAddr = new QLabel("bin起始地址");
                QLineEdit *editBinAddr = new QLineEdit("0x08000000");
                m_binAddrLoayout->addWidget(labBinAddr);
                m_binAddrLoayout->addWidget(editBinAddr);
                QSpacerItem *item = new QSpacerItem(40,
                                                    40,
                                                    QSizePolicy::Expanding,
                                                    QSizePolicy::Fixed);
                m_binAddrLoayout->addSpacerItem(item);
                int insertIndex = ui->verticalLayout->indexOf(ui->btnDownLoad);
                qDebug() << "index" << insertIndex;
                ui->verticalLayout->insertLayout(2, m_binAddrLoayout);
            }
        } else {
        }
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
