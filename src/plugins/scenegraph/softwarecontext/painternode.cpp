/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Quick 2D Renderer module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "painternode.h"
#include "pixmaptexture.h"
#include <qmath.h>

QT_BEGIN_NAMESPACE

PainterNode::PainterNode(QQuickPaintedItem *item)
    : QSGPainterNode()
    , m_preferredRenderTarget(QQuickPaintedItem::Image)
    , m_actualRenderTarget(QQuickPaintedItem::Image)
    , m_item(item)
    , m_texture(0)
    , m_dirtyContents(false)
    , m_opaquePainting(false)
    , m_linear_filtering(false)
    , m_mipmapping(false)
    , m_smoothPainting(false)
    , m_extensionsChecked(false)
    , m_multisamplingSupported(false)
    , m_fastFBOResizing(false)
    , m_fillColor(Qt::transparent)
    , m_contentsScale(1.0)
    , m_dirtyGeometry(false)
{
    setMaterial((QSGMaterial*)1);
    setGeometry((QSGGeometry*)1);
}

PainterNode::~PainterNode()
{
    delete m_texture;
}

void PainterNode::setPreferredRenderTarget(QQuickPaintedItem::RenderTarget target)
{
    if (m_preferredRenderTarget == target)
        return;

    m_preferredRenderTarget = target;
}

void PainterNode::setSize(const QSize &size)
{
    if (size == m_size)
        return;

    m_size = size;

    m_dirtyGeometry = true;
}

void PainterNode::setDirty(const QRect &dirtyRect)
{
    m_dirtyContents = true;
    m_dirtyRect = dirtyRect;
    markDirty(DirtyMaterial);
}

void PainterNode::setOpaquePainting(bool opaque)
{
    if (opaque == m_opaquePainting)
        return;

    m_opaquePainting = opaque;
}

void PainterNode::setLinearFiltering(bool linearFiltering)
{
    if (linearFiltering == m_linear_filtering)
        return;

    m_linear_filtering = linearFiltering;
}

void PainterNode::setMipmapping(bool mipmapping)
{
    if (mipmapping == m_mipmapping)
        return;

    m_mipmapping = mipmapping;
}

void PainterNode::setSmoothPainting(bool s)
{
    if (s == m_smoothPainting)
        return;

    m_smoothPainting = s;
}

void PainterNode::setFillColor(const QColor &c)
{
    if (c == m_fillColor)
        return;

    m_fillColor = c;
    markDirty(DirtyMaterial);
}

void PainterNode::setContentsScale(qreal s)
{
    if (s == m_contentsScale)
        return;

    m_contentsScale = s;
    markDirty(DirtyMaterial);
}

void PainterNode::setFastFBOResizing(bool dynamic)
{
    m_fastFBOResizing = dynamic;
}

QImage PainterNode::toImage() const
{
    return m_pixmap.toImage();
}

void PainterNode::update()
{
    if (m_dirtyGeometry) {
        m_pixmap = QPixmap(m_textureSize);
        if (!m_opaquePainting)
            m_pixmap.fill(Qt::transparent);

        if (m_texture)
            delete m_texture;
        m_texture = new PixmapTexture(m_pixmap);
    }

    if (m_dirtyContents)
        paint();

    m_dirtyGeometry = false;
    m_dirtyContents = false;
}

void PainterNode::paint(QPainter *painter)
{
    painter->drawPixmap(0, 0, m_size.width(), m_size.height(), m_pixmap);
}

void PainterNode::paint()
{
    QRect dirtyRect = m_dirtyRect.isNull() ? QRect(0, 0, m_size.width(), m_size.height()) : m_dirtyRect;

    QPainter painter;

    painter.begin(&m_pixmap);
    if (m_smoothPainting) {
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    }

    QRect clipRect;

    if (m_contentsScale == 1) {
        qreal scaleX = m_textureSize.width() / (qreal) m_size.width();
        qreal scaleY = m_textureSize.height() / (qreal) m_size.height();
        painter.scale(scaleX, scaleY);
        clipRect = dirtyRect;
    } else {
        painter.scale(m_contentsScale, m_contentsScale);

        QRect sclip(qFloor(dirtyRect.x()/m_contentsScale),
                    qFloor(dirtyRect.y()/m_contentsScale),
                    qCeil(dirtyRect.width()/m_contentsScale+dirtyRect.x()/m_contentsScale-qFloor(dirtyRect.x()/m_contentsScale)),
                    qCeil(dirtyRect.height()/m_contentsScale+dirtyRect.y()/m_contentsScale-qFloor(dirtyRect.y()/m_contentsScale)));

        clipRect = sclip;
    }

    if (!m_dirtyRect.isNull())
        painter.setClipRect(clipRect);

    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(clipRect, m_fillColor);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    m_item->paint(&painter);
    painter.end();

    m_dirtyRect = QRect();
}


void PainterNode::setTextureSize(const QSize &size)
{
    if (size == m_textureSize)
        return;

    m_textureSize = size;
    m_dirtyGeometry = true;
}

QT_END_NAMESPACE
