#include "sportstracker.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlError>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QLabel>
#include <QSqlQuery>
#include <QTabWidget>
#include <QDate>

SportsTracker::SportsTracker(QWidget *parent)
    : QMainWindow(parent),
      stackedWidget(new QStackedWidget(this)),
      sportsTree(new QTreeWidget()),
      matchesList(new QListWidget()),
      standingsTable(new QTableWidget()),
      statsTable(new QTableWidget()),
      scorersTable(new QTableWidget()),
      team1RecentMatches(new QTableWidget()),
      team2RecentMatches(new QTableWidget()),
      headToHeadMatches(new QTableWidget()),
      matchTitle(new QLabel()),
      backButton1(new QPushButton("Назад к турнирам")),
      backButton2(new QPushButton("Назад к матчам")),
      leftPanelStack(new QStackedWidget()),
      statsTabs(new QTabWidget()),
      currentTournamentId(-1),
      currentTournamentName("")
{
    setWindowTitle("Просмотр спортивных результатов");
    resize(1600, 1000);

    if (!initializeDatabase()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось инициализировать базу данных");
        return;
    }

    setupUI();
    loadSports();
}

SportsTracker::~SportsTracker()
{
    if (db.isOpen()) {
        db.close();
    }
}

bool SportsTracker::initializeDatabase()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    QString dbPath = "/home/qt/database/sports.db";

    QDir().mkpath(QDir::homePath() + "/database");
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        qDebug() << "Cannot open database:" << db.lastError();
        return false;
    }

    QStringList tables = db.tables();
    if (tables.isEmpty()) {
        QFile scriptFile(":/createdb.sql");
        if (!scriptFile.open(QIODevice::ReadOnly)) {
            qDebug() << "Cannot open SQL script file";
            return false;
        }

        QTextStream stream(&scriptFile);
        QStringList scriptQueries = stream.readAll().split(';');

        foreach (const QString &query, scriptQueries) {
            if (query.trimmed().isEmpty()) continue;

            QSqlQuery sqlQuery;
            if (!sqlQuery.exec(query)) {
                qDebug() << "Failed to execute query:" << query << "\nError:" << sqlQuery.lastError();
                return false;
            }
        }
    }

    return true;
}

