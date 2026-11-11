#include "utils.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>

QString getAssetPath(const QString &fileName)
{
    // If empty or already exists as is
    if (fileName.isEmpty() || QFileInfo::exists(fileName)) {
        return fileName;
    }

    QString appDir = QCoreApplication::applicationDirPath();

    QStringList candidates = {
        appDir + "/" + fileName,
        appDir + "/images/" + fileName,
        appDir + "/assets/" + fileName,
        appDir + "/../" + fileName,
        appDir + "/../images/" + fileName,
        appDir + "/../assets/" + fileName,
        "images/" + fileName,
        "assets/" + fileName,
        "../images/" + fileName,
        "../assets/" + fileName
    };

    for (const QString &candidate : candidates) {
        if (QFileInfo::exists(candidate)) {
            return QDir::cleanPath(candidate);
        }
    }

    return fileName;
}
