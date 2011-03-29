#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

private slots:
    void on_actionLogin_triggered();
    void on_actionSync_triggered();
    void on_bConvert_clicked();
};


// rghaabat global variables
// from: http://stackoverflow.com/questions/3747085/global-variables-in-qt
class Reghaabat
{
private:
    Reghaabat();
    static Reghaabat* m_Instance;

public:
    int userId;
    QString userName, userPermission;

    static Reghaabat* instance()
    {
        if (! m_Instance)
            m_Instance = new Reghaabat;
        return m_Instance;
    }
};

#endif // MAINWINDOW_H