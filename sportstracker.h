#ifndef SPORTSTRACKER_H
#define SPORTSTRACKER_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QLabel>
#include <QTableWidget>
#include <QSqlDatabase>
#include <QSqlQuery>

class SportsTracker : public QMainWindow
{
    Q_OBJECT

public:
    explicit SportsTracker(QWidget *parent = nullptr);
    ~SportsTracker();

private:
    void setupUI();
    bool initializeDatabase();
    void loadSports();
    void loadTournaments(QTreeWidgetItem *sportItem);
    void loadMatches(QTreeWidgetItem *tournamentItem);
    void loadMatchStats(int matchId, const QString &matchInfo);

    enum ItemType { SportItem = 1, TournamentItem = 2, MatchItem = 3 };

    QTreeWidget *sportsTree;
    QTableWidget *statsTable;
    QLabel *matchTitle;

    QSqlDatabase db;
};

#endif // SPORTSTRACKER_H
