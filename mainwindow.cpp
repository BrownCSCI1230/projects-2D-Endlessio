#include "mainwindow.h"
#include "settings.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QLabel>
#include <QGroupBox>
#include <QTabWidget>
#include <QScrollArea>
#include <QCheckBox>
#include <iostream>

MainWindow::MainWindow()
{
    setWindowTitle("2D Projects: Brush and Filter");

    // loads in settings from last run or uses default values
    settings.loadSettingsOrDefaults();

    QHBoxLayout *hLayout = new QHBoxLayout(); // horizontal layout for canvas and controls panel
    QVBoxLayout *vLayout = new QVBoxLayout(); // vertical layout for control panel

    vLayout->setAlignment(Qt::AlignTop);

    hLayout->addLayout(vLayout);
    setLayout(hLayout);

    setupCanvas2D();
    resize(800, 600);

    // makes the canvas into a scroll area
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidget(m_canvas);
    scrollArea->setWidgetResizable(true);
    hLayout->addWidget(scrollArea, 1);

    // groupings by project
    QWidget *brushGroup = new QWidget();
    QVBoxLayout *brushLayout = new QVBoxLayout();
    brushLayout->setAlignment(Qt::AlignTop);
    brushGroup->setLayout(brushLayout);

    QWidget *filterGroup = new QWidget();
    QVBoxLayout *filterLayout = new QVBoxLayout();
    filterLayout->setAlignment(Qt::AlignTop);
    filterGroup->setLayout(filterLayout);

    QScrollArea *controlsScroll = new QScrollArea();
    QTabWidget *tabs = new QTabWidget();
    controlsScroll->setWidget(tabs);
    controlsScroll->setWidgetResizable(true);

    tabs->addTab(brushGroup, "Brush");
    tabs->addTab(filterGroup, "Filter");

    vLayout->addWidget(controlsScroll);

    // brush selection
    addHeading(brushLayout, "Brush");
    addRadioButton(brushLayout, "Constant", settings.brushType == BRUSH_CONSTANT, [this]{ setBrushType(BRUSH_CONSTANT); });
    addRadioButton(brushLayout, "Linear", settings.brushType == BRUSH_LINEAR, [this]{ setBrushType(BRUSH_LINEAR); });
    addRadioButton(brushLayout, "Quadratic", settings.brushType == BRUSH_QUADRATIC, [this]{ setBrushType(BRUSH_QUADRATIC); });
    addRadioButton(brushLayout, "Smudge", settings.brushType == BRUSH_SMUDGE, [this]{ setBrushType(BRUSH_SMUDGE); });

    // brush parameters
    addSpinBox(brushLayout, "red", 0, 255, 1, settings.brushColor.r, [this](int value){ setUIntVal(settings.brushColor.r, value); });
    addSpinBox(brushLayout, "green", 0, 255, 1, settings.brushColor.g, [this](int value){ setUIntVal(settings.brushColor.g, value); });
    addSpinBox(brushLayout, "blue", 0, 255, 1, settings.brushColor.b, [this](int value){ setUIntVal(settings.brushColor.b, value); });
    addSpinBox(brushLayout, "alpha", 0, 255, 1, settings.brushColor.a, [this](int value){ setUIntVal(settings.brushColor.a, value); });
    addSpinBox(brushLayout, "radius", 0, 100, 1, settings.brushRadius, [this](int value){ setIntVal(settings.brushRadius, value); });

    // extra credit brushes
    addHeading(brushLayout, "Extra Credit Brushes");
    addRadioButton(brushLayout, "Spray", settings.brushType == BRUSH_SPRAY, [this]{ setBrushType(BRUSH_SPRAY); });
    addSpinBox(brushLayout, "density", 1, 100, 1, settings.brushDensity, [this](int value){ setIntVal(settings.brushDensity, value); });
    addRadioButton(brushLayout, "Speed", settings.brushType == BRUSH_SPEED, [this]{ setBrushType(BRUSH_SPEED); });
    addRadioButton(brushLayout, "Fill", settings.brushType == BRUSH_FILL, [this]{ setBrushType(BRUSH_FILL); });
    addRadioButton(brushLayout, "Custom", settings.brushType == BRUSH_CUSTOM, [this]{ setBrushType(BRUSH_CUSTOM); });
    addCheckBox(brushLayout, "Fix alpha blending", settings.fixAlphaBlending, [this](bool value){ setBoolVal(settings.fixAlphaBlending, value); });

    // clearing canvas
    addPushButton(brushLayout, "Clear canvas", &MainWindow::onClearButtonClick);

    // filters
    addHeading(filterLayout, "Filter");
    addRadioButton(filterLayout, "Edge detect", settings.filterType == FILTER_EDGE_DETECT,  [this]{ setFilterType(FILTER_EDGE_DETECT); });
    addDoubleSpinBox(filterLayout, "sensitivity", 0.01, 1, 0.01, settings.edgeDetectSensitivity, 2, [this](float value){ setFloatVal(settings.edgeDetectSensitivity, value); });

    addRadioButton(filterLayout, "Blur", settings.filterType == FILTER_BLUR, [this]{ setFilterType(FILTER_BLUR); });
    addSpinBox(filterLayout, "radius", 0, 100, 1, settings.blurRadius, [this](int value){ setIntVal(settings.blurRadius, value); });

    addRadioButton(filterLayout, "Scale", settings.filterType == FILTER_SCALE, [this]{ setFilterType(FILTER_SCALE); });
    addDoubleSpinBox(filterLayout, "x", 0.1, 10, 0.1, settings.scaleX, 2, [this](float value){ setFloatVal(settings.scaleX, value); });
    addDoubleSpinBox(filterLayout, "y", 0.1, 10, 0.1, settings.scaleY, 2, [this](float value){ setFloatVal(settings.scaleY, value); });

    // extra credit filters
    addHeading(filterLayout, "Extra Credit Filters");
    addRadioButton(filterLayout, "Median", settings.filterType == FILTER_MEDIAN,  [this]{ setFilterType(FILTER_MEDIAN); });
    addSpinBox(filterLayout, "radius", 1, 100, 1, settings.medianRadius, [this](int value){ setIntVal(settings.medianRadius, value); });

    addRadioButton(filterLayout, "Chromatic aberration", settings.filterType == FILTER_CHROMATIC,  [this]{ setFilterType(FILTER_CHROMATIC); });
    addDoubleSpinBox(filterLayout, "lambda 1", 1e-7, 1e-5, 1e-7, settings.lambda_1, 8, [this](float value){ setFloatVal(settings.lambda_1, value); });
    addDoubleSpinBox(filterLayout, "lambda 2", 1e-7, 1e-5, 1e-7, settings.lambda_2, 8, [this](float value){ setFloatVal(settings.lambda_2, value); });
    addDoubleSpinBox(filterLayout, "lambda 3", 1e-7, 1e-5, 1e-7, settings.lambda_3, 8, [this](float value){ setFloatVal(settings.lambda_3, value); });

    addRadioButton(filterLayout, "Tone mapping", settings.filterType == FILTER_MAPPING,  [this]{ setFilterType(FILTER_MAPPING); });
    addCheckBox(filterLayout, "Non linear function", settings.nonLinearMap, [this](bool value){ setBoolVal(settings.nonLinearMap, value); });
    addDoubleSpinBox(filterLayout, "gamma", 0.1, 2, 0.1, settings.gamma, 2, [this](float value){ setFloatVal(settings.gamma, value); });

    addRadioButton(filterLayout, "Rotation", settings.filterType == FILTER_ROTATION,  [this]{ setFilterType(FILTER_ROTATION); });
    addDoubleSpinBox(filterLayout, "angle", -360, 360, 0.1, settings.rotationAngle, 2, [this](float value){ setFloatVal(settings.rotationAngle, value); });

    addRadioButton(filterLayout, "Bilteral smooth", settings.filterType == FILTER_BILATERAL,  [this]{ setFilterType(FILTER_BILATERAL); });
    addSpinBox(filterLayout, "radius", 1, 100, 1, settings.bilateralRadius, [this](int value){ setIntVal(settings.bilateralRadius, value); });

    // filter push buttons
    addPushButton(filterLayout, "Load Image", &MainWindow::onUploadButtonClick);
    addPushButton(filterLayout, "Apply Filter", &MainWindow::onFilterButtonClick);
    addPushButton(filterLayout, "Revert Image", &MainWindow::onRevertButtonClick);
}

