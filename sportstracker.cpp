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
#include <QFont>
#include <QScrollBar>
#include <QComboBox>
#include <QPushButton>
#include <QButtonGroup>

SportsTracker::SportsTracker(QWidget *parent)
    : QMainWindow(parent),
      stackedWidget(new QStackedWidget(this)),
      sportsTree(new QTreeWidget()),
      matchesList(new QListWidget()),
      standingsTable(new QTableWidget()),
      statsTable(new QTableWidget()),
      lineupsTable(new QTableWidget()),
      scorersTable(new QTableWidget()),
      team1RecentMatches(new QTableWidget()),
      team2RecentMatches(new QTableWidget()),
      headToHeadMatches(new QTableWidget()),
      matchTitle(new QLabel()),
      backButton1(new QPushButton("Назад к турнирам")),
      backButton2(new QPushButton("Назад к матчам")),
      leftPanelStack(new QStackedWidget()),
      statsTabs(new QTabWidget()),
      matchOverviewTab(new QWidget()),
      historyTab(new QWidget()),
      currentTournamentId(-1),
      currentMatchId(-1),
      currentTournamentName(""),
      currentRound(-1),
      currentRoundPage(0),
      roundsPerPage(5),
      roundsPopup(nullptr),
      roundButton(nullptr),
      roundsGroup(nullptr),
      db(QSqlDatabase::addDatabase("QSQLITE"))
{
    setWindowTitle("SportsTracker - Анализ спортивных результатов");
    resize(1400, 800);
    setStyleSheet("QMainWindow { background-color: #f5f5f5; }");

    if (!initializeDatabase()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось подключиться к базе данных");
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
    QString dbPath = QDir::homePath() + "/database/sports.db";

    if (!QDir().mkpath(QDir::homePath() + "/database")) {
        qDebug() << "Не удалось создать директорию для БД";
        return false;
    }

    db.setDatabaseName(dbPath);

    if (!db.open()) {
        qDebug() << "Не удалось открыть БД:" << db.lastError().text();
        return false;
    }

    QStringList tables = db.tables();
    QStringList requiredTables = {"sports", "tournaments", "teams", "matches", "standings",
                                "players", "match_lineups", "match_events", "match_stats"};

    for (const QString &table : requiredTables) {
        if (!tables.contains(table)) {
            qDebug() << "Таблица" << table << "не найдена в БД";
            QMessageBox::warning(this, "Ошибка",
                QString("Таблица %1 не найдена в базе данных").arg(table));
            return false;
        }
    }

    return true;
}

void SportsTracker::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setStyleSheet("background-color: #f5f5f5;");
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(15);

    stackedWidget->setStyleSheet("QStackedWidget { background: white; border-radius: 8px; }");
    mainLayout->addWidget(stackedWidget);

    // Страница выбора турнира
    QWidget *selectionPage = new QWidget();
    QHBoxLayout *selectionLayout = new QHBoxLayout(selectionPage);
    selectionLayout->setContentsMargins(15, 15, 15, 15);
    selectionLayout->setSpacing(20);

    sportsTree->setHeaderHidden(true);
    sportsTree->setStyleSheet(
        "QTreeWidget { font-size: 16px; background: white; border: 1px solid #ddd; "
        "border-radius: 6px; padding: 5px; }"
        "QTreeWidget::item { height: 30px; padding: 5px; }"
        "QTreeWidget::item:hover { background: #e6f2ff; }"
        "QTreeWidget::item:selected { background: #cce0ff; color: black; }");
    sportsTree->setColumnCount(1);
    connect(sportsTree, &QTreeWidget::itemClicked, this, &SportsTracker::onTournamentClicked);
    selectionLayout->addWidget(sportsTree, 1);
    stackedWidget->addWidget(selectionPage);

    // Страница турнира
    QWidget *tournamentPage = new QWidget();
    QHBoxLayout *tournamentLayout = new QHBoxLayout(tournamentPage);
    tournamentLayout->setContentsMargins(15, 15, 15, 15);
    tournamentLayout->setSpacing(20);

    // Левая панель (матчи/статистика)
    leftPanelStack->setStyleSheet("QStackedWidget { background: transparent; }");

    // Страница списка матчей
    QWidget *matchesPage = new QWidget();
    QVBoxLayout *matchesLayout = new QVBoxLayout(matchesPage);
    matchesLayout->setContentsMargins(0, 0, 0, 0);
    matchesLayout->setSpacing(10);

    QWidget *roundSelectorWidget = new QWidget();
    QHBoxLayout *roundSelectorLayout = new QHBoxLayout(roundSelectorWidget);
    roundSelectorLayout->setContentsMargins(0, 0, 0, 10);

    QLabel *roundLabel = new QLabel("Тур:");
    roundSelectorLayout->addWidget(roundLabel);

    roundButton = new QPushButton("Выбрать тур");
    roundButton->setStyleSheet(
        "QPushButton { padding: 5px 10px; background: #4a90e2; color: white; "
        "border-radius: 4px; }"
        "QPushButton:hover { background: #3a7bc8; }");
    connect(roundButton, &QPushButton::clicked, this, &SportsTracker::showRoundSelectionPopup);
    roundSelectorLayout->addWidget(roundButton);
    roundSelectorLayout->addStretch();

    matchesLayout->addWidget(roundSelectorWidget);

    matchesList->setStyleSheet(
        "QListWidget { font-size: 14px; background: white; border: 1px solid #ddd; "
        "border-radius: 6px; }"
        "QListWidget::item { padding: 8px; border-bottom: 1px solid #eee; }"
        "QListWidget::item:hover { background: #e6f2ff; }"
        "QListWidget::item:selected { background: #cce0ff; }");
    matchesList->setAlternatingRowColors(false);
    connect(matchesList, &QListWidget::itemClicked, this, &SportsTracker::showMatchStats);
    matchesLayout->addWidget(matchesList, 1);

    backButton1->setStyleSheet(
        "QPushButton { font-size: 14px; padding: 8px 16px; background: #4a90e2; "
        "color: white; border-radius: 4px; border: none; }"
        "QPushButton:hover { background: #3a7bc8; }");
    connect(backButton1, &QPushButton::clicked, [this]() { stackedWidget->setCurrentIndex(0); });
    matchesLayout->addWidget(backButton1, 0, Qt::AlignRight);
    leftPanelStack->addWidget(matchesPage);

    // Страница статистики матча
    QWidget *statsPage = new QWidget();
    QVBoxLayout *statsPageLayout = new QVBoxLayout(statsPage);
    statsPageLayout->setContentsMargins(0, 0, 0, 0);
    statsPageLayout->setSpacing(15);

    matchTitle->setStyleSheet(
        "QLabel { font-size: 20px; font-weight: bold; color: #333; padding: 10px 0; }");
    matchTitle->setAlignment(Qt::AlignCenter);
    statsPageLayout->addWidget(matchTitle);

    statsTabs->setStyleSheet(
        "QTabWidget::pane { border: 1px solid #ddd; border-radius: 6px; background: white; }"
        "QTabBar::tab { padding: 8px 16px; background: #f0f0f0; border: 1px solid #ddd; "
        "border-bottom: none; border-top-left-radius: 4px; border-top-right-radius: 4px; }"
        "QTabBar::tab:selected { background: white; border-bottom: 1px solid white; "
        "margin-bottom: -1px; }");

    // Вкладка обзора матча
    QVBoxLayout *overviewLayout = new QVBoxLayout(matchOverviewTab);
    overviewLayout->setContentsMargins(10, 10, 10, 10);
    overviewLayout->setSpacing(15);

    QString tableStyle =
        "QTableWidget { font-size: 14px; background: white; border: 1px solid #ddd; "
        "border-radius: 6px; gridline-color: #eee; }"
        "QHeaderView::section { background-color: #4a90e2; color: white; padding: 6px; "
        "font-weight: bold; border: none; }"
        "QTableWidget::item { padding: 5px; }"
        "QTableWidget::item:selected { background: #cce0ff; }";

    statsTable->setStyleSheet(tableStyle);
    statsTable->verticalHeader()->setVisible(false);
    statsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    statsTable->setAlternatingRowColors(false);
    overviewLayout->addWidget(new QLabel("<b style='font-size: 16px;'>Основная статистика матча</b>"));
    overviewLayout->addWidget(statsTable);

    lineupsTable->setStyleSheet(tableStyle +
        "QTableView::item { border-right: 1px solid #ddd; }"
        "QTableView::item:nth-child(4) { border-right: 2px solid #aaa; }");
    lineupsTable->verticalHeader()->setVisible(false);
    lineupsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    lineupsTable->setAlternatingRowColors(false);
    overviewLayout->addWidget(new QLabel("<b style='font-size: 16px;'>Составы команд</b>"));
    overviewLayout->addWidget(lineupsTable);

    scorersTable->setStyleSheet(tableStyle);
    scorersTable->verticalHeader()->setVisible(false);
    scorersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    scorersTable->setAlternatingRowColors(false);
    overviewLayout->addWidget(new QLabel("<b style='font-size: 16px;'>Ход матча</b>"));
    overviewLayout->addWidget(scorersTable);

    statsTabs->addTab(matchOverviewTab, "Обзор матча");

    // Вкладка истории
    QVBoxLayout *historyLayout = new QVBoxLayout(historyTab);
    historyLayout->setContentsMargins(10, 10, 10, 10);
    historyLayout->setSpacing(15);

    QLabel *recentMatchesLabel = new QLabel("<b style='font-size: 16px;'>Последние матчи команд</b>");
    historyLayout->addWidget(recentMatchesLabel);

    QWidget *recentMatchesWidget = new QWidget();
    QHBoxLayout *recentMatchesLayout = new QHBoxLayout(recentMatchesWidget);
    recentMatchesLayout->setContentsMargins(0, 0, 0, 0);
    recentMatchesLayout->setSpacing(15);

    team1RecentMatches->setStyleSheet(tableStyle);
    team1RecentMatches->verticalHeader()->setVisible(false);
    team1RecentMatches->setEditTriggers(QAbstractItemView::NoEditTriggers);
    team1RecentMatches->setAlternatingRowColors(false);
    recentMatchesLayout->addWidget(team1RecentMatches);

    team2RecentMatches->setStyleSheet(tableStyle);
    team2RecentMatches->verticalHeader()->setVisible(false);
    team2RecentMatches->setEditTriggers(QAbstractItemView::NoEditTriggers);
    team2RecentMatches->setAlternatingRowColors(false);
    recentMatchesLayout->addWidget(team2RecentMatches);

    historyLayout->addWidget(recentMatchesWidget);

    QLabel *h2hLabel = new QLabel("<b style='font-size: 16px;'>История очных встреч</b>");
    historyLayout->addWidget(h2hLabel);

    headToHeadMatches->setStyleSheet(tableStyle);
    headToHeadMatches->verticalHeader()->setVisible(false);
    headToHeadMatches->setEditTriggers(QAbstractItemView::NoEditTriggers);
    headToHeadMatches->setAlternatingRowColors(false);
    historyLayout->addWidget(headToHeadMatches);

    statsTabs->addTab(historyTab, "История");
    statsPageLayout->addWidget(statsTabs, 1);

    backButton2->setStyleSheet(backButton1->styleSheet());
    connect(backButton2, &QPushButton::clicked, this, &SportsTracker::showMatchesList);
    statsPageLayout->addWidget(backButton2, 0, Qt::AlignRight);
    leftPanelStack->addWidget(statsPage);

    tournamentLayout->addWidget(leftPanelStack, 1);

    // Правая панель - турнирная таблица
    QWidget *standingsWidget = new QWidget();
    QVBoxLayout *standingsLayout = new QVBoxLayout(standingsWidget);
    standingsLayout->setContentsMargins(0, 0, 0, 0);
    standingsLayout->setSpacing(10);

    QLabel *standingsLabel = new QLabel("<b style='font-size: 18px;'>Турнирная таблица</b>");
    standingsLabel->setAlignment(Qt::AlignCenter);
    standingsLayout->addWidget(standingsLabel);

    standingsTable->setStyleSheet(tableStyle);
    standingsTable->verticalHeader()->setVisible(false);
    standingsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    standingsTable->setAlternatingRowColors(false);
    standingsTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QHeaderView* header = standingsTable->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Interactive);
    header->setDefaultAlignment(Qt::AlignCenter);

    standingsTable->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    standingsTable->verticalScrollBar()->setSingleStep(20);

    standingsLayout->addWidget(standingsTable);
    tournamentLayout->addWidget(standingsWidget, 1);

    stackedWidget->addWidget(tournamentPage);
}

