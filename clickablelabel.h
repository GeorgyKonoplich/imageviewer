#ifndef CLICKABLELABEL
#define CLICKABLELABEL

#include <QLabel>
#include <QMouseEvent>
#include <QDebug>

class ClickableLabel : public QLabel
{
Q_OBJECT
public:
    explicit ClickableLabel(QWidget* parent=0);
    ~ClickableLabel();
signals:
    void clicked();
protected:
    void mousePressEvent(QMouseEvent* event);
public:
    int x, y;
    QMouseEvent* ev;
};

#endif // CLICKABLELABEL

