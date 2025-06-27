#ifndef SPORTSTRACKER_H
#define SPORTSTRACKER_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QListWidget>
#include <QLabel>
#include <QMap>
#include <QStringList>
#include <QTableWidget>

class SportsTracker : public QMainWindow
{
    Q_OBJECT

public:
    explicit SportsTracker(QWidget *parent = nullptr);

private:
    struct MatchResult {
        QString date;
        QString team1;
        QString team2;
        QString score;
        QString tournament;
        QString details;
        QMap<QString, QString> team1Stats;
        QMap<QString, QString> team2Stats;
    };

    void setupUI();
    void loadSportsData();
    void showSportTournaments(const QString &sportName);
    void showTournamentResults(const QString &sportName, const QString &tournamentName);
    void showMatchDetails(const MatchResult &match);

    QStackedWidget *stackedWidget;
    QListWidget *sportsList;
    QListWidget *tournamentsList;
    QListWidget *matchesList;
    QLabel *sportTitle;
    QLabel *tournamentTitle;
    QLabel *matchTitle;
    QTableWidget *statsTable;

    QMap<QString, QMap<QString, QList<MatchResult>>> sportTournamentResults;
};

#endif // SPORTSTRACKER_H