void SportsTracker::loadSports()
{
    sportsTree->clear();

    QSqlQuery query("SELECT id, name FROM sports ORDER BY name", db);
    if (!query.exec()) {
        qDebug() << "Ошибка загрузки видов спорта:" << query.lastError().text();
        return;
    }

    while (query.next()) {
        QTreeWidgetItem *sportItem = new QTreeWidgetItem(sportsTree);
        sportItem->setText(0, query.value(1).toString());
        sportItem->setData(0, Qt::UserRole, query.value(0).toInt());
        new QTreeWidgetItem(sportItem); // Пустой child для отображения стрелки раскрытия
    }

    connect(sportsTree, &QTreeWidget::itemExpanded, this, &SportsTracker::loadTournaments);
}

void SportsTracker::loadTournaments(QTreeWidgetItem *sportItem)
{
    if (sportItem->childCount() == 1 && sportItem->child(0)->text(0).isEmpty()) {
        delete sportItem->takeChild(0);

        int sportId = sportItem->data(0, Qt::UserRole).toInt();
        QSqlQuery query(db);
        query.prepare("SELECT id, name FROM tournaments WHERE sport_id = ? ORDER BY name");
        query.addBindValue(sportId);

        if (!query.exec()) {
            qDebug() << "Ошибка загрузки турниров:" << query.lastError().text();
            return;
        }

        while (query.next()) {
            QTreeWidgetItem *tournamentItem = new QTreeWidgetItem(sportItem);
            tournamentItem->setText(0, query.value(1).toString());
            tournamentItem->setData(0, Qt::UserRole, query.value(0).toInt());
        }
    }
}

