#include "sportstracker.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QListWidgetItem>
#include <QScrollArea>
#include <QHeaderView>

SportsTracker::SportsTracker(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Просмотр спортивных результатов");
    resize(1400, 900);

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
        showSportTournaments(item->text());
    });
    selectionLayout->addWidget(sportsList, 1);

    stackedWidget->addWidget(selectionPage);

    // 2. Страница выбора турнира
    QWidget *tournamentsPage = new QWidget();
    QVBoxLayout *tournamentsLayout = new QVBoxLayout(tournamentsPage);

    // Заголовок
    sportTitle = new QLabel();
    sportTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50;");
    sportTitle->setAlignment(Qt::AlignCenter);
    tournamentsLayout->addWidget(sportTitle);

    // Подзаголовок
    QLabel *subtitleLabel = new QLabel("Выберите турнир");
    subtitleLabel->setStyleSheet("font-size: 20px; color: #2c3e50;");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    tournamentsLayout->addWidget(subtitleLabel);

    // Список турниров
    tournamentsList = new QListWidget();
    tournamentsList->setStyleSheet(sportsList->styleSheet());
    connect(tournamentsList, &QListWidget::itemClicked, [this](QListWidgetItem *item){
        showTournamentResults(sportTitle->text().replace(" - турниры", ""), item->text());
    });
    tournamentsLayout->addWidget(tournamentsList, 1);

    // Кнопка назад
    QPushButton *backButton = new QPushButton("Назад к видам спорта");
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
    tournamentsLayout->addWidget(backButton, 0, Qt::AlignRight);

    stackedWidget->addWidget(tournamentsPage);

    // 3. Страница результатов турнира
    QWidget *resultsPage = new QWidget();
    QVBoxLayout *resultsLayout = new QVBoxLayout(resultsPage);

    // Заголовок
    tournamentTitle = new QLabel();
    tournamentTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50;");
    tournamentTitle->setAlignment(Qt::AlignCenter);
    resultsLayout->addWidget(tournamentTitle);

    // Подзаголовок
    QLabel *matchesLabel = new QLabel("Выберите матч");
    matchesLabel->setStyleSheet("font-size: 20px; color: #2c3e50;");
    matchesLabel->setAlignment(Qt::AlignCenter);
    resultsLayout->addWidget(matchesLabel);

    // Список матчей
    matchesList = new QListWidget();
    matchesList->setStyleSheet(sportsList->styleSheet());
    resultsLayout->addWidget(matchesList, 1);

    // Кнопка назад
    QPushButton *backButton2 = new QPushButton("Назад к турнирам");
    backButton2->setStyleSheet(backButton->styleSheet());
    connect(backButton2, &QPushButton::clicked, [this](){
        stackedWidget->setCurrentIndex(1);
    });
    resultsLayout->addWidget(backButton2, 0, Qt::AlignRight);

    stackedWidget->addWidget(resultsPage);

    // 4. Страница деталей матча
    QWidget *matchPage = new QWidget();
    QVBoxLayout *matchLayout = new QVBoxLayout(matchPage);

    // Заголовок
    matchTitle = new QLabel();
    matchTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50;");
    matchTitle->setAlignment(Qt::AlignCenter);
    matchLayout->addWidget(matchTitle);

    // Таблица статистики
    statsTable = new QTableWidget();
    statsTable->setStyleSheet(
        "QTableWidget {"
        "   font-size: 16px;"
        "   background: #f9f9f9;"
        "   border: 2px solid #3498db;"
        "   border-radius: 10px;"
        "}"
        "QHeaderView::section {"
        "   background-color: #3498db;"
        "   color: white;"
        "   padding: 5px;"
        "   font-weight: bold;"
        "}"
    );
    statsTable->verticalHeader()->setVisible(false);
    statsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    statsTable->setSelectionMode(QAbstractItemView::NoSelection);
    matchLayout->addWidget(statsTable, 1);

    // Кнопка назад
    QPushButton *backButton3 = new QPushButton("Назад к матчам");
    backButton3->setStyleSheet(backButton->styleSheet());
    connect(backButton3, &QPushButton::clicked, [this](){
        stackedWidget->setCurrentIndex(2);
    });
    matchLayout->addWidget(backButton3, 0, Qt::AlignRight);

    stackedWidget->addWidget(matchPage);

    // Главный layout
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addWidget(stackedWidget);
}

