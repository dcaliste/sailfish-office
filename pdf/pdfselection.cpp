/*
 * Copyright (C) 2015-2016 Caliste Damien.
 * Contact: Damien Caliste <dcaliste@free.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2 only.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "pdfselection.h"
#include "pdfdocument.h"

class PDFSelection::Private
{
public:
    Private()
        : canvas(nullptr)
        , pageIndexStart(-1)
        , boxIndexStart(-1)
        , pageIndexStop(-1)
        , boxIndexStop(-1)
        , wiggle(4.)
    {
    }

    PDFCanvas *canvas;

    QPointF start;
    QPointF stop;

    int pageIndexStart;
    int boxIndexStart;

    int pageIndexStop;
    int boxIndexStop;

    qreal wiggle;

    enum Position {
        At,
        Before,
        After
    };
    void textBoxId(const QPointF &point, Position position, int *pageIndex, int *boxIndex);
    int sliceCount(int pageIndex1, int boxIndex1, int pageIndex2, int boxIndex2);
};

PDFSelection::PDFSelection(QObject *parent)
  : QAbstractListModel(parent), d(new Private)
{
}

PDFSelection::~PDFSelection()
{
    delete d;
}

QHash<int, QByteArray> PDFSelection::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles.insert(Rect, "rect");
    roles.insert(Text, "text");

    return roles;
}

QVariant PDFSelection::data(const QModelIndex& index, int role) const
{
    PDFDocument *doc = d->canvas->document();
    int pageIndex = -1;
    int boxIndex;
    QVariant result;

    if (index.isValid()) {
        boxIndex = index.row();
        if (boxIndex > -1 && boxIndex < count()) {
            boxIndex += d->boxIndexStart;
            for (pageIndex = d->pageIndexStart;
                 pageIndex <= d->pageIndexStop; pageIndex++) {
                int nBoxes = doc->textBoxesAtPage(pageIndex).count();
                if (boxIndex < nBoxes) {
                    break;
                } else {
                    boxIndex -= nBoxes;
                }
            }
        }
    }
    if (pageIndex >= 0) {
        const PDFDocument::TextList &boxes = doc->textBoxesAtPage(pageIndex);
        const QPair<QRectF, Poppler::TextBox*> &box = boxes.at(boxIndex);
        switch(role) {
        case Rect:
            result.setValue<QRectF>(d->canvas->fromPageToItem(pageIndex, box.first));
            break;
        case Text:
            result.setValue<QString>(box.second->text());
            break;
        default:
            result.setValue<QString>(QString("Unknown role: %1").arg(role));
            break;
        }
    }
    return result;
}

int PDFSelection::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return count();
}

int PDFSelection::Private::sliceCount(int pageIndex1, int boxIndex1,
                                      int pageIndex2, int boxIndex2)
{
    if (!canvas)
        return 0;
    PDFDocument *doc = canvas->document();
    if (!doc)
        return 0;

    if (pageIndex1 < pageIndex2 || (pageIndex1 == pageIndex2 && boxIndex1 < boxIndex2)) {
        int n = -boxIndex1;
        for (int i = pageIndex1; i < pageIndex2; i++) {
            n += doc->textBoxesAtPage(i).count();
        }
        return n + boxIndex2;
    } else {
        int n = -boxIndex2;
        for (int i = pageIndex2; i < pageIndex1; i++) {
            n += doc->textBoxesAtPage(i).count();
        }
        return -n - boxIndex1;
    }
}

int PDFSelection::count() const
{
    if (d->pageIndexStart < 0 || d->pageIndexStop < 0 ||
        d->boxIndexStart < 0 || d->boxIndexStop < 0) {
        return 0;
    }

    if (!d->canvas || !d->canvas->document())
        return 0;

    return d->sliceCount(d->pageIndexStart, d->boxIndexStart,
                         d->pageIndexStop, d->boxIndexStop) + 1;
}

PDFCanvas* PDFSelection::canvas() const
{
    return d->canvas;
}

void PDFSelection::setCanvas(PDFCanvas *newCanvas)
{
    if (newCanvas != d->canvas) {
        if (d->canvas)
            d->canvas->disconnect(this);
        d->canvas = newCanvas;

        connect(d->canvas, &PDFCanvas::pageLayoutChanged, this, &PDFSelection::startChanged);
        connect(d->canvas, &PDFCanvas::pageLayoutChanged, this, &PDFSelection::stopChanged);
        connect(d->canvas, &PDFCanvas::pageLayoutChanged, this, &PDFSelection::onLayoutChanged);

        emit canvasChanged();
    }
}

float PDFSelection::wiggle() const
{
    return d->wiggle;
}

void PDFSelection::setWiggle(float newValue)
{
    if (newValue != d->wiggle) {
        d->wiggle = newValue;
        emit wiggleChanged();
    }
}

void PDFSelection::Private::textBoxId(const QPointF &point, Position position,
                                      int *pageIndex, int *boxIndex)
{
    *pageIndex = -1;
    *boxIndex  = -1;
    if (!canvas || !canvas->document())
        return;
    
    // point is given in canvas coordinates.
    QPair<int, QRectF> at = canvas->pageAtPoint(point);
    if (at.first < 0)
        return;
    *pageIndex = at.first;

    const PDFDocument::TextList &boxes = canvas->document()->textBoxesAtPage(*pageIndex);
    switch (position) {
    case PDFSelection::Private::At: {
        *boxIndex = -1;
        qreal squaredDistanceMin = wiggle * wiggle;
        int i = 0;
        for (PDFDocument::TextList::const_iterator box = boxes.begin();
             box != boxes.end(); box++) {
            qreal squaredDistance =
                canvas->squaredDistanceFromRect(at.second, box->first, point);
                
            if (squaredDistance < squaredDistanceMin) {
                *boxIndex = i;
                squaredDistanceMin = squaredDistance;
            }
            i += 1;
        }
        break;
    }
    case PDFSelection::Private::Before: {
        QPointF reducedCoordPoint {point.x() / at.second.width(),
                (point.y() - at.second.y()) / at.second.height()};
        *boxIndex = -1;
        for (PDFDocument::TextList::const_iterator box = boxes.begin();
             box != boxes.end(); box++) {
            /* Stop counting boxes up as soon as a box after reducedCoordPoint is found. */
            if (box->first.y() > reducedCoordPoint.y() ||
                (box->first.y() + box->first.height() > reducedCoordPoint.y() &&
                 box->first.x() > reducedCoordPoint.x())) {
                if (*boxIndex == -1) {
                    *pageIndex -= 1;
                    if (*pageIndex >= 0)
                        *boxIndex = canvas->document()->textBoxesAtPage(*pageIndex).length() - 1;
                }
                return;
            }
            *boxIndex += 1;
        }
        *boxIndex = boxes.length() - 1;
        break;
    }
    case PDFSelection::Private::After: {
        QPointF reducedCoordPoint {point.x() / at.second.width(),
                (point.y() - at.second.y()) / at.second.height()};
        *boxIndex = 0;
        for (PDFDocument::TextList::const_iterator box = boxes.begin();
             box != boxes.end(); box++) {
            /* Stop counting boxes down as soon as a box after reducedCoordPoint is found. */
            if (box->first.y() > reducedCoordPoint.y() ||
                (box->first.y() + box->first.height() > reducedCoordPoint.y() &&
                 box->first.x() + box->first.width() > reducedCoordPoint.x())) {
                return;
            }
            *boxIndex += 1;
        }
        *pageIndex += 1;
        if (*pageIndex >= canvas->document()->pageCount())
            *pageIndex = -1;
        *boxIndex = 0;
        break;
    }
    default:
        break;
    }
}

