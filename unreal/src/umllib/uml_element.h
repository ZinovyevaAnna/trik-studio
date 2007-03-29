#ifndef UML_ELEMENT_H
#define UML_ELEMENT_H

#include <QtGui/QGraphicsItem>
#include <QtCore/QModelIndex>

namespace UML {
    class Element : public QGraphicsItem
    {
    public:
        Element();

        QPersistentModelIndex index() const { return dataIndex; };
	void setIndex(QPersistentModelIndex &index);

        virtual void updateData();

	int uuid() const;    
    protected:
	QPersistentModelIndex dataIndex;

	int m_uuid;
    };
};

#endif
