/*
 * Copyright (C) 2015 Caliste Damien.
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

#ifndef PDFSELECTION_H
#define PDFSELECTION_H

#include <QtCore/QAbstractListModel>

#include "pdfcanvas.h"

class PDFSelection : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(PDFCanvas* canvas READ canvas WRITE setCanvas NOTIFY canvasChanged)
    Q_PROPERTY(QPointF start READ start WRITE setStart NOTIFY startChanged)
    Q_PROPERTY(QPointF stop READ stop WRITE setStop NOTIFY stopChanged)
    Q_PROPERTY(QString text READ text NOTIFY textChanged)
    Q_PROPERTY(float wiggle READ wiggle WRITE setWiggle NOTIFY wiggleChanged)

public:
    enum PDFSelectionRoles {
        Rect = Qt::UserRole + 1,
        Text
    };
    explicit PDFSelection(QObject *parent = 0);
    virtual ~PDFSelection();

    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual int rowCount(const QModelIndex &parent) const;
    virtual QHash<int, QByteArray> roleNames() const;

    int count() const;
    PDFCanvas* canvas() const;
    void setCanvas(PDFCanvas *newCanvas);
    float wiggle() const;
    void setWiggle(float newValue);

    /**
     * Change current selection to match the word that is at point in canvas coordinates.
     * If there is no word at point, the selection is invalidated (ie. count is set to
     * zero).
     */
    Q_INVOKABLE void selectAt(const QPointF &point);
    Q_INVOKABLE void unselect();

    /**
     * Return a point for the start handle of the selection in canvas coordinates.
     */
    QPointF start() const;
    /**
     * Change the start of the selection to the start of the word at point.
     * point is given in canvas coordinates. If there is no word at point,
     * the selection is left unchanged.
     */
    void setStart(const QPointF &point);

    /**
     * Return a point for the stop handle of the selection in canvas coordinates.
     */
    QPointF stop() const;
    /**
     * Change the stop of the selection to the end of the word at point.
     * point is given in canvas coordinates. If there is no word at point,
     * the selection is left unchanged.
     */
    void setStop(const QPointF &point);

    QString text() const;

Q_SIGNALS:
    void countChanged();
    void canvasChanged();
    void startChanged();
    void stopChanged();
    void textChanged();
    void wiggleChanged();

private:
    class Private;
    Private * const d;

    void onLayoutChanged();
};

#endif // PDFSELECTION_H
