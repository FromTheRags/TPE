#ifndef UTILS_H
#define UTILS_H

#include <QString>

/**
 * @brief Resolves an asset file path across running configurations:
 *        - CLion build directories
 *        - Application bundle / install directory
 *        - Source directory
 *        - Current working directory
 */
QString getAssetPath(const QString &fileName);

#endif // UTILS_H