void SportsTracker::onTournamentClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    if (item->parent()) {
        int tournamentId = item->data(0, Qt::UserRole).toInt();
        QString tournamentName = item->text(0);
        showTournamentPage(tournamentId, tournamentName);
    }
}

void SportsTracker::showTournamentPage(int tournamentId, const QString &tournamentName)
{
    currentTournamentId = tournamentId;
    currentTournamentName = tournamentName;
    currentRoundPage = 0;
    loadMatchesAndStandings();
    stackedWidget->setCurrentIndex(1);
    leftPanelStack->setCurrentIndex(0);
    setWindowTitle(tournamentName);
}

void SportsTracker::loadMatchesAndStandings()
{
    if (currentTournamentId == -1) return;

    // Загрузка информации о турах
    QSqlQuery roundsQuery(db);
    roundsQuery.prepare("SELECT DISTINCT round FROM matches WHERE tournament_id = ? ORDER BY round");
    roundsQuery.addBindValue(currentTournamentId);

    allRounds.clear();
    if (roundsQuery.exec()) {
        while (roundsQuery.next()) {
            allRounds.append(roundsQuery.value(0).toInt());
        }
    } else {
        qDebug() << "Ошибка загрузки туров:" << roundsQuery.lastError().text();
    }

    // Загружаем последний тур по умолчанию
    if (!allRounds.isEmpty()) {
        currentRound = allRounds.last();
        roundButton->setText(QString("Тур %1").arg(currentRound));
    } else {
        currentRound = -1;
        roundButton->setText("Все туры");
    }

    loadMatchesForCurrentRound();
    loadStandings();
}