QPointF PDFSelection::start() const
{
    if (!d->canvas || d->pageIndexStart < 0)
        return QPointF();
    
    return d->canvas->fromPageToItem(d->pageIndexStart, d->start);
}

void PDFSelection::setStart(const QPointF &point)
{
    int pageIndex, boxIndex;
    d->textBoxId(point, PDFSelection::Private::After, &pageIndex, &boxIndex);
    if (pageIndex < 0 || boxIndex < 0
        || (pageIndex == d->pageIndexStart && boxIndex == d->boxIndexStart)) {
        return;
    }
    if (pageIndex > d->pageIndexStop
        || (pageIndex == d->pageIndexStop && boxIndex > d->boxIndexStop)) {
        return;
    }

    const PDFDocument::TextList &boxes = d->canvas->document()->textBoxesAtPage(pageIndex);
    QRectF box = boxes[boxIndex].first;

    int nBoxes = d->sliceCount(pageIndex, boxIndex, d->pageIndexStart, d->boxIndexStart);
    if (nBoxes > 0) {
        beginInsertRows(QModelIndex(), 0, nBoxes - 1);
        d->pageIndexStart = pageIndex;
        d->boxIndexStart = boxIndex;
        endInsertRows();
    } else {
        beginRemoveRows(QModelIndex(), 0, -nBoxes - 1);
        d->pageIndexStart = pageIndex;
        d->boxIndexStart = boxIndex;
        endRemoveRows();
    }

    d->start = QPointF(box.x(), box.y() + box.height() / 2.);
    emit startChanged();

    emit countChanged();
    emit textChanged();
}

