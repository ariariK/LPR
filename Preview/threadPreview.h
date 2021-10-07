#ifndef THREADPREVIEW_H
#define THREADPREVIEW_H

#include <QThread>
#include <QtDebug>


class ThreadPreview : public QThread
{
    Q_OBJECT

public:
    explicit ThreadPreview(QObject *parent = 0);
    ~ThreadPreview();

    void InitPreview();
    void ExitPreview();

private:
    void run();

signals:
    void Send();
};

#endif // THREADPREVIEW_H
