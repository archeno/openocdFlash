#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "versionConfig.h"
#include <cJSON/cJSON.h>
#include <QComboBox>
#include <QCompleter>
#include <QDebug>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QRegularExpression>
#include <QThread>
#include <QTimer>

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
            if (m_jsObj["version"].isString()) {
                //read value
                qDebug() << m_jsObj["version"].toString();
            }
            //            ui->combDebuggerType->currentText() = m_jsObj["debugerType"].toString();
            //            ui->combMcuSelect->currentText() = m_jsObj["mcuType"].toString();
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
    m_jsObj["debugerType"] = "cmsis-dap";
    m_jsObj["mcuType"] = "stm32f4x";
}
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), m_binAddr("0x08000000"),
      myprocess(new QProcess(this)), m_timerShowDownloadStatus(new QTimer(this))
{
    ui->setupUi(this);
    m_timerShowDownloadStatus->setSingleShot(true);
    connect(m_timerShowDownloadStatus, &QTimer::timeout, [=]() {
        if (m_downloadFlag == DOWNLOAD_OK) {
            ui->plainTextEdit->appendHtml(
                "<h2><font color=\"green\"; font-size=20;>====烧录成功====</font></h2>");
        } else if (m_downloadFlag == DOWNLOAD_FAILED) {
            ui->plainTextEdit->appendHtml(
                "<h2><font color=\"red\"; font-size=20;>====烧录失败！====</font></h2>");
        }
    });

    loadJsfile();
    ui->combDebuggerType->setCurrentText(m_jsObj["debugerType"].toString());
    m_buggerCofigType = ui->combDebuggerType->currentText().toLower() + ".cfg";

    resize(1024, 768);
    connect(myprocess, &QProcess::readyReadStandardOutput, this, &MainWindow::readProcessOutput);
    connect(myprocess, &QProcess::readyReadStandardError, this, &MainWindow::readProcessOutput);
    connect(myprocess, &QProcess::errorOccurred, this, &MainWindow::dealError);
    connect(this, &MainWindow::filetypeTobeDownloadChanged, this, &MainWindow::on_fileTypeChanged);

    ui->combMcuSelect->setEditable(true);
    ui->combMcuSelect->setMinimumWidth(200);
    ui->combMcuSelect->setCurrentText(m_jsObj["mcuType"].toString());
    m_mcuType = ui->combMcuSelect->currentText().toLower() + ".cfg";

    QStringList mcuList;
    mcuList << "stm32f1x"
            << "stm32f3x"
            << "stm32f4x"
            << "stm32f7x";
    QCompleter *mcuSelectCompleter = new QCompleter(mcuList, ui->combMcuSelect);
    mcuSelectCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    mcuSelectCompleter->setFilterMode(Qt::MatchContains);
    ui->combMcuSelect->setCompleter(mcuSelectCompleter);
}

MainWindow::~MainWindow()
{
    saveJsfile();
    delete ui;
}

void MainWindow::readProcessOutput()
{
    //    int downLoadFlag = 0;
    // 输出 OpenOCD 的输出信息
    QString outputStr;
    QByteArray output = myprocess->readAllStandardOutput();
    if (output.isEmpty()) {
        output = myprocess->readAllStandardError();
    }
    if (!output.isEmpty()) {
        m_timerShowDownloadStatus->setInterval(200);
        qDebug() << "--step--" << ++m_step;
        outputStr = QString::fromUtf8(output);
        qDebug() << outputStr;
        QStringList lines = outputStr.split("\r\n");
        for (const QString &line : lines) {
            if (line.contains("*")) { // 过滤条件
                ui->plainTextEdit->appendPlainText(line);
                if (line.contains("Verified OK")) {
                    m_downloadFlag = DOWNLOAD_OK;
                } else if (line.contains(("Programming Failed"))) {
                }
                emit downloadProcessChanged(m_step);
            } else if (line.contains("Error")) {
                m_downloadFlag = DOWNLOAD_FAILED;
                emit downloadFaild();
                ui->plainTextEdit->appendHtml(
                    QString("<font color=\"red\";>Error:%1</font>").arg(line.mid(6)));
            }
        }
    }
    if (m_downloadFlag != DOWNLOAD_INIT) {
        m_timerShowDownloadStatus->start(200);
    }
    //        if (successFlag) {
    //            ui->plainTextEdit->appendHtml(
    //                "<h2><font color=\"green\"; font-size=20;>烧录成功</font></h2>");
    //        }
    //    if (failedFlag) {
    //    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug() << "windownClosed!";
}