void SportsTracker::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    stackedWidget = new QStackedWidget(centralWidget);

    // 1. Страница выбора турнира
    QWidget *selectionPage = new QWidget();
    QHBoxLayout *selectionLayout = new QHBoxLayout(selectionPage);

    sportsTree->setHeaderHidden(true);
    sportsTree->setStyleSheet(
        "QTreeWidget {"
        "   font-size: 18px;"
        "   background: #f9f9f9;"
        "   border: 2px solid #3498db;"
        "   border-radius: 10px;"
        "   padding: 5px;"
        "}"
        "QTreeWidget::item {"
        "   padding: 12px;"
        "   border-bottom: 1px solid #ddd;"
        "}"
        "QTreeWidget::item:hover {"
        "   background: #e0f7fa;"
        "}"
        "QTreeWidget::item:selected {"
        "   background: #b3e5fc;"
        "}"
    );
    sportsTree->setColumnCount(1);
    connect(sportsTree, &QTreeWidget::itemClicked, this, &SportsTracker::onTournamentClicked);
    selectionLayout->addWidget(sportsTree);
    stackedWidget->addWidget(selectionPage);

    // 2. Страница турнира (матчи + таблица)
    QWidget *tournamentPage = new QWidget();
    QHBoxLayout *tournamentLayout = new QHBoxLayout(tournamentPage);

    // Левая панель - StackedWidget для переключения между списком матчей и статистикой
    leftPanelStack = new QStackedWidget();

    // Страница списка матчей
    QWidget *matchesPage = new QWidget();
    QVBoxLayout *matchesLayout = new QVBoxLayout(matchesPage);

    matchesList->setStyleSheet(
        "QListWidget {"
        "   font-size: 16px;"
        "   background: #f9f9f9;"
        "   border: 2px solid #3498db;"
        "   border-radius: 10px;"
        "   padding: 5px;"
        "}"
        "QListWidget::item {"
        "   padding: 10px;"
        "   border-bottom: 1px solid #ddd;"
        "}"
        "QListWidget::item:hover {"
        "   background: #e0f7fa;"
        "}"
    );
    connect(matchesList, &QListWidget::itemClicked, this, &SportsTracker::showMatchStats);
    matchesLayout->addWidget(matchesList);

    backButton1->setStyleSheet(
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
    connect(backButton1, &QPushButton::clicked, [this]() {
        stackedWidget->setCurrentIndex(0);
    });
    matchesLayout->addWidget(backButton1, 0, Qt::AlignRight);
    leftPanelStack->addWidget(matchesPage);

    // Страница статистики матча
    QWidget *statsPage = new QWidget();
    QVBoxLayout *statsPageLayout = new QVBoxLayout(statsPage);

    matchTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50;");
    matchTitle->setAlignment(Qt::AlignCenter);
    statsPageLayout->addWidget(matchTitle);

    // Настройка QTabWidget для разных видов статистики
    statsTabs->setStyleSheet(
        "QTabWidget::pane { border: 1px solid #3498db; border-radius: 5px; }"
        "QTabBar::tab { padding: 8px; background: #f0f0f0; }"
        "QTabBar::tab:selected { background: #3498db; color: white; }"
    );

    // Вкладка основной статистики
    QWidget *basicStatsTab = new QWidget();
    QVBoxLayout *basicStatsLayout = new QVBoxLayout(basicStatsTab);
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
    basicStatsLayout->addWidget(statsTable);
    statsTabs->addTab(basicStatsTab, "Основная статистика");

    // Вкладка забивших голов
    QWidget *scorersTab = new QWidget();
    QVBoxLayout *scorersLayout = new QVBoxLayout(scorersTab);
    scorersTable->setStyleSheet(statsTable->styleSheet());
    scorersTable->verticalHeader()->setVisible(false);
    scorersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    scorersLayout->addWidget(scorersTable);
    statsTabs->addTab(scorersTab, "Голы и события");

    // Вкладка последних матчей команд
    QWidget *recentMatchesTab = new QWidget();
    QHBoxLayout *recentMatchesLayout = new QHBoxLayout(recentMatchesTab);

    QWidget *team1RecentWidget = new QWidget();
    QVBoxLayout *team1RecentLayout = new QVBoxLayout(team1RecentWidget);
    QLabel *team1RecentLabel = new QLabel("Последние матчи команды 1");
    team1RecentLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    team1RecentLayout->addWidget(team1RecentLabel);
    team1RecentMatches->setStyleSheet(statsTable->styleSheet());
    team1RecentMatches->verticalHeader()->setVisible(false);
    team1RecentMatches->setEditTriggers(QAbstractItemView::NoEditTriggers);
    team1RecentLayout->addWidget(team1RecentMatches);
    recentMatchesLayout->addWidget(team1RecentWidget);

    QWidget *team2RecentWidget = new QWidget();
    QVBoxLayout *team2RecentLayout = new QVBoxLayout(team2RecentWidget);
    QLabel *team2RecentLabel = new QLabel("Последние матчи команды 2");
    team2RecentLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    team2RecentLayout->addWidget(team2RecentLabel);
    team2RecentMatches->setStyleSheet(statsTable->styleSheet());
    team2RecentMatches->verticalHeader()->setVisible(false);
    team2RecentMatches->setEditTriggers(QAbstractItemView::NoEditTriggers);
    team2RecentLayout->addWidget(team2RecentMatches);
    recentMatchesLayout->addWidget(team2RecentWidget);

    statsTabs->addTab(recentMatchesTab, "Последние матчи команд");

    // Вкладка очных встреч
    QWidget *h2hTab = new QWidget();
    QVBoxLayout *h2hLayout = new QVBoxLayout(h2hTab);
    headToHeadMatches->setStyleSheet(statsTable->styleSheet());
    headToHeadMatches->verticalHeader()->setVisible(false);
    headToHeadMatches->setEditTriggers(QAbstractItemView::NoEditTriggers);
    h2hLayout->addWidget(headToHeadMatches);
    statsTabs->addTab(h2hTab, "История встреч");

    statsPageLayout->addWidget(statsTabs, 1);

    backButton2->setStyleSheet(backButton1->styleSheet());
    connect(backButton2, &QPushButton::clicked, this, &SportsTracker::showMatchesList);
    statsPageLayout->addWidget(backButton2, 0, Qt::AlignRight);
    leftPanelStack->addWidget(statsPage);

    tournamentLayout->addWidget(leftPanelStack, 40);

    // Правая панель - турнирная таблица
    standingsTable->setStyleSheet(
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
    standingsTable->verticalHeader()->setVisible(false);
    standingsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tournamentLayout->addWidget(standingsTable, 60);
    stackedWidget->addWidget(tournamentPage);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addWidget(stackedWidget);
}

void SportsTracker::loadSports()
{
    sportsTree->clear();

    QSqlQuery query("SELECT id, name FROM sports ORDER BY name");
    while (query.next()) {
        QTreeWidgetItem *sportItem = new QTreeWidgetItem(sportsTree);
        sportItem->setText(0, query.value(1).toString());
        sportItem->setData(0, Qt::UserRole, query.value(0).toInt());

        new QTreeWidgetItem(sportItem);
    }

    connect(sportsTree, &QTreeWidget::itemExpanded, this, &SportsTracker::loadTournaments);
}

void SportsTracker::loadTournaments(QTreeWidgetItem *sportItem)
{
    if (sportItem->childCount() == 1 && sportItem->child(0)->text(0).isEmpty()) {
        delete sportItem->takeChild(0);

        int sportId = sportItem->data(0, Qt::UserRole).toInt();
        QSqlQuery query;
        query.prepare("SELECT id, name FROM tournaments WHERE sport_id = ? ORDER BY name");
        query.addBindValue(sportId);

        if (query.exec()) {
            while (query.next()) {
                QTreeWidgetItem *tournamentItem = new QTreeWidgetItem(sportItem);
                tournamentItem->setText(0, query.value(1).toString());
                tournamentItem->setData(0, Qt::UserRole, query.value(0).toInt());
            }
        }
    }
}

void SportsTracker::onTournamentClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    if (item->parent()) { // Кликнули на турнир (у него есть родитель - вид спорта)
        int tournamentId = item->data(0, Qt::UserRole).toInt();
        QString tournamentName = item->text(0);
        showTournamentPage(tournamentId, tournamentName);
    }
}

