#include "gradient.h"

#include <QPainter>
#include <QColorDialog>
#include <QMouseEvent>

Gradient::Gradient(QWidget *parent) : QWidget(parent),_gradient{{0.0, Qt::white}, {1.0, Qt::black}}
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    connect(this, &Gradient::gradientChanged, this, QOverload<>::of(&Gradient::update));
}

Gradient::Gradient(const QVector<StopColor> gradient, QWidget *parent)
    : QWidget(parent), _gradient(gradient)
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    connect(this, &Gradient::gradientChanged, this, QOverload<>::of(&Gradient::update));
}

void Gradient::setGradient(QVector<StopColor> gradient)
{
    constrainGradient(gradient);
    sortGradient(gradient);
    if(gradient == _gradient)
        return;

    _gradient = gradient;
    emit gradientChanged(_gradient);
}

void Gradient::addStop(const StopColor &stopColor)
{
    Q_ASSERT(stopColor.stop >= 0 && stopColor.stop <= 1);
    int index{0};
    for( const auto &stop : qAsConst(_gradient)){
        if(index < stop.stop)
            index++;
        else
            break;
    }
    _gradient.insert(index, stopColor);
    emit gradientChanged(_gradient);
}

void Gradient::removeStopAtPosition(int position)
{
    if(position >= 0 && position < _gradient.size()){
        _gradient.removeAt(position);
        gradientChanged(_gradient);
    }
}

void Gradient::setColorAtPosition(int position, QColor color)
{
    if(position >= 0 && position < _gradient.size()){
        _gradient[position].color = color;
    }
}

void Gradient::chooseColorAtPosition(int position, QColor color)
{
    if(!color.isValid()){
        QColorDialog *dlg = new QColorDialog();

        if(dlg->exec())
            color = dlg->selectedColor();
        else
            return;
    }
    setColorAtPosition(position, color);
}

void Gradient::constrainGradient(QVector<StopColor> &gradient)
{
    // Ensure values within valid range.
    for (StopColor &stopColor : gradient)
        stopColor.stop = std::max(0.0, std::min(1.0, stopColor.stop));
}

void Gradient::sortGradient(QVector<StopColor> &gradient)
{
    std::sort(gradient.begin(), gradient.end(), [](const StopColor &a, const StopColor &b)->bool {return  a.stop < b.stop;});
}

StopColor *Gradient::findStopHandleForEvent(QMouseEvent *e, QVector<StopColor> toExclude)
{
    int midPoint = height() / 2;

    if(e->y() >= midPoint - _handle_h && e->y() <= midPoint + _handle_h){
        for(StopColor &stopPoint : (_gradient)){
            if(toExclude.contains(stopPoint)){
                continue;
            }
            if(e->x() >= stopPoint.stop * width() - _handle_w && e->x() <= stopPoint.stop * width() + _handle_w)
                return &stopPoint;
        }
    }
    return nullptr;
}

void Gradient::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);
    QPainter painter(this);

    QLinearGradient gradient = QLinearGradient(0,0, width(), 0);

    for(const auto &stopPoint : qAsConst(_gradient)) {
        gradient.setColorAt(stopPoint.stop, stopPoint.color);
    }

    painter.fillRect(rect(),gradient);

    int y = painter.device()->height() / 2;

    //Draw the stop handles
    for (const auto &stopPoints : qAsConst(_gradient)){
        painter.setPen(stopPoints.color);
        painter.drawLine(stopPoints.stop * width(), y - _handle_h, stopPoints.stop * width(), y + _handle_h);

        painter.setPen(Qt::red);

        const QRect rect {
            static_cast<int>(stopPoints.stop * width() - _handle_w/2),
                    y - _handle_h/2,
                    _handle_w,
                    _handle_h
        };
        painter.drawRect(rect);
    }
    painter.end();
}

QSize Gradient::sizeHint() const
{
    return QSize(200,50);
}

void Gradient::mousePressEvent(QMouseEvent *e)
{
    auto stopColor = findStopHandleForEvent(e);
    if(stopColor){
        if(e->button() == Qt::RightButton){
            chooseColorAtPosition(stopColor->stop, stopColor->color);
        } else {
            dragStopColor = stopColor;
        }
    }
}

void Gradient::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
    if(dragStopColor != nullptr) {
        dragStopColor = nullptr;
        sortGradient(_gradient);
    }
}

void Gradient::mouseMoveEvent(QMouseEvent *e)
{
    if(dragStopColor){
        dragStopColor->stop = e->x() / static_cast<double>(width());
        constrainGradient(_gradient);
        emit update();
    }
}

void Gradient::mouseDoubleClickEvent(QMouseEvent *e)
{
    auto stopColor = findStopHandleForEvent(e);
    if(stopColor){
        _gradient.removeOne(*stopColor);
    } else  {
        addStop({(e->x()/static_cast<double>(width())), QColor()});
    }
}
