#include "canvas2d.h"
#include <QPainter>
#include <QMessageBox>
#include <QFileDialog>
#include <iostream>
#include <cmath>
#include "settings.h"
#include <queue>
using namespace std;

/**
 * @brief Initializes new 500x500 canvas
 */
void Canvas2D::init() {
    m_width = 500;
    m_height = 500;
    clearCanvas();
    updateBrush(settings);
    prev_canvas.push_front(m_data);
}

/**
 * @brief Canvas2D::clearCanvas sets all canvas pixels to blank white
 */
void Canvas2D::clearCanvas() {
    m_data.assign(m_width * m_height, RGBA{255, 255, 255, 255});
    settings.imagePath = "";
    displayImage();
}

void Canvas2D::prevCanvas() {
    if (prev_canvas.size()>0) {
        m_data = prev_canvas.front();
        prev_canvas.pop_front();
    }
    displayImage();
}

/**
 * @brief Stores the image specified from the input file in this class's
 * `std::vector<RGBA> m_image`.
 * Also saves the image width and height to canvas width and height respectively.
 * @param file: file path to an image
 * @return True if successfully loads image, False otherwise.
 */
bool Canvas2D::loadImageFromFile(const QString &file) {
    QImage myImage;
    if (!myImage.load(file)) {
        std::cout<<"Failed to load in image"<<std::endl;
        return false;
    }
    myImage = myImage.convertToFormat(QImage::Format_RGBX8888);
    m_width = myImage.width();
    m_height = myImage.height();
    QByteArray arr = QByteArray::fromRawData((const char*) myImage.bits(), myImage.sizeInBytes());

    m_data.clear();
    m_data.reserve(m_width * m_height);
    for (int i = 0; i < arr.size() / 4.f; i++){
        m_data.push_back(RGBA{(std::uint8_t) arr[4*i], (std::uint8_t) arr[4*i+1], (std::uint8_t) arr[4*i+2], (std::uint8_t) arr[4*i+3]});
    }
    displayImage();
    return true;
}

/**
 * @brief Get Canvas2D's image data and display this to the GUI
 */
void Canvas2D::displayImage() {
    QByteArray* img = new QByteArray(reinterpret_cast<const char*>(m_data.data()), 4*m_data.size());
    QImage now = QImage((const uchar*)img->data(), m_width, m_height, QImage::Format_RGBX8888);
    setPixmap(QPixmap::fromImage(now));
    setFixedSize(m_width, m_height);
    update();
}

/**
 * @brief Canvas2D::resize resizes canvas to new width and height
 * @param w
 * @param h
 */
void Canvas2D::resize(int w, int h) {
    m_width = w;
    m_height = h;
    m_data.resize(w * h);
    displayImage();
}

/**
 * @brief Called when the filter button is pressed in the UI
 */
void Canvas2D::filterImage() {
    // Filter TODO: apply the currently selected filter to the loaded image
}

/**
 * @brief Called when any of the parameters in the UI are modified.
 */
void Canvas2D::settingsChanged() {
    // this saves your UI settings locally to load next time you run the program
    settings.saveSettings();

    // 1. update brush
    if (settings.brushType != prev_brush_type || settings.brushRadius != prev_brush_radius || settings.brushDensity != prev_density) {
        prev_brush_type = settings.brushType;
        prev_brush_radius = settings.brushRadius;
        prev_density = settings.brushDensity;
        updateBrush(settings);
    }
}

/**
 * @brief These functions are called when the mouse is clicked and dragged on the canvas
 */
void Canvas2D::mouseDown(int x, int y) {
    if (settings.brushType == BRUSH_SMUDGE) {
        formPrevColor(x, y);
    }
    if (settings.brushType == BRUSH_FILL) {
        RGBA target_color = m_data[pos2index(x, y, m_width)];
        fillBucket(x, y, target_color);
    }
    if (settings.brushType == BRUSH_COLOR_PICKER) {
        pickColor(x, y);
        return;
    }
    if (settings.brushType == BRUSH_ERASER_CONNECTED) {
        eraserConnected(x, y);
        displayImage();
        return;
    }

    drawStamp(x-settings.brushRadius, y-settings.brushRadius);
    displayImage();
}

