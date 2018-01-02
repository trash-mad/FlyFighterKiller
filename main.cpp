#include <QApplication>
#include <QFile>
#include <QByteArray>
#include <QMainWindow>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include <QDialog>
#include <QFrame>
#include <QHBoxLayout>

#include <iostream>
#include "resources.cpp"

#include <QTimer>
#include <QObject>
#include <QPushButton>
#include <QLabel>
#include <QTableView>
#include <QStandardItemModel>
#include <QStandardItem>

#include <QMessageBox>
#include <QInputDialog>
Resources res;

//Для получения лямбды в параметры конструктора
#include <functional>

class RecordsWindow{
QDialog *dialog;
QHBoxLayout *horizontalLayout;
QFrame *frame;
QVBoxLayout *verticalLayout;
QLabel *label;
QTableView *tableView;

public:
    RecordsWindow(){
        dialog=new QDialog();
        setupUi(dialog);

        //Заполнение таблицы рекордов
        QStandardItemModel *model = new QStandardItemModel;
        QStandardItem *item;

        QStringList horizontalHeader;
        horizontalHeader << "Имя" << "Сбито самолетов";
        model->setHorizontalHeaderLabels(horizontalHeader);

        QSqlQuery query;
        int currentline=0;
        query.exec("SELECT Рекорды.[Имя], Рекорды.[Число сбитых самолетов] FROM Рекорды ORDER BY  Рекорды.[Число сбитых самолетов] DESC;");
        while (query.next())
        {
            item = new QStandardItem(query.value(0).toString());
            model->setItem(currentline, 0, item);
            item = new QStandardItem(query.value(1).toString());
            model->setItem(currentline, 1, item);
            currentline++;
        }

        tableView->setModel(model);
        tableView->resizeRowsToContents();
        tableView->resizeColumnsToContents();

        dialog->show();
    }

    void setupUi(QDialog *Dialog)
    {
        Dialog->resize(518, 518);
        horizontalLayout = new QHBoxLayout(Dialog);
        frame = new QFrame(Dialog);
        frame->setMinimumSize(QSize(500, 500));
        frame->setMaximumSize(QSize(500, 500));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        verticalLayout = new QVBoxLayout(frame);
        label = new QLabel(frame);
        label->setMinimumSize(QSize(0, 61));
        label->setMaximumSize(QSize(16777215, 61));
        QFont font;
        font.setFamily(QStringLiteral("Segoe UI"));
        font.setPointSize(20);
        font.setBold(false);
        font.setWeight(50);
        label->setFont(font);
        label->setAlignment(Qt::AlignCenter);
        label->setText("Рекорды");
        verticalLayout->addWidget(label);
        tableView = new QTableView(frame);
        verticalLayout->addWidget(tableView);
        horizontalLayout->addWidget(frame);
    }
};

class GameWindow{

const int DefaultFireBallX=215;
const int DefaultFireBallY=230;

const int DefaultBombX=25;
const int MaxBombY=255;

QDialog *Dialog;
QHBoxLayout *horizontalLayout;
QFrame *GameField;
QFrame *FlyFighter;
QFrame *FireBall;
QFrame *Bomb;

QTimer *ftimer; //Таймер истребителя зеро
QTimer *atimer; //Таймер артилерии
QTimer *btimer; //Таймер бомбы

QPushButton *Button; //Кнопка вызова огня
std::function<void (int)> callback;

int count=0;
double flyfighterspeed=8.0;
public:
    GameWindow(std::function<void (int)> callback){
        this->callback=callback;
        Dialog = new QDialog();
        setupUi(Dialog);

        QObject::connect(Dialog,&QDialog::accepted,[this](){FinishGame();});

        ftimer=new QTimer();
        ftimer->setInterval(8);
        QObject::connect(ftimer,&QTimer::timeout,[this](){

            if(FlyFighter->pos().x()==-1*FlyFighter->width()){
                FlyFighter->move(GameField->width()+FlyFighter->width(),FlyFighter->pos().y());
                FlyFighter->setVisible(true);
            }

            FlyFighter->move(FlyFighter->pos().x()-1,FlyFighter->pos().y());

            if(FlyFighter->isVisible()&&FlyFighter->pos().x()==DefaultBombX){
                Bomb->setVisible(true);
                btimer->start();
                ftimer->stop();
                FlyFighter->setVisible(false);
            }
        });

        atimer=new QTimer();
        atimer->setInterval(7);
        QObject::connect(atimer,&QTimer::timeout,[this](){
            FireBall->move(FireBall->pos().x()+1,FireBall->pos().y()-2);

            if(FireBall->isVisible()&&FireBall->pos().x()>FlyFighter->pos().x()&&FireBall->pos().x()<FlyFighter->pos().x()+FlyFighter->width()){
                if(FireBall->pos().y()<FlyFighter->pos().y()&&FireBall->pos().y()>FlyFighter->pos().y()-FlyFighter->height()){
                    qDebug()<<"Попадание!";
                    atimer->stop();
                    count++;
                    if(flyfighterspeed>4)flyfighterspeed-=0.20;
                    ftimer->setInterval((int)flyfighterspeed);
                    FireBall->setVisible(false);
                    FlyFighter->setVisible(false);
                    return;
                }
            }

            if(FireBall->pos().x()>GameField->height()){
                atimer->stop();
                FireBall->setVisible(false);
            }

        });

        btimer=new QTimer();
        btimer->setInterval(7);
        QObject::connect(btimer,&QTimer::timeout,[this](){
            Bomb->move(Bomb->pos().x(),Bomb->pos().y()+1);
            if(Bomb->pos().y()>MaxBombY){
                btimer->stop();
                Bomb->setVisible(false);
                Dialog->accept();
            }
        });

        ftimer->start();

        Dialog->show();
    }

private:

