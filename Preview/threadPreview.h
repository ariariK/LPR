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
    void run(int data);

signals:
    void Send(int data);
};

#endif // THREADPREVIEW_H
