#include "sportstracker.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QListWidgetItem>
#include <QScrollArea>
#include <QDebug>

SportsTracker::SportsTracker(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Просмотр спортивных результатов");
    resize(1200, 800);

    setupUI();
    loadSportsData();

    stackedWidget->setCurrentIndex(0);
}

void SportsTracker::setupUI()
{
    // Главный виджет
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Основной stacked widget
    stackedWidget = new QStackedWidget(centralWidget);

    // 1. Страница выбора спорта
    QWidget *selectionPage = new QWidget();
    QVBoxLayout *selectionLayout = new QVBoxLayout(selectionPage);

    // Заголовок
    QLabel *titleLabel = new QLabel("Выберите вид спорта");
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50;");
    titleLabel->setAlignment(Qt::AlignCenter);
    selectionLayout->addWidget(titleLabel);

    // Список видов спорта
    sportsList = new QListWidget();
    sportsList->setStyleSheet(
        "QListWidget {"
        "   font-size: 18px;"
        "   background: #f9f9f9;"
        "   border: 2px solid #3498db;"
        "   border-radius: 10px;"
        "   padding: 5px;"
        "}"
        "QListWidget::item {"
        "   padding: 12px;"
        "   border-bottom: 1px solid #ddd;"
        "}"
        "QListWidget::item:hover {"
        "   background: #e0f7fa;"
        "}"
        "QListWidget::item:selected {"
        "   background: #b3e5fc;"
        "}"
    );
    connect(sportsList, &QListWidget::itemClicked, [this](QListWidgetItem *item){
        showSportResults(item->text());
    });
    selectionLayout->addWidget(sportsList, 1);

    stackedWidget->addWidget(selectionPage);

    // 2. Страница результатов
    QWidget *resultsPage = new QWidget();
    QVBoxLayout *resultsLayout = new QVBoxLayout(resultsPage);

    // Заголовок
    sportTitle = new QLabel();
    sportTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50;");
    sportTitle->setAlignment(Qt::AlignCenter);
    resultsLayout->addWidget(sportTitle);

    // Отображение результатов
    resultsDisplay = new QLabel();
    resultsDisplay->setStyleSheet("font-size: 16px;");
    resultsDisplay->setWordWrap(true);
    resultsDisplay->setTextFormat(Qt::RichText);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidget(resultsDisplay);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("QScrollArea { border: none; }");
    resultsLayout->addWidget(scrollArea, 1);

    // Кнопка назад
    QPushButton *backButton = new QPushButton("Назад к списку");
    backButton->setStyleSheet(
        "QPushButton {"
        "   font-size: 16px;"
        "   padding: 10px 20px;"
        "   background: #3498db;"
        "   color: white;"
        "   border-radius: 5px;"
        "   border: none;"
        "}"
        "QPushButton:hover {"
        "   background: #2980b9;"
        "}"
    );
    connect(backButton, &QPushButton::clicked, [this](){
        stackedWidget->setCurrentIndex(0);
    });
    resultsLayout->addWidget(backButton, 0, Qt::AlignRight);

    stackedWidget->addWidget(resultsPage);

    // Главный layout
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addWidget(stackedWidget);
}

