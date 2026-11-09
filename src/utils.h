#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <string>

/**
 * @brief Resolves an asset file path across running configurations:
 *        - CLion build directories
 *        - Application bundle / install directory
 *        - Source directory
 *        - Current working directory
 */
QString getAssetPath(const QString &fileName);

/**
 * @brief Standard C++ string version of getAssetPath for SDL 1.2 routines.
 */
std::string getAssetPathStd(const std::string &fileName);

#endif // UTILS_H