void SportsTracker::showTournamentPage(int tournamentId, const QString &tournamentName)
{
    currentTournamentId = tournamentId;
    currentTournamentName = tournamentName;
    loadMatchesAndStandings(tournamentId);
    stackedWidget->setCurrentIndex(1);
    leftPanelStack->setCurrentIndex(0); // Показываем список матчей
    setWindowTitle(tournamentName);
}

void SportsTracker::loadMatchesAndStandings(int tournamentId)
{
    matchesList->clear();
    standingsTable->clear();

    // Загрузка матчей
    QSqlQuery matchesQuery;
    matchesQuery.prepare(
        "SELECT id, date, team1, team2, score "
        "FROM matches WHERE tournament_id = ? ORDER BY date DESC"
    );
    matchesQuery.addBindValue(tournamentId);

    if (matchesQuery.exec()) {
        while (matchesQuery.next()) {
            QString matchText = QString("%1: %2 %3 - %4 %5")
                .arg(matchesQuery.value(1).toString())
                .arg(matchesQuery.value(2).toString())
                .arg(matchesQuery.value(4).toString().split(":").first())
                .arg(matchesQuery.value(4).toString().split(":").last())
                .arg(matchesQuery.value(3).toString());

            QListWidgetItem *item = new QListWidgetItem(matchText, matchesList);
            item->setData(Qt::UserRole, matchesQuery.value(0).toInt());
        }
    }

    // Загрузка турнирной таблицы
    QSqlQuery standingsQuery;
    standingsQuery.prepare(
        "SELECT team, points, games, wins, draws, losses, goals_for, goals_against "
        "FROM standings WHERE tournament_id = ? ORDER BY points DESC"
    );
    standingsQuery.addBindValue(tournamentId);

    if (standingsQuery.exec()) {
        standingsTable->setRowCount(0);
        standingsTable->setColumnCount(8);
        standingsTable->setHorizontalHeaderLabels(
            {"Команда", "Очки", "И", "В", "Н", "П", "ЗГ", "ПГ"});

        while (standingsQuery.next()) {
            int row = standingsTable->rowCount();
            standingsTable->insertRow(row);

            for (int col = 0; col < 8; ++col) {
                standingsTable->setItem(row, col,
                    new QTableWidgetItem(standingsQuery.value(col).toString()));
            }
        }
        standingsTable->resizeColumnsToContents();
    }
}

