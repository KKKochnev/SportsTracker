#ifndef SPORTSTRACKER_H
#define SPORTSTRACKER_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QListWidget>
#include <QTableWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QSqlDatabase>
#include <QLabel>
#include <QTabWidget>

class SportsTracker : public QMainWindow
{
    Q_OBJECT

public:
    explicit SportsTracker(QWidget *parent = nullptr);
    ~SportsTracker();

private slots:
    void showTournamentPage(int tournamentId, const QString &tournamentName);
    void onTournamentClicked(QTreeWidgetItem *item, int column);

private:
    void setupUI();
    bool initializeDatabase();
    void loadSports();
    void loadTournaments(QTreeWidgetItem *sportItem);
    void loadMatchesAndStandings(int tournamentId);
    void showMatchStats(QListWidgetItem *item);
    void showMatchesList();
    void loadMatchStats(int matchId, const QString& team1, const QString& team2);
    void loadScorers(int matchId, const QString& team1, const QString& team2);
    void loadRecentMatches(const QString& team, const QDate& beforeDate, QTableWidget* table);
    void loadHeadToHeadMatches(const QString& team1, const QString& team2, const QDate& beforeDate, QTableWidget* table);

    QStackedWidget *stackedWidget;
    QTreeWidget *sportsTree;
    QListWidget *matchesList;
    QTableWidget *standingsTable;
    QTableWidget *statsTable;
    QTableWidget *scorersTable;
    QTableWidget *team1RecentMatches;
    QTableWidget *team2RecentMatches;
    QTableWidget *headToHeadMatches;
    QLabel *matchTitle;
    QPushButton *backButton1;
    QPushButton *backButton2;
    QStackedWidget *leftPanelStack;
    QTabWidget *statsTabs;
    int currentTournamentId;
    QString currentTournamentName;
    QSqlDatabase db;
};

#endif // SPORTSTRACKER_H
