#include "sportstracker.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QDate>

SportsTracker::SportsTracker(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Просмотр спортивных результатов");
    resize(1200, 800);

    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    setupSportsSelection();
    setupResultsView();

    loadSportsData();

    stackedWidget->setCurrentIndex(0);
}

void SportsTracker::setupSportsSelection()
{
    QWidget *selectionPage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(selectionPage);
    layout->setContentsMargins(20, 20, 20, 20);

    QLabel *title = new QLabel("Выберите вид спорта");
    title->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50;");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    sportsList = new QListWidget();
    sportsList->setStyleSheet(
        "QListWidget {"
        "   font-size: 18px;"
        "   background: #f9f9f9;"
        "   border: 2px solid #3498db;"
        "   border-radius: 10px;"
        "}"
        "QListWidget::item {"
        "   padding: 15px;"
        "   border-bottom: 1px solid #ddd;"
        "}"
        "QListWidget::item:hover {"
        "   background: #e0f7fa;"
        "}"
        "QListWidget::item:selected {"
        "   background: #b3e5fc;"
        "}"
    );
    sportsList->setIconSize(QSize(64, 64));
    connect(sportsList, &QListWidget::itemClicked, [this](QListWidgetItem *item) {
        showSportResults(item->text());
    });
    layout->addWidget(sportsList, 1);

    selectionPage->setLayout(layout);
    stackedWidget->addWidget(selectionPage);
}

void SportsTracker::setupResultsView()
{
    QWidget *resultsPage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(resultsPage);
    layout->setContentsMargins(20, 20, 20, 20);

    sportTitle = new QLabel();
    sportTitle->setStyleSheet("font-size: 26px; font-weight: bold; color: #2c3e50; margin-bottom: 20px;");
    sportTitle->setAlignment(Qt::AlignCenter);
    layout->addWidget(sportTitle);

    resultsDisplay = new QLabel();
    resultsDisplay->setStyleSheet("font-size: 16px;");
    resultsDisplay->setWordWrap(true);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidget(resultsDisplay);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("QScrollArea { border: none; }");
    layout->addWidget(scrollArea, 1);

    QPushButton *backButton = new QPushButton("Назад");
    backButton->setStyleSheet(
        "QPushButton {"
        "   font-size: 16px;"
        "   padding: 10px 20px;"
        "   background: #3498db;"
        "   color: white;"
        "   border-radius: 5px;"
        "}"
        "QPushButton:hover {"
        "   background: #2980b9;"
        "}"
    );
    connect(backButton, &QPushButton::clicked, [this]() {
        stackedWidget->setCurrentIndex(0);
    });
    layout->addWidget(backButton, 0, Qt::AlignRight);

    resultsPage->setLayout(layout);
    stackedWidget->addWidget(resultsPage);
}

void SportsTracker::loadSportsData()
{
    // Очищаем предыдущие данные
    sportResults.clear();
    sportsList->clear();

    // Добавляем виды спорта с иконками
    QStringList sports;
    sports << "Футбол" << "Хоккей" << "Волейбол";
    sportsList->addItems(sports);

    // Футбольные матчи
    QList<MatchResult> footballMatches;
    footballMatches.append({
        "15.10.2023",
        "Барселона",
        "Реал Мадрид",
        "2:1",
        "Ла Лига"
    });
    footballMatches.append({
        "22.10.2023",
        "Манчестер Юнайтед",
        "Ливерпуль",
        "1:1",
        "Английская Премьер-лига"
    });
    footballMatches.append({
        "28.10.2023",
        "Бавария",
        "Боруссия Дортмунд",
        "4:0",
        "Бундеслига"
    });
    sportResults["Футбол"] = footballMatches;

    // Хоккейные матчи
    QList<MatchResult> hockeyMatches;
    hockeyMatches.append({
        "14.10.2023",
        "СКА",
        "ЦСКА",
        "3:2 (ОТ)",
        "КХЛ"
    });
    hockeyMatches.append({
        "20.10.2023",
        "Вашингтон Кэпиталз",
        "Питтсбург Пингвинз",
        "4:3",
        "НХЛ"
    });
    hockeyMatches.append({
        "27.10.2023",
        "Торонто Мейпл Лифс",
        "Монреаль Канадиенс",
        "2:5",
        "НХЛ"
    });
    sportResults["Хоккей"] = hockeyMatches;

    // Волейбольные матчи
    QList<MatchResult> volleyballMatches;
    volleyballMatches.append({
        "12.10.2023",
        "Зенит",
        "Локомотив",
        "3:1 (25:22, 23:25, 25:20, 25:18)",
        "Суперлига"
    });
    volleyballMatches.append({
        "19.10.2023",
        "Бразилия",
        "Россия",
        "2:3 (25:23, 22:25, 25:22, 20:25, 13:15)",
        "Лига наций"
    });
    volleyballMatches.append({
        "26.10.2023",
        "Италия",
        "Польша",
        "3:0 (25:21, 25:19, 25:17)",
        "Чемпионат Европы"
    });
    sportResults["Волейбол"] = volleyballMatches;
}

void SportsTracker::showSportResults(const QString &sportName)
{
    sportTitle->setText(sportName);

    QString resultsHtml = "<html><body style='font-family: Arial;'>";
    resultsHtml += "<h2 style='color: #2c3e50; margin-bottom: 20px;'>Последние матчи</h2>";

    if (sportResults.contains(sportName)) {
        resultsHtml += "<table style='width: 100%; border-collapse: collapse;'>";
        resultsHtml += "<tr style='background-color: #3498db; color: white;'>"
                      "<th style='padding: 10px; text-align: left;'>Дата</th>"
                      "<th style='padding: 10px; text-align: left;'>Команда 1</th>"
                      "<th style='padding: 10px; text-align: center;'>Счет</th>"
                      "<th style='padding: 10px; text-align: right;'>Команда 2</th>"
                      "<th style='padding: 10px; text-align: left;'>Турнир</th></tr>";

        bool alternate = false;
        for (const MatchResult &match : sportResults[sportName]) {
            QString rowColor = alternate ? "#f2f2f2" : "#ffffff";
            resultsHtml += QString("<tr style='background-color: %1;'>").arg(rowColor);
            resultsHtml += QString("<td style='padding: 10px;'>%1</td>").arg(match.date);
            resultsHtml += QString("<td style='padding: 10px; font-weight: bold;'>%1</td>").arg(match.team1);
            resultsHtml += QString("<td style='padding: 10px; text-align: center; font-weight: bold; color: #e74c3c;'>%1</td>").arg(match.score);
            resultsHtml += QString("<td style='padding: 10px; font-weight: bold; text-align: right;'>%1</td>").arg(match.team2);
            resultsHtml += QString("<td style='padding: 10px; color: #7f8c8d;'>%1</td>").arg(match.tournament);
            resultsHtml += "</tr>";
            alternate = !alternate;
        }
        resultsHtml += "</table>";
    } else {
        resultsHtml += "<p style='color: #e74c3c;'>Нет данных о результатах</p>";
    }

    resultsHtml += "</body></html>";
    resultsDisplay->setText(resultsHtml);

    stackedWidget->setCurrentIndex(1);
}