/**
 * @brief Sets up Canvas2D
 */
void MainWindow::setupCanvas2D() {
    m_canvas = new Canvas2D();
    m_canvas->init();

    if (!settings.imagePath.isEmpty()) {
        m_canvas->loadImageFromFile(settings.imagePath);
    }
}


// ------ FUNCTIONS FOR ADDING UI COMPONENTS ------

void MainWindow::addHeading(QBoxLayout *layout, QString text) {
    QFont font;
    font.setPointSize(16);
    font.setBold(true);

    QLabel *label = new QLabel(text);
    label->setFont(font);
    layout->addWidget(label);
}

void MainWindow::addLabel(QBoxLayout *layout, QString text) {
    layout->addWidget(new QLabel(text));
}

void MainWindow::addRadioButton(QBoxLayout *layout, QString text, bool value, auto function) {
    QRadioButton *button = new QRadioButton(text);
    button->setChecked(value);
    layout->addWidget(button);
    connect(button, &QRadioButton::clicked, this, function);
}

void MainWindow::addSpinBox(QBoxLayout *layout, QString text, int min, int max, int step, int val, auto function) {
    QSpinBox *box = new QSpinBox();
    box->setMinimum(min);
    box->setMaximum(max);
    box->setSingleStep(step);
    box->setValue(val);
    QHBoxLayout *subLayout = new QHBoxLayout();
    addLabel(subLayout, text);
    subLayout->addWidget(box);
    layout->addLayout(subLayout);
    connect(box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, function);
}

