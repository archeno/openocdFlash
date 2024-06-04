#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCloseEvent>
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

protected:
    void closeEvent(QCloseEvent *event) override;
signals:
    void filetypeTobeDownloadChanged(QString filetype);
    void downloadProcessChanged(qint32 step);
    void downloadFaild();

private slots:
    void on_btnLoadFile_clicked();

    void on_btnDownLoad_clicked();
    void dealError(QProcess::ProcessError error);

    void on_combDebuggerType_currentTextChanged(const QString &arg1);

    void on_btnClearText_clicked();

    void on_fileTypeChanged(QString fileType);

    void on_combMcuSelect_currentIndexChanged(const QString &arg1);

private:
    void deletelayout(QLayout *layout);

private:
    void createJsonObj(void);
    void setDefaultConfig(void);
    void loadJsfile();
    void saveJsfile();
    void filterAndDisplayOutput(const QByteArray &data);

private:
    enum { DOWNLOAD_INIT, DOWNLOAD_OK, DOWNLOAD_FAILED } downloadStatus;
    Ui::MainWindow *ui;
    QString m_fileName;
    QString m_binAddr;
    QString m_buggerCofigType;
    QString m_mcuType;
    QProcess *myprocess;
    QHBoxLayout *m_binAddrLoayout;
    QJsonObject m_jsObj;
    quint8 m_step = 0;
    quint8 m_downloadFlag = DOWNLOAD_INIT;
    QTimer *m_timerShowDownloadStatus;
};
#endif // MAINWINDOW_H
