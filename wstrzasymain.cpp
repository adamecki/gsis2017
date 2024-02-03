#include "wstrzasymain.h"
#include "./ui_wstrzasymain.h"
// Project specific header files above

#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
// Qt specific libraries above

// C++ standard libraries below
#include <algorithm>
#include <vector>
#include <string>
#include <math.h>


// ************************************************** //
// NOTHING BUT MESS IS WHAT HAVE I DONE! (Look below) //
// ************************************************** //

// In the code, what is called "zdarzenie" is a whole incident class, incl. shock and object data.
// However, in GUI "zdarzenie" is a shock. Please be aware of it.


// Define a repeatable class for an incident (shock and object data (mostly as double bot strings for date, time and address exist too))
class Incident {
    private:
        // Subclasses for data variables
        class Shock {
            public:     double X, Y, e, t, h;
                        QString date, time;
        };
        class Object {
            public:     double X, Y, wf;
                        QString address;
        };
    public:
        Shock wstrzas;
        Object obiekt;

        // Functions which calculate PGVHmax and PGAH10
        double PGVHmax() {
            double d = sqrt(pow((obiekt.X - wstrzas.X) / 1000, 2) + pow((obiekt.Y - wstrzas.Y) / 1000, 2));
            double r = sqrt(pow(d, 2) + pow(wstrzas.h, 2));

            double VMD = (1.48 * 0.001 * pow(log10(wstrzas.e), 1.23) - 0.011) * (1.55 * pow(r, 0.135) * exp(-0.77 * r) + 0.04);

            return VMD * obiekt.wf;
        }   
        double PGAH10() {
            double d = sqrt(pow((obiekt.X - wstrzas.X) / 1000, 2) + pow((obiekt.Y - wstrzas.Y) / 1000, 2));
            double r = sqrt(pow(d, 2) + pow(wstrzas.h, 2));

            double AMD = (1.33 * 0.001 * pow(log10(wstrzas.e), 2.66) - 0.089) * (1.53 * pow(r, 0.155) * exp(-0.65 * r) + 0.014);

            return AMD * obiekt.wf;
        }

        // Functions which rate the shock level from 0 to 6
        int PGVHmax_level() {
            double pgv = PGVHmax();

            if(wstrzas.t <= 1.5) {
                if(pgv <= 0.005) {
                    return 0;
                } else if(pgv <= 0.02) {
                    return 1;
                } else if(pgv <= 0.035) {
                    return 2;
                } else if(pgv <= 0.05) {
                    return 3;
                } else if(pgv <= 0.07) {
                    return 4;
                } else if(pgv <= 0.11) {
                    return 5;
                } else {
                    return 6;
                }
            } else if(wstrzas.t <= 3) {
                if(pgv <= 0.005) {
                    return 0;
                } else if(pgv <= (((-2 / 3) * wstrzas.t) + 3) / 100) {
                    return 1;
                } else if(pgv <= (((-2 / 3) * wstrzas.t) + 4.5) / 100) {
                    return 2;
                } else if(pgv <= (((-2 / 3) * wstrzas.t) + 6) / 100) {
                    return 3;
                } else if(pgv <= (((-2 / 3) * wstrzas.t) + 8) / 100) {
                    return 4;
                } else if(pgv <= (((-2 / 3) * wstrzas.t) + 12) / 100) {
                    return 5;
                } else {
                    return 6;
                }
            } else {
                if(pgv <= 0.005) {
                    return 0;
                } else if(pgv <= 0.01) {
                    return 1;
                } else if(pgv <= 0.025) {
                    return 2;
                } else if(pgv <= 0.04) {
                    return 3;
                } else if(pgv <= 0.06) {
                    return 4;
                } else if(pgv <= 0.1) {
                    return 5;
                } else {
                    return 6;
                }
            }
        }
        int PGAH10_level() {
            double pga = PGAH10();

            if(wstrzas.t <= 1.5) {
                if(pga <= 0.15) {
                    return 0;
                } else if(pga <= 0.6) {
                    return 1;
                } else if(pga <= 0.9) {
                    return 2;
                } else if(pga <= 1.2) {
                    return 3;
                } else if(pga <= 1.6) {
                    return 4;
                } else if(pga <= 2.3) {
                    return 5;
                } else {
                    return 6;
                }
            } else if(wstrzas.t <= 3) {
                if(pga <= 0.15) {
                    return 0;
                } else if(pga <= ((-0.2 * wstrzas.t) + 0.9)) {
                    return 1;
                } else if(pga <= ((-0.2 * wstrzas.t) + 1.2)) {
                    return 2;
                } else if(pga <= ((-0.2 * wstrzas.t) + 1.5)) {
                    return 3;
                } else if(pga <= ((-0.2 * wstrzas.t) + 1.9)) {
                    return 4;
                } else if(pga <= ((-0.2 * wstrzas.t) + 2.6)) {
                    return 5;
                } else {
                    return 6;
                }
            } else {
                if(pga <= 0.15) {
                    return 0;
                } else if(pga <= 0.3) {
                    return 1;
                } else if(pga <= 0.6) {
                    return 2;
                } else if(pga <= 0.9) {
                    return 3;
                } else if(pga <= 1.3) {
                    return 4;
                } else if(pga <= 2) {
                    return 5;
                } else {
                    return 6;
                }
            }
        }

        // Overall level of a shock; it just takes the bigger return value of the two previous function
        int overall_level() {
            int V = PGVHmax_level();
            int A = PGAH10_level();

            if(V > A) {
                return V;
            } else {
                return A;
            }
        }
};

std::vector<Incident> zdarzenia;    // Vector of incidents for multiple calculations at once
Incident dummy_incident;            // Will be pushed back as an empty class to the previous vector

std::vector<QString> dataline_q;    // Vector of file's lines
std::vector<QString> insert_to_incident_q;      // Vector of values in file's line

std::vector<double> insert_to_incident_double;  // The same, but excluding address, time and date, which are QStrings. Used for to convert to double for calculation

QString input_file_path = "in.txt";         // Default open file path for the first rile reading mode. Will be modified when "Przeglądaj..." clicked
QString input_object_file_path = "in.txt";  // Default open file path for the second rile reading mode. Also will be modified like the variable above

// Default communicates for "Komunikaty" field
const QString parser_choice_1_communicate = "Format linijki pliku wejściowego dla parsera\nZdarzenia dla obiektów:\n\nE Xz Yz t h (d) (g)\n\nWytłumaczenie skrótów w polu Informacje.";
const QString parser_choice_2_communicate = "Format linijki pliku wejściowego dla parsera\nObiekty dla zdarzenia:\n\nXo Yo Wf (a)\n\nWytłumaczenie skrótów w polu Informacje.";

QString errdump = ""; // Errors about incorrect lines and general alerts and communicated will appear there, and later on in the "Komunikaty" field

QDateTime now; // Used to set today's date in GUI when program is started

WstrzasyMain::WstrzasyMain(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::WstrzasyMain)
{
    ui->setupUi(this);
    this->setFixedSize(size());
    this->setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);

    this->ui->onebyone_date->setDate(now.currentDateTime().date());
    this->ui->onebyone_time->setTime(now.currentDateTime().time());

    this->ui->input_shock_date->setDate(now.currentDateTime().date());
    this->ui->input_shock_time->setTime(now.currentDateTime().time());

    this->ui->alert_text->setText(parser_choice_1_communicate);
}


