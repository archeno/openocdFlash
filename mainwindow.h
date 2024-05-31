#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QHBoxLayout>
#include <QJsonObject>
#include <QMainWindow>
#include <QProcess>
#include <QString>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void readProcessOutput();

signals:
    void filetypeTobeDownloadChanged(QString filetype);

private slots:
    void on_btnLoadFile_clicked();

    void on_btnDownLoad_clicked();
    void dealError(QProcess::ProcessError error);

    void on_combDebuggerType_currentTextChanged(const QString &arg1);

    void on_combMcuSelect_currentTextChanged(const QString &arg1);

    void on_btnClearText_clicked();

    void on_fileTypeChanged(QString fileType);

private:
    void deletelayout(QLayout *layout);

private:
    void createJsonObj(void);
    void setDefaultConfig(void);
    void loadJsfile();
    void saveJsfile();

private:
    Ui::MainWindow *ui;
    QString m_fileName;
    QString m_binAddr;
    QString m_buggerCofigType;
    QString m_mcuType;
    QProcess *myprocess;
    QHBoxLayout *m_binAddrLoayout;
    QJsonObject m_jsObj;
};
#endif // MAINWINDOW_H
