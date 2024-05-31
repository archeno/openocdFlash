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
      m_binAddr("0x08000000")
{
    ui->setupUi(this);
    m_buggerCofigType = ui->combDebuggerType->currentText().toLower() + ".cfg";
    m_mcuType = ui->combMcuSelect->currentText().toLower() + ".cfg";
    resize(1024, 768);
    connect(myprocess, &QProcess::readyReadStandardOutput, this, &MainWindow::readProcessOutput);
    connect(myprocess, &QProcess::readyReadStandardError, this, &MainWindow::readProcessOutput);
    connect(myprocess, &QProcess::errorOccurred, this, &MainWindow::dealError);
    connect(this, &MainWindow::filetypeTobeDownloadChanged, this, &MainWindow::on_fileTypeChanged);

    ui->combMcuSelect->setEditable(true);
    ui->combMcuSelect->setMinimumWidth(200);
    ui->combMcuSelect->setEditText("Select Mcu...");
    QStringList mcuList;
    mcuList << "stm32f1x"
            << "stm32f3x"
            << "stm32f4x";
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
    static QString fileNameLast;
    QString fileName
        = QFileDialog::getOpenFileName(this,
                                       tr("Open File"),
                                       "",
                                       tr("bin文件 (*.bin);; elf文件(*.elf);;所有文件(*.*)"));
    if (!fileName.isEmpty()) {
        ui->labFileInfo->setText(fileName);
        QFileInfo fileInfo(fileName);
        m_fileName = fileInfo.suffix();
        if (fileName != fileNameLast) {
            fileNameLast = fileName;
            //发射信号 文件类型更改
            emit filetypeTobeDownloadChanged(m_fileName);
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
    qDebug() << "Command:" << program;
    qDebug() << "arguments:" << arguments;
    qDebug() << "Working Directory:" << myprocess->workingDirectory();
    QFileInfo checkFile(program);
    myprocess->start(program, arguments);
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