void SportsTracker::showMatchStats(QListWidgetItem *item)
{
    if (!item) return;

    int matchId = item->data(Qt::UserRole).toInt();
    QString matchInfo = item->text();

    // Очищаем все таблицы
    statsTable->clear();
    scorersTable->clear();
    team1RecentMatches->clear();
    team2RecentMatches->clear();
    headToHeadMatches->clear();

    matchTitle->setText(matchInfo);

    // Получаем основную информацию о матче
    QSqlQuery matchQuery;
    matchQuery.prepare("SELECT team1, team2, date FROM matches WHERE id = ?");
    matchQuery.addBindValue(matchId);

    if (matchQuery.exec() && matchQuery.next()) {
        QString team1 = matchQuery.value(0).toString();
        QString team2 = matchQuery.value(1).toString();
        QDate matchDate = QDate::fromString(matchQuery.value(2).toString(), Qt::ISODate);

        // 1. Основная статистика матча
        loadMatchStats(matchId, team1, team2);

        // 2. Список забивших голов (для футбола) или отличившихся игроков
        loadScorers(matchId, team1, team2);

        // 3. Последние 5 матчей каждой команды
        loadRecentMatches(team1, matchDate, team1RecentMatches);
        loadRecentMatches(team2, matchDate, team2RecentMatches);

        // 4. История очных встреч
        loadHeadToHeadMatches(team1, team2, matchDate, headToHeadMatches);
    }

    leftPanelStack->setCurrentIndex(1); // Показываем статистику матча
}

void SportsTracker::loadMatchStats(int matchId, const QString& team1, const QString& team2)
{
    QSqlQuery statsQuery;
    statsQuery.prepare(
        "SELECT stat_name, "
        "(SELECT stat_value FROM match_stats WHERE match_id = ? AND team = 1 AND stat_name = ms.stat_name) as team1_value, "
        "(SELECT stat_value FROM match_stats WHERE match_id = ? AND team = 2 AND stat_name = ms.stat_name) as team2_value "
        "FROM match_stats ms WHERE match_id = ? GROUP BY stat_name"
    );
    statsQuery.addBindValue(matchId);
    statsQuery.addBindValue(matchId);
    statsQuery.addBindValue(matchId);

    if (statsQuery.exec()) {
        statsTable->setRowCount(0);
        statsTable->setColumnCount(3);
        statsTable->setHorizontalHeaderLabels({"Показатель", team1, team2});

        while (statsQuery.next()) {
            int row = statsTable->rowCount();
            statsTable->insertRow(row);

            statsTable->setItem(row, 0, new QTableWidgetItem(statsQuery.value(0).toString()));
            statsTable->setItem(row, 1, new QTableWidgetItem(statsQuery.value(1).toString()));
            statsTable->setItem(row, 2, new QTableWidgetItem(statsQuery.value(2).toString()));
        }
        statsTable->resizeColumnsToContents();
        statsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    }
}

