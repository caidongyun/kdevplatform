/*  This file is part of KDevelop
    Copyright 2007 Alexander Dymo <adymo@kdevelop.org>
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "templatesmodel.h"

#include <KTar>
#include <KZip>
#include <KStandardDirs>
#include <KComponentData>
#include <KDebug>
#include <KConfigGroup>
#include <KLocale>
#include <KMimeType>

#include <QFileInfo>
#include <QDir>

using namespace KDevelop;

class KDevelop::TemplatesModelPrivate
{
public:
    TemplatesModelPrivate(const KComponentData& componentData);

    QByteArray descriptionResourceType;
    QByteArray templateResourceType;
    QByteArray previewResourceType;

    QMap<QString, QStandardItem*> templateItems;
    KComponentData componentData;

    /**
     * Extracts description files from all available template archives and saves them to a location
     * determined by descriptionResourceType().
     **/
    void extractTemplateDescriptions();

    /**
     * Creates a model item for the template @p name in category @p category
     *
     * @param name the name of the new template
     * @param category the category of the new template
     * @param parent the parent item
     * @return the created item
     **/
    QStandardItem *createItem(const QString& name, const QString& category, QStandardItem* parent);
};

TemplatesModelPrivate::TemplatesModelPrivate(const KComponentData& componentData)
: componentData(componentData)
{

}


TemplatesModel::TemplatesModel(const KComponentData& componentData, QObject* parent)
: QStandardItemModel(parent)
, d(new TemplatesModelPrivate(componentData))
{

}

TemplatesModel::~TemplatesModel()
{
    delete d;
}


void TemplatesModel::refresh()
{
    clear();
    d->templateItems.clear();
    d->templateItems[""] = invisibleRootItem();
    d->extractTemplateDescriptions();

    KStandardDirs *dirs = d->componentData.dirs();
    const QStringList templateDescriptions = dirs->findAllResources(d->descriptionResourceType);
    const QStringList templateArchives = dirs->findAllResources(d->templateResourceType);
    foreach (const QString &templateDescription, templateDescriptions)
    {
        QFileInfo fi(templateDescription);
        bool archiveFound = false;
        foreach( const QString& templateArchive, templateArchives )
        {
            if( QFileInfo(templateArchive).baseName() == fi.baseName() )
            {
                archiveFound = true;

                KConfig templateConfig(templateDescription);
                KConfigGroup general(&templateConfig, "General");
                QString name = general.readEntry("Name");
                QString category = general.readEntry("Category");
                QString comment = general.readEntry("Comment");

                QStandardItem *templateItem = d->createItem(name, category, invisibleRootItem());
                templateItem->setData(templateDescription, DescriptionFileRole);
                templateItem->setData(templateArchive, ArchiveFileRole);
                templateItem->setData(comment, CommentRole);

                if (!d->previewResourceType.isEmpty() && general.hasKey("Icon"))
                {
                    foreach (const QString& icon, dirs->findAllResources(d->previewResourceType))
                    {
                        if (QFileInfo(icon).baseName() == fi.baseName())
                        {
                            templateItem->setData(icon, IconNameRole);
                            break;
                        }
                    }
                }
            }
        }

        if (!archiveFound)
        {
            // Template file doesn't exist anymore, so remove the description
            // saves us the extra lookups for templateExists on the next run
            QFile(templateDescription).remove();
        }
    }
}

QStandardItem *TemplatesModelPrivate::createItem(const QString& name, const QString& category, QStandardItem* parent)
{
    QStringList path = category.split('/');

    QStringList currentPath;
    foreach (const QString& entry, path)
    {
        currentPath << entry;
        if (!templateItems.contains(currentPath.join("/"))) {
            QStandardItem *item = new QStandardItem(entry);
            item->setEditable(false);
            parent->appendRow(item);
            templateItems[currentPath.join("/")] = item;
            parent = item;
        } else {
            parent = templateItems[currentPath.join("/")];
        }
    }

    QStandardItem *templateItem = new QStandardItem(name);
    templateItem->setEditable(false);
    parent->appendRow(templateItem);
    return templateItem;
}

