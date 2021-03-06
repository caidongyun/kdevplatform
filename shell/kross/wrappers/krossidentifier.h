#ifndef KROSSIDENTIFIER_H
#define KROSSIDENTIFIER_H

#include<QtCore/QVariant>

//This is file has been generated by xmltokross, you should not edit this file but the files used to generate it.

namespace KDevelop { class IndexedIdentifier; }
namespace KDevelop { class IndexedQualifiedIdentifier; }
namespace KDevelop { class Identifier; }
namespace KDevelop { class QualifiedIdentifier; }
namespace KDevelop { class IndexedTypeIdentifier; }
namespace Handlers
{
	QVariant kDevelopIndexedTypeIdentifierHandler(KDevelop::IndexedTypeIdentifier* type);
	QVariant kDevelopIndexedTypeIdentifierHandler(const KDevelop::IndexedTypeIdentifier* type);

	QVariant kDevelopQualifiedIdentifierHandler(KDevelop::QualifiedIdentifier* type);
	QVariant kDevelopQualifiedIdentifierHandler(const KDevelop::QualifiedIdentifier* type);

	QVariant kDevelopIdentifierHandler(KDevelop::Identifier* type);
	QVariant kDevelopIdentifierHandler(const KDevelop::Identifier* type);

	QVariant kDevelopIndexedQualifiedIdentifierHandler(KDevelop::IndexedQualifiedIdentifier* type);
	QVariant kDevelopIndexedQualifiedIdentifierHandler(const KDevelop::IndexedQualifiedIdentifier* type);

	QVariant kDevelopIndexedIdentifierHandler(KDevelop::IndexedIdentifier* type);
	QVariant kDevelopIndexedIdentifierHandler(const KDevelop::IndexedIdentifier* type);

}

#endif