void SportsTracker::loadScorers(int matchId, const QString& team1, const QString& team2)
{
    QSqlQuery scorersQuery;
    scorersQuery.prepare(
        "SELECT player_name, team, minute, description "
        "FROM match_events "
        "WHERE match_id = ? AND (event_type = 'goal' OR event_type = 'notable_action') "
        "ORDER BY minute"
    );
    scorersQuery.addBindValue(matchId);

    if (scorersQuery.exec()) {
        scorersTable->setRowCount(0);
        scorersTable->setColumnCount(4);
        scorersTable->setHorizontalHeaderLabels({"Игрок", "Команда", "Минута", "Описание"});

        while (scorersQuery.next()) {
            int row = scorersTable->rowCount();
            scorersTable->insertRow(row);

            QString teamName = scorersQuery.value(1).toInt() == 1 ? team1 : team2;

            scorersTable->setItem(row, 0, new QTableWidgetItem(scorersQuery.value(0).toString()));
            scorersTable->setItem(row, 1, new QTableWidgetItem(teamName));
            scorersTable->setItem(row, 2, new QTableWidgetItem(scorersQuery.value(2).toString()));
            scorersTable->setItem(row, 3, new QTableWidgetItem(scorersQuery.value(3).toString()));
        }
        scorersTable->resizeColumnsToContents();
    }
}

void SportsTracker::loadRecentMatches(const QString& team, const QDate& beforeDate, QTableWidget* table)
{
    QSqlQuery recentQuery;
    recentQuery.prepare(
        "SELECT m.date, t1.name, t2.name, m.score "
        "FROM matches m "
        "JOIN teams t1 ON m.team1 = t1.id "
        "JOIN teams t2 ON m.team2 = t2.id "
        "WHERE (t1.name = ? OR t2.name = ?) AND m.date < ? "
        "ORDER BY m.date DESC LIMIT 5"
    );
    recentQuery.addBindValue(team);
    recentQuery.addBindValue(team);
    recentQuery.addBindValue(beforeDate.toString(Qt::ISODate));

    if (recentQuery.exec()) {
        table->setRowCount(0);
        table->setColumnCount(4);
        table->setHorizontalHeaderLabels({"Дата", "Команда 1", "Команда 2", "Счет"});

        while (recentQuery.next()) {
            int row = table->rowCount();
            table->insertRow(row);

            for (int col = 0; col < 4; ++col) {
                table->setItem(row, col, new QTableWidgetItem(recentQuery.value(col).toString()));
            }
        }
        table->resizeColumnsToContents();
    }
}

void SportsTracker::loadHeadToHeadMatches(const QString& team1, const QString& team2, const QDate& beforeDate, QTableWidget* table)
{
    QSqlQuery h2hQuery;
    h2hQuery.prepare(
        "SELECT m.date, t1.name, t2.name, m.score "
        "FROM matches m "
        "JOIN teams t1 ON m.team1 = t1.id "
        "JOIN teams t2 ON m.team2 = t2.id "
        "WHERE ((t1.name = ? AND t2.name = ?) OR (t1.name = ? AND t2.name = ?)) AND m.date < ? "
        "ORDER BY m.date DESC LIMIT 10"
    );
    h2hQuery.addBindValue(team1);
    h2hQuery.addBindValue(team2);
    h2hQuery.addBindValue(team2);
    h2hQuery.addBindValue(team1);
    h2hQuery.addBindValue(beforeDate.toString(Qt::ISODate));

    if (h2hQuery.exec()) {
        table->setRowCount(0);
        table->setColumnCount(4);
        table->setHorizontalHeaderLabels({"Дата", "Команда 1", "Команда 2", "Счет"});

        while (h2hQuery.next()) {
            int row = table->rowCount();
            table->insertRow(row);

            for (int col = 0; col < 4; ++col) {
                table->setItem(row, col, new QTableWidgetItem(h2hQuery.value(col).toString()));
            }
        }
        table->resizeColumnsToContents();
    }
}

void SportsTracker::showMatchesList()
{
    leftPanelStack->setCurrentIndex(0); // Показываем список матчей
}