WstrzasyMain::~WstrzasyMain()
{
    delete ui;
}


void WstrzasyMain::on_inject_object_data_clicked()
{
    QFile data(input_object_file_path);     // Declare filestream

    std::vector<double> incident_data;
    QString incident_data_str[7];

    incident_data_str[0] = this->ui->input_shock_e->text();
    incident_data_str[1] = this->ui->input_shock_x->text();
    incident_data_str[2] = this->ui->input_shock_y->text();
    incident_data_str[3] = this->ui->input_shock_t->text();
    incident_data_str[4] = this->ui->input_shock_h->text();

    if(this->ui->input_shock_date->date().day() < 10) {     incident_data_str[5] = "0";     }
    incident_data_str[5] += (std::to_string(this->ui->input_shock_date->date().day()) + ".");

    if(this->ui->input_shock_date->date().month() < 10) {   incident_data_str[5] += "0";    }
    incident_data_str[5] += (std::to_string(this->ui->onebyone_date->date().month()) + "." + std::to_string(this->ui->onebyone_date->date().year()));

    if(this->ui->input_shock_time->time().hour() < 10) {    incident_data_str[6] = "0";     }
    incident_data_str[6] += (std::to_string(this->ui->onebyone_time->time().hour()) + ":");

    if(this->ui->input_shock_time->time().minute() < 10) {  incident_data_str[6] += "0";    }
    incident_data_str[6] += (std::to_string(this->ui->onebyone_time->time().minute()));

    for(int i = 0; i < 5; i++) {
        try {
            incident_data.push_back(incident_data_str[i].replace(',', '.').toDouble());
        } catch (...) {}
    }

    if(data.open(QFile::ReadOnly) && incident_data.size() == 5) {
        // Adjust UI
        this->ui->input_shock_e->setEnabled(false);
        this->ui->input_shock_x->setEnabled(false);
        this->ui->input_shock_y->setEnabled(false);
        this->ui->input_shock_t->setEnabled(false);
        this->ui->input_shock_h->setEnabled(false);
        this->ui->input_shock_date->setEnabled(false);
        this->ui->input_shock_time->setEnabled(false);

        this->ui->inject_object_data->setEnabled(false);
        this->ui->browse_object->setEnabled(false);
        this->ui->clear_everything_object->setEnabled(true);

        // Read line by line
        QTextStream in(&data);

        while(!in.atEnd()) {
            dataline_q.push_back(in.readLine());
        }
        // End reading line by line

        data.close(); // Close file stream

        // Place values into correct places
        for(int e = 0; e < dataline_q.size(); e++) {
            QStringList dummy = dataline_q[e].split('\t');

            for(QString f : dummy) {
                f.replace(',', '.');

                if(f.contains(' ')) {
                    QStringList dummy_s = f.split(' ');

                    for(QString g : dummy_s) {
                        if(g != "") {
                            if(insert_to_incident_q.size() < 3) {
                                insert_to_incident_q.push_back(g);
                            } else if (insert_to_incident_q.size() == 3) {
                                insert_to_incident_q.push_back(g);
                            } else {
                                insert_to_incident_q[insert_to_incident_q.size() - 1] += (" " + g);
                            }
                        }
                    }
                } else {
                    if(f != "") {
                        if(insert_to_incident_q.size() < 3) {
                            insert_to_incident_q.push_back(f);
                        } else if(insert_to_incident_q.size() == 3) {
                            insert_to_incident_q.push_back(f);
                        } else {
                            insert_to_incident_q[insert_to_incident_q.size() - 1] += (" " + f);
                        }
                    }
                }
            }

            if(insert_to_incident_q.size() >= 3) {
                for(int i = 0; i < 3; i++) {
                    try {
                        insert_to_incident_double.push_back(insert_to_incident_q[i].toDouble());
                    } catch(...) {}
                }

                if(insert_to_incident_double.size() == 3) {
                    zdarzenia.push_back(dummy_incident);

                    zdarzenia[zdarzenia.size() - 1].wstrzas.e = incident_data[0];
                    zdarzenia[zdarzenia.size() - 1].wstrzas.X = incident_data[1];
                    zdarzenia[zdarzenia.size() - 1].wstrzas.Y = incident_data[2];
                    zdarzenia[zdarzenia.size() - 1].wstrzas.t = incident_data[3];
                    zdarzenia[zdarzenia.size() - 1].wstrzas.h = incident_data[4];

                    zdarzenia[zdarzenia.size() - 1].wstrzas.date = incident_data_str[5];
                    zdarzenia[zdarzenia.size() - 1].wstrzas.time = incident_data_str[6];

                    zdarzenia[zdarzenia.size() - 1].obiekt.X = insert_to_incident_double[0];
                    zdarzenia[zdarzenia.size() - 1].obiekt.Y = insert_to_incident_double[1];
                    zdarzenia[zdarzenia.size() - 1].obiekt.wf = insert_to_incident_double[2];

                    if(insert_to_incident_q.size() >= 4) {
                        zdarzenia[zdarzenia.size() - 1].obiekt.address = insert_to_incident_q[3];
                    }

                    errdump += ("Linijka " + QString::fromStdString(std::to_string(e + 1)) + " wygląda dobrze.\n\n");

                    insert_to_incident_double.clear();
                    insert_to_incident_q.clear();
                } else {
                    errdump += ("Ignorowanie linijki " + QString::fromStdString(std::to_string(e + 1)) + " (niewłaściwa ilość danych).\n\n");

                    insert_to_incident_double.clear();
                    insert_to_incident_q.clear();
                }
            } else if(insert_to_incident_q.size() == 0) {
                errdump += ("Ignorowanie linijki " + QString::fromStdString(std::to_string(e + 1)) + " (pusta).\n\n");

                insert_to_incident_double.clear();
                insert_to_incident_q.clear();
            } else {
                errdump += ("Ignorowanie linijki " + QString::fromStdString(std::to_string(e + 1)) + " (niewłaściwa ilość danych).\n\n");

                insert_to_incident_double.clear();
                insert_to_incident_q.clear();
            }
        }
        // End positioning values

        // Push data into the input data field
        this->ui->input_data->setText("");
        this->ui->alert_text->setText(this->ui->alert_text->toPlainText() + "\n\n" + errdump);

        int incident_counter = 1;
        for(Incident e : zdarzenia) {
            if(incident_counter != 1) {
                this->ui->input_data->setText(this->ui->input_data->toPlainText() + "\n\n");
            }
            this->ui->input_data->setText(this->ui->input_data->toPlainText() + "[Obliczenie " + QString::fromStdString(std::to_string(incident_counter)) + "]");

            if(e.wstrzas.date != "") {
                this->ui->input_data->setText(this->ui->input_data->toPlainText() + "\nData zdarzenia: " + e.wstrzas.date);
            }
            if(e.wstrzas.time != "") {
                this->ui->input_data->setText(this->ui->input_data->toPlainText() + "\nGodzina zdarzenia: " + e.wstrzas.time);
            }

            this->ui->input_data->setText(this->ui->input_data->toPlainText() + "\nE zdarzenia: " + QString::fromStdString(std::to_string(e.wstrzas.e)));
            this->ui->input_data->setText(this->ui->input_data->toPlainText() + "\nX zdarzenia: " + QString::fromStdString(std::to_string(e.wstrzas.X)));
            this->ui->input_data->setText(this->ui->input_data->toPlainText() + "\nY zdarzenia: " + QString::fromStdString(std::to_string(e.wstrzas.Y)));
            this->ui->input_data->setText(this->ui->input_data->toPlainText() + "\nt zdarzenia: " + QString::fromStdString(std::to_string(e.wstrzas.t)));
            this->ui->input_data->setText(this->ui->input_data->toPlainText() + "\nh zdarzenia: " + QString::fromStdString(std::to_string(e.wstrzas.h)));

            if(e.obiekt.address != "") {
                this->ui->input_data->setText(this->ui->input_data->toPlainText() + "\nAdres obiektu: " + e.obiekt.address);
            }
            this->ui->input_data->setText(this->ui->input_data->toPlainText() + "\nX obiektu: " + QString::fromStdString(std::to_string(e.obiekt.X)));
            this->ui->input_data->setText(this->ui->input_data->toPlainText() + "\nY obiektu: " + QString::fromStdString(std::to_string(e.obiekt.Y)));
            this->ui->input_data->setText(this->ui->input_data->toPlainText() + "\nWf obiektu: " + QString::fromStdString(std::to_string(e.obiekt.wf)));
            incident_counter++;
        }

        this->ui->calculate->setEnabled(true);
    } else {
        data.close();
        QMessageBox err_msg;
        err_msg.setWindowTitle("Błąd!");
        err_msg.setInformativeText("Plik o podanej nazwie nie istnieje, jest niewłaściwy, lub wprowadzono nieprawidłowe dane obiektu.");
        err_msg.setStandardButtons(QMessageBox::Close);
        err_msg.exec();
    }
}


