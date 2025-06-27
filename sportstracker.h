#ifndef SPORTSTRACKER_H
#define SPORTSTRACKER_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QListWidget>
#include <QLabel>
#include <QMap>
#include <QStringList>

class SportsTracker : public QMainWindow
{
    Q_OBJECT

public:
    SportsTracker(QWidget *parent = nullptr);
    void loadSportsData();

private:
    void setupSportsSelection();
    void setupResultsView();
    void showSportResults(const QString &sportName);

    struct MatchResult {
        QString date;
        QString team1;
        QString team2;
        QString score;
        QString tournament;
    };

    QStackedWidget *stackedWidget;
    QListWidget *sportsList;
    QLabel *sportTitle;
    QLabel *resultsDisplay;

    QMap<QString, QList<MatchResult>> sportResults;
};

#endif // SPORTSTRACKER_H
