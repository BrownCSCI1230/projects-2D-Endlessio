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
 * @brief Get Canvas2D's image data and display this to the GUI
 */
void Canvas2D::myDisplayImage(vector<RGBA> &data, int width, int height) {
    QByteArray* img = new QByteArray(reinterpret_cast<const char*>(data.data()), 4*data.size());
    QImage now = QImage((const uchar*)img->data(), width, height, QImage::Format_RGBX8888);
    setPixmap(QPixmap::fromImage(now));
    setFixedSize(width, height);
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
//    displayImage();
}


/**
 * @brief Called when the filter button is pressed in the UI
 */
void Canvas2D::filterImage() {
    // Filter TODO: apply the currently selected filter to the loaded image
    switch (settings.filterType) {
      case FILTER_BLUR: {
        std::vector<float> filter = createBlurFilter();
        int filter_width = settings.blurRadius*2+1;
        int filter_height = 1;
        auto first_pass = convolve2D(m_data, filter, filter_width, filter_height, false);
        auto second_pass = convolve2D(first_pass, filter, filter_height, filter_width, false);
        updateCanvas(second_pass);
        displayImage();
        break;
      }
      case FILTER_EDGE_DETECT: {
        filterGray(m_data);
        std::vector<float> sobel_x_row = {-1, 0, 1};
        std::vector<float> sobel_x_col = {1, 2, 1};
        auto first_pass_x = convolve2D(m_data, sobel_x_row, 3, 1, true);
        auto second_pass_x = convolve2D(first_pass_x, sobel_x_col, 1, 3, true);
        std::vector<float> sobel_y_row = {1, 2, 1};
        std::vector<float> sobel_y_col = {1, 0, -1};
        auto first_pass_y = convolve2D(m_data, sobel_y_row, 3, 1, true);
        auto second_pass_y = convolve2D(first_pass_y, sobel_y_col, 1, 3, true);

        auto result = getEdgeMagnitude(second_pass_x, second_pass_y);
        updateCanvas(result);
        displayImage();
        break;
      }
      case FILTER_SCALE: {
        int output_width = round(m_width*settings.scaleX);
        int output_height = round(m_height*settings.scaleY);
        auto scaledX = getScaledImageX(m_data, m_width, m_height, settings.scaleX, output_width, m_height);
        auto scaledY = getScaledImageY(scaledX, output_width, m_height, settings.scaleY, output_width, output_height);
        resize(output_width, output_height);
        updateCanvas(scaledY);
        displayImage();
        break;
      }
      case FILTER_MEDIAN: {
        vector<RGBA> res = convolve2D_medium(m_data, settings.medianRadius);
        updateCanvas(res);
        displayImage();
        break;
      }
      case FILTER_BILATERAL: {
        double sigma_s = 3.0;
        double sigma_r = 0.1;
        vector<RGBA> res = convolve2D_bilateral(m_data, settings.bilateralRadius, sigma_s, sigma_r);
        updateCanvas(res);
        displayImage();
        break;
      }
      default:{
        cout << "not implemented" << endl;
      }
    }
}

uint8_t _get_medium_color(priority_queue <int>& heap){
    int size = (heap.size()-1)/2;
    while (heap.size()>size) {
        heap.pop();
    }
    return heap.top();
}

RGBA Canvas2D::getMedium(size_t centerIndex, int row, int col, int radius) {
    RGBA medium_color;
    priority_queue <int> red_heap;
    priority_queue <int> green_heap;
    priority_queue <int> blue_heap;
    for (int r = -radius; r<=radius; r++) {
        for (int c = -radius; c<=radius; c++) {
            auto n_r = row+r;
            auto n_c = col+r;
            RGBA canvas_color;
            if (n_r>=0 && n_r<m_height && n_c>=0 && n_c<m_width) {
                canvas_color = m_data[n_r*m_width + n_c];
            } else {
                continue;
            }
            red_heap.push(canvas_color.r);
            green_heap.push(canvas_color.g);
            blue_heap.push(canvas_color.b);
        }
    }
    medium_color = {_get_medium_color(red_heap), _get_medium_color(green_heap), _get_medium_color(blue_heap), 255};
    return medium_color;
}

vector<RGBA> Canvas2D::convolve2D_medium(std::vector<RGBA> &data, int radius) {
    std::vector<RGBA> result(data.size());
    for (int r = 0; r < m_height; r++) {
        for (int c = 0; c < m_width; c++) {
            size_t centerIndex = r * m_width + c;
            RGBA medium_color = getMedium(centerIndex, r, c, radius);
            result[centerIndex] = medium_color;
         }
    }
    return result;
}


inline std::uint8_t floatToUint8(float x) {
    return round(x * 255.f);
}


float triangle(float x, float a) {
    float r = a < 1 ? 1.0 / a : 1.0;
    if ((x < -r) || (x > r)) {
        return 0.0;
    } else {
        return (1.0 - fabs(x) / r) / r;
    }
}