void MainWindow::on_btnLoadFile_clicked()
{
    static QString fileTypeLast;
    QString fileType;
    QString fileName
        = QFileDialog::getOpenFileName(this,
                                       tr("Open File"),
                                       "",
                                       tr("bin文件 (*.bin);; elf文件(*.elf);;所有文件(*.*)"));
    if (!fileName.isEmpty()) {
        QFileInfo fileInfo(fileName);
        m_fileName = fileName;

        //文件大小
        ui->labFileInfo->setText(fileName + "  " + QString::number(fileInfo.size() / 1024.0, 'f', 2)
                                 + "KB");

        fileType = fileInfo.suffix();
        if (fileType != fileTypeLast) {
            fileTypeLast = fileType;
            //发射信号 文件类型更改
            emit filetypeTobeDownloadChanged(fileType);
        }
    } else {
    }
}

void MainWindow::on_btnDownLoad_clicked()
{
    QString program = "OpenOCD/bin/openocd.exe";
    QString cfgInterfacePath = "OpenOCD/share/openocd/scripts/interface/" + m_buggerCofigType;
    QString TargetPath = "OpenOCD/share/openocd/scripts/target/" + m_mcuType;
    QStringList arguments;
    arguments << "-f" << cfgInterfacePath << "-f" << TargetPath << "-c"
              << QString("program %1 %2 verify reset exit").arg(m_fileName).arg(m_binAddr);
    QString argumentStr = arguments.join(" ");
    qDebug() << "Command:" << program;
    qDebug() << "arguments:" << arguments;
    qDebug() << "Working Directory:" << myprocess->workingDirectory();
    //    ui->plainTextEdit->appendHtml(
    //        QString("<p><font color=\"red\";>Command: </font>%1 %2</p>").arg(program).arg(argumentStr));
    m_step = 0;
    m_downloadFlag = DOWNLOAD_INIT;
    myprocess->start(program, arguments);
}

void MainWindow::dealError(QProcess::ProcessError error)
{
    qDebug() << error << myprocess->errorString();
}

void MainWindow::on_combDebuggerType_currentTextChanged(const QString &arg1)
{
    m_buggerCofigType = arg1.toLower() + ".cfg";
    m_jsObj["debugerType"] = arg1;
    qDebug() << "m_buggerCofigFileName" << m_buggerCofigType;
}

void MainWindow::on_btnClearText_clicked()
{
    ui->plainTextEdit->clear();
}
void MainWindow::deletelayout(QLayout *layout)
{
    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != nullptr) {
        if (QWidget *widget = child->widget()) {
            delete widget;
        } else if (QLayout *layoutitem = child->layout()) {
            deletelayout(layoutitem); //递归删除
        }
        delete child;
    }
    delete layout;
    layout = nullptr;
}
void MainWindow::on_fileTypeChanged(QString fileType)
{
    qDebug() << "received signals: filetypechanged, filetype= " << fileType;
    static QHBoxLayout *binAddrLayout = nullptr;
    int btnDownloadLayoutIndex;
    if (fileType == "bin") {
        binAddrLayout = new QHBoxLayout();
        QLabel *labBinAddr = new QLabel("bin起始地址:");
        QLineEdit *editBinAddr = new QLineEdit(m_binAddr);
        connect(editBinAddr, &QLineEdit::textChanged, [=]() {
            m_binAddr = editBinAddr->text().trimmed();
        });
        editBinAddr->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        QSpacerItem *spaceItem = new QSpacerItem(40,
                                                 40,
                                                 QSizePolicy::Expanding,
                                                 QSizePolicy::Minimum);

        binAddrLayout->addWidget(labBinAddr);
        binAddrLayout->addWidget(editBinAddr);
        binAddrLayout->addItem(spaceItem);

        btnDownloadLayoutIndex = ui->verticalLayout->indexOf(ui->horizontalLayout_2);
        ui->verticalLayout->insertLayout(btnDownloadLayoutIndex, binAddrLayout);
    } else {
        if (binAddrLayout != nullptr) {
            qDebug() << "begin to delte binAddrlayout";
            ui->verticalLayout->removeItem(binAddrLayout);
            deletelayout(binAddrLayout);
        }
    }
}

void MainWindow::on_combMcuSelect_currentIndexChanged(const QString &arg1)
{
    m_mcuType = arg1.toLower() + ".cfg";
    m_jsObj["mcuType"] = arg1;
}