void SportsTracker::loadSportsData()
{
    // Очищаем старые данные
    sportResults.clear();
    sportsList->clear();

    // Добавляем виды спорта
    QStringList sports;
    sports << "Футбол" << "Хоккей" << "Волейбол" << "Баскетбол" << "Гандбол";
    sportsList->addItems(sports);

    // Футбол
    sportResults["Футбол"] = {
        {"15.10.2023", "Барселона", "ПСЖ", "3:2", "Лига Чемпионов", "Месси - 2 гола"},
        {"18.10.2023", "Манчестер Юнайтед", "Челси", "1:1", "АПЛ", "Роналду - гол с пенальти"},
        {"21.10.2023", "Бавария", "Боруссия Дортмунд", "4:0", "Бундеслига", "Кейн - хет-трик"}
    };

    // Хоккей
    sportResults["Хоккей"] = {
        {"14.10.2023", "СКА", "ЦСКА", "4:3", "КХЛ", "Овертайм. Буташов - хет-трик"},
        {"17.10.2023", "Тампа Бэй", "Колорадо", "2:5", "НХЛ", "Маккиннон - 3 гола"},
        {"20.10.2023", "Торонто", "Монреаль", "5:2", "НХЛ", "Мэттьюс - 2 гола"}
    };

    // Волейбол
    sportResults["Волейбол"] = {
        {"12.10.2023", "Зенит", "Локомотив", "3:1", "Суперлига", "25:22, 23:25, 25:20, 25:18"},
        {"16.10.2023", "Бразилия", "Россия", "2:3", "Лига наций", "25:23, 22:25, 25:22, 20:25, 13:15"},
        {"19.10.2023", "Италия", "Польша", "3:0", "Чемпионат Европы", "25:21, 25:19, 25:17"}
    };

    // Баскетбол (добавлен)
    sportResults["Баскетбол"] = {
        {"13.10.2023", "Лейкерс", "Уорриорз", "112:108", "NBA", "ЛеБрон Джеймс - 32 очка"},
        {"17.10.2023", "ЦСКА", "Реал Мадрид", "89:84", "Евролига", "Алексей Швед - 24 очка"},
        {"22.10.2023", "Бруклин", "Милуоки", "115:111", "NBA", "Кевин Дюрант - 38 очков"}
    };

    // Гандбол (добавлен)
    sportResults["Гандбол"] = {
        {"11.10.2023", "Киль", "Фленсбург", "28:26", "Бундеслига", "Эрик Йоханнесен - 9 голов"},
        {"15.10.2023", "Барселона", "ПСЖ", "32:30", "Лига Чемпионов", "Дика Домбровска - 11 голов"},
        {"20.10.2023", "Чеховские медведи", "СКИФ", "27:25", "Чемпионат России", "Сергей Косоротов - 7 голов"}
    };
}

void SportsTracker::showSportResults(const QString &sportName)
{
    sportTitle->setText(sportName + " - последние результаты");

    QString html = "<html><head><style>"
        "body { font-family: Arial, sans-serif; margin: 20px; }"
        "h2 { color: #2c3e50; margin-bottom: 20px; }"
        "table { width: 100%; border-collapse: collapse; margin-top: 15px; }"
        "th { background-color: #3498db; color: white; padding: 12px; text-align: left; }"
        "td { padding: 10px; border-bottom: 1px solid #ddd; vertical-align: top; }"
        "tr:nth-child(even) { background-color: #f9f9f9; }"
        ".score { font-weight: bold; color: #e74c3c; text-align: center; }"
        ".tournament { color: #7f8c8d; font-style: italic; }"
        ".details { color: #555; font-size: 14px; }"
        "</style></head><body>";

    html += "<h2>Последние матчи</h2>";

    if (sportResults.contains(sportName)) {
        html += "<table>"
                "<tr>"
                "<th style='width: 15%;'>Дата</th>"
                "<th style='width: 20%;'>Турнир</th>"
                "<th style='width: 20%;'>Команда 1</th>"
                "<th style='width: 10%;'>Счёт</th>"
                "<th style='width: 20%;'>Команда 2</th>"
                "<th style='width: 15%;'>Детали</th>"
                "</tr>";

        for (const MatchResult &match : sportResults[sportName]) {
            html += QString("<tr>"
                          "<td>%1</td>"
                          "<td class='tournament'>%2</td>"
                          "<td><b>%3</b></td>"
                          "<td class='score'>%4</td>"
                          "<td><b>%5</b></td>"
                          "<td class='details'>%6</td>"
                          "</tr>")
                    .arg(match.date, match.tournament, match.team1,
                         match.score, match.team2, match.details);
        }
        html += "</table>";
    } else {
        html += "<p style='color: #e74c3c; font-weight: bold;'>Нет данных о результатах</p>";
    }

    html += "</body></html>";
    resultsDisplay->setText(html);
    stackedWidget->setCurrentIndex(1);
}