std::vector<RGBA> Canvas2D::getScaledImageY(std::vector<RGBA> &data, int input_width, int input_height, float scaleY, int output_width, int output_height) {
    std::vector<RGBA> result(output_width*output_height);
    float supportY = (scaleY > 1.0) ? 1.0 : 1.0 / scaleY;
    float center;
    int cur_idx;
    for (int row = 0; row < output_height; row ++ ){
        for (int col = 0; col < output_width; col ++ ){
            float weights_sum = 0.0;
            float center = row / scaleY + (1 - scaleY) / (2 * scaleY);
            int left = ceil(center - supportY);
            int right = floor(center + supportY);
            float acc_r = 0.0;
            float acc_g = 0.0;
            float acc_b = 0.0;
            for (int idx = left; idx <= right; idx ++) {
                if (idx>=0 && idx<input_height) {
                    cur_idx = idx*input_width+col;
                    RGBA cur_color = data[cur_idx];
                    weights_sum += triangle(idx - center, scaleY);
                    acc_r += triangle(idx - center, scaleY) * (cur_color.r / 255.0);
                    acc_b += triangle(idx - center, scaleY) * (cur_color.b / 255.0);
                    acc_g += triangle(idx - center, scaleY) * (cur_color.g / 255.0);
                }
            }
            auto n_r = floatToUint8(acc_r/weights_sum);
            auto n_g = floatToUint8(acc_g/weights_sum);
            auto n_b = floatToUint8(acc_b/weights_sum);
            result[row * output_width + col] = RGBA{n_r, n_g, n_b, 255};
        }
    }
    return result;
}

std::vector<RGBA> Canvas2D::getScaledImageX(std::vector<RGBA> &data, int input_width, int input_height, float scaleX, int output_width, int output_height) {
    std::vector<RGBA> result(output_width*output_height);
    float supportX = (scaleX > 1.0) ? 1.0 : 1.0 / scaleX;
    float center;
    int cur_idx;
    for (int row = 0; row < input_height; row ++ ){
        for (int col = 0; col < output_width; col ++ ){
            float weights_sum = 0.0;
            float center = col / scaleX + (1 - scaleX) / (2 * scaleX);
            int left = ceil(center - supportX);
            int right = floor(center + supportX);
            float acc_r = 0.0;
            float acc_g = 0.0;
            float acc_b = 0.0;
            for (int idx = left; idx <= right; idx ++) {
                if (idx>=0 && idx<input_width) {
                    cur_idx = row*input_width+idx;
                    RGBA cur_color = data[cur_idx];
                    weights_sum += triangle(idx - center, scaleX);
                    acc_r += triangle(idx - center, scaleX) * (cur_color.r / 255.0);
                    acc_b += triangle(idx - center, scaleX) * (cur_color.b / 255.0);
                    acc_g += triangle(idx - center, scaleX) * (cur_color.g / 255.0);
                }
            }
            auto n_r = floatToUint8(acc_r/weights_sum);
            auto n_g = floatToUint8(acc_g/weights_sum);
            auto n_b = floatToUint8(acc_b/weights_sum);
            result[row * output_width + col] = RGBA{n_r, n_g, n_b, 255};
        }
    }
    return result;
}


std::uint8_t rgbaToGray(const RGBA &pixel) {
    std::uint8_t intensity = 0.299*pixel.r + 0.587*pixel.g + 0.114*pixel.b;
    return intensity;
}

void Canvas2D::filterGray(std::vector<RGBA> &data) {
    for (int i = 0; i<m_data.size(); i++) {
        RGBA &currentPixel = m_data[i];

        std::uint8_t gray_pixel = rgbaToGray(currentPixel);
        currentPixel.r = gray_pixel;
        currentPixel.g = gray_pixel;
        currentPixel.b = gray_pixel;
    }
}

vector<RGBA> Canvas2D::getEdgeMagnitude(std::vector<RGBA> &x, std::vector<RGBA> &y) {
    float s = settings.edgeDetectSensitivity;
    std::vector<RGBA> result(m_data.size());
    for (int i = 0; i < m_data.size(); i++) {
        float mag = s * sqrt(pow(x[i].r, 2) + pow(y[i].r, 2));
        result[i].r = mag;
        result[i].g = mag;
        result[i].b = mag;
    }
    return result;
}


void Canvas2D::updateCanvas(std::vector<RGBA> &target) {
    for (int i = 0; i<m_data.size(); i++) {
        m_data[i] = target[i];
    }
}




std::vector<float> Canvas2D::createBlurFilter() {
    int size = settings.blurRadius*2 + 1;
    std::vector<float> filter(size);
    float sigma = settings.blurRadius / 3.0;
    float norm = 0.0;
    for(int i = 0; i < size; i++) {
        float x = i - settings.blurRadius;
        filter[i] = (1/(sqrt(2*M_PI*pow(sigma, 2)))) * (exp(-(pow(x,2) / (2 * pow(sigma, 2)))));
    }

    return filter;
}

RGBA Canvas2D::getPixelReflected(std::vector<RGBA> &data, int width, int height, int x, int y) {
    int newX;
    int newY;
    if (x<0) {
        newX = -x;
    } else if (x>=width) {
        newX = (width-1)-(x % width);
    } else {
        newX = x;
    }

    if (y<0) {
        newY = -y;
    } else if (y>=height) {
        newY = (height-1)-(x % height);
    } else {
        newY = y;
    }
    return data[width * newY + newX];
}

