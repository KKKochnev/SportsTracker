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

    QStackedWidget *stackedWidget;
    QTreeWidget *sportsTree;
    QListWidget *matchesList;
    QTableWidget *standingsTable;
    QTableWidget *statsTable;
    QLabel *matchTitle;
    QPushButton *backButton1;
    QPushButton *backButton2;
    QWidget *matchesPanel;  // Добавлено для управления отображением
    QStackedWidget *leftPanelStack;  // Добавлено для переключения между списком матчей и статистикой
    int currentTournamentId;
    QString currentTournamentName;
    QSqlDatabase db;
};

#endif // SPORTSTRACKER_H
