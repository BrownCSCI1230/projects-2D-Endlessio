#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSlider>
#include <QSpinBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QBoxLayout>

#include "canvas2d.h"

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow();

private:
    void setupCanvas2D();
    Canvas2D *m_canvas;

    void addHeading(QBoxLayout *layout, QString text);
    void addLabel(QBoxLayout *layout, QString text);
    void addRadioButton(QBoxLayout *layout, QString text, bool value, auto function);
    void addSpinBox(QBoxLayout *layout, QString text, int min, int max, int step, int val, auto function);
    void addDoubleSpinBox(QBoxLayout *layout, QString text, double min, double max, double step, double val, int decimal, auto function);
    void addPushButton(QBoxLayout *layout, QString text, auto function);
    void addCheckBox(QBoxLayout *layout, QString text, bool value, auto function);

private slots:
    void setBrushType(int type);
    void setFilterType(int type);

    void setUIntVal(std::uint8_t &setValue, int newValue);
    void setIntVal(int &setValue, int newValue);
    void setFloatVal(float &setValue, float newValue);
    void setBoolVal(bool &setValue, bool newValue);

    void onClearButtonClick();
    void onFilterButtonClick();
    void onRevertButtonClick();
    void onUploadButtonClick();
};
#endif // MAINWINDOW_H
