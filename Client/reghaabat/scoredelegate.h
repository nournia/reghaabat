#ifndef SCOREDELEGATE_H
#define SCOREDELEGATE_H

#include <helper.h>
#include <QItemDelegate>
#include <QSpinBox>

class ScoreDelegate : public QItemDelegate
{
public:
    ScoreDelegate();

    ScoreDelegate(QObject *parent = 0) : QItemDelegate(parent) {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QSpinBox* editor = new QSpinBox(parent);
        editor->setMaximum(200);
        editor->setMinimum(0);
        editor->setSuffix("%");
        return editor;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        QString value = index.model()->data(index, Qt::EditRole).toString();
        value = value.left(value.size()-1);
        QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
        spinBox->setValue(value.toInt());
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
    {
        QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
        model->setData(index, (float)spinBox->value()/100, Qt::EditRole);
    }
};

#endif // SCOREDELEGATE_H