void WstrzasyMain::on_inject_incidents_clicked()
{
    QFile data(input_file_path);

    std::vector<double> object_data;
    QString object_data_str[4];

    object_data_str[0] = this->ui->input_object_x->text();
    object_data_str[1] = this->ui->input_object_y->text();
    object_data_str[2] = this->ui->input_object_wf->text();
    object_data_str[3] = this->ui->input_object_address->text();

    for(QString e : object_data_str) {
        try {
            object_data.push_back(e.replace(',', '.').toDouble());
        } catch(...) {}
    }

    if(data.open(QFile::ReadOnly) && object_data.size() == 4) {
        // Adjust UI
        this->ui->input_object_x->setEnabled(false);
        this->ui->input_object_y->setEnabled(false);
        this->ui->input_object_wf->setEnabled(false);
        this->ui->input_object_address->setEnabled(false);

        this->ui->inject_incidents->setEnabled(false);
        this->ui->browse_button->setEnabled(false);
        this->ui->clear_everything->setEnabled(true);

        // Read line by line
        QTextStream in(&data);

        while(!in.atEnd()) {
            dataline_q.push_back(in.readLine());
        }
        // End reading line by line

        data.close(); // Close file stream

        // Place values into correct spots
        for(int e = 0; e < dataline_q.size(); e++) {
            QStringList dummy = dataline_q[e].split('\t');

            for(QString f : dummy) {
                f.replace(',', '.');

                if(f.contains(' ')) {
                    QStringList dummy_s = f.split(' ');

                    for(QString g : dummy_s) {
                        if(g != "") {
                            insert_to_incident_q.push_back(g);
                        }
                    }
                } else {
                    if(f != "") {
                        insert_to_incident_q.push_back(f);
                    }
                }
            }

            if(insert_to_incident_q.size() >= 5) {
                for(int i = 0; i < 5; i++) {
                    try {
                        insert_to_incident_double.push_back(insert_to_incident_q[i].toDouble());
                    } catch(...) {}
                }

                if(insert_to_incident_double.size() == 5) {
                    zdarzenia.push_back(dummy_incident);

                    zdarzenia[zdarzenia.size() - 1].obiekt.X = object_data[0];
                    zdarzenia[zdarzenia.size() - 1].obiekt.Y = object_data[1];
                    zdarzenia[zdarzenia.size() - 1].obiekt.wf = object_data[2];
                    zdarzenia[zdarzenia.size() - 1].obiekt.address = object_data_str[3];

                    zdarzenia[zdarzenia.size() -1 ].wstrzas.e = insert_to_incident_double[0];
                    zdarzenia[zdarzenia.size() -1 ].wstrzas.X = insert_to_incident_double[1];
                    zdarzenia[zdarzenia.size() -1 ].wstrzas.Y = insert_to_incident_double[2];
                    zdarzenia[zdarzenia.size() -1 ].wstrzas.t = insert_to_incident_double[3];
                    zdarzenia[zdarzenia.size() -1 ].wstrzas.h = insert_to_incident_double[4];

                    if(insert_to_incident_q.size() >= 6) {
                        zdarzenia[zdarzenia.size() - 1].wstrzas.date = insert_to_incident_q[5];
                        if(insert_to_incident_q.size() >= 7) {
                            zdarzenia[zdarzenia.size() - 1].wstrzas.time = insert_to_incident_q[6];
                        }
                    }

                    errdump += ("Linijka " + QString::fromStdString(std::to_string(e + 1)) + " wygląda dobrze.\n\n");

                    insert_to_incident_double.clear();
                    insert_to_incident_q.clear();
                } else {
                    errdump += ("Ignorowanie linijki " + QString::fromStdString(std::to_string(e + 1)) + " (niewłaściwa ilość danych).\n\n");

                    insert_to_incident_double.clear();
                    insert_to_incident_q.clear();
                }
            } else if(insert_to_incident_q.size() == 0) {
                errdump += ("Ignorowanie linijki " + QString::fromStdString(std::to_string(e + 1)) + " (pusta).\n\n");

                insert_to_incident_double.clear();
                insert_to_incident_q.clear();
            } else {
                errdump += ("Ignorowanie linijki " + QString::fromStdString(std::to_string(e + 1)) + " (niewłaściwa ilość danych).\n\n");

                insert_to_incident_double.clear();
                insert_to_incident_q.clear();
            }
        }
        // End positioning values

        // Push data into the input data field
        this->ui->input_data->setText("");
        this->ui->alert_text->setText(this->ui->alert_text->toPlainText() + "\n\n" + errdump);

        int incident_counter = 1;
        for(Incident e : zdarzenia) {
            if(incident_counter != 1) {
                this->ui->input_data->setText(this->ui->input_data->toPlainText() + "\n\n");
            }
            this->ui->input_data->setText(this->ui->input_data->toPlainText() + "[Obliczenie " + QString::fromStdString(std::to_string(incident_counter)) + "]");

            if(e.wstrzas.date != "") {
                this->ui->input_data->setText(this->ui->input_data->toPlainText() + "\nData zdarzenia: " + e.wstrzas.date);
            }
            if(e.wstrzas.time != "") {
                this->ui->input_data->setText(this->ui->input_data->toPlainText() + "\nGodzina zdarzenia: " + e.wstrzas.time);
            }

            this->ui->input_data->setText(this->ui->input_data->toPlainText() + "\nE zdarzenia: " + QString::fromStdString(std::to_string(e.wstrzas.e)));
            this->ui->input_data->setText(this->ui->input_data->toPlainText() + "\nX zdarzenia: " + QString::fromStdString(std::to_string(e.wstrzas.X)));
            this->ui->input_data->setText(this->ui->input_data->toPlainText() + "\nY zdarzenia: " + QString::fromStdString(std::to_string(e.wstrzas.Y)));
            this->ui->input_data->setText(this->ui->input_data->toPlainText() + "\nt zdarzenia: " + QString::fromStdString(std::to_string(e.wstrzas.t)));
            this->ui->input_data->setText(this->ui->input_data->toPlainText() + "\nh zdarzenia: " + QString::fromStdString(std::to_string(e.wstrzas.h)));

            if(e.obiekt.address != "") {
                this->ui->input_data->setText(this->ui->input_data->toPlainText() + "\nAdres obiektu: " + e.obiekt.address);
            }
            this->ui->input_data->setText(this->ui->input_data->toPlainText() + "\nX obiektu: " + QString::fromStdString(std::to_string(e.obiekt.X)));
            this->ui->input_data->setText(this->ui->input_data->toPlainText() + "\nY obiektu: " + QString::fromStdString(std::to_string(e.obiekt.Y)));
            this->ui->input_data->setText(this->ui->input_data->toPlainText() + "\nWf obiektu: " + QString::fromStdString(std::to_string(e.obiekt.wf)));
            incident_counter++;
        }

        this->ui->calculate->setEnabled(true);
    } else {
        data.close();
        QMessageBox err_msg;
        err_msg.setWindowTitle("Błąd!");
        err_msg.setInformativeText("Plik o podanej nazwie nie istnieje, jest niewłaściwy, lub wprowadzono nieprawidłowe dane obiektu.");
        err_msg.setStandardButtons(QMessageBox::Close);
        err_msg.exec();
    }
}


