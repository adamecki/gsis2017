#ifndef WSTRZASYMAIN_H
#define WSTRZASYMAIN_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class WstrzasyMain;
}
QT_END_NAMESPACE

class WstrzasyMain : public QMainWindow
{
    Q_OBJECT

public:
    WstrzasyMain(QWidget *parent = nullptr);
    ~WstrzasyMain();

private slots:
    void on_inject_incidents_clicked();

    void on_calculate_clicked();

    void on_clear_everything_clicked();

    void on_save_clicked();

    void on_browse_button_clicked();

    void on_clear_everything_onebyone_clicked();

    void on_inject_data_onebyone_clicked();

    void on_input_tabview_currentChanged(int index);

    void on_clear_everything_object_clicked();

    void on_inject_object_data_clicked();

    void on_browse_object_clicked();

private:
    Ui::WstrzasyMain *ui;
};
#endif // WSTRZASYMAIN_H