    void setupUi(QDialog *Dialog)
    {
        Dialog->setObjectName(QStringLiteral("Dialog"));
        Dialog->resize(500, 500);
        Dialog->setStyleSheet(QLatin1String("#Dialog{background-color: black;} #GameButton{color: transparent; background-color: transparent;} #GameField{background-color: white; background-image: url(\""+qApp->applicationDirPath().toLatin1()+"//background.png\");} #FlyFighter{background-image: url(\""+qApp->applicationDirPath().toLatin1()+"//flyfighter.png\");}#FireBall{background-image: url(\""+qApp->applicationDirPath().toLatin1()+"//fireball.png\");}#Bomb{background-image: url(\""+qApp->applicationDirPath().toLatin1()+"//bomb.png\");}"));
        horizontalLayout = new QHBoxLayout(Dialog);
        GameField = new QFrame(Dialog);
        GameField->setObjectName(QStringLiteral("GameField"));
        GameField->setMinimumSize(QSize(480, 320));
        GameField->setMaximumSize(QSize(480, 320));
        GameField->setFrameShape(QFrame::StyledPanel);
        GameField->setFrameShadow(QFrame::Raised);
        horizontalLayout->addWidget(GameField);
        FlyFighter=new QFrame(GameField);
        FlyFighter->setObjectName(QStringLiteral("FlyFighter"));
        FlyFighter->setMinimumSize(QSize(50, 13));
        FlyFighter->setMaximumSize(QSize(50, 13));
        FlyFighter->move(GameField->width()+FlyFighter->width(),50);

        FireBall=new QFrame(GameField);
        FireBall->setObjectName(QStringLiteral("FireBall"));
        FireBall->setMinimumSize(QSize(10, 10));
        FireBall->setMaximumSize(QSize(10, 10));
        FireBall->move(DefaultFireBallX,DefaultFireBallY);
        FireBall->setVisible(false);

        Bomb=new QFrame(GameField);
        Bomb->setObjectName(QStringLiteral("Bomb"));
        Bomb->setMinimumSize(QSize(15, 17));
        Bomb->setMaximumSize(QSize(15, 17));
        Bomb->move(DefaultBombX,FlyFighter->pos().y()+FlyFighter->height()+3);
        Bomb->setVisible(false);


        Button = new QPushButton(GameField);
        Button->setGeometry(0,0,GameField->width(),GameField->height());
        Button->setObjectName(QStringLiteral("GameButton"));
        QObject::connect(Button,&QPushButton::clicked,[this](){
            if(!atimer->isActive()){
                FireBall->move(DefaultFireBallX,DefaultFireBallY);
                FireBall->setVisible(true);
                atimer->start();
            }
        });
    }

    //Очистка памяти и передача результата
    void FinishGame(){
        Dialog->close();
        delete Dialog;
        delete ftimer;
        delete atimer;
        delete btimer;
        callback(count);
    }

};