void WstrzasyMain::on_calculate_clicked()
{
    this->ui->calculate->setEnabled(false);
    this->ui->output_data->setText("");

    int incident_counter = 1;
    for(Incident e : zdarzenia) {
        if(incident_counter != 1) {
            this->ui->output_data->setText(this->ui->output_data->toPlainText() + "\n\n");
        }
        this->ui->output_data->setText(this->ui->output_data->toPlainText() + "[Wyniki obliczenia " + QString::fromStdString(std::to_string(incident_counter)) + "]");

        this->ui->output_data->setText(this->ui->output_data->toPlainText() + "\nPGVHmax zdarzenia [m/s]: " + QString::fromStdString(std::to_string(e.PGVHmax())));
        this->ui->output_data->setText(this->ui->output_data->toPlainText() + "\nPGAH10 zdarzenia [m/s^2]: " + QString::fromStdString(std::to_string(e.PGAH10())));
        this->ui->output_data->setText(this->ui->output_data->toPlainText() + "\nIGSIS-2017 - PGVHmax zdarzenia [0-6]: " + QString::fromStdString(std::to_string(e.PGVHmax_level())));
        this->ui->output_data->setText(this->ui->output_data->toPlainText() + "\nIGSIS-2017 - PGAH10 zdarzenia [0-6]: " + QString::fromStdString(std::to_string(e.PGAH10_level())));
        this->ui->output_data->setText(this->ui->output_data->toPlainText() + "\nIGSIS-2017 zdarzenia [0-6]: " + QString::fromStdString(std::to_string(e.overall_level())));
        
        incident_counter++;
    }

    this->ui->save->setEnabled(true);
}


void WstrzasyMain::on_clear_everything_clicked()
{
    this->ui->input_filename->setText("in.txt");
    input_file_path = "in.txt";
    this->ui->browse_button->setEnabled(true);

    this->ui->input_object_x->setEnabled(true);
    this->ui->input_object_y->setEnabled(true);
    this->ui->input_object_wf->setEnabled(true);
    this->ui->input_object_address->setEnabled(true);

    this->ui->input_object_x->setText("");
    this->ui->input_object_y->setText("");
    this->ui->input_object_wf->setText("");
    this->ui->input_object_address->setText("");

    this->ui->save->setEnabled(false);
    this->ui->calculate->setEnabled(false);

    this->ui->alert_text->setText(parser_choice_1_communicate);
    this->ui->input_data->setText("Oczekiwanie na dane wejściowe...");
    this->ui->output_data->setText("Oczekiwanie na wykonanie obliczeń...");

    zdarzenia.clear();
    dataline_q.clear();
    insert_to_incident_q.clear();

    errdump = "";

    this->ui->inject_incidents->setEnabled(true);
    this->ui->clear_everything->setEnabled(false);
}


void WstrzasyMain::on_clear_everything_object_clicked()
{
    this->ui->input_object_filename->setText("in.txt");
    input_object_file_path = "in.txt";
    this->ui->browse_object->setEnabled(true);

    this->ui->input_shock_e->setEnabled(true);
    this->ui->input_shock_x->setEnabled(true);
    this->ui->input_shock_y->setEnabled(true);
    this->ui->input_shock_t->setEnabled(true);
    this->ui->input_shock_h->setEnabled(true);
    this->ui->input_shock_date->setEnabled(true);
    this->ui->input_shock_time->setEnabled(true);

    this->ui->input_shock_e->setText("");
    this->ui->input_shock_x->setText("");
    this->ui->input_shock_y->setText("");
    this->ui->input_shock_t->setText("");
    this->ui->input_shock_h->setText("0,5");
    this->ui->input_shock_date->setDate(now.currentDateTime().date());
    this->ui->input_shock_time->setTime(now.currentDateTime().time());

    this->ui->save->setEnabled(false);
    this->ui->calculate->setEnabled(false);

    this->ui->alert_text->setText(parser_choice_2_communicate);
    this->ui->input_data->setText("Oczekiwanie na dane wejściowe...");
    this->ui->output_data->setText("Oczekiwanie na wykonanie obliczeń...");

    zdarzenia.clear();
    dataline_q.clear();
    insert_to_incident_q.clear();

    errdump = "";

    this->ui->inject_object_data->setEnabled(true);
    this->ui->clear_everything_object->setEnabled(false);
}


