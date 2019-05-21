#ifndef QPW_VariantPropertyManagerH
#define QPW_VariantPropertyManagerH

#include "qtvariantproperty.h"

namespace QPW {

class VariantPropertyManager : public QtVariantPropertyManager {
	Q_OBJECT
public:

	/*! enum used for userdefined variant types. */
	enum UserTypeDefinitions {
		StringReadOnly =	QVariant::UserType + 1,
		IsoplethDataLines = QVariant::UserType + 2,
		NUM_UTD
	};

	explicit VariantPropertyManager(QObject *parent = 0);

	~VariantPropertyManager();

	virtual QVariant value(const QtProperty *property) const;
	virtual int valueType(int propertyType) const;
	virtual bool isPropertyTypeSupported(int propertyType) const;
	virtual QVariant attributeValue(const QtProperty *property, const QString &attribute) const;

	unsigned int objectId(const QtProperty *property) const;
	QString reference(const QtProperty *property) const;
	QString valueText(const QtProperty *property) const;

public slots:

	virtual void setValue(QtProperty *property, const QVariant &val);
	virtual void setAttribute(QtProperty *property,
				const QString &attribute, const QVariant &value);
	void setReference(QtProperty *property, QString refValue);
	void setObjectId(QtProperty *property, unsigned int refValue);

signals:
	void valueChanged2(QtProperty *, const QVariant &);

protected:

	virtual void initializeProperty(QtProperty *property);
	virtual void uninitializeProperty(QtProperty *property);

private slots:

	void slotValueChanged(QtProperty *property, const QVariant &value);
	void slotPropertyDestroyed(QtProperty *property);

private:

	/*! used for references to identicate properties after value changed */
	struct RefData {
		QString			reference;
		unsigned int	objectId;

		RefData() : reference(QString("")), objectId(0xFFFFFFFF) {}
	};

	/*! used for position values with x1,x2,y1,y2. */
	struct Vector4DData {
		QVariant value;
		QtVariantProperty *x1;
		QtVariantProperty *y1;
		QtVariantProperty *x2;
		QtVariantProperty *y2;
	};

	/*! used for coordinate values. */
	struct Vector3DData {
		QVariant value;
		QtVariantProperty *x;
		QtVariantProperty *y;
		QtVariantProperty *z;
	};

	/*! used for point values with name 'Z1' for x and 'Z2' for y*/
	struct Vector2DData {
		QVariant value;
		QtVariantProperty *z1;
		QtVariantProperty *z2;
	};

	/*! used for IDs: 'ID: 111'*/
	struct UIntData {
		QVariant value;
	};

	/*! used for readonly string values */
	struct StringData {
		QVariant value;
	};

	/*! used visibilty of isopleth data lines */
	struct IsoplethLinedata {
		QtVariantProperty *limit;	// 1  0x01
		QtVariantProperty *days1;	// 2  0x02
		QtVariantProperty *days2;	// 4  0x04
		QtVariantProperty *days4;	// 8  0x08
		QtVariantProperty *days8;	// 16 0x10
		QtVariantProperty *days16;	// 32 0x20
		QVariant value;
	};

	QMap<const QtProperty *, RefData>	   propToRef;

	QMap<const QtProperty *, Vector4DData> propertyToVector4DData;
	QMap<const QtProperty *, QtProperty *> x1ToProperty;
	QMap<const QtProperty *, QtProperty *> y1ToProperty;
	QMap<const QtProperty *, QtProperty *> x2ToProperty;
	QMap<const QtProperty *, QtProperty *> y2ToProperty;

	QMap<const QtProperty *, Vector3DData> propertyToVector3DData;
	QMap<const QtProperty *, QtProperty *> xToProperty;
	QMap<const QtProperty *, QtProperty *> yToProperty;
	QMap<const QtProperty *, QtProperty *> zToProperty;

	QMap<const QtProperty *, Vector2DData> propertyToVector2DData;
	QMap<const QtProperty *, QtProperty *> z1ToProperty;
	QMap<const QtProperty *, QtProperty *> z2ToProperty;

	QMap<const QtProperty *, UIntData> propertyToUIntData;

	QMap<const QtProperty *, StringData> propertyToStringData;

	QMap<const QtProperty *, IsoplethLinedata> propertyToIsoplethLineData;
	QMap<const QtProperty *, QtProperty *> limitToProperty;
	QMap<const QtProperty *, QtProperty *> days1ToProperty;
	QMap<const QtProperty *, QtProperty *> days2ToProperty;
	QMap<const QtProperty *, QtProperty *> days4ToProperty;
	QMap<const QtProperty *, QtProperty *> days8ToProperty;
	QMap<const QtProperty *, QtProperty *> days16ToProperty;

};

} //namespace QPW {


#endif // QPW_VariantPropertyManagerH
