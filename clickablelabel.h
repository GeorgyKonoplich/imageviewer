#ifndef CLICKABLELABEL
#define CLICKABLELABEL

#include <QLabel>
#include <QMouseEvent>
#include <QDebug>
#include <QRubberBand>

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
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
public:
    int x, y;
    QMouseEvent* ev;
    QPoint origin;
    QRubberBand* rubberBand;
};

#endif // CLICKABLELABEL