void SportsTracker::loadMatchesForCurrentRound()
{
    matchesList->clear();
    if (currentTournamentId == -1) return;

    QSqlQuery matchesQuery(db);
    QString queryStr =
        "SELECT m.id, m.date, t1.name, t2.name, "
        "CASE WHEN m.score IS NULL THEN '-' ELSE m.score END as score "
        "FROM matches m "
        "JOIN teams t1 ON m.team1_id = t1.id "
        "JOIN teams t2 ON m.team2_id = t2.id "
        "WHERE m.tournament_id = ? ";

    if (currentRound > 0) {
        queryStr += "AND m.round = ? ";
    }

    queryStr += "ORDER BY m.date DESC";

    matchesQuery.prepare(queryStr);
    matchesQuery.addBindValue(currentTournamentId);

    if (currentRound > 0) {
        matchesQuery.addBindValue(currentRound);
    }

    if (!matchesQuery.exec()) {
        qDebug() << "Ошибка загрузки матчей:" << matchesQuery.lastError().text();
        return;
    }

    while (matchesQuery.next()) {
        QString score = matchesQuery.value(4).toString();
        if (score == "-") score = "? - ?";

        QString matchText = QString("%1: %2 %3 %4")
            .arg(matchesQuery.value(1).toDate().toString("dd.MM.yyyy"))
            .arg(matchesQuery.value(2).toString())
            .arg(score)
            .arg(matchesQuery.value(3).toString());

        QListWidgetItem *item = new QListWidgetItem(matchText, matchesList);
        item->setData(Qt::UserRole, matchesQuery.value(0).toInt());
    }
}

void SportsTracker::loadStandings()
{
    standingsTable->clear();
    if (currentTournamentId == -1) return;

    QSqlQuery standingsQuery(db);
    standingsQuery.prepare(
        "SELECT s.position, t.name, s.points, s.games_played, s.wins, s.draws, s.losses, "
        "s.goals_for, s.goals_against, (s.goals_for - s.goals_against) as goal_difference "
        "FROM standings s "
        "JOIN teams t ON s.team_id = t.id "
        "WHERE s.tournament_id = ? "
        "ORDER BY s.position"
    );
    standingsQuery.addBindValue(currentTournamentId);

    if (!standingsQuery.exec()) {
        qDebug() << "Ошибка загрузки турнирной таблицы:" << standingsQuery.lastError().text();
        return;
    }

    standingsTable->setRowCount(0);
    standingsTable->setColumnCount(10);
    QStringList headers = {"Поз", "Команда", "О", "И", "В", "Н", "П", "ЗГ", "ПГ", "РГ"};
    standingsTable->setHorizontalHeaderLabels(headers);

    while (standingsQuery.next()) {
        int row = standingsTable->rowCount();
        standingsTable->insertRow(row);

        int position = standingsQuery.value(0).toInt();
        QColor rowColor = Qt::white;

        if (position <= 4) {
            rowColor = QColor(220, 255, 220);
        } else if (position <= 6) {
            rowColor = QColor(220, 220, 255);
        } else if (position >= 18) {
            rowColor = QColor(255, 220, 220);
        }

        for (int col = 0; col < 10; ++col) {
            QTableWidgetItem *item = new QTableWidgetItem(standingsQuery.value(col).toString());
            item->setTextAlignment(col == 1 ? Qt::AlignLeft | Qt::AlignVCenter : Qt::AlignCenter);
            item->setBackground(rowColor);

            if (col == 1) {
                item->setToolTip(standingsQuery.value(col).toString());
            }

            standingsTable->setItem(row, col, item);
        }
    }

    standingsTable->setColumnWidth(0, 40);
    standingsTable->setColumnWidth(1, 300);
    standingsTable->setColumnWidth(2, 30);
    standingsTable->setColumnWidth(3, 30);
    standingsTable->setColumnWidth(4, 30);
    standingsTable->setColumnWidth(5, 30);
    standingsTable->setColumnWidth(6, 30);
    standingsTable->setColumnWidth(7, 30);
    standingsTable->setColumnWidth(8, 30);
    standingsTable->setColumnWidth(9, 30);

    standingsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
}

