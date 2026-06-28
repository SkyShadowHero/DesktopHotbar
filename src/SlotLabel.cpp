#include "SlotLabel.h"
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>

SlotLabel::SlotLabel(int index, QWidget *parent)
    : QLabel(parent), m_index(index)
{
    setAlignment(Qt::AlignCenter);
    setAcceptDrops(true);
    setMouseTracking(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
}

void SlotLabel::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked(m_index);
    }
    QLabel::mousePressEvent(event);
}

void SlotLabel::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void SlotLabel::dropEvent(QDropEvent *event) {
    const QList<QUrl> urls = event->mimeData()->urls();
    if (!urls.isEmpty()) {
        QString filePath = urls.first().toLocalFile();
        emit fileDropped(filePath, m_index);
    }
}