QPointF PDFSelection::stop() const
{
    if (!d->canvas || d->pageIndexStop < 0)
        return QPointF();
    
    return d->canvas->fromPageToItem(d->pageIndexStop, d->stop);
}

void PDFSelection::setStop(const QPointF &point)
{
    int pageIndex, boxIndex;
    d->textBoxId(point, PDFSelection::Private::Before, &pageIndex, &boxIndex);
    if (pageIndex < 0 || boxIndex < 0
        || (pageIndex == d->pageIndexStop && boxIndex == d->boxIndexStop)) {
        return;
    }
    if (pageIndex < d->pageIndexStart
        || (pageIndex == d->pageIndexStart && boxIndex < d->boxIndexStart)) {
        return;
    }

    const PDFDocument::TextList &boxes = d->canvas->document()->textBoxesAtPage(pageIndex);
    QRectF box = boxes[boxIndex].first;

    int count_ = count(); 
    int nBoxes = d->sliceCount(d->pageIndexStop, d->boxIndexStop, pageIndex, boxIndex);
    if (nBoxes > 0) {
        beginInsertRows(QModelIndex(), count_, count_ + nBoxes - 1);
        d->pageIndexStop = pageIndex;
        d->boxIndexStop = boxIndex;
        endInsertRows();
    } else {
        beginRemoveRows(QModelIndex(), count_ + nBoxes, count_ - 1);
        d->pageIndexStop = pageIndex;
        d->boxIndexStop = boxIndex;
        endRemoveRows();
    }

    d->stop = QPointF(box.x() + box.width(), box.y() + box.height() / 2.);
    emit stopChanged();

    emit countChanged();
    emit textChanged();
}

void PDFSelection::selectAt(const QPointF &point)
{
    if (d->pageIndexStart >= 0 && d->boxIndexStart >= 0
        && d->pageIndexStop >= 0 && d->boxIndexStop >= 0) {
        unselect();
    }

    int pageIndex, boxIndex;
    d->textBoxId(point, PDFSelection::Private::At, &pageIndex, &boxIndex);
    if (pageIndex < 0 || boxIndex < 0)
        return;

    const PDFDocument::TextList &boxes = d->canvas->document()->textBoxesAtPage(pageIndex);
    QRectF box = boxes[boxIndex].first;

    beginInsertRows(QModelIndex(), 0, 0);
    d->pageIndexStart = pageIndex;
    d->boxIndexStart = boxIndex;
    d->pageIndexStop = pageIndex;
    d->boxIndexStop = boxIndex;
    endInsertRows();

    d->start = QPointF(box.x(), box.y() + box.height() / 2.);
    emit startChanged();
    d->stop = QPointF(box.x() + box.width(), box.y() + box.height() / 2.);
    emit stopChanged();

    emit countChanged();
    emit textChanged();
}

void PDFSelection::unselect()
{
    beginResetModel();
    d->pageIndexStart = d->pageIndexStop = -1;
    d->boxIndexStart = d->boxIndexStop = -1;
    endResetModel();
    emit countChanged();
    emit textChanged();
}

void PDFSelection::onLayoutChanged()
{
    emit dataChanged(createIndex(0, 0), createIndex(count() - 1, 0), QVector<int>{Rect});
}

QString PDFSelection::text() const
{
    QString out;

    if (d->pageIndexStart < 0 || d->pageIndexStop < 0 ||
        d->boxIndexStart < 0 || d->boxIndexStop < 0) {
        return out;
    }

    if (!d->canvas)
        return out;
    PDFDocument *doc = d->canvas->document();
    if (!doc)
        return out;
    
    int i, j;
    for (i = d->pageIndexStart; i <= d->pageIndexStop; i++) {
        const PDFDocument::TextList &boxes = doc->textBoxesAtPage(i);
        for (j = ((i == d->pageIndexStart) ? d->boxIndexStart : 0);
             j < ((i == d->pageIndexStop) ? d->boxIndexStop : boxes.length());
             j++) {
            Poppler::TextBox *tbox = boxes.value(j).second;
            out += tbox->text() + (tbox->hasSpaceAfter() ? " " : "");
        }
    }
    const PDFDocument::TextList &boxes = doc->textBoxesAtPage(d->pageIndexStop);
    out += boxes.value(d->boxIndexStop).second->text();
    return out;
}