class MainWindow{
QHBoxLayout *horizontalLayout;
QFrame *frame;
QVBoxLayout *verticalLayout;
QLabel *label;
QPushButton *pushButton;
QPushButton *pushButton_2;
QDialog *dialog;

GameWindow *gwindow;
RecordsWindow *rwindow;

QString name;
public:
    MainWindow(){
        dialog=new QDialog();
        setupUi(dialog);

        bool ok;
        name = QInputDialog::getMultiLineText(Q_NULLPTR,"Введите ваше имя:", "Ваше имя:", "Сергей Пахомов", &ok);
        if (!ok)name="Без имени";
    }
    void setupUi(QDialog *Dialog)
    {
       Dialog->resize(567, 470);
       horizontalLayout = new QHBoxLayout(Dialog);
       frame = new QFrame(Dialog);
       frame->setMinimumSize(QSize(271, 261));
       frame->setMaximumSize(QSize(271, 261));
       frame->setFrameShape(QFrame::StyledPanel);
       frame->setFrameShadow(QFrame::Raised);
       verticalLayout = new QVBoxLayout(frame);
       label = new QLabel(frame);
       label->setMinimumSize(QSize(251, 127));
       label->setText("Какие самолеты?");
       QFont font;
       font.setFamily(QStringLiteral("Segoe UI"));
       font.setPointSize(20);
       font.setBold(false);
       font.setWeight(50);
       label->setFont(font);
       label->setAlignment(Qt::AlignCenter);
       verticalLayout->addWidget(label);
       pushButton = new QPushButton(frame);
       pushButton->setMinimumSize(QSize(0, 51));
       pushButton->setFocus();
       pushButton->setText("Начать игру");
       QObject::connect(pushButton,&QPushButton::clicked,[this](){
           dialog->hide();
           gwindow = new GameWindow([this](int count){
               //Если передаешь лямбду в параметры конструктора, на объект в полях класса ОБЯЗАТЕЛЬНО должна быть ссылка
               //Иначе память очистится и вызовется дичь, а не метод класса, в котором идет работа
               QMessageBox msg;
               msg.setText("Игра окончена. Вы сбили "+QVariant(count).toString()+" самолетов!"+name);
               msg.exec();

               QSqlQuery deleteq;
               deleteq.exec("DELETE FROM Рекорды WHERE  Рекорды.[Имя] LIKE '"+name+"' AND Рекорды.[Число сбитых самолетов]<"+QVariant(count).toString()+";");

               qDebug() << deleteq.lastError();

               QSqlQuery checkq;
               int checkcount=0;
               checkq.exec("SELECT Рекорды.[Имя], Рекорды.[Число сбитых самолетов] FROM Рекорды WHERE  Рекорды.[Имя] LIKE '"+name+"' AND Рекорды.[Число сбитых самолетов]>="+QVariant(count).toString()+";");
               while(checkq.next()){
                   checkcount++;
               }

               qDebug() << deleteq.lastError();
               qDebug() << "Count="<<checkcount;

               if(checkcount==0){
                   QSqlQuery addq;
                   addq.exec("INSERT INTO Рекорды([Имя], [Число сбитых самолетов] )VALUES ('"+name+"',"+QVariant(count).toString()+");");
                   qDebug() << addq.lastError();
                   qDebug() << "Добавление";
               }

               dialog->show();
           });
       });

       verticalLayout->addWidget(pushButton);
       pushButton_2 = new QPushButton(frame);
       pushButton_2->setMinimumSize(QSize(0, 51));
       pushButton_2->setText("Рекорды");
       QObject::connect(pushButton_2,&QPushButton::clicked,[this](){
           RecordsWindow rw;
       });
       verticalLayout->addWidget(pushButton_2);
       horizontalLayout->addWidget(frame);
   }

   void Show(){
       dialog->show();
   }

   void Hide(){
       dialog->hide();
   }

};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //Код для получения HEX из файла для вкомпилирования в эту программу...
    //Загрузить можно до 16 килобайт, так что сохраняем только через Photoshop, а не Paint!!!
    //Использование: раскоментировать код снизу, закоментить все после
    //Скомпилировать и запустить бинарник, перенаправив поток ввода-вывода в файл.
    /*QFile file("/database.mdb");
    file.open(QIODevice::ReadOnly);
    std::cout << qCompress(file.readAll()).toHex().toStdString();
    file.close();
    return 0;*/

    //Распаковка ресурсов
    if(!QFile::exists(a.applicationDirPath()+"/background.png")){
        QFile file(a.applicationDirPath()+"/background.png");
        file.open(QIODevice::WriteOnly);
        file.write(res.GetBackgroundImage());
        file.close();
    }
    if(!QFile::exists(a.applicationDirPath()+"/flyfighter.png")){
        QFile file(a.applicationDirPath()+"/flyfighter.png");
        file.open(QIODevice::WriteOnly);
        file.write(res.GetFlyFighterImage());
        file.close();
    }
    if(!QFile::exists(a.applicationDirPath()+"/fireball.png")){
        QFile file(a.applicationDirPath()+"/fireball.png");
        file.open(QIODevice::WriteOnly);
        file.write(res.GetFireBall());
        file.close();
    }
    if(!QFile::exists(a.applicationDirPath()+"/bomb.png")){
        QFile file(a.applicationDirPath()+"/bomb.png");
        file.open(QIODevice::WriteOnly);
        file.write(res.GetBomb());
        file.close();
    }
    if(!QFile::exists(a.applicationDirPath()+"/database.mdb")){
        QFile file(a.applicationDirPath()+"/database.mdb");
        file.open(QIODevice::WriteOnly);
        file.write(res.GetDb());
        file.close();
    }

    //Подключение к базе данных
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    db.setDatabaseName("Driver={Microsoft Access Driver (*.mdb)};DSN='';DBQ="+a.applicationDirPath()+"/database.mdb");
    if(!db.open())
    {
        QMessageBox msg;
        msg.setText("Не удается подключиться к базе данных!"+db.lastError().text());
        msg.exec();
        return 0;
    };

    MainWindow mw;
    mw.Show();

    return a.exec();
}