void WstrzasyMain::on_save_clicked()
{
    QFileDialog save_file(this);
    save_file.setFileMode(QFileDialog::AnyFile);

    QString save_filename = save_file.getSaveFileName();

    if(save_filename != "") {
        if(!save_filename.endsWith(".txt")) {
            save_filename += ".txt";
        }

        QFile output(save_filename);
        output.open(QFile::WriteOnly);

        QTextStream to_output(&output);

        to_output << "Obliczanie stopnia intensywności sejsmicznej dla wstrząsów górniczych wg skali GSIS-2017\n";
        to_output << "Program wykonany na podstawie:\n";
        to_output << "G. Mutke - Metoda prognozowania parametrów drgań podłoża generowanych wstrząsami górniczymi w obszarze GZW.\nRozprawa doktorska, Główny Instytut Górnictwa, Katowice. (1991)\n\n";

        int incident_counter = 1;
        QString tmp_ln;
        QTextStream ln(&tmp_ln);

        if(this->ui->input_tabview->currentIndex() == 0) {
            to_output << "Wyniki obliczeń dla poniższego obiektu:\n";

            if(zdarzenia[0].obiekt.address != "") {
                to_output << "Adres:\t\t\t\t\t" << zdarzenia[0].obiekt.address << "\n";
            }

            to_output << "Lokalizacja X [m]:\t\t\t" << zdarzenia[0].obiekt.X << "\n";
            to_output << "Lokalizacja Y [m]:\t\t\t" << zdarzenia[0].obiekt.Y << "\n";
            to_output << "Współczynnik amplifikacji drgań Wf:\t" << zdarzenia[0].obiekt.wf << "\n\n";

            to_output << "Wersja czytelna w edytorze tekstu (wersja do skopiowania do arkusza kalkulacyjnego niżej):\n";
            to_output << "l.p.\t\tEnergia [J]\tX [m]\t\tY[m]\t\tCzas [s]\tGłębokość [km]\tPGVHmax [m/s]\tPGAH10 [m/s^2]\tIGSIS2017 - PGVHmax [0-6]\tIGSIS2017 - PGAH10 [0-6]\tIGSIS2017 [0-6]\tData\t\tGodzina\n";

            for(Incident e : zdarzenia) {
                double pgv = e.PGVHmax();   // These variables are used twice so
                double pga = e.PGAH10();    // why not reduce the number of calculations?

                to_output << incident_counter << "\t\t";

                // Shock energy
                ln.setRealNumberNotation(QTextStream::SmartNotation);

                ln << e.wstrzas.e;
                to_output << tmp_ln.replace('.', ',') << "\t";

                if(tmp_ln.length() < 8) {
                    to_output << "\t";
                }

                tmp_ln = "";

                // Shock X
                ln << e.wstrzas.X;
                to_output << tmp_ln.replace('.', ',') << "\t";

                if(tmp_ln.length() < 8) {
                    to_output << "\t";
                }

                tmp_ln = "";

                // Shock Y
                ln << e.wstrzas.Y;
                to_output << tmp_ln.replace('.', ',') << "\t";

                if(tmp_ln.length() < 8) {
                    to_output << "\t";
                }

                tmp_ln = "";

                // Shock time
                ln << e.wstrzas.t;
                to_output << tmp_ln.replace('.', ',') << "\t";

                if(tmp_ln.length() < 8) {
                    to_output << "\t";
                }

                tmp_ln = "";

                // Shock depth
                ln << e.wstrzas.h;
                to_output << tmp_ln.replace('.', ',') << "\t";

                if(tmp_ln.length() < 8) {
                    to_output << "\t";
                }

                tmp_ln = "";

                // PGVHmax
                ln << pgv;
                to_output << tmp_ln.replace('.', ',') << "\t";

                if(tmp_ln.length() < 8) {
                    to_output << "\t";
                }

                tmp_ln = "";

                // PGAH10
                ln << pga;
                to_output << tmp_ln.replace('.', ',') << "\t";

                if(tmp_ln.length() < 8) {
                    to_output << "\t";
                }

                tmp_ln = "";

                // Levels
                ln << e.PGVHmax_level();
                to_output << tmp_ln.replace('.', ',') << "\t\t\t\t";
                tmp_ln = "";

                ln << e.PGAH10_level();
                to_output << tmp_ln.replace('.', ',') << "\t\t\t\t";
                tmp_ln = "";

                ln << e.overall_level();
                to_output << tmp_ln.replace('.', ',');
                tmp_ln = "";

                // Optional date and time
                if(e.wstrzas.date != "") {
                    to_output << "\t\t" << e.wstrzas.date;

                    if(e.wstrzas.time != "") {
                        to_output << "\t" << e.wstrzas.time;
                    }
                }

                // New line
                to_output << "\n";

                incident_counter++;
            }

            to_output << "\nWersja do skopiowania do arkusza kalkulacyjnego (mniej czytelna w edytorze tekstu):\n";
            to_output << "l.p.\tEnergia [J]\tX [m]\tY [m]\tCzas [s]\tGłębokość [km]\tPGVHmax [m/s]\tPGAH10 [m/s^2]\tIGSIS2017 - PGVHmax [0-6]\tIGSIS2017 - PGAH10 [0-6]\tIGSIS2017 [0-6]\tData\tGodzina\n";
            incident_counter = 1;

            for(Incident e : zdarzenia) {
                double pgv = e.PGVHmax();   // These variables are used twice so
                double pga = e.PGAH10();    // why not reduce the number of calculations?

                to_output << incident_counter << "\t";

                // Shock energy
                ln.setRealNumberNotation(QTextStream::SmartNotation);

                ln << e.wstrzas.e;
                to_output << tmp_ln.replace('.', ',') << "\t";
                tmp_ln = "";

                // Shock X
                ln << e.wstrzas.X;
                to_output << tmp_ln.replace('.', ',') << "\t";
                tmp_ln = "";

                // Shock Y
                ln << e.wstrzas.Y;
                to_output << tmp_ln.replace('.', ',') << "\t";
                tmp_ln = "";

                // Shock time
                ln << e.wstrzas.t;
                to_output << tmp_ln.replace('.', ',') << "\t";
                tmp_ln = "";

                // Shock depth
                ln << e.wstrzas.h;
                to_output << tmp_ln.replace('.', ',') << "\t";
                tmp_ln = "";

                // PGVHmax
                ln << pgv;
                to_output << tmp_ln.replace('.', ',') << "\t";
                tmp_ln = "";

                // PGAH10
                ln << pga;
                to_output << tmp_ln.replace('.', ',') << "\t";
                tmp_ln = "";

                // Levels
                ln << e.PGVHmax_level();
                to_output << tmp_ln.replace('.', ',') << "\t";
                tmp_ln = "";

                ln << e.PGAH10_level();
                to_output << tmp_ln.replace('.', ',') << "\t";
                tmp_ln = "";

                ln << e.overall_level();
                to_output << tmp_ln.replace('.', ',');
                tmp_ln = "";

                // Optional date and time
                if(e.wstrzas.date != "") {
                    to_output << "\t" << e.wstrzas.date;

                    if(e.wstrzas.time != "") {
                        to_output << "\t" << e.wstrzas.time;
                    }
                }

                // New line
                to_output << "\n";

                incident_counter++;
            }
        } else if(this->ui->input_tabview->currentIndex() == 1) {
            to_output << "Wyniki obliczeń dla poniższego zdarzenia:\n";

            if(zdarzenia[0].wstrzas.date != "") {
                to_output << "Data wystąpienia:\t" << zdarzenia[0].wstrzas.date << "\n";

                if(zdarzenia[0].wstrzas.time != "") {
                    to_output << "Godzina wystąpienia:\t" << zdarzenia[0].wstrzas.time << "\n";
                }
            }

            to_output << "Lokalizacja X [m]:\t" << zdarzenia[0].wstrzas.X << "\n";
            to_output << "Lokalizacja Y [m]:\t" << zdarzenia[0].wstrzas.Y << "\n";

            to_output << "Czas trwania [s]:\t" << zdarzenia[0].wstrzas.t << "\n";
            to_output << "Głębokość ogniska [km]:\t" << zdarzenia[0].wstrzas.h << "\n\n";

            to_output << "Wersja czytelna w edytorze tekstu (wersja do skopiowania do arkusza kalkulacyjnego niżej):\n";
            to_output << "l.p.\t\tX [m]\t\tY [m]\t\tWsp. amplifikacji drgań (Wf)\tPGVHmax [m/s]\tPGAH10 [m/s^2]\tIGSIS2017 - PGVHmax [0-6]\tIGSIS2017 - PGAH10 [0-6]\tIGSIS2017 [0-6]\tAdres obiektu\n";

            for(Incident e : zdarzenia) {
                double pgv = e.PGVHmax();   // These variables are used twice so
                double pga = e.PGAH10();    // why not reduce the number of calculations?

                to_output << incident_counter << "\t\t";

                // Object X
                ln.setRealNumberNotation(QTextStream::SmartNotation);

                ln << e.obiekt.X;
                to_output << tmp_ln.replace('.', ',') << "\t";

                if(tmp_ln.length() < 8) {
                    to_output << "\t";
                }

                tmp_ln = "";

                // Object Y
                ln << e.obiekt.Y;
                to_output << tmp_ln.replace('.', ',') << "\t";

                if(tmp_ln.length() < 8) {
                    to_output << "\t";
                }

                tmp_ln = "";

                // Object shock amplification factor
                ln << e.obiekt.wf;
                to_output << tmp_ln.replace('.', ',') << "\t\t";

                if(tmp_ln.length() < 16) {
                    to_output << "\t";

                    if(tmp_ln.length() < 8) {
                        to_output << "\t";
                    }
                }

                tmp_ln = "";

                // PGVHmax
                ln << pgv;
                to_output << tmp_ln.replace('.', ',') << "\t";

                if(tmp_ln.length() < 8) {
                    to_output << "\t";
                }

                tmp_ln = "";

                // PGAH10
                ln << pga;
                to_output << tmp_ln.replace('.', ',') << "\t";

                if(tmp_ln.length() < 8) {
                    to_output << "\t";
                }

                tmp_ln = "";

                // Levels
                ln << e.PGVHmax_level();
                to_output << tmp_ln.replace('.', ',') << "\t\t\t\t";
                tmp_ln = "";

                ln << e.PGAH10_level();
                to_output << tmp_ln.replace('.', ',') << "\t\t\t\t";
                tmp_ln = "";

                ln << e.overall_level();
                to_output << tmp_ln.replace('.', ',');
                tmp_ln = "";

                // Optional address
                if(e.wstrzas.date != "") {
                    to_output << "\t\t" << e.obiekt.address;
                }

                // New line
                to_output << "\n";

                incident_counter++;
            }

            to_output << "\nWersja do skopiowania do arkusza kalkulacyjnego (mniej czytelna w edytorze tekstu):\n";
            to_output << "l.p.\tX [m]\tY [m]\tWsp. amplifikacji drgań (Wf)\tPGVHmax [m/s]\tPGAH10 [m/s^2]\tIGSIS2017 - PGVHmax [0-6]\tIGSIS2017 - PGAH10 [0-6]\tIGSIS2017 [0-6]\tAdres obiektu\n";
            incident_counter = 1;

            for(Incident e : zdarzenia) {
                double pgv = e.PGVHmax();   // These variables are used twice so
                double pga = e.PGAH10();    // why not reduce the number of calculations?

                to_output << incident_counter << "\t";

                // Object X
                ln.setRealNumberNotation(QTextStream::SmartNotation);

                ln << e.obiekt.X;
                to_output << tmp_ln.replace('.', ',') << "\t";
                tmp_ln = "";

                // Object Y
                ln << e.obiekt.Y;
                to_output << tmp_ln.replace('.', ',') << "\t";
                tmp_ln = "";

                // Object shock amplification factor
                ln << e.obiekt.wf;
                to_output << tmp_ln.replace('.', ',') << "\t";
                tmp_ln = "";

                // PGVHmax
                ln << pgv;
                to_output << tmp_ln.replace('.', ',') << "\t";
                tmp_ln = "";

                // PGAH10
                ln << pga;
                to_output << tmp_ln.replace('.', ',') << "\t";
                tmp_ln = "";

                // Levels
                ln << e.PGVHmax_level();
                to_output << tmp_ln.replace('.', ',') << "\t";
                tmp_ln = "";

                ln << e.PGAH10_level();
                to_output << tmp_ln.replace('.', ',') << "\t";
                tmp_ln = "";

                ln << e.overall_level();
                to_output << tmp_ln.replace('.', ',');
                tmp_ln = "";

                // Optional address
                if(e.obiekt.address != "") {
                    to_output << "\t" << e.obiekt.address;
                }

                // New line
                to_output << "\n";

                incident_counter++;
            }
        } else {
            to_output << "Wyniki obliczenia dla poniższych danych:\n";

            to_output << "Zdarzenie:\n";

            if(zdarzenia[0].wstrzas.date != "") {
                to_output << "Data wystąpienia:\t\t\t" << zdarzenia[0].wstrzas.date << "\n";

                if(zdarzenia[0].wstrzas.time != "") {
                    to_output << "Godzina wystąpienia:\t\t\t" << zdarzenia[0].wstrzas.time << "\n";
                }
            }

            to_output << "Lokalizacja X [m]:\t\t\t" << zdarzenia[0].wstrzas.X << "\n";
            to_output << "Lokalizacja Y [m]:\t\t\t" << zdarzenia[0].wstrzas.Y << "\n";

            to_output << "Czas trwania [s]:\t\t\t" << zdarzenia[0].wstrzas.t << "\n";
            to_output << "Głębokość ogniska [km]:\t\t\t" << zdarzenia[0].wstrzas.h << "\n\n";

            to_output << "Obiekt:\n";

            if(zdarzenia[0].obiekt.address != "") {
                to_output << "Adres:\t\t\t\t\t" << zdarzenia[0].obiekt.address << "\n";
            }

            to_output << "Lokalizacja X [m]:\t\t\t" << zdarzenia[0].obiekt.X << "\n";
            to_output << "Lokalizacja Y [m]:\t\t\t" << zdarzenia[0].obiekt.Y << "\n";
            to_output << "Współczynnik amplifikacji drgań Wf:\t" << zdarzenia[0].obiekt.wf << "\n\n";

            to_output << "Wynik:\n";

            to_output << "PGVHmax [m/s]:\t\t\t\t" << zdarzenia[0].PGVHmax() << "\n";
            to_output << "PGAH10 [m/s^2]:\t\t\t\t" << zdarzenia[0].PGAH10() << "\n";
            to_output << "IGSIS2017 - PGVHmax [0-6]:\t\t" << zdarzenia[0].PGVHmax_level() << "\n";
            to_output << "IGSIS2017 - PGAH10 [0-6]:\t\t" << zdarzenia[0].PGAH10_level() << "\n";
            to_output << "IGSIS2017 [0-6]:\t\t\t" << zdarzenia[0].overall_level() << "\n\n";
        }

        output.close();

        // Message informing about file being saved successfully
        QMessageBox save_msg;
        save_msg.setWindowTitle("Gotowe!");
        save_msg.setInformativeText("Plik o podanej nazwie został zapisany!");
        save_msg.setStandardButtons(QMessageBox::Close);
        save_msg.exec();
    }
}