void Canvas2D::mouseDragged(int x, int y) {
    drawStamp(x-settings.brushRadius, y-settings.brushRadius);
    if (settings.brushType == BRUSH_SMUDGE) {
        formPrevColor(x, y);
    }
    displayImage();
}

void Canvas2D::mouseUp(int x, int y) {
    if (prev_canvas.size() >= max_depth) {
        prev_canvas.pop_back();
    }
    prev_canvas.push_front(m_data);
}

//helper functions
int Canvas2D::pos2index(int x, int y, int width) {
    return y*width+x;
}

std::vector<int> Canvas2D::index2pos(int index, int width) {
    int row = index / width;
    int col = index % width;
    std::vector<int> res{ col, row };
    return res;
}


uint8_t Canvas2D::float2int(float intensity) {
    uint8_t res = round(intensity * 255);
    return res;
}

float Canvas2D::int2float(uint8_t intensity) {
    float res = intensity/255.0;
    return res;
}

auto Canvas2D::getBrushDistance(int current, int center) {
    int r = settings.brushRadius;
    std::vector<int> cur_pos = index2pos(current, 2*r+1);
    std::vector<int> center_pos = index2pos(center, 2*r+1);
    auto distance = round(sqrt(pow((cur_pos[0]-center_pos[0]), 2)+pow((cur_pos[1]-center_pos[1]), 2)));
    return distance;
}

void Canvas2D::formPrevColor(int col, int row) {
    int color_mask_size = (2*settings.brushRadius+1)*(2*settings.brushRadius+1);
    prev_color.assign(color_mask_size, RGBA{0,0,0,0});
    int r = settings.brushRadius;
    int cnt = 0;
    int start_row = row - r;
    int end_row = row + r + 1;
    int start_col = col - r;
    int end_col = col + r + 1;

    for (int i = start_row; i < end_row; i++){
        for (int j = start_col; j < end_col; j++){
            if (0<=i && i<m_height && 0<=j && j<m_width) {
                prev_color[cnt] = m_data[pos2index(j, i, m_width)];
            }
            cnt += 1;
        }
    }
}

void Canvas2D::updateBrush(Settings settings) {
    int brush_size = (2*settings.brushRadius+1)*(2*settings.brushRadius+1);
    brush.assign(brush_size, 0);

    int center = (brush_size-1)/2;
    int r = settings.brushRadius;

    switch (settings.brushType) {
      case BRUSH_CONSTANT:
        for (int i = 0; i<brush_size; i++){
            auto distance = getBrushDistance(i, center);
            if (distance <= r) {
                brush[i] = 1;
            }
        }
        break;
      case BRUSH_LINEAR:
        for (int i = 0; i<brush_size; i++){
            auto distance = getBrushDistance(i, center);
            if (distance <= r) {
                brush[i] = std::max(0.0, 1-distance/r);
            }
        }
        break;
      case BRUSH_QUADRATIC:
        // C = 1; B = -2/r; A = 1/r^2
        for (int i = 0; i<brush_size; i++){
            auto distance = getBrushDistance(i, center);
            if (distance <= r) {
                brush[i] = std::max(0.0,(1.0/(r*r))*pow(distance,2)-2.0/r*distance+1);
            }
        }
        break;
      case BRUSH_SMUDGE:
        for (int i = 0; i<brush_size; i++){
            auto distance = getBrushDistance(i, center);
            if (distance <= r) {
                brush[i] = std::max(0.0,(1.0/(r*r))*pow(distance,2)-2.0/r*distance+1);
            }
        }
        break;
      case BRUSH_SPRAY:
        for (int i = 0; i<brush_size; i++){
            auto distance = getBrushDistance(i, center);
            if (distance <= r) {
                brush[i] = 1;
            }
        }
        break;
      case BRUSH_ERASER:
        for (int i = 0; i<brush_size; i++){
            auto distance = getBrushDistance(i, center);
            if (distance <= r) {
                brush[i] = 1;
            }
        }
        break;
    case BRUSH_ERASER_CONNECTED:
      break;
      default:
        std::cout << "INVALID BRUSH TYPE";
    }

}

