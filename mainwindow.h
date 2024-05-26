#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
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

private slots:
    void on_btnLoadFile_clicked();

    void on_btnDownLoad_clicked();
    void dealError(QProcess::ProcessError error);

    void on_combDebuggerType_currentTextChanged(const QString &arg1);

    void on_combMcuSelect_currentTextChanged(const QString &arg1);

    void on_btnClearText_clicked();

private:
    Ui::MainWindow *ui;
    QString m_fileName;
    QString m_buggerCofigType;
    QString m_mcuType;
    QProcess *myprocess;
};
#endif // MAINWINDOW_H