void WstrzasyMain::on_browse_button_clicked()
{
    QFileDialog browse_file(this);
    browse_file.tr("text files (*.txt)");

    QString browse_filename = browse_file.getOpenFileName();

    this->ui->input_filename->setText(browse_filename);
    input_file_path = browse_filename;
}


void WstrzasyMain::on_clear_everything_onebyone_clicked()
{
    this->ui->onebyone_incident_e->setEnabled(true);
    this->ui->onebyone_incident_x->setEnabled(true);
    this->ui->onebyone_incident_y->setEnabled(true);
    this->ui->onebyone_incident_t->setEnabled(true);
    this->ui->onebyone_incident_h->setEnabled(true);

    this->ui->onebyone_date->setEnabled(true);
    this->ui->onebyone_time->setEnabled(true);

    this->ui->onebyone_object_x->setEnabled(true);
    this->ui->onebyone_object_y->setEnabled(true);
    this->ui->onebyone_object_wf->setEnabled(true);
    this->ui->onebyone_object_address->setEnabled(true);

    this->ui->inject_data_onebyone->setEnabled(true);
    this->ui->clear_everything_onebyone->setEnabled(false);

    this->ui->onebyone_incident_e->setText("");
    this->ui->onebyone_incident_x->setText("");
    this->ui->onebyone_incident_y->setText("");
    this->ui->onebyone_incident_t->setText("");
    this->ui->onebyone_incident_h->setText("0,5");

    this->ui->onebyone_date->setDate(now.currentDateTime().date());
    this->ui->onebyone_time->setTime(now.currentDateTime().time());

    this->ui->onebyone_object_x->setText("");
    this->ui->onebyone_object_y->setText("");
    this->ui->onebyone_object_wf->setText("");
    this->ui->onebyone_object_address->setText("");

    zdarzenia.clear();

    this->ui->alert_text->setText("");
    this->ui->input_data->setText("Oczekiwanie na dane wejściowe...");
    this->ui->output_data->setText("Oczekiwanie na wykonanie obliczeń...");

    errdump = "";
}


