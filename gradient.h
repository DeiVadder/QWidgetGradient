#ifndef GRADIENT_H
#define GRADIENT_H

#include <QWidget>
#include <QLinearGradient>

struct StopColor{
    double stop;
    QColor color;

    StopColor() : stop{0.0}, color{QColor()}{}
    StopColor(double pos, QColor col) : stop{pos}, color{col}{}
    StopColor(const StopColor &sc) : stop{sc.stop}, color{sc.color} {}
    StopColor &operator=(const StopColor &other) {
        stop = other.stop;
        color = other.color;
        return *this;
    }
    friend bool operator==(const StopColor &a, const StopColor &b) {
            return a.stop == b.stop && a.color == b.color;
        }
};

class Gradient : public QWidget
{
    Q_OBJECT
public:
    explicit Gradient(QWidget *parent = nullptr);
    explicit Gradient(const QVector<StopColor> gradient, QWidget *parent = nullptr);

    [[nodiscard]] const inline QVector<StopColor> &gradient() const noexcept {return _gradient;}

public slots:
    void setGradient(QVector<StopColor> gradient);
    void addStop(const StopColor &stopColor);
    void removeStopAtPosition(int position);
    void setColorAtPosition(int position, QColor color);
    void chooseColorAtPosition(int position, QColor color = QColor());

signals:
    void gradientChanged(const QVector<StopColor> &gradient);

private:
    void constrainGradient(QVector<StopColor> &gradient);
    void sortGradient(QVector<StopColor> &gradient);
    void updateEndStops();

    StopColor *findStopHandleForEvent(QMouseEvent *e, QVector<StopColor> toExclude = QVector<StopColor>{});

protected:
    void paintEvent(QPaintEvent *e)override;
    QSize sizeHint() const override;

    void mousePressEvent(QMouseEvent *e)override;
    void mouseReleaseEvent(QMouseEvent *e)override;
    void mouseMoveEvent(QMouseEvent *e)override;
    void mouseDoubleClickEvent(QMouseEvent *e)override;


private:
    QVector<StopColor> _gradient;
    StopColor *dragStopColor = nullptr;

    QVector<StopColor> endStops;

    int _handle_w{10};
    int _handle_h{10};
    QPoint _drag_position;
};

#endif // GRADIENT_H