void Canvas2D::drawStamp(int start_col, int start_row) {
    int row = settings.brushRadius*2+1;
    int col = settings.brushRadius*2+1;
    int cnt = 0;
    uint8_t r = settings.brushColor.r;
    uint8_t g = settings.brushColor.g;
    uint8_t b = settings.brushColor.b;
    float a = int2float(settings.brushColor.a);

    for (int i=0; i<row; i++) {
        for (int j=0; j<col; j++){
            int cur_row = i+start_row;
            int cur_col = j+start_col;
            float brush_intensity = brush[cnt];
            if (0<=cur_row && cur_row<m_height && 0<=cur_col && cur_col<m_width) {
                int canvas_idx = pos2index(cur_col, cur_row, m_width);
                if (settings.brushType == BRUSH_SMUDGE) {
                    r = prev_color[cnt].r;
                    g = prev_color[cnt].g;
                    b = prev_color[cnt].b;
                    a = 1.0;
                }
                if (settings.brushType == BRUSH_SPRAY) {
                    bool hit = (rand() % 100) > settings.brushDensity/6;
                    if (hit) {
                        brush_intensity = 0;
                    }
                }
                if (settings.brushType == BRUSH_ERASER) {
                    r = init_color.r;
                    g = init_color.g;
                    b = init_color.b;
                    a = 1.0;
                }

                // change red
                m_data[canvas_idx].r = 0.5 + a * r * brush_intensity + m_data[canvas_idx].r * (1-brush_intensity*a);
                // change green
                m_data[canvas_idx].g = 0.5 + a * g * brush_intensity + m_data[canvas_idx].g * (1-brush_intensity*a);
                // change blue
                m_data[canvas_idx].b = 0.5 + a * b * brush_intensity + m_data[canvas_idx].b * (1-brush_intensity*a);
            }
            cnt += 1;
        }
    }

}


bool _check_color(RGBA color1, RGBA color2) {
    if (color1.r == color2.r && color1.g == color2.g && color1.b == color2.b && color1.a == color2.a) {
        return true;
    }
    return false;
};


void Canvas2D::fillBucket(int col, int row, RGBA target_color) {
    queue<vector<int>> q;
    vector<vector<int>> v(m_height,vector<int>(m_width,0));
    q.push({row, col});

    int dir_row[4] = {0,1,0,-1};
    int dir_col[4] = {1,0,-1,0};
    while (q.size()) {
        int cur_row = q.front()[0];
        int cur_col = q.front()[1];
        int cur_idx = pos2index(cur_col, cur_row, m_width);
        q.pop();
        RGBA cur_color = m_data[cur_idx];
        if (_check_color(cur_color, target_color)) {
            m_data[cur_idx] = settings.brushColor;
            for (int i=0;i<4;i++) {
                int nr=cur_row+dir_row[i],nc=cur_col+dir_col[i];
                if (nr>=0 && nc>=0 && nr!=m_height && nc!=m_width && v[nr][nc]==0) {
                    v[nr][nc]=1;
                    q.push({nr,nc});
                }
            }
        }
    }
}

void Canvas2D::pickColor(int col, int row) {
    settings.brushColor = m_data[pos2index(col, row, m_width)];
//    emit pickColorChanged(10);
}

void Canvas2D::eraserConnected(int col, int row) {
    queue<vector<int>> q;
    vector<vector<int>> v(m_height,vector<int>(m_width,0));
    q.push({row, col});

    int dir_row[4] = {0,1,0,-1};
    int dir_col[4] = {1,0,-1,0};
    while (q.size()) {
        int cur_row = q.front()[0];
        int cur_col = q.front()[1];
        int cur_idx = pos2index(cur_col, cur_row, m_width);
        q.pop();
        RGBA cur_color = m_data[cur_idx];
        if (!_check_color(cur_color, init_color)) {
            m_data[cur_idx] = init_color;
            for (int i=0;i<4;i++) {
                int nr=cur_row+dir_row[i],nc=cur_col+dir_col[i];
                if (nr>=0 && nc>=0 && nr!=m_height && nc!=m_width && v[nr][nc]==0) {
                    v[nr][nc]=1;
                    q.push({nr,nc});
                }
            }
        }
    }
}

