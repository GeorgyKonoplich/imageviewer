#include "clickablelabel.h"

ClickableLabel::ClickableLabel(QWidget* parent)
    : QLabel(parent)
{
}

ClickableLabel::~ClickableLabel()
{
}


void ClickableLabel::mousePressEvent(QMouseEvent *event)
{
    origin = event->pos();
    if (!rubberBand)
        rubberBand = new QRubberBand(QRubberBand::Line, this);
    //rubberBand->setGeometry(QRect(origin, QSize()));
    //rubberBand->show();
    this->x = 4;
    this->y = 5;
    ev = event;
    emit clicked();
}


void ClickableLabel::mouseMoveEvent(QMouseEvent *event)
{
    //rubberBand->setGeometry(QRect(origin, event->pos()).normalized());
    //rubberBand->move(event->pos());
}

void ClickableLabel::mouseReleaseEvent(QMouseEvent *event)
{
    //rubberBand->hide();
    // determine selection, for example using QRect::intersects()
    // and QRect::contains().
}
