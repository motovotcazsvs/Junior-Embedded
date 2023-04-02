#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QTime>
#include <QSql>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>


void createFileCSV();
void editFileCSV();
void createDataBase();
void editDataBase();

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    createFileCSV();
    editFileCSV();
    createDataBase();

    return a.exec();
}

//створив файл csv, та наповнив його згенерованими випадковими символами
void createFileCSV()
{
    QFile file("randomСharacters.csv");
    QTextStream out(&file);
    if(file.open(QIODevice::WriteOnly | QIODevice::Truncate)){
        QTime point(0, 0, 0);
        qsrand(point.secsTo(QTime::currentTime()));
        QString c;
        c.append("0123456789AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz");
        for(int record = 1; record <= 1024; record++){
            for(int number_column = 1; number_column <= 6; number_column++){
                for(int width_column = 1; width_column <= 8; width_column++){
                    out << c.at(qrand()%(int)c.length());
                }
                if(number_column != 6) out << ",";
            }
            if(record != 1024) out << "\r\n";
        }
        file.close();
    }

}


//зчитує файл csv, замінює не парні цифри на # та видаляє запис у якому всі 6 рядків починаються з голосної букви
void editFileCSV()
{
//считываем файл
    QFile file("randomСharacters.csv");
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
        QTextStream in(&file);
        QStringList records;
        while(!in.atEnd()){
            records.append(in.readLine());
        }
        file.close();

//заменяем символ не четной цифры на #
        QFile file_edit("randomСharactersEdit.csv");
        if(file_edit.open(QIODevice::WriteOnly | QIODevice::Truncate)){
            QTextStream out(&file_edit);
            int position_records = 1;
            foreach(QString record, records){
                QString edit_record;
                int position_record = 0;
                bool delete_column[6];
                for(int i = 0; i < 6; i++) delete_column[i] = false;
                foreach(QChar c, record){
                    switch(c.unicode()){
                    case '1': case '3': case '5': case '7': case '9':
                        c = '#';
                        break;
                    }
                    edit_record.append(c);

//удаляем строки с гласными буквами в начале столбца
                    if(position_record % 9 == 0){
                        switch(c.unicode()){
                        case 'A': case 'a': case 'E': case 'e': case 'I': case 'i':
                        case 'O': case 'o': case 'U': case 'u': case 'Y': case 'y':
                            delete_column[position_record / 9] = true;
                            break;
                        }
                    }
                    position_record++;
                }
                if(delete_column[0] && delete_column[1] && delete_column[2] && delete_column[3] && delete_column[4] && delete_column[5]){
                    qDebug() << "delete record position" << position_records << record;
                }
                else {
                    out << edit_record;
                    out << "\r\n";
                }
                position_records++;
            }
        }
        file_edit.close();
    }
}

//зчитує файл randomСharacters.csv, створює базу даних randomСharacters.db і заповнюєм її даними зчитаними
void createDataBase()
{
//считываем файл
    QFile file("randomСharacters.csv");
    QStringList records;
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
        QTextStream in(&file);
        while(!in.atEnd()){
            records.append(in.readLine());
        }
        file.close();
    }

//создаем файл базы данных
    if(QFile::exists("randomСharacters.db")){
        QFile::remove("randomСharacters.db");
    }
    QSqlDatabase db;
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("randomСharacters.db");
    if(db.open() == false){
        qDebug() << "error open data base!";
        return;
    }
//создаем таблицу
    QSqlQuery query;
    if(!query.exec("CREATE TABLE RANDOMCHAR (Column1 TEXT, Column2 TEXT, Column3 TEXT, Colum41 TEXT, Column5 TEXT, Column6 TEXT)")){
        qDebug() << "DataBase: error of create RANDOMCHAR";
        qDebug() << query.lastError().text();
    }

//заполняем таблицу
    foreach(QString record, records){
        query.prepare("INSERT INTO RANDOMCHAR VALUES (:Column1, :Column2, :Column3, :Column4, :Column5, :Column6)");
        query.bindValue(":Column1", record.mid(0, 8));
        query.bindValue(":Column2", record.mid(9, 8));
        query.bindValue(":Column3", record.mid(18, 8));
        query.bindValue(":Column4", record.mid(27, 8));
        query.bindValue(":Column5", record.mid(36, 8));
        query.bindValue(":Column6", record.mid(45, 8));
        if(!query.exec()){
            qDebug() << "error insert into RANDOMCHAR";
            qDebug() << query.lastError().text();
        }
    }
    editDataBase();
    db.close();
}

//видаляє записи, у яких у другому стовпці перший символ цифра.
void editDataBase()
{
//удаляем строки
    QSqlQuery query;
    if(!query.exec("DELETE FROM RANDOMCHAR WHERE Column2 GLOB '[0-9]*'")){
        qDebug() << "error delete row RANDOMCHAR";
        qDebug() << query.lastError().text();
    }
}
