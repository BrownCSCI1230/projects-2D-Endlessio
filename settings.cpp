/*!

 Settings.h
 CS123 Support Code

 @author  Evan Wallace (edwallac)
 @date    9/1/2010

 This file contains various settings and enumerations that you will need to
 use in the various assignments. The settings are bound to the GUI via static
 data bindings.

**/

#include "settings.h"
#include <QSettings>

Settings settings;

/**
 * @brief Loads the application settings
 */
void Settings::loadSettingsOrDefaults() {
    // Load image settings
    QSettings s("CS123", "CS123");

    brushType = s.value("brushType", BRUSH_LINEAR).toInt();
    brushRadius = s.value("brushRadius", 10).toInt();
    brushColor.r = s.value("brushRed", 0).toInt();
    brushColor.g = s.value("brushGreen", 0).toInt();
    brushColor.b = s.value("brushBlue", 0).toInt();
    brushColor.a = s.value("brushAlpha", 255).toInt();
    brushDensity = s.value("brushDensity", 5).toInt();
    fixAlphaBlending = s.value("fixAlphaBlending", false).toBool();

    filterType = s.value("filterType", FILTER_EDGE_DETECT).toInt();
    edgeDetectSensitivity = s.value("edgeDetectSensitivity", 0.5f).toDouble();
    blurRadius = s.value("blurRadius", 10).toInt();
    scaleX = s.value("scaleX", 2).toDouble();
    scaleY = s.value("scaleY", 2).toDouble();
    medianRadius = s.value("medianRadius", 1).toInt();
    rotationAngle = s.value("rotationAngle", 90.0).toFloat();
    bilateralRadius = s.value("bilateral radius", 1).toInt();
    lambda_1 = s.value("lambda 1", 1e-7).toFloat();
    lambda_2 = s.value("lambda 1", 5e-7).toFloat();
    lambda_3 = s.value("lambda 1", 1e-6).toFloat();
    nonLinearMap = s.value("nonLinearMap", false).toBool();
    gamma = s.value("gamma", 0.1).toFloat();

    imagePath = s.value("imagePath", "").toString();
}

/**
 * @brief Saves settings from this session to be loaded
 * in for next session.
 */
void Settings::saveSettings() {
    QSettings s("CS123", "CS123");

    s.setValue("brushType", brushType);
    s.setValue("brushRadius", brushRadius);
    s.setValue("brushRed", brushColor.r);
    s.setValue("brushGreen", brushColor.g);
    s.setValue("brushBlue", brushColor.b);
    s.setValue("brushAlpha", brushColor.a);
    s.setValue("brushDensity", brushDensity);
    s.setValue("fixAlphaBlending", fixAlphaBlending);

    s.setValue("filterType", filterType);
    s.setValue("edgeDetectSensitivity", edgeDetectSensitivity);
    s.setValue("blurRadius", blurRadius);
    s.setValue("scaleX", scaleX);
    s.setValue("scaleY", scaleY);
    s.setValue("medianRadius", medianRadius);
    s.setValue("rotationAngle", rotationAngle);
    s.setValue("bilateralRadius", bilateralRadius);
    s.setValue("lambda 1", lambda_1);
    s.setValue("lambda 2", lambda_2);
    s.setValue("lambda 3", lambda_3);
    s.setValue("nonLinearMap", nonLinearMap);
    s.setValue("gamma", gamma);

    s.setValue("imagePath", imagePath);
}