void SportsTracker::showRoundSelectionPopup()
{
    if (allRounds.isEmpty()) return;

    if (!roundsPopup) {
        roundsPopup = new QWidget(nullptr, Qt::Popup);
        roundsPopup->setStyleSheet(
            "background: white; border: 1px solid #ddd; border-radius: 4px;");

        QVBoxLayout *layout = new QVBoxLayout(roundsPopup);
        layout->setContentsMargins(5, 5, 5, 5);
        layout->setSpacing(5);

        roundsGroup = new QButtonGroup(this);
        connect(roundsGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked),
                this, &SportsTracker::onRoundSelected);
    }

    // Очищаем предыдущие кнопки
    QLayoutItem* child;
    while ((child = roundsPopup->layout()->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    roundsGroup->buttons().clear();

    // Добавляем кнопки для текущей страницы
    int start = currentRoundPage * roundsPerPage;
    int end = qMin(start + roundsPerPage, allRounds.size());

    for (int i = start; i < end; ++i) {
        int round = allRounds[i];
        QPushButton *roundBtn = new QPushButton(QString::number(round));
        roundBtn->setStyleSheet(
            "QPushButton { padding: 5px 10px; background: white; border: 1px solid #ddd; "
            "border-radius: 3px; }"
            "QPushButton:hover { background: #e6f2ff; }");
        roundsGroup->addButton(roundBtn, round);
        roundsPopup->layout()->addWidget(roundBtn);
    }

    // Добавляем кнопки навигации, если нужно
    if (allRounds.size() > roundsPerPage) {
        QHBoxLayout *navLayout = new QHBoxLayout();

        QPushButton *prevBtn = new QPushButton("<");
        prevBtn->setEnabled(currentRoundPage > 0);
        prevBtn->setFixedWidth(30);
        prevBtn->setStyleSheet(
            "QPushButton { padding: 2px; background: #f0f0f0; border: 1px solid #ddd; "
            "border-radius: 3px; }"
            "QPushButton:hover { background: #e0e0e0; }"
            "QPushButton:disabled { color: #aaa; }");

        QPushButton *nextBtn = new QPushButton(">");
        nextBtn->setEnabled((currentRoundPage + 1) * roundsPerPage < allRounds.size());
        nextBtn->setFixedWidth(30);
        nextBtn->setStyleSheet(prevBtn->styleSheet());

        connect(prevBtn, &QPushButton::clicked, [this]() {
            if (currentRoundPage > 0) {
                currentRoundPage--;
                showRoundSelectionPopup();
            }
        });

        connect(nextBtn, &QPushButton::clicked, [this]() {
            if ((currentRoundPage + 1) * roundsPerPage < allRounds.size()) {
                currentRoundPage++;
                showRoundSelectionPopup();
            }
        });

        navLayout->addWidget(prevBtn);
        navLayout->addStretch();
        navLayout->addWidget(nextBtn);

        QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(roundsPopup->layout());
        if (mainLayout) {
            mainLayout->addLayout(navLayout);
        }
    }

    // Показываем попап
    QPoint pos = roundButton->mapToGlobal(QPoint(0, roundButton->height()));
    roundsPopup->move(pos);
    roundsPopup->adjustSize();
    roundsPopup->show();
}

void SportsTracker::onRoundSelected(QAbstractButton *button)
{
    if (!roundsPopup || !button) return;
    roundsPopup->hide();
    currentRound = roundsGroup->id(button);
    roundButton->setText(QString("Тур %1").arg(currentRound));
    loadMatchesForCurrentRound();
}

void SportsTracker::showMatchStats(QListWidgetItem *item)
{
    if (!item) return;

    currentMatchId = item->data(Qt::UserRole).toInt();
    matchTitle->setText(item->text());

    statsTable->clear();
    lineupsTable->clear();
    scorersTable->clear();
    team1RecentMatches->clear();
    team2RecentMatches->clear();
    headToHeadMatches->clear();

    QSqlQuery matchQuery(db);
    matchQuery.prepare(
        "SELECT t1.name, t2.name, m.date "
        "FROM matches m "
        "JOIN teams t1 ON m.team1_id = t1.id "
        "JOIN teams t2 ON m.team2_id = t2.id "
        "WHERE m.id = ?"
    );
    matchQuery.addBindValue(currentMatchId);

    if (matchQuery.exec() && matchQuery.next()) {
        QString team1 = matchQuery.value(0).toString();
        QString team2 = matchQuery.value(1).toString();
        QDate matchDate = QDate::fromString(matchQuery.value(2).toString(), Qt::ISODate);

        loadMatchStats(currentMatchId, team1, team2);
        loadLineups(currentMatchId, team1, team2);
        loadScorers(currentMatchId, team1, team2);
        loadRecentMatches(team1, matchDate, team1RecentMatches);
        loadRecentMatches(team2, matchDate, team2RecentMatches);
        loadHeadToHeadMatches(team1, team2, matchDate, headToHeadMatches);
    } else {
        qDebug() << "Ошибка загрузки данных матча:" << matchQuery.lastError().text();
    }

    leftPanelStack->setCurrentIndex(1);
}

void SportsTracker::loadMatchStats(int matchId, const QString& team1, const QString& team2)
{
    QSqlQuery statsQuery(db);
    statsQuery.prepare(
        "SELECT stat_name, "
        "(SELECT stat_value FROM match_stats WHERE match_id = ? AND team_id = (SELECT id FROM teams WHERE name = ?) AND stat_name = ms.stat_name) as team1_value, "
        "(SELECT stat_value FROM match_stats WHERE match_id = ? AND team_id = (SELECT id FROM teams WHERE name = ?) AND stat_name = ms.stat_name) as team2_value "
        "FROM match_stats ms WHERE match_id = ? GROUP BY stat_name"
    );
    statsQuery.addBindValue(matchId);
    statsQuery.addBindValue(team1);
    statsQuery.addBindValue(matchId);
    statsQuery.addBindValue(team2);
    statsQuery.addBindValue(matchId);

    if (statsQuery.exec()) {
        statsTable->setRowCount(0);
        statsTable->setColumnCount(3);

        // Убираем стандартные заголовки
        statsTable->setHorizontalHeaderLabels({"", "", ""});
        statsTable->horizontalHeader()->setVisible(false);

        // Добавляем заголовки с названиями команд
        QFont headerFont;
        headerFont.setBold(true);
        headerFont.setPointSize(12);

        QTableWidgetItem *team1Header = new QTableWidgetItem(team1);
        team1Header->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        team1Header->setFont(headerFont);

        QTableWidgetItem *team2Header = new QTableWidgetItem(team2);
        team2Header->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        team2Header->setFont(headerFont);

        statsTable->insertRow(0);
        statsTable->setItem(0, 0, team1Header);
        statsTable->setItem(0, 2, team2Header);

        // Основные данные статистики
        int currentRow = 1;
        while (statsQuery.next()) {
            statsTable->insertRow(currentRow);

            // Команда 1
            QTableWidgetItem *team1Item = new QTableWidgetItem(statsQuery.value(1).toString());
            team1Item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            statsTable->setItem(currentRow, 0, team1Item);

            // Название показателя
            QTableWidgetItem *statNameItem = new QTableWidgetItem(statsQuery.value(0).toString());
            statNameItem->setTextAlignment(Qt::AlignCenter);
            statNameItem->setFont(headerFont);
            statsTable->setItem(currentRow, 1, statNameItem);

            // Команда 2
            QTableWidgetItem *team2Item = new QTableWidgetItem(statsQuery.value(2).toString());
            team2Item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            statsTable->setItem(currentRow, 2, team2Item);

            currentRow++;
        }

        // Настройка внешнего вида таблицы
        statsTable->verticalHeader()->setVisible(false);
        statsTable->setShowGrid(false);
        statsTable->setStyleSheet(
            "QTableWidget { border: none; background: white; }"
            "QTableWidget::item { border: none; padding: 5px; }");

        // Настройка ширины столбцов
        statsTable->setColumnWidth(0, 175);
        statsTable->setColumnWidth(1, 250);
        statsTable->setColumnWidth(2, 150);
        statsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    } else {
        qDebug() << "Ошибка загрузки статистики матча:" << statsQuery.lastError().text();
    }
}

void SportsTracker::loadLineups(int matchId, const QString& team1, const QString& team2)
{
    // Получаем фактические ID команд
    int team1Id = -1, team2Id = -1;
    QSqlQuery teamQuery(db);

    teamQuery.prepare("SELECT id FROM teams WHERE name = ?");
    teamQuery.addBindValue(team1);
    if (teamQuery.exec() && teamQuery.next()) {
        team1Id = teamQuery.value(0).toInt();
    }

    teamQuery.prepare("SELECT id FROM teams WHERE name = ?");
    teamQuery.addBindValue(team2);
    if (teamQuery.exec() && teamQuery.next()) {
        team2Id = teamQuery.value(0).toInt();
    }

    // Загружаем составы для обеих команд, разделяя на основных и запасных
    QVector<QStringList> team1Starters;
    QVector<QStringList> team1Substitutes;
    QVector<QStringList> team2Starters;
    QVector<QStringList> team2Substitutes;

    QSqlQuery lineupsQuery(db);
    lineupsQuery.prepare(
        "SELECT p.name, ml.team_id, ml.position, ml.is_starting, ml.jersey_number "
        "FROM match_lineups ml "
        "JOIN players p ON ml.player_id = p.id "
        "WHERE ml.match_id = ? "
        "ORDER BY ml.team_id, ml.is_starting DESC, ml.position"
    );
    lineupsQuery.addBindValue(matchId);

    if (!lineupsQuery.exec()) {
        qDebug() << "Ошибка загрузки составов:" << lineupsQuery.lastError().text();
        return;
    }

    while (lineupsQuery.next()) {
        int teamId = lineupsQuery.value(1).toInt();
        QStringList playerData;
        playerData << lineupsQuery.value(4).toString()  // номер
                   << lineupsQuery.value(0).toString()  // имя
                   << lineupsQuery.value(2).toString(); // позиция

        if (teamId == team1Id) {
            if (lineupsQuery.value(3).toBool()) {
                team1Starters.append(playerData);
            } else {
                team1Substitutes.append(playerData);
            }
        } else if (teamId == team2Id) {
            if (lineupsQuery.value(3).toBool()) {
                team2Starters.append(playerData);
            } else {
                team2Substitutes.append(playerData);
            }
        }
    }

    // Настраиваем таблицу для отображения составов
    lineupsTable->setRowCount(0);
    lineupsTable->setColumnCount(4); // 2 колонки (игроки команд) с номерами и именами

    // Убираем стандартные заголовки
    lineupsTable->setHorizontalHeaderLabels({"", "", "", ""});
    lineupsTable->horizontalHeader()->setVisible(false);
    lineupsTable->verticalHeader()->setVisible(false);

    // Настраиваем стиль таблицы
    lineupsTable->setStyleSheet(
        "QTableWidget { border: none; background: white; }"
        "QTableWidget::item { border: none; padding: 3px; }");

    // Добавляем заголовок "Основной состав"
    int currentRow = 0;
    lineupsTable->insertRow(currentRow);
    QTableWidgetItem *startersHeader = new QTableWidgetItem("Основной состав");
    startersHeader->setTextAlignment(Qt::AlignCenter);
    QFont headerFont;
    headerFont.setBold(true);
    headerFont.setPointSize(12);
    startersHeader->setFont(headerFont);
    lineupsTable->setItem(currentRow, 0, startersHeader);
    lineupsTable->setSpan(currentRow, 0, 1, 4);
    currentRow++;

    // Определяем максимальное количество игроков в стартовых составах
    int maxStarters = qMax(team1Starters.size(), team2Starters.size());

    // Заполняем стартовые составы
    for (int i = 0; i < maxStarters; ++i) {
        lineupsTable->insertRow(currentRow);

        // Игрок команды 1
        if (i < team1Starters.size()) {
            QString playerText = team1Starters[i][0] + " " + team1Starters[i][1] + " (" + team1Starters[i][2] + ")";
            QTableWidgetItem *team1Item = new QTableWidgetItem(playerText);
            team1Item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            lineupsTable->setItem(currentRow, 0, team1Item);
            lineupsTable->setItem(currentRow, 1, new QTableWidgetItem("")); // Пустая ячейка-разделитель
        }

        // Игрок команды 2
        if (i < team2Starters.size()) {
            QString playerText = team2Starters[i][0] + " " + team2Starters[i][1] + " (" + team2Starters[i][2] + ")";
            QTableWidgetItem *team2Item = new QTableWidgetItem(playerText);
            team2Item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            lineupsTable->setItem(currentRow, 2, team2Item);
            lineupsTable->setItem(currentRow, 3, new QTableWidgetItem("")); // Пустая ячейка-разделитель
        }

        currentRow++;
    }

    // Добавляем заголовок "Запасной состав"
    lineupsTable->insertRow(currentRow);
    QTableWidgetItem *substitutesHeader = new QTableWidgetItem("Запасной состав");
    substitutesHeader->setTextAlignment(Qt::AlignCenter);
    substitutesHeader->setFont(headerFont);
    lineupsTable->setItem(currentRow, 0, substitutesHeader);
    lineupsTable->setSpan(currentRow, 0, 1, 4);
    currentRow++;

    // Определяем максимальное количество игроков в запасных составах
    int maxSubstitutes = qMax(team1Substitutes.size(), team2Substitutes.size());

    // Заполняем запасные составы
    for (int i = 0; i < maxSubstitutes; ++i) {
        lineupsTable->insertRow(currentRow);

        // Игрок команды 1
        if (i < team1Substitutes.size()) {
            QString playerText = team1Substitutes[i][0] + " " + team1Substitutes[i][1] + " (" + team1Substitutes[i][2] + ")";
            QTableWidgetItem *team1Item = new QTableWidgetItem(playerText);
            team1Item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            lineupsTable->setItem(currentRow, 0, team1Item);
            lineupsTable->setItem(currentRow, 1, new QTableWidgetItem("")); // Пустая ячейка-разделитель
        }

        // Игрок команды 2
        if (i < team2Substitutes.size()) {
            QString playerText = team2Substitutes[i][0] + " " + team2Substitutes[i][1] + " (" + team2Substitutes[i][2] + ")";
            QTableWidgetItem *team2Item = new QTableWidgetItem(playerText);
            team2Item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            lineupsTable->setItem(currentRow, 2, team2Item);
            lineupsTable->setItem(currentRow, 3, new QTableWidgetItem("")); // Пустая ячейка-разделитель
        }

        currentRow++;
    }

    // Настраиваем ширину столбцов
    lineupsTable->setColumnWidth(0, 250); // Игроки команды 1
    lineupsTable->setColumnWidth(1, 20);  // Разделитель
    lineupsTable->setColumnWidth(2, 250); // Игроки команды 2
    lineupsTable->setColumnWidth(3, 20);  // Разделитель
}

void SportsTracker::loadScorers(int matchId, const QString& team1, const QString& team2)
{
    QSqlQuery scorersQuery(db);
    scorersQuery.prepare(
        "SELECT me.event_type, me.minute, p.name, me.description, me.team_id "
        "FROM match_events me "
        "LEFT JOIN players p ON me.player_id = p.id "
        "WHERE me.match_id = ? "
        "ORDER BY me.minute"
    );
    scorersQuery.addBindValue(matchId);

    scorersTable->setRowCount(0);
    scorersTable->setColumnCount(5);
    scorersTable->setHorizontalHeaderLabels({"Тип", "Минута", "Игрок", "Описание", "Команда"});

    if (!scorersQuery.exec()) {
        qDebug() << "Ошибка загрузки событий матча:" << scorersQuery.lastError().text();
        return;
    }

    // Получаем фактические ID команд
    int team1Id = -1, team2Id = -1;
    QSqlQuery teamQuery(db);

    teamQuery.prepare("SELECT id FROM teams WHERE name = ?");
    teamQuery.addBindValue(team1);
    if (teamQuery.exec() && teamQuery.next()) {
        team1Id = teamQuery.value(0).toInt();
    }

    teamQuery.prepare("SELECT id FROM teams WHERE name = ?");
    teamQuery.addBindValue(team2);
    if (teamQuery.exec() && teamQuery.next()) {
        team2Id = teamQuery.value(0).toInt();
    }

    while (scorersQuery.next()) {
        int row = scorersTable->rowCount();
        scorersTable->insertRow(row);

        QString eventType = scorersQuery.value(0).toString();
        if (eventType == "goal") eventType = "Гол";
        else if (eventType == "yellow_card") eventType = "ЖК";
        else if (eventType == "red_card") eventType = "КК";
        else if (eventType == "substitution") eventType = "Замена";

        int teamId = scorersQuery.value(4).toInt();
        QString teamName = (teamId == team1Id) ? team1 : team2;
        QString playerName = scorersQuery.value(2).isNull() ? "-" : scorersQuery.value(2).toString();

        scorersTable->setItem(row, 0, new QTableWidgetItem(eventType));
        scorersTable->setItem(row, 1, new QTableWidgetItem(scorersQuery.value(1).toString()));
        scorersTable->setItem(row, 2, new QTableWidgetItem(playerName));
        scorersTable->setItem(row, 3, new QTableWidgetItem(scorersQuery.value(3).toString()));
        scorersTable->setItem(row, 4, new QTableWidgetItem(teamName));
    }
    scorersTable->resizeColumnsToContents();
}

void SportsTracker::loadRecentMatches(const QString& team, const QDate& beforeDate, QTableWidget* table)
{
    QSqlQuery recentQuery(db);
    recentQuery.prepare(
        "SELECT m.date, t1.name, t2.name, m.score "
        "FROM matches m "
        "JOIN teams t1 ON m.team1_id = t1.id "
        "JOIN teams t2 ON m.team2_id = t2.id "
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
    } else {
        qDebug() << "Ошибка загрузки последних матчей:" << recentQuery.lastError().text();
    }
}

void SportsTracker::loadHeadToHeadMatches(const QString& team1, const QString& team2, const QDate& beforeDate, QTableWidget* table)
{
    QSqlQuery h2hQuery(db);
    h2hQuery.prepare(
        "SELECT m.date, t1.name, t2.name, m.score "
        "FROM matches m "
        "JOIN teams t1 ON m.team1_id = t1.id "
        "JOIN teams t2 ON m.team2_id = t2.id "
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
    } else {
        qDebug() << "Ошибка загрузки очных встреч:" << h2hQuery.lastError().text();
    }
}

void SportsTracker::showMatchesList()
{
    leftPanelStack->setCurrentIndex(0);
}
