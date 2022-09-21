#ifndef CANVAS2D_H
#define CANVAS2D_H

#include <QLabel>
#include <QMouseEvent>
#include <array>
#include "rgba.h"
#include "settings.h"
#include <deque>

class Canvas2D : public QLabel {
    Q_OBJECT
public:
    int m_width = 0;
    int m_height = 0;
    int prev_brush_type;
    int prev_brush_radius;
    int prev_density;
    int max_depth = 5;
    RGBA init_color = RGBA{255, 255, 255, 255};
    std::deque<std::vector<RGBA>> prev_canvas;

    void init();
    void clearCanvas();
    bool loadImageFromFile(const QString &file);
    void displayImage();
    void resize(int w, int h);

    // This will be called when the settings have changed
    void settingsChanged();

    // Filter TODO: implement
    void filterImage();

    // My Fun Part Exploration
    void prevCanvas();

private:
    std::vector<RGBA> m_data;
    std::vector<float> brush;
    std::vector<RGBA> prev_color;

    void mouseDown(int x, int y);
    void mouseDragged(int x, int y);
    void mouseUp(int x, int y);

    // These are functions overriden from QWidget that we've provided
    // to prevent you from having to interact with Qt's mouse events.
    // These will pass the mouse coordinates to the above mouse functions
    // that you will have to fill in.
    virtual void mousePressEvent(QMouseEvent* event) override {
        auto [x, y] = std::array{ event->position().x(), event->position().y() };
        mouseDown(static_cast<int>(x), static_cast<int>(y));
    }
    virtual void mouseMoveEvent(QMouseEvent* event) override {
        auto [x, y] = std::array{ event->position().x(), event->position().y() };
        mouseDragged(static_cast<int>(x), static_cast<int>(y));
    }
    virtual void mouseReleaseEvent(QMouseEvent* event) override {
        auto [x, y] = std::array{ event->position().x(), event->position().y() };
        mouseUp(static_cast<int>(x), static_cast<int>(y));
    }

    // helper function
    int pos2index(int x, int y, int width);
    std::vector<int> index2pos(int index, int width);
    uint8_t float2int(float intensity);
    float int2float(uint8_t intensity);
    // basic brush-related function
    void updateBrush(Settings settings);
    auto getBrushDistance(int current, int center);
    void drawStamp(int start_col, int start_row);
    // smudge brush
    void formPrevColor(int col, int row);
    // fill bucket
    void fillBucket(int col, int row, RGBA target_color);
    // my fun exploration
    void pickColor(int col, int row);
    void eraserConnected(int col, int row);

signals:
    void pickColorChanged(int val);
};

#endif // CANVAS2D_H