void TemplatesModelPrivate::extractTemplateDescriptions()
{
    KStandardDirs *dirs = componentData.dirs();
    QStringList templateArchives = dirs->findAllResources(templateResourceType);

    kDebug() << descriptionResourceType;
    QString localDescriptionsDir = dirs->saveLocation(descriptionResourceType);

    foreach (const QString &archName, templateArchives)
    {
        kDebug() << "processing template" << archName;

        QScopedPointer<KArchive> templateArchive;
        if (QFileInfo(archName).completeSuffix() == "zip")
        {
            templateArchive.reset(new KZip(archName));
        }
        else
        {
            templateArchive.reset(new KTar(archName));
        }

        if (templateArchive->open(QIODevice::ReadOnly))
        {
            /*
             * This class looks for template description files in the following order
             *
             * - "basename.kdevtemplate"
             * - "*.kdevtemplate"
             * - "basename.desktop"
             * - "*.desktop"
             *
             * This is done because application templates can contain .desktop files used by the application
             * so the kdevtemplate suffix must have priority.
             */
            QFileInfo templateInfo(archName);
            const KArchiveEntry *templateEntry =
                templateArchive->directory()->entry(templateInfo.baseName() + ".kdevtemplate");

            if (!templateEntry || !templateEntry->isFile())
            {
                /*
                 * First, if the .kdevtemplate file is not found by name,
                 * we check all the files in the archive for any .kdevtemplate file
                 *
                 * This is needed because kde-files.org renames downloaded files
                 */
                foreach (const QString& entryName, templateArchive->directory()->entries())
                {
                    if (entryName.endsWith(".kdevtemplate"))
                    {
                        templateEntry = templateArchive->directory()->entry(entryName);
                        break;
                    }
                }
            }

            if (!templateEntry || !templateEntry->isFile())
            {
                templateEntry = templateArchive->directory()->entry(templateInfo.baseName() + ".desktop");
            }

            if (!templateEntry || !templateEntry->isFile())
            {
                foreach (const QString& entryName, templateArchive->directory()->entries())
                {
                    if (entryName.endsWith(".desktop"))
                    {
                        templateEntry = templateArchive->directory()->entry(entryName);
                        break;
                    }
                }
            }
            if (!templateEntry || !templateEntry->isFile())
            {
                kDebug() << "template" << archName << "does not contain .kdevtemplate or .desktop file";
                continue;
            }
            const KArchiveFile *templateFile = static_cast<const KArchiveFile*>(templateEntry);

            kDebug() << "copy template description to" << localDescriptionsDir;
            templateFile->copyTo(localDescriptionsDir);

            /*
             * Rename the extracted description
             * so that its basename matches the basename of the template archive
             */
            QFileInfo descriptionInfo(localDescriptionsDir + templateEntry->name());
            QString destinationName = localDescriptionsDir + templateInfo.baseName() + '.' + descriptionInfo.suffix();
            QFile::rename(descriptionInfo.absoluteFilePath(), destinationName);

            if (!previewResourceType.isEmpty())
            {
                KConfig config(destinationName);
                KConfigGroup group(&config, "General");
                if (group.hasKey("Icon"))
                {
                    const KArchiveEntry* iconEntry = templateArchive->directory()->entry(group.readEntry("Icon"));
                    if (iconEntry && iconEntry->isFile())
                    {
                        const KArchiveFile* iconFile = static_cast<const KArchiveFile*>(iconEntry);
                        QString saveDir = dirs->saveLocation(previewResourceType);
                        iconFile->copyTo(saveDir);
                        QFileInfo iconInfo(saveDir + templateEntry->name());
                        QFile::rename(iconInfo.absoluteFilePath(), saveDir + templateInfo.baseName() + '.' + iconInfo.suffix());
                    }
                }
            }
        }
        else
        {
            kDebug() << "could not open template" << archName;
        }
    }
}

QModelIndexList TemplatesModel::templateIndexes(const QString& fileName) const
{
    QFileInfo info(fileName);
    QString description = d->componentData.dirs()->findResource(d->descriptionResourceType, info.baseName() + ".kdevtemplate");
    if (description.isEmpty())
    {
        description = d->componentData.dirs()->findResource(d->descriptionResourceType, info.baseName() + ".desktop");
    }

    QModelIndexList indexes;

    if (!description.isEmpty())
    {
        KConfig templateConfig(description);
        KConfigGroup general(&templateConfig, "General");
        QStringList categories = general.readEntry("Category").split('/');

        QStringList levels;
        foreach (const QString& category, categories)
        {
            levels << category;
            indexes << d->templateItems[levels.join(QString('/'))]->index();
        }

        if (!indexes.isEmpty())
        {
            QString name = general.readEntry("Name");
            QStandardItem* categoryItem = d->templateItems[levels.join(QString('/'))];
            for (int i = 0; i < categoryItem->rowCount(); ++i)
            {
                QStandardItem* templateItem = categoryItem->child(i);
                if (templateItem->text() == name)
                {
                    indexes << templateItem->index();
                    break;
                }
            }
        }
    }

    return indexes;
}

QByteArray TemplatesModel::descriptionResourceType() const
{
    return d->descriptionResourceType;
}

void TemplatesModel::setDescriptionResourceType(const QByteArray& type)
{
    d->descriptionResourceType = type;
}

QByteArray TemplatesModel::previewResourceType() const
{
    return d->previewResourceType;
}

void TemplatesModel::setPreviewResourceType (const QByteArray& type)
{
    d->previewResourceType = type;
}

QByteArray TemplatesModel::templateResourceType() const
{
    return d->templateResourceType;
}

void TemplatesModel::setTemplateResourceType(const QByteArray& type)
{
    d->templateResourceType = type;
}

KComponentData TemplatesModel::componentData() const
{
    return d->componentData;
}

QString TemplatesModel::loadTemplateFile(const QString& fileName)
{
    QString saveLocation = componentData().dirs()->saveLocation("apptemplates");
    QFileInfo info(fileName);
    QString destination = saveLocation + info.baseName();

    KMimeType::Ptr mimeType = KMimeType::findByUrl(KUrl(fileName));
    kDebug() << "Loaded file" << fileName << "with type" << mimeType->name();

    if (mimeType->name() == "application/x-desktop")
    {
        kDebug() << "Loaded desktop file" << info.absoluteFilePath() << ", compressing";
#ifdef Q_WS_WIN
        destination += ".zip";
        KZip archive(destination);
#else
        destination += ".tar.bz2";
        KTar archive(destination, "application/x-bzip");
#endif //Q_WS_WIN

        archive.open(QIODevice::WriteOnly);

        QDir dir(info.absoluteDir());
        QDir::Filters filter = QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot;
        foreach (const QFileInfo& entry, dir.entryInfoList(filter))
        {
            if (entry.isFile())
            {
                archive.addLocalFile(entry.absoluteFilePath(), entry.fileName());
            }
            else if (entry.isDir())
            {
                archive.addLocalDirectory(entry.absoluteFilePath(), entry.fileName());
            }
        }
        archive.close();
    }
    else
    {
        kDebug() << "Copying" << fileName << "to" << saveLocation;
        QFile::copy(fileName, destination);
    }

    refresh();

    return destination;
}