void WstrzasyMain::on_inject_data_onebyone_clicked()
{
    Incident zdarzenie;
    std::string temp;
    bool do_continue = true;

    if(do_continue) {
        try {
            temp = this->ui->onebyone_incident_e->text().toStdString();
            std::replace(temp.begin(), temp.end(), ',', '.');
            zdarzenie.wstrzas.e = std::stod(temp);
            errdump += "Energia sejsmiczna wczytana pomyślnie.\n\n";
        } catch (...) {
            errdump += "Błąd wczytywania wartości energii sejsmicznej!\n\n";
            do_continue = false;
        }
    }

    if(do_continue) {
        try {
            temp = this->ui->onebyone_incident_x->text().toStdString();
            std::replace(temp.begin(), temp.end(), ',', '.');
            zdarzenie.wstrzas.X = std::stod(temp);
            errdump += "X zdarzenia wczytane pomyślnie.\n\n";
        } catch (...) {
            errdump += "Błąd wczytywania wartości X zdarzenia!\n\n";
            do_continue = false;
        }
    }

    if(do_continue) {
        try {
            temp = this->ui->onebyone_incident_y->text().toStdString();
            std::replace(temp.begin(), temp.end(), ',', '.');
            zdarzenie.wstrzas.Y = std::stod(temp);
            errdump += "Y zdarzenia wczytane pomyślnie.\n\n";
        } catch (...) {
            errdump += "Błąd wczytywania wartości Y zdarzenia!\n\n";
            do_continue = false;
        }
    }

    if(do_continue) {
        try {
            temp = this->ui->onebyone_incident_t->text().toStdString();
            std::replace(temp.begin(), temp.end(), ',', '.');
            zdarzenie.wstrzas.t = std::stod(temp);
            errdump += "Czas zdarzenia wczytany pomyślnie.\n\n";
        } catch (...) {
            errdump += "Błąd wczytywania czasu zdarzenia!\n\n";
            do_continue = false;
        }
    }

    if(do_continue) {
        try {
            temp = this->ui->onebyone_incident_h->text().toStdString();
            std::replace(temp.begin(), temp.end(), ',', '.');
            zdarzenie.wstrzas.h = std::stod(temp);
            errdump += "Głębokość ogniska zdarzenia wczytana pomyślnie.\n\n";
        } catch (...) {
            errdump += "Błąd wczytywania wartości głębokości ogniska zdarzenia!\n\n";
            do_continue = false;
        }
    }

    if(do_continue) {
        try {
            temp = this->ui->onebyone_object_x->text().toStdString();
            std::replace(temp.begin(), temp.end(), ',', '.');
            zdarzenie.obiekt.X = std::stod(temp);
            errdump += "X obiektu wczytane pomyślnie.\n\n";
        } catch (...) {
            errdump += "Błąd wczytywania wartości X obiektu!\n\n";
            do_continue = false;
        }
    }

    if(do_continue) {
        try {
            temp = this->ui->onebyone_object_y->text().toStdString();
            std::replace(temp.begin(), temp.end(), ',', '.');
            zdarzenie.obiekt.Y = std::stod(temp);
            errdump += "Y obiektu wczytane pomyślnie.\n\n";
        } catch (...) {
            errdump += "Błąd wczytywania wartości Y obiektu!\n\n";
            do_continue = false;
        }
    }

    if(do_continue) {
        try {
            temp = this->ui->onebyone_object_wf->text().toStdString();
            std::replace(temp.begin(), temp.end(), ',', '.');
            zdarzenie.obiekt.wf = std::stod(temp);
            errdump += "Współczynnik amplifikacji dla obiektu wczytany pomyślnie.\n\n";
        } catch (...) {
            errdump += "Błąd wczytywania współczynnika amplifikacji dla obiektu!\n\n";
            do_continue = false;
        }
    }

    this->ui->alert_text->setText(errdump);

    if(do_continue) {
        // Adjust UI
        this->ui->onebyone_incident_e->setEnabled(false);
        this->ui->onebyone_incident_x->setEnabled(false);
        this->ui->onebyone_incident_y->setEnabled(false);
        this->ui->onebyone_incident_t->setEnabled(false);
        this->ui->onebyone_incident_h->setEnabled(false);

        this->ui->onebyone_date->setEnabled(false);
        this->ui->onebyone_time->setEnabled(false);

        this->ui->onebyone_object_x->setEnabled(false);
        this->ui->onebyone_object_y->setEnabled(false);
        this->ui->onebyone_object_wf->setEnabled(false);
        this->ui->onebyone_object_address->setEnabled(false);

        this->ui->inject_data_onebyone->setEnabled(false);
        this->ui->clear_everything_onebyone->setEnabled(true);

        zdarzenie.obiekt.address = this->ui->onebyone_object_address->text();

        if(this->ui->onebyone_date->date().day() < 10) {
            zdarzenie.wstrzas.date = "0";
        }

        zdarzenie.wstrzas.date += (std::to_string(this->ui->onebyone_date->date().day()) + ".");

        if(this->ui->onebyone_date->date().month() < 10) {
            zdarzenie.wstrzas.date += "0";
        }

        zdarzenie.wstrzas.date += (std::to_string(this->ui->onebyone_date->date().month()) + "." + std::to_string(this->ui->onebyone_date->date().year()));

        if(this->ui->onebyone_time->time().hour() < 10) {
            zdarzenie.wstrzas.time = "0";
        }

        zdarzenie.wstrzas.time += (std::to_string(this->ui->onebyone_time->time().hour()) + ":");

        if(this->ui->onebyone_time->time().minute() < 10) {
            zdarzenie.wstrzas.time += "0";
        }

        zdarzenie.wstrzas.time += (std::to_string(this->ui->onebyone_time->time().minute()));

        zdarzenia.push_back(zdarzenie);

        // Push data into input data field
        this->ui->input_data->setText("[Obliczenie 1]");

        if(zdarzenia[0].wstrzas.date != "") {
            this->ui->input_data->setText(this->ui->input_data->toPlainText() + "\nData zdarzenia: " + zdarzenia[0].wstrzas.date);
        }
        if(zdarzenia[0].wstrzas.time != "") {
            this->ui->input_data->setText(this->ui->input_data->toPlainText() + "\nGodzina zdarzenia: " + zdarzenia[0].wstrzas.time);
        }

        this->ui->input_data->setText(this->ui->input_data->toPlainText() + "\nE zdarzenia: " + QString::fromStdString(std::to_string(zdarzenia[0].wstrzas.e)));
        this->ui->input_data->setText(this->ui->input_data->toPlainText() + "\nX zdarzenia: " + QString::fromStdString(std::to_string(zdarzenia[0].wstrzas.X)));
        this->ui->input_data->setText(this->ui->input_data->toPlainText() + "\nY zdarzenia: " + QString::fromStdString(std::to_string(zdarzenia[0].wstrzas.Y)));
        this->ui->input_data->setText(this->ui->input_data->toPlainText() + "\nt zdarzenia: " + QString::fromStdString(std::to_string(zdarzenia[0].wstrzas.t)));
        this->ui->input_data->setText(this->ui->input_data->toPlainText() + "\nh zdarzenia: " + QString::fromStdString(std::to_string(zdarzenia[0].wstrzas.h)));

        if(zdarzenia[0].obiekt.address != "") {
            this->ui->input_data->setText(this->ui->input_data->toPlainText() + "\nAdres obiektu: " + zdarzenia[0].obiekt.address);
        }
        this->ui->input_data->setText(this->ui->input_data->toPlainText() + "\nX obiektu: " + QString::fromStdString(std::to_string(zdarzenia[0].obiekt.X)));
        this->ui->input_data->setText(this->ui->input_data->toPlainText() + "\nY obiektu: " + QString::fromStdString(std::to_string(zdarzenia[0].obiekt.Y)));
        this->ui->input_data->setText(this->ui->input_data->toPlainText() + "\nWf obiektu: " + QString::fromStdString(std::to_string(zdarzenia[0].obiekt.wf)));

        this->ui->calculate->setEnabled(true);
    } else {
        QMessageBox error_onebyone;
        error_onebyone.setWindowTitle("Błąd!");
        error_onebyone.setInformativeText("Jedna lub więcej wartości nie zostały wprowadzone prawidłowo. Po więcej szczegółów sprawdź pole Komunikaty.");
        error_onebyone.setStandardButtons(QMessageBox::Close);
        error_onebyone.exec();
    }
}