std::vector<RGBA> Canvas2D::convolve2D(std::vector<RGBA> &data, std::vector<float> &filter, int filter_width, int filter_height, bool edge_flag) {
    std::vector<RGBA> result(data.size());

    for (int r = 0; r < m_height; r++) {
        for (int c = 0; c < m_width; c++) {
            size_t centerIndex = r * m_width + c;

            float redAcc = 0;
            float greenAcc = 0;
            float blueAcc = 0;
            float weights_sum = 0.0;

            for (int row = filter_height-1; row>=0; row--) {
                for (int col = filter_width-1; col>=0; col--) {
                    RGBA canvas_color;
                    int shift_row = filter_height/2-row;
                    int shift_col = filter_width/2-col;

                    int canvas_row = r+shift_row;
                    int canvas_col = c+shift_col;

                    if (canvas_row>=0 && canvas_row<m_height && canvas_col>=0 && canvas_col<m_width) {
                        canvas_color = data[canvas_row*m_width + canvas_col];
                    } else {
                        canvas_color = getPixelReflected(data, m_width, m_height, canvas_col, canvas_row);
                    }

                    int filter_index = row*filter_width+col;

                    weights_sum += filter[filter_index];

                    redAcc += filter[filter_index] * canvas_color.r;
                    greenAcc += filter[filter_index] * canvas_color.g;
                    blueAcc += filter[filter_index] * canvas_color.b;
                }
            }
            if (edge_flag) {
                redAcc = abs(redAcc) > 255 ? 255 : abs(redAcc);
                greenAcc = abs(greenAcc) > 255 ? 255 : abs(greenAcc);
                blueAcc = abs(blueAcc) > 255 ? 255 : abs(blueAcc);
                result[centerIndex] = RGBA{floatToUint8(redAcc/255), floatToUint8(greenAcc/255), floatToUint8(blueAcc/255), 255};
            } else {
                result[centerIndex] = RGBA{floatToUint8(redAcc/255/weights_sum), floatToUint8(greenAcc/255/weights_sum), floatToUint8(blueAcc/255/weights_sum), 255};
            }
         }
    }
    return result;
}

float _gaussian(float x, double sigma) {
    return (1/(sqrt(2*M_PI*pow(sigma, 2)))) * (exp(-(pow(x,2) / (2 * pow(sigma, 2)))));
}

float _distance(int r, int c, int n_r, int n_c) {
    return sqrt(pow(r - n_r, 2) + pow(c - n_c, 2));
}

void Canvas2D::apply_bilateral(vector<RGBA> &m_data, vector<RGBA> &result, int row, int col, double sigma_s, double sigma_r, int radius) {
    float acc_red = 0;
    float acc_green = 0;
    float acc_blue = 0;
    float Wp_r = 0;
    float Wp_g = 0;
    float Wp_b = 0;

    int origin_idx = row*m_width+col;

    for (int r = -radius; r<=radius; r++) {
        for (int c = -radius; c<=radius; c++) {
            int n_r = row+r;
            int n_c = col+c;

            if (!(n_r>=0 && n_r<m_height && n_c>=0 && n_c<m_width)) {
                continue;
            }
            int cur_idx = n_r*m_width+n_c;

            auto space_gaussian = _gaussian(_distance(row, col, n_r, n_c), sigma_s);
            auto range_gaussian_r = _gaussian(m_data[origin_idx].r/255.0-m_data[cur_idx].r/255.0, sigma_r);
            auto range_gaussian_g = _gaussian(m_data[origin_idx].g/255.0-m_data[cur_idx].g/255.0, sigma_r);
            auto range_gaussian_b = _gaussian(m_data[origin_idx].b/255.0-m_data[cur_idx].b/255.0, sigma_r);

            acc_red += (m_data[cur_idx].r/255.0)*(space_gaussian*range_gaussian_r);
            acc_blue += (m_data[cur_idx].b/255.0)*(space_gaussian*range_gaussian_b);
            acc_green += (m_data[cur_idx].g/255.0)*(space_gaussian*range_gaussian_g);

            Wp_r += space_gaussian*range_gaussian_r;
            Wp_g += space_gaussian*range_gaussian_g;
            Wp_b += space_gaussian*range_gaussian_b;
        }
    }

    acc_red /= Wp_r;
    acc_green /= Wp_g;
    acc_blue /= Wp_b;
    result[origin_idx] = {float2int(acc_red), float2int(acc_green), float2int(acc_blue), 255};
}

std::vector<RGBA> Canvas2D::convolve2D_bilateral(std::vector<RGBA> &data, int radius, double sigma_s, double sigma_r) {
    std::vector<RGBA> result(data.size());

    for (int r = 0; r < m_height; r++) {
        for (int c = 0; c < m_width; c++) {
            apply_bilateral(m_data, result, r, c, sigma_s, sigma_r, radius);
         }
    }
    return result;
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