void SportsTracker::loadSportsData()
{
    sportTournamentResults.clear();
    sportsList->clear();

    // Добавляем виды спорта
    QStringList sports;
    sports << "Футбол" << "Хоккей" << "Волейбол" << "Баскетбол" << "Гандбол";
    sportsList->addItems(sports);

    // Футбол
    QMap<QString, QList<MatchResult>> footballTournaments;

    MatchResult match1;
    match1.date = "15.10.2023";
    match1.team1 = "Барселона";
    match1.team2 = "ПСЖ";
    match1.score = "3:2";
    match1.tournament = "Лига Чемпионов";
    match1.details = "Месси - 2 гола";
    match1.team1Stats = {
        {"Владение мячом", "62%"},
        {"Удары", "18"},
        {"Удары в створ", "8"},
        {"Угловые", "7"},
        {"Фолы", "12"}
    };
    match1.team2Stats = {
        {"Владение мячом", "38%"},
        {"Удары", "10"},
        {"Удары в створ", "5"},
        {"Угловые", "3"},
        {"Фолы", "15"}
    };

    MatchResult match2;
    match2.date = "18.10.2023";
    match2.team1 = "Манчестер Юнайтед";
    match2.team2 = "Челси";
    match2.score = "1:1";
    match2.tournament = "Английская Премьер-Лига";
    match2.details = "Роналду - гол с пенальти";
    match2.team1Stats = {
        {"Владение мячом", "45%"},
        {"Удары", "12"},
        {"Удары в створ", "4"},
        {"Угловые", "5"},
        {"Фолы", "14"}
    };
    match2.team2Stats = {
        {"Владение мячом", "55%"},
        {"Удары", "15"},
        {"Удары в створ", "6"},
        {"Угловые", "7"},
        {"Фолы", "10"}
    };

    footballTournaments["Лига Чемпионов"] = {match1};
    footballTournaments["Английская Премьер-Лига"] = {match2};
    sportTournamentResults["Футбол"] = footballTournaments;

    // Хоккей
    QMap<QString, QList<MatchResult>> hockeyTournaments;

    MatchResult match3;
    match3.date = "14.10.2023";
    match3.team1 = "СКА";
    match3.team2 = "ЦСКА";
    match3.score = "4:3";
    match3.tournament = "КХЛ";
    match3.details = "Овертайм. Буташов - хет-трик";
    match3.team1Stats = {
        {"Броски", "32"},
        {"Реализация", "12.5%"},
        {"Выигранные вбрасывания", "54%"},
        {"Штраф", "8 минут"},
        {"Силовые приемы", "18"}
    };
    match3.team2Stats = {
        {"Броски", "28"},
        {"Реализация", "10.7%"},
        {"Выигранные вбрасывания", "46%"},
        {"Штраф", "10 минут"},
        {"Силовые приемы", "22"}
    };

    hockeyTournaments["КХЛ"] = {match3};
    sportTournamentResults["Хоккей"] = hockeyTournaments;

    // Остальные виды спорта и турниры можно добавить аналогично
    // ...
}

void SportsTracker::showSportTournaments(const QString &sportName)
{
    sportTitle->setText(sportName + " - турниры");
    tournamentsList->clear();

    if (sportTournamentResults.contains(sportName)) {
        QStringList tournaments = sportTournamentResults[sportName].keys();
        tournamentsList->addItems(tournaments);
    }

    stackedWidget->setCurrentIndex(1);
}

void SportsTracker::showTournamentResults(const QString &sportName, const QString &tournamentName)
{
    tournamentTitle->setText(sportName + " - " + tournamentName);
    matchesList->clear();

    if (sportTournamentResults.contains(sportName) &&
        sportTournamentResults[sportName].contains(tournamentName)) {

        for (const MatchResult &match : sportTournamentResults[sportName][tournamentName]) {
            QString itemText = QString("%1: %2 %3 - %4 %5")
                              .arg(match.date)
                              .arg(match.team1)
                              .arg(match.score.split(":").first())
                              .arg(match.score.split(":").last())
                              .arg(match.team2);

            QListWidgetItem *item = new QListWidgetItem(itemText, matchesList);
            item->setData(Qt::UserRole, QVariant::fromValue(match));
        }

        connect(matchesList, &QListWidget::itemClicked, [this](QListWidgetItem *item){
            MatchResult match = item->data(Qt::UserRole).value<MatchResult>();
            showMatchDetails(match);
        });
    }

    stackedWidget->setCurrentIndex(2);
}

void SportsTracker::showMatchDetails(const MatchResult &match)
{
    matchTitle->setText(QString("%1 %2 - %3 %4 (%5)")
                       .arg(match.team1)
                       .arg(match.score.split(":").first())
                       .arg(match.score.split(":").last())
                       .arg(match.team2)
                       .arg(match.date));

    // Настраиваем таблицу статистики
    statsTable->clear();
    statsTable->setRowCount(match.team1Stats.size());
    statsTable->setColumnCount(3);

    QStringList headers;
    headers << "Показатель" << match.team1 << match.team2;
    statsTable->setHorizontalHeaderLabels(headers);

    int row = 0;
    for (auto it = match.team1Stats.constBegin(); it != match.team1Stats.constEnd(); ++it) {
        QTableWidgetItem *statName = new QTableWidgetItem(it.key());
        QTableWidgetItem *team1Value = new QTableWidgetItem(it.value());
        QTableWidgetItem *team2Value = new QTableWidgetItem(match.team2Stats[it.key()]);

        statsTable->setItem(row, 0, statName);
        statsTable->setItem(row, 1, team1Value);
        statsTable->setItem(row, 2, team2Value);

        row++;
    }

    statsTable->resizeColumnsToContents();
    statsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    stackedWidget->setCurrentIndex(3);
}