void WstrzasyMain::on_input_tabview_currentChanged(int index)
{
    this->ui->save->setEnabled(false);
    this->ui->calculate->setEnabled(false);

    this->ui->input_data->setText("Oczekiwanie na dane wejściowe...");
    this->ui->output_data->setText("Oczekiwanie na wykonanie obliczeń...");

    zdarzenia.clear();

    errdump = "";
    this->ui->alert_text->setText(parser_choice_1_communicate);

    this->ui->inject_incidents->setEnabled(true);
    this->ui->clear_everything->setEnabled(false);

    if(index == 0) {
        this->ui->alert_text->setText(parser_choice_1_communicate);

        this->ui->input_filename->setText("in.txt");
        input_file_path = "in.txt";
        this->ui->browse_button->setEnabled(true);

        this->ui->input_object_x->setEnabled(true);
        this->ui->input_object_y->setEnabled(true);
        this->ui->input_object_wf->setEnabled(true);
        this->ui->input_object_address->setEnabled(true);

        this->ui->input_object_x->setText("");
        this->ui->input_object_y->setText("");
        this->ui->input_object_wf->setText("");
        this->ui->input_object_address->setText("");

        this->ui->input_data->setText("Oczekiwanie na dane wejściowe...");
        this->ui->output_data->setText("Oczekiwanie na wykonanie obliczeń...");

        errdump = "";

        zdarzenia.clear();
        dataline_q.clear();
        insert_to_incident_q.clear();
    } else if (index == 1) {
        this->ui->alert_text->setText(parser_choice_2_communicate);

        this->ui->input_object_filename->setText("in.txt");
        input_object_file_path = "in.txt";
        this->ui->browse_object->setEnabled(true);

        this->ui->input_shock_e->setEnabled(true);
        this->ui->input_shock_x->setEnabled(true);
        this->ui->input_shock_y->setEnabled(true);
        this->ui->input_shock_t->setEnabled(true);
        this->ui->input_shock_h->setEnabled(true);
        this->ui->input_shock_date->setEnabled(true);
        this->ui->input_shock_time->setEnabled(true);

        this->ui->input_shock_e->setText("");
        this->ui->input_shock_x->setText("");
        this->ui->input_shock_y->setText("");
        this->ui->input_shock_t->setText("");
        this->ui->input_shock_h->setText("0,5");
        this->ui->input_shock_date->setDate(now.currentDateTime().date());
        this->ui->input_shock_time->setTime(now.currentDateTime().time());

        this->ui->input_data->setText("Oczekiwanie na dane wejściowe...");
        this->ui->output_data->setText("Oczekiwanie na wykonanie obliczeń...");

        errdump = "";

        zdarzenia.clear();
        dataline_q.clear();
        insert_to_incident_q.clear();
    } else {
        this->ui->alert_text->setText("");

        this->ui->onebyone_incident_e->setEnabled(true);
        this->ui->onebyone_incident_x->setEnabled(true);
        this->ui->onebyone_incident_y->setEnabled(true);
        this->ui->onebyone_incident_t->setEnabled(true);
        this->ui->onebyone_incident_h->setEnabled(true);

        this->ui->onebyone_date->setEnabled(true);
        this->ui->onebyone_time->setEnabled(true);

        this->ui->onebyone_object_x->setEnabled(true);
        this->ui->onebyone_object_y->setEnabled(true);
        this->ui->onebyone_object_wf->setEnabled(true);
        this->ui->onebyone_object_address->setEnabled(true);

        this->ui->inject_data_onebyone->setEnabled(true);
        this->ui->clear_everything_onebyone->setEnabled(false);

        this->ui->onebyone_incident_e->setText("");
        this->ui->onebyone_incident_x->setText("");
        this->ui->onebyone_incident_y->setText("");
        this->ui->onebyone_incident_t->setText("");
        this->ui->onebyone_incident_h->setText("0,5");

        this->ui->onebyone_date->setDate(now.currentDateTime().date());
        this->ui->onebyone_time->setTime(now.currentDateTime().time());

        this->ui->onebyone_object_x->setText("");
        this->ui->onebyone_object_y->setText("");
        this->ui->onebyone_object_wf->setText("");
        this->ui->onebyone_object_address->setText("");

        this->ui->input_data->setText("Oczekiwanie na dane wejściowe...");
        this->ui->output_data->setText("Oczekiwanie na wykonanie obliczeń...");

        errdump = "";

        zdarzenia.clear();
        dataline_q.clear();
        insert_to_incident_q.clear();
    }
}


void WstrzasyMain::on_browse_object_clicked()
{
    QFileDialog browse_object(this);
    browse_object.tr("text files (*.txt)");

    QString browse_object_filename = browse_object.getOpenFileName();

    this->ui->input_object_filename->setText(browse_object_filename);
    input_object_file_path = browse_object_filename;
}


