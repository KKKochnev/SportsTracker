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
    explicit SportsTracker(QWidget *parent = nullptr);

private:
    struct MatchResult {
        QString date;
        QString team1;
        QString team2;
        QString score;
        QString tournament;
        QString details;
    };

    void setupUI();
    void loadSportsData();
    void showSportResults(const QString &sportName);

    QStackedWidget *stackedWidget;
    QListWidget *sportsList;
    QLabel *sportTitle;
    QLabel *resultsDisplay;

    QMap<QString, QList<MatchResult>> sportResults;
};

#endif // SPORTSTRACKER_H
