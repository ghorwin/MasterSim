#include "QPW_VariantPropertyManager.h"

#include "qtvariantproperty.h"
#include "qteditorfactory.h"
#include "qttreepropertybrowser.h"

#include <QVector4D>
#include <QVector3D>
#include <QVector2D>

namespace QPW {

VariantPropertyManager::VariantPropertyManager(QObject *parent) :
	QtVariantPropertyManager(parent)
{
	connect(this, SIGNAL(valueChanged(QtProperty *, const QVariant &)),
				this, SLOT(slotValueChanged(QtProperty *, const QVariant &)));
	connect(this, SIGNAL(propertyDestroyed(QtProperty *)),
				this, SLOT(slotPropertyDestroyed(QtProperty *)));
}

VariantPropertyManager::~VariantPropertyManager()
{

}

void VariantPropertyManager::slotValueChanged(QtProperty *property, const QVariant &value)
{
	if (xToProperty.contains(property)) {
		QtProperty *pointProperty = xToProperty[property];
		QVariant v = this->value(pointProperty);
//		QVector3D p = qVariantValue<QVector3D>(v);
		QVector3D p = v.value<QVector3D>();
//		p.setX(qVariantValue<double>(value));
		p.setX(value.value<double>());
		setValue(pointProperty, p);
	} else if (yToProperty.contains(property)) {
		QtProperty *pointProperty = yToProperty[property];
		QVariant v = this->value(pointProperty);
//		QVector3D p = qVariantValue<QVector3D>(v);
		QVector3D p = v.value<QVector3D>();
//		p.setY(qVariantValue<double>(value));
		p.setY(value.value<double>());
		setValue(pointProperty, p);
	} else if (zToProperty.contains(property)) {
		QtProperty *pointProperty = zToProperty[property];
		QVariant v = this->value(pointProperty);
//		QVector3D p = qVariantValue<QVector3D>(v);
		QVector3D p = v.value<QVector3D>();
//		p.setZ(qVariantValue<double>(value));
		p.setZ(value.value<double>());
		setValue(pointProperty, p);
	} else if (z1ToProperty.contains(property)) {
		QtProperty *pointProperty = z1ToProperty[property];
		QVariant v = this->value(pointProperty);
//		QVector2D p = qVariantValue<QVector2D>(v);
		QVector2D p = v.value<QVector2D>();
//		p.setX(qVariantValue<double>(value));
		p.setX(value.value<double>());
		setValue(pointProperty, p);
	} else if (z2ToProperty.contains(property)) {
		QtProperty *pointProperty = z2ToProperty[property];
		QVariant v = this->value(pointProperty);
//		QVector2D p = qVariantValue<QVector2D>(v);
		QVector2D p = v.value<QVector2D>();
//		p.setY(qVariantValue<double>(value));
		p.setY(value.value<double>());
		setValue(pointProperty, p);
	} else if (x1ToProperty.contains(property)) {
		QtProperty *pointProperty = x1ToProperty[property];
		QVariant v = this->value(pointProperty);
//		QVector4D p = qVariantValue<QVector4D>(v);
		QVector4D p = v.value<QVector4D>();
//		p.setX(qVariantValue<double>(value));
		p.setX(value.value<double>());
		setValue(pointProperty, p);
	} else if (y1ToProperty.contains(property)) {
		QtProperty *pointProperty = y1ToProperty[property];
		QVariant v = this->value(pointProperty);
//		QVector4D p = qVariantValue<QVector4D>(v);
		QVector4D p = v.value<QVector4D>();
//		p.setY(qVariantValue<double>(value));
		p.setY(value.value<double>());
		setValue(pointProperty, p);
	} else if (x2ToProperty.contains(property)) {
		QtProperty *pointProperty = x2ToProperty[property];
		QVariant v = this->value(pointProperty);
//		QVector4D p = qVariantValue<QVector4D>(v);
		QVector4D p = v.value<QVector4D>();
//		p.setZ(qVariantValue<double>(value));
		p.setZ(value.value<double>());
		setValue(pointProperty, p);
	} else if (y2ToProperty.contains(property)) {
		QtProperty *pointProperty = y2ToProperty[property];
		QVariant v = this->value(pointProperty);
//		QVector4D p = qVariantValue<QVector4D>(v);
		QVector4D p = v.value<QVector4D>();
//		p.setW(qVariantValue<double>(value));
		p.setW(value.value<double>());
		setValue(pointProperty, p);
	} else if (limitToProperty.contains(property)) {
		QtProperty *boolProperty = limitToProperty[property];
		QVariant v = this->value(boolProperty);
		int p = v.toInt();
		bool val = value.toBool();
		if(val) {
			p = p | 0x01;
		}
		else {
			p = p & ~0x01;
		}
		setValue(boolProperty, p);
	} else if (days1ToProperty.contains(property)) {
		QtProperty *boolProperty = days1ToProperty[property];
		QVariant v = this->value(boolProperty);
		int p = v.toInt();
		bool val = value.toBool();
		if(val) {
			p = p | 0x02;
		}
		else {
			p = p & ~0x02;
		}
		setValue(boolProperty, p);
	} else if (days2ToProperty.contains(property)) {
		QtProperty *boolProperty = days2ToProperty[property];
		QVariant v = this->value(boolProperty);
		int p = v.toInt();
		bool val = value.toBool();
		if(val) {
			p = p | 0x04;
		}
		else {
			p = p & ~0x04;
		}
		setValue(boolProperty, p);
	} else if (days4ToProperty.contains(property)) {
		QtProperty *boolProperty = days4ToProperty[property];
		QVariant v = this->value(boolProperty);
		int p = v.toInt();
		bool val = value.toBool();
		if(val) {
			p = p | 0x08;
		}
		else {
			p = p & ~0x08;
		}
		setValue(boolProperty, p);
	} else if (days8ToProperty.contains(property)) {
		QtProperty *boolProperty = days8ToProperty[property];
		QVariant v = this->value(boolProperty);
		int p = v.toInt();
		bool val = value.toBool();
		if(val) {
			p = p | 0x10;
		}
		else {
			p = p & ~0x10;
		}
		setValue(boolProperty, p);
	} else if (days16ToProperty.contains(property)) {
		QtProperty *boolProperty = days16ToProperty[property];
		QVariant v = this->value(boolProperty);
		int p = v.toInt();
		bool val = value.toBool();
		if(val) {
			p = p | 0x20;
		}
		else {
			p = p & ~0x20;
		}
		setValue(boolProperty, p);
	}
}

void VariantPropertyManager::slotPropertyDestroyed(QtProperty *property)
{
	if (xToProperty.contains(property)) {
		QtProperty *pointProperty = xToProperty[property];
		propertyToVector3DData[pointProperty].x = 0;
		xToProperty.remove(property);
	} else if (yToProperty.contains(property)) {
		QtProperty *pointProperty = yToProperty[property];
		propertyToVector3DData[pointProperty].y = 0;
		yToProperty.remove(property);
	} else if (zToProperty.contains(property)) {
		QtProperty *pointProperty = zToProperty[property];
		propertyToVector3DData[pointProperty].z = 0;
		zToProperty.remove(property);
	} else if (z1ToProperty.contains(property)) {
		QtProperty *pointProperty = z1ToProperty[property];
		propertyToVector2DData[pointProperty].z1 = 0;
		z1ToProperty.remove(property);
	} else if (z2ToProperty.contains(property)) {
		QtProperty *pointProperty = z2ToProperty[property];
		propertyToVector2DData[pointProperty].z2 = 0;
		z2ToProperty.remove(property);
	} else if (x1ToProperty.contains(property)) {
		QtProperty *pointProperty = x1ToProperty[property];
		propertyToVector4DData[pointProperty].x1 = 0;
		x1ToProperty.remove(property);
	} else if (y1ToProperty.contains(property)) {
		QtProperty *pointProperty = y1ToProperty[property];
		propertyToVector4DData[pointProperty].y1 = 0;
		y1ToProperty.remove(property);
	} else if (x2ToProperty.contains(property)) {
		QtProperty *pointProperty = x2ToProperty[property];
		propertyToVector4DData[pointProperty].x2 = 0;
		x2ToProperty.remove(property);
	} else if (y2ToProperty.contains(property)) {
		QtProperty *pointProperty = y2ToProperty[property];
		propertyToVector4DData[pointProperty].y2 = 0;
		y2ToProperty.remove(property);
	} else if (limitToProperty.contains(property)) {
		QtProperty *boolProperty = limitToProperty[property];
		propertyToIsoplethLineData[boolProperty].limit = 0;
		limitToProperty.remove(property);
	} else if (days1ToProperty.contains(property)) {
		QtProperty *boolProperty = days1ToProperty[property];
		propertyToIsoplethLineData[boolProperty].days1 = 0;
		days1ToProperty.remove(property);
	} else if (days2ToProperty.contains(property)) {
		QtProperty *boolProperty = days2ToProperty[property];
		propertyToIsoplethLineData[boolProperty].days2 = 0;
		days2ToProperty.remove(property);
	} else if (days4ToProperty.contains(property)) {
		QtProperty *boolProperty = days4ToProperty[property];
		propertyToIsoplethLineData[boolProperty].days4 = 0;
		days4ToProperty.remove(property);
	} else if (days8ToProperty.contains(property)) {
		QtProperty *boolProperty = days8ToProperty[property];
		propertyToIsoplethLineData[boolProperty].days8 = 0;
		days8ToProperty.remove(property);
	} else if (days16ToProperty.contains(property)) {
		QtProperty *boolProperty = days16ToProperty[property];
		propertyToIsoplethLineData[boolProperty].days16 = 0;
		days16ToProperty.remove(property);
	}
}

bool VariantPropertyManager::isPropertyTypeSupported(int propertyType) const
{
	if (propertyType == QVariant::Vector3D)
		return true;
	else if (propertyType == QVariant::Vector2D)
		return true;
	else if (propertyType == QVariant::Vector4D)
		return true;
	else if (propertyType == QVariant::UInt)
		return true;
	else if (propertyType == StringReadOnly)
		return true;
	else if (propertyType == IsoplethDataLines)
		return true;
	return QtVariantPropertyManager::isPropertyTypeSupported(propertyType);
}

QVariant VariantPropertyManager::attributeValue(const QtProperty *property, const QString &attribute) const {

	if (attribute == "reference")
		return reference(property);
	else if (attribute == "objectId")
		return objectId(property);

	return QtVariantPropertyManager::attributeValue(property, attribute);
}

QString VariantPropertyManager::reference(const QtProperty *property) const {

	return propToRef[property].reference;
}

unsigned int VariantPropertyManager::objectId(const QtProperty *property) const {

	return propToRef[property].objectId;
}

void VariantPropertyManager::setReference(QtProperty *property, QString refValue) {

	propToRef[property].reference = refValue;
}

void VariantPropertyManager::setObjectId(QtProperty *property, unsigned int refValue) {

	propToRef[property].objectId = refValue;
}

int VariantPropertyManager::valueType(int propertyType) const
{
	if (propertyType == QVariant::Vector3D)
		return QVariant::Vector3D;
	else if (propertyType == QVariant::Vector4D)
		return QVariant::Vector4D;
	else if (propertyType == QVariant::Vector2D)
		return QVariant::Vector2D;
	else if (propertyType == QVariant::UInt)
		return QVariant::UInt;
	else if (propertyType == StringReadOnly)	// StringData
		return StringReadOnly;
	else if (propertyType == IsoplethDataLines)	// IsoplethLinedata
		return IsoplethDataLines;
	return QtVariantPropertyManager::valueType(propertyType);
}

QVariant VariantPropertyManager::value(const QtProperty *property) const
{
	if (propertyToVector3DData.contains(property))
		return propertyToVector3DData[property].value;
	else if (propertyToVector2DData.contains(property))
		return propertyToVector2DData[property].value;
	else if (propertyToVector4DData.contains(property))
		return propertyToVector4DData[property].value;
	else if (propertyToUIntData.contains(property))
		return propertyToUIntData[property].value;
	else if (propertyToStringData.contains(property))
		return propertyToStringData[property].value;
	else if (propertyToIsoplethLineData.contains(property))
		return propertyToIsoplethLineData[property].value;

	return QtVariantPropertyManager::value(property);
}


QString VariantPropertyManager::valueText(const QtProperty *property) const
{
	if (propertyToVector3DData.contains(property)) {
		QVariant v = propertyToVector3DData[property].value;
//		QVector3D p = qVariantValue<QVector3D>(v);
		QVector3D p = v.value<QVector3D>();
		return QString(tr("(%1, %2, %3)").arg(QString::number(p.x()))
								 .arg(QString::number(p.y())).arg(QString::number(p.z())));
	}
	else if (propertyToVector2DData.contains(property)) {
		QVariant v = propertyToVector2DData[property].value;
//		QVector2D p = qVariantValue<QVector2D>(v);
		QVector2D p = v.value<QVector2D>();
		return QString(tr("(%1, %2)").arg(QString::number(p.x()))
								 .arg(QString::number(p.y())));
	}
	else if (propertyToVector4DData.contains(property)) {
		QVariant v = propertyToVector4DData[property].value;
//		QVector4D p = qVariantValue<QVector4D>(v);
		QVector4D p = v.value<QVector4D>();
		return QString(tr("(%1, %2) (%3, %4)").arg(QString::number(p.x()))
											  .arg(QString::number(p.y()))
											  .arg(QString::number(p.z()))
											  .arg(QString::number(p.w())));
	}
	else if (propertyToUIntData.contains(property)) {
		QVariant v = propertyToUIntData[property].value;
//		uint p = qVariantValue<uint>(v);
		uint p = v.value<uint>();
		return QString(tr("ID: %1").arg(QString::number(p)));
	}
	else if (propertyToStringData.contains(property)) {
		QVariant v = propertyToStringData[property].value;
//		QString p = qVariantValue<QString>(v);
		QString p = v.value<QString>();
		return QString(tr("%1").arg(p));
	}
	else if (propertyToIsoplethLineData.contains(property)) {
		QVariant v = propertyToIsoplethLineData[property].value;
		int p = v.toInt();
		return QString(tr("%1").arg(p));
	}
	return QtVariantPropertyManager::valueText(property);
}

void VariantPropertyManager::setValue(QtProperty *property, const QVariant &val)
{
	if (propertyToVector3DData.contains(property)) {
		if (val.type() != QVariant::Vector3D && !val.canConvert(QVariant::Vector3D))
			return;
//		QVector3D p = qVariantValue<QVector3D>(val);
		QVector3D p = val.value<QVector3D>();
		Vector3DData d = propertyToVector3DData[property];
		d.value = p;
		if (d.x)
			d.x->setValue(p.x());
		if (d.y)
			d.y->setValue(p.y());
		if (d.z)
			d.z->setValue(p.z());
		propertyToVector3DData[property] = d;
		emit propertyChanged(property);
		emit valueChanged(property, p);
		return;
	}
	else if (propertyToVector4DData.contains(property)) {
		if (val.type() != QVariant::Vector4D && !val.canConvert(QVariant::Vector4D))
			return;
//		QVector34D p = qVariantValue<QVector4D>(val);
		QVector4D p = val.value<QVector4D>();
		Vector4DData d = propertyToVector4DData[property];
		d.value = p;
		if (d.x1)
			d.x1->setValue(p.x());
		if (d.y1)
			d.y1->setValue(p.y());
		if (d.x2)
			d.x2->setValue(p.z());
		if (d.y2)
			d.y2->setValue(p.w());
		propertyToVector4DData[property] = d;
		emit propertyChanged(property);
		emit valueChanged(property, p);
		return;
	}
	else if (propertyToVector2DData.contains(property)) {
		if (val.type() != QVariant::Vector2D && !val.canConvert(QVariant::Vector2D))
			return;
//		QVector2D p = qVariantValue<QVector2D>(val);
		QVector2D p = val.value<QVector2D>();
		Vector2DData d = propertyToVector2DData[property];
		d.value = p;
		if (d.z1)
			d.z1->setValue(p.x());
		if (d.z2)
			d.z2->setValue(p.y());
		propertyToVector2DData[property] = d;
		emit propertyChanged(property);
		emit valueChanged(property, p);
		return;
	}
	else if (propertyToUIntData.contains(property)) {
		if (val.type() != QVariant::UInt && !val.canConvert(QVariant::UInt))
			return;
//		uint p = qVariantValue<uint>(val);
		uint p = val.value<uint>();
		UIntData d = propertyToUIntData[property];
		d.value = p;

		propertyToUIntData[property] = d;
		emit propertyChanged(property);
		emit valueChanged(property, p);
		return;
	}
	else if (propertyToStringData.contains(property)) {
		if (val.type() != (QVariant::Type)StringReadOnly && !val.canConvert(QVariant::String))
			return;
//		QString p = qVariantValue<QString>(val);
		QString p = val.value<QString>();
		StringData d = propertyToStringData[property];
		d.value = p;

		propertyToStringData[property] = d;
		emit propertyChanged(property);
		emit valueChanged(property, p);
		return;
	}
	else if (propertyToIsoplethLineData.contains(property)) {
		if (val.type() != (QVariant::Type)IsoplethDataLines && !val.canConvert(QVariant::Int))
			return;
		int p = val.toInt();
		IsoplethLinedata d = propertyToIsoplethLineData[property];
		d.value = p;

		propertyToIsoplethLineData[property] = d;
		if(d.limit)
			d.limit->setValue((p & 0x01) != 0);
		if(d.days1)
			d.days1->setValue((p & 0x02) != 0);
		if(d.days2)
			d.days2->setValue((p & 0x04) != 0);
		if(d.days4)
			d.days4->setValue((p & 0x08) != 0);
		if(d.days8)
			d.days8->setValue((p & 0x10) != 0);
		if(d.days16)
			d.days16->setValue((p & 0x20) != 0);
		emit propertyChanged(property);
		emit valueChanged(property, p);
		return;
	}
	QtVariantPropertyManager::setValue(property, val);
}

void VariantPropertyManager::setAttribute(QtProperty *property,
		const QString &attribute, const QVariant &value)
{

	if (attribute == "reference") {
//		setReference(property, qVariantValue<QString>(value));
		setReference(property, value.value<QString>());
		return;
	}
	else if (attribute == "objectId") {
//		setObjectId(property, qVariantValue<unsigned int>(value));
		setObjectId(property, value.value<unsigned int>());
		return;
	}

	QtVariantPropertyManager::setAttribute(property, attribute, value);
}



void VariantPropertyManager::initializeProperty(QtProperty *property)
{
	if (propertyType(property) == QVariant::Vector3D) {
		Vector3DData d;

		d.value = QVector3D(0, 0, 0);

		VariantPropertyManager *that = (VariantPropertyManager *)this;

		d.x = that->addProperty(QVariant::Double);
		d.x->setPropertyName(tr("X"));
		property->addSubProperty(d.x);
		xToProperty[d.x] = property;

		d.y = that->addProperty(QVariant::Double);
		d.y->setPropertyName(tr("Y"));
		property->addSubProperty(d.y);
		yToProperty[d.y] = property;

		d.z = that->addProperty(QVariant::Double);
		d.z->setPropertyName(tr("Z"));
		property->addSubProperty(d.z);
		zToProperty[d.z] = property;

		propertyToVector3DData[property] = d;
	}
	else if (propertyType(property) == QVariant::Vector4D) {
		Vector4DData d;

		d.value = QVector4D(0, 0, 0, 0);

		VariantPropertyManager *that = (VariantPropertyManager *)this;

		d.x1 = that->addProperty(QVariant::Double);
		d.x1->setPropertyName(tr("X1"));
		property->addSubProperty(d.x1);
		x1ToProperty[d.x1] = property;

		d.y1 = that->addProperty(QVariant::Double);
		d.y1->setPropertyName(tr("Y1"));
		property->addSubProperty(d.y1);
		y1ToProperty[d.y1] = property;

		d.x2 = that->addProperty(QVariant::Double);
		d.x2->setPropertyName(tr("X2"));
		property->addSubProperty(d.x2);
		x2ToProperty[d.x2] = property;

		d.y2 = that->addProperty(QVariant::Double);
		d.y2->setPropertyName(tr("Y2"));
		property->addSubProperty(d.y2);
		y2ToProperty[d.y2] = property;

		propertyToVector4DData[property] = d;
	}
	else if (propertyType(property) == QVariant::Vector2D) {
		Vector2DData d;

		d.value = QVector2D(0, 0);

		VariantPropertyManager *that = (VariantPropertyManager *)this;

		d.z1 = that->addProperty(QVariant::Double);
		d.z1->setPropertyName(tr("Z1"));
		d.z1->setAttribute( tr("decimals"), 3);
		property->addSubProperty(d.z1);
		z1ToProperty[d.z1] = property;

		d.z2 = that->addProperty(QVariant::Double);
		d.z2->setPropertyName(tr("Z2"));
		d.z2->setAttribute( tr("decimals"), 3);
		property->addSubProperty(d.z2);
		z2ToProperty[d.z2] = property;

		propertyToVector2DData[property] = d;
	}
	else if (propertyType(property) == QVariant::UInt) {

		UIntData d;
		d.value = 0;

		propertyToUIntData[property] = d;
	}
	else if (propertyType(property) == StringReadOnly) {

		StringData d;
		d.value = QString();

		propertyToStringData[property] = d;
	}
	else if (propertyType(property) == IsoplethDataLines) {

		IsoplethLinedata d;
		d.value = 0;

		VariantPropertyManager *that = (VariantPropertyManager *)this;

		d.limit = that->addProperty(QVariant::Bool);
		d.limit->setPropertyName(tr("Limit for germination"));
		property->addSubProperty(d.limit);
		limitToProperty[d.limit] = property;

		d.days1 = that->addProperty(QVariant::Bool);
		d.days1->setPropertyName(tr("Germination after one day"));
		property->addSubProperty(d.days1);
		days1ToProperty[d.days1] = property;

		d.days2 = that->addProperty(QVariant::Bool);
		d.days2->setPropertyName(tr("Germination after two days"));
		property->addSubProperty(d.days2);
		days2ToProperty[d.days2] = property;

		d.days4 = that->addProperty(QVariant::Bool);
		d.days4->setPropertyName(tr("Germination after 4 days"));
		property->addSubProperty(d.days4);
		days4ToProperty[d.days4] = property;

		d.days8 = that->addProperty(QVariant::Bool);
		d.days8->setPropertyName(tr("Germination after 8 days"));
		property->addSubProperty(d.days8);
		days8ToProperty[d.days8] = property;

		d.days16 = that->addProperty(QVariant::Bool);
		d.days16->setPropertyName(tr("Germination after 16 days"));
		property->addSubProperty(d.days16);
		days16ToProperty[d.days16] = property;

		propertyToIsoplethLineData[property] = d;
	}
	QtVariantPropertyManager::initializeProperty(property);
}

void VariantPropertyManager::uninitializeProperty(QtProperty *property)
{
	if (propertyToVector2DData.contains(property)) {
		Vector2DData d = propertyToVector2DData[property];
		if (d.z1)
			z1ToProperty.remove(d.z1);
		if (d.z2)
			z2ToProperty.remove(d.z2);
		propertyToVector3DData.remove(property);
	}
	else if (propertyToVector3DData.contains(property)) {
		Vector3DData d = propertyToVector3DData[property];
		if (d.x)
			xToProperty.remove(d.x);
		if (d.y)
			yToProperty.remove(d.y);
		if (d.z)
			zToProperty.remove(d.z);
		propertyToVector3DData.remove(property);
	}
	else if (propertyToVector4DData.contains(property)) {
		Vector4DData d = propertyToVector4DData[property];
		if (d.x1)
			xToProperty.remove(d.x1);
		if (d.y1)
			yToProperty.remove(d.y1);
		if (d.x2)
			xToProperty.remove(d.x2);
		if (d.y2)
			yToProperty.remove(d.y2);
		propertyToVector4DData.remove(property);
	}
	else if (propertyToUIntData.contains(property)) {
		propertyToUIntData.remove(property);
	}
	else if (propertyToStringData.contains(property)) {
		propertyToStringData.remove(property);
	}
	else if (propertyToIsoplethLineData.contains(property)) {
		IsoplethLinedata d = propertyToIsoplethLineData[property];
		if (d.limit)
			limitToProperty.remove(d.limit);
		if (d.days1)
			days1ToProperty.remove(d.days1);
		if (d.days2)
			days2ToProperty.remove(d.days2);
		if (d.days4)
			days4ToProperty.remove(d.days4);
		if (d.days8)
			days8ToProperty.remove(d.days8);
		if (d.days16)
			days16ToProperty.remove(d.days16);
		propertyToIsoplethLineData.remove(property);
	}
	QtVariantPropertyManager::uninitializeProperty(property);
}

} //namespace QPW

#include "moc_QPW_VariantPropertyManager.cpp"
