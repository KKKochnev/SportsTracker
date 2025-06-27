#include "sportstracker.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlError>
#include <QFile>
#include <QDir>
#include <QDebug>

SportsTracker::SportsTracker(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Просмотр спортивных результатов");
    resize(1400, 900);

    if (!initializeDatabase()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось инициализировать базу данных");
        return;
    }

    setupUI();
    loadSports();
}

SportsTracker::~SportsTracker()
{
    db.close();
}

bool SportsTracker::initializeDatabase()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("/home/qt/database/sports.db");

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

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    // Древовидный список слева (40% ширины)
    sportsTree = new QTreeWidget();
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
    sportsTree->setColumnWidth(0, 400);
    connect(sportsTree, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem *item, int) {
        if (item->data(0, Qt::UserRole + 1).toInt() == MatchItem) {
            int matchId = item->data(0, Qt::UserRole).toInt();
            loadMatchStats(matchId, item->text(0));
        }
    });
    mainLayout->addWidget(sportsTree, 4); // 40% ширины

    // Панель статистики справа (60% ширины)
    QWidget *statsPanel = new QWidget();
    QVBoxLayout *statsLayout = new QVBoxLayout(statsPanel);
    statsLayout->setContentsMargins(20, 0, 0, 0);

    matchTitle = new QLabel("Выберите матч для просмотра статистики");
    matchTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50;");
    matchTitle->setAlignment(Qt::AlignCenter);
    statsLayout->addWidget(matchTitle);

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
    statsLayout->addWidget(statsTable);

    mainLayout->addWidget(statsPanel, 6); // 60% ширины
}

void SportsTracker::loadSports()
{
    sportsTree->clear();

    QSqlQuery query("SELECT id, name FROM sports ORDER BY name");
    while (query.next()) {
        int id = query.value(0).toInt();
        QString name = query.value(1).toString();

        QTreeWidgetItem *sportItem = new QTreeWidgetItem(sportsTree);
        sportItem->setText(0, name);
        sportItem->setData(0, Qt::UserRole, id);
        sportItem->setData(0, Qt::UserRole + 1, SportItem);

        // Добавляем пустой элемент для отображения стрелки раскрытия
        new QTreeWidgetItem(sportItem);
    }

    // Обработка раскрытия узлов
    connect(sportsTree, &QTreeWidget::itemExpanded, this, [this](QTreeWidgetItem *item) {
        int type = item->data(0, Qt::UserRole + 1).toInt();

        if (type == SportItem && item->childCount() == 1 && item->child(0)->text(0).isEmpty()) {
            delete item->takeChild(0); // Удаляем placeholder
            loadTournaments(item);
        }
        else if (type == TournamentItem && item->childCount() == 1 && item->child(0)->text(0).isEmpty()) {
            delete item->takeChild(0); // Удаляем placeholder
            loadMatches(item);
        }
    });
}

void SportsTracker::loadTournaments(QTreeWidgetItem *sportItem)
{
    int sportId = sportItem->data(0, Qt::UserRole).toInt();

    QSqlQuery query;
    query.prepare("SELECT id, name FROM tournaments WHERE sport_id = ? ORDER BY name");
    query.addBindValue(sportId);

    if (!query.exec()) {
        qDebug() << "Failed to load tournaments:" << query.lastError();
        return;
    }

    while (query.next()) {
        int id = query.value(0).toInt();
        QString name = query.value(1).toString();

        QTreeWidgetItem *tournamentItem = new QTreeWidgetItem(sportItem);
        tournamentItem->setText(0, name);
        tournamentItem->setData(0, Qt::UserRole, id);
        tournamentItem->setData(0, Qt::UserRole + 1, TournamentItem);

        // Добавляем пустой элемент для отображения стрелки раскрытия
        new QTreeWidgetItem(tournamentItem);
    }
}

void SportsTracker::loadMatches(QTreeWidgetItem *tournamentItem)
{
    int tournamentId = tournamentItem->data(0, Qt::UserRole).toInt();

    QSqlQuery query;
    query.prepare("SELECT id, date, team1, team2, score FROM matches WHERE tournament_id = ? ORDER BY date DESC");
    query.addBindValue(tournamentId);

    if (!query.exec()) {
        qDebug() << "Failed to load matches:" << query.lastError();
        return;
    }

    while (query.next()) {
        int id = query.value(0).toInt();
        QString date = query.value(1).toString();
        QString team1 = query.value(2).toString();
        QString team2 = query.value(3).toString();
        QString score = query.value(4).toString();

        QString matchText = QString("%1: %2 %3 - %4 %5")
                          .arg(date)
                          .arg(team1)
                          .arg(score.split(":").first())
                          .arg(score.split(":").last())
                          .arg(team2);

        QTreeWidgetItem *matchItem = new QTreeWidgetItem(tournamentItem);
        matchItem->setText(0, matchText);
        matchItem->setData(0, Qt::UserRole, id);
        matchItem->setData(0, Qt::UserRole + 1, MatchItem);
    }
}

void SportsTracker::loadMatchStats(int matchId, const QString &matchInfo)
{
    statsTable->clear();
    matchTitle->setText(matchInfo);

    // Получаем информацию о командах
    QSqlQuery matchQuery;
    matchQuery.prepare("SELECT team1, team2 FROM matches WHERE id = ?");
    matchQuery.addBindValue(matchId);

    if (!matchQuery.exec() || !matchQuery.next()) {
        qDebug() << "Failed to load match info:" << matchQuery.lastError();
        return;
    }

    QString team1 = matchQuery.value(0).toString();
    QString team2 = matchQuery.value(1).toString();

    // Получаем статистику матча
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

    if (!statsQuery.exec()) {
        qDebug() << "Failed to load match stats:" << statsQuery.lastError();
        return;
    }

    // Настраиваем таблицу
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
