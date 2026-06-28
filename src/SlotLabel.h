#ifndef SLOTLABEL_H
#define SLOTLABEL_H

#include <QLabel>

class SlotLabel : public QLabel {
    Q_OBJECT
public:
    explicit SlotLabel(int index, QWidget *parent = nullptr);

signals:
    void clicked(int index);
    void fileDropped(const QString &filePath, int index);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    int m_index;
};

#endif // SLOTLABEL_H