void MainWindow::addDoubleSpinBox(QBoxLayout *layout, QString text, double min, double max, double step, double val, int decimal, auto function) {
    QDoubleSpinBox *box = new QDoubleSpinBox();
    box->setMinimum(min);
    box->setMaximum(max);
    box->setSingleStep(step);
    box->setValue(val);
    box->setDecimals(decimal);
    QHBoxLayout *subLayout = new QHBoxLayout();
    addLabel(subLayout, text);
    subLayout->addWidget(box);
    layout->addLayout(subLayout);
    connect(box, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, function);
}

void MainWindow::addPushButton(QBoxLayout *layout, QString text, auto function) {
    QPushButton *button = new QPushButton(text);
    layout->addWidget(button);
    connect(button, &QPushButton::clicked, this, function);
}

void MainWindow::addCheckBox(QBoxLayout *layout, QString text, bool val, auto function) {
    QCheckBox *box = new QCheckBox(text);
    box->setChecked(val);
    layout->addWidget(box);
    connect(box, &QCheckBox::clicked, this, function);
}



// ------ FUNCTIONS FOR UPDATING SETTINGS ------

void MainWindow::setBrushType(int type) {
    settings.brushType = type;
    m_canvas->settingsChanged();
}

void MainWindow::setFilterType(int type) {
    settings.filterType = type;
    m_canvas->settingsChanged();
}

void MainWindow::setUIntVal(std::uint8_t &setValue, int newValue) {
    setValue = newValue;
    m_canvas->settingsChanged();
}

void MainWindow::setIntVal(int &setValue, int newValue) {
    setValue = newValue;
    m_canvas->settingsChanged();
}

void MainWindow::setFloatVal(float &setValue, float newValue) {
    setValue = newValue;
    m_canvas->settingsChanged();
}

void MainWindow::setBoolVal(bool &setValue, bool newValue) {
    setValue = newValue;
    m_canvas->settingsChanged();
}


// ------ PUSH BUTTON FUNCTIONS ------

void MainWindow::onClearButtonClick() {
    m_canvas->resize(m_canvas->parentWidget()->size().width(), m_canvas->parentWidget()->size().height());
    m_canvas->clearCanvas();
}

void MainWindow::onFilterButtonClick() {
    m_canvas->filterImage();
}

void MainWindow::onRevertButtonClick() {
    m_canvas->loadImageFromFile(settings.imagePath);
}

void MainWindow::onUploadButtonClick() {
    // Get new image path selected by user
    QString file = QFileDialog::getOpenFileName(this, tr("Open Image"), QDir::homePath(), tr("Image Files (*.png *.jpg *.jpeg)"));
    if (file.isEmpty()) { return; }
    settings.imagePath = file;

    // Display new image
    m_canvas->loadImageFromFile(settings.imagePath);

    m_canvas->settingsChanged();
}
