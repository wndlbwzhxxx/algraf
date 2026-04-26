#include "graphwidget.h"
#include <QMouseEvent>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QtMath>
#include <limits>
#include <QJsonDocument>
#include <QInputDialog>
#include <QDebug>
#include <QCoreApplication>

const int INF = std::numeric_limits<int>::max() / 2;

Vertex::Vertex(int id, const QPointF& pos, QGraphicsItem* parent)
    : QGraphicsEllipseItem(parent), m_id(id), m_dragging(false)
{
    setRect(-20, -20, 40, 40);
    setPos(pos);
    m_originalColor = QColor(0, 150, 200);
    setBrush(QBrush(m_originalColor));
    setPen(QPen(Qt::white, 2));
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setAcceptHoverEvents(true);
    setZValue(1);

    m_label = new QGraphicsTextItem(QString::number(id), this);
    m_label->setDefaultTextColor(Qt::white);
    m_label->setFont(QFont("Arial", 10, QFont::Bold));
    m_label->setZValue(2);

    m_distanceLabel = new QGraphicsTextItem("", this);
    m_distanceLabel->setDefaultTextColor(Qt::yellow);
    m_distanceLabel->setFont(QFont("Arial", 8));
    m_distanceLabel->setZValue(2);

    updateLabel();
}

void Vertex::setColor(const QColor& color) {
    setBrush(QBrush(color));
}

void Vertex::setText(const QString& text) {
    m_label->setPlainText(text);
    updateLabel();
}

void Vertex::setDistance(int dist) {
    if (dist == INF || dist == 0)
        m_distanceLabel->setPlainText("");
    else
        m_distanceLabel->setPlainText(QString::number(dist));
    updateLabel();
}

void Vertex::resetAppearance() {
    setBrush(QBrush(m_originalColor));
    setPen(QPen(Qt::white, 2));
    m_label->setDefaultTextColor(Qt::white);
    m_distanceLabel->setPlainText("");
    updateLabel();
}

void Vertex::updateLabel() {
    QRectF rect = boundingRect();
    m_label->setPos(rect.center().x() - m_label->boundingRect().width() / 2,
                    rect.center().y() - m_label->boundingRect().height() / 2 - 5);
    m_distanceLabel->setPos(rect.center().x() - m_distanceLabel->boundingRect().width() / 2,
                            rect.center().y() + 5);
}

void Vertex::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_dragStart = pos();
        m_dragging = true;
        event->accept();
    }
    QGraphicsEllipseItem::mousePressEvent(event);
}

void Vertex::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    if (m_dragging) {
        QPointF newPos = pos() + event->pos() - event->lastPos();
        setPos(newPos);
        event->accept();
    }
    QGraphicsEllipseItem::mouseMoveEvent(event);
}

void Vertex::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton && m_dragging) {
        m_dragging = false;
        if (scene()) {
            GraphWidget* gw = qobject_cast<GraphWidget*>(scene()->views().first());
            if (gw) gw->updateEdgesGeometry();
        }
        event->accept();
    }
    QGraphicsEllipseItem::mouseReleaseEvent(event);
}

void Vertex::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    setPen(QPen(Qt::yellow, 3));
    QGraphicsEllipseItem::hoverEnterEvent(event);
}

void Vertex::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    setPen(QPen(Qt::white, 2));
    QGraphicsEllipseItem::hoverLeaveEvent(event);
}

GraphWidget::GraphWidget(QWidget* parent)
    : QGraphicsView(parent),
      m_nextVertexId(1),
      m_mode(ModeMove),
      m_directedMode(false),
      m_tempEdgeSource(nullptr),
      m_tempEdgeWeight(1),
      m_currentAlgorithm(AlgoBFS),
      m_animationTimer(nullptr),
      m_animationSpeedMs(1000),
      m_isRunning(false),
      m_isPaused(false),
      m_currentStepIndex(-1),
      m_startVertexId(-1),
      m_endVertexId(-1),
      m_bfIteration(0),
      m_kruskalIndex(0),
      m_sccPhase(0)
{
    m_scene = new QGraphicsScene(this);
    m_scene->setSceneRect(0, 0, 800, 600);
    m_scene->setBackgroundBrush(QColor(30, 30, 40));
    setScene(m_scene);
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);

    setMouseTracking(true);
    viewport()->setMouseTracking(true);

    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, &QTimer::timeout, this, &GraphWidget::animateStep);
    setAnimationSpeed(1000);
}

GraphWidget::~GraphWidget() {
    clearGraph();
}

void GraphWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        QPointF scenePos = mapToScene(event->pos());

        switch (m_mode) {
            case ModeAddVertex:
                addVertex(scenePos);
                emit graphChanged();
                break;

            case ModeAddEdge: {
                Vertex* clickedVertex = nullptr;
                for (Vertex* v : m_vertices) {
                    if (v->contains(v->mapFromScene(scenePos))) {
                        clickedVertex = v;
                        break;
                    }
                }

                if (clickedVertex) {
                    if (m_tempEdgeSource == nullptr) {
                        m_tempEdgeSource = clickedVertex;
                        highlightVertex(clickedVertex->getId(), Qt::green);
                        logStep("First vertex selected: " + QString::number(clickedVertex->getId()),
                                "Click on another vertex to create an edge");
                    } else if (m_tempEdgeSource != clickedVertex) {
                        int weight = m_tempEdgeWeight;
                        addEdge(m_tempEdgeSource->getId(), clickedVertex->getId(), weight);
                        resetColors();
                        m_tempEdgeSource = nullptr;
                        emit graphChanged();
                        logStep("Edge creation completed", "Edge added to the graph");
                    } else {
                        resetColors();
                        m_tempEdgeSource = nullptr;
                    }
                }
                break;
            }

            case ModeRemove: {
                Edge* selectedEdge = nullptr;
                double minDistance = 15.0;

                for (Edge* e : m_edges) {
                    Vertex* from = m_vertices.value(e->fromId);
                    Vertex* to = m_vertices.value(e->toId);
                    if (!from || !to) continue;

                    QPointF p1 = from->pos();
                    QPointF p2 = to->pos();
                    QPointF clickPos = scenePos;
                    QPointF lineVec = p2 - p1;
                    double lineLen = sqrt(lineVec.x() * lineVec.x() + lineVec.y() * lineVec.y());

                    if (lineLen < 0.001) continue;

                    QPointF clickVec = clickPos - p1;
                    double t = (clickVec.x() * lineVec.x() + clickVec.y() * lineVec.y()) / (lineLen * lineLen);
                    t = qMax(0.0, qMin(1.0, t));

                    QPointF closestPoint = p1 + t * lineVec;
                    double distance = sqrt(pow(clickPos.x() - closestPoint.x(), 2) +
                                           pow(clickPos.y() - closestPoint.y(), 2));

                    if (distance < minDistance) {
                        minDistance = distance;
                        selectedEdge = e;
                    }
                }

                if (selectedEdge) {
                    for (int i = 0; i < m_edges.size(); ++i) {
                        if (m_edges[i] == selectedEdge) {
                            Edge* e = m_edges[i];
                            m_scene->removeItem(e->lineItem);
                            if (e->weightItem) m_scene->removeItem(e->weightItem);
                            if (e->arrowItem) m_scene->removeItem(e->arrowItem);
                            delete e->lineItem;
                            delete e->weightItem;
                            delete e->arrowItem;
                            logStep("Removed edge " + QString::number(e->fromId) + " -> " +
                                    QString::number(e->toId), "Edge removed from graph");
                            delete e;
                            m_edges.removeAt(i);
                            emit graphChanged();
                            break;
                        }
                    }
                } else {
                    Vertex* clickedVertex = nullptr;
                    for (Vertex* v : m_vertices) {
                        if (v->contains(v->mapFromScene(scenePos))) {
                            clickedVertex = v;
                            break;
                        }
                    }

                    if (clickedVertex) {
                        int vid = clickedVertex->getId();
                        for (int i = m_edges.size() - 1; i >= 0; --i) {
                            Edge* e = m_edges[i];
                            if (e->fromId == vid || e->toId == vid) {
                                m_scene->removeItem(e->lineItem);
                                if (e->weightItem) m_scene->removeItem(e->weightItem);
                                if (e->arrowItem) m_scene->removeItem(e->arrowItem);
                                delete e->lineItem;
                                delete e->weightItem;
                                delete e->arrowItem;
                                delete e;
                                m_edges.removeAt(i);
                            }
                        }
                        m_scene->removeItem(clickedVertex);
                        m_vertices.remove(vid);
                        delete clickedVertex;
                        logStep("Removed vertex " + QString::number(vid), "Vertex and all its edges removed");
                        emit graphChanged();
                    }
                }
                break;
            }

            case ModeDeleteEdge: {
                Edge* selectedEdge = nullptr;
                double minDistance = 15.0;

                for (Edge* e : m_edges) {
                    Vertex* from = m_vertices.value(e->fromId);
                    Vertex* to = m_vertices.value(e->toId);
                    if (!from || !to) continue;

                    QPointF p1 = from->pos();
                    QPointF p2 = to->pos();
                    QPointF clickPos = scenePos;
                    QPointF lineVec = p2 - p1;
                    double lineLen = sqrt(lineVec.x() * lineVec.x() + lineVec.y() * lineVec.y());

                    if (lineLen < 0.001) continue;

                    QPointF clickVec = clickPos - p1;
                    double t = (clickVec.x() * lineVec.x() + clickVec.y() * lineVec.y()) / (lineLen * lineLen);
                    t = qMax(0.0, qMin(1.0, t));

                    QPointF closestPoint = p1 + t * lineVec;
                    double distance = sqrt(pow(clickPos.x() - closestPoint.x(), 2) +
                                           pow(clickPos.y() - closestPoint.y(), 2));

                    if (distance < minDistance) {
                        minDistance = distance;
                        selectedEdge = e;
                    }
                }

                if (selectedEdge) {
                    for (int i = 0; i < m_edges.size(); ++i) {
                        if (m_edges[i] == selectedEdge) {
                            Edge* e = m_edges[i];
                            m_scene->removeItem(e->lineItem);
                            if (e->weightItem) m_scene->removeItem(e->weightItem);
                            if (e->arrowItem) m_scene->removeItem(e->arrowItem);
                            delete e->lineItem;
                            delete e->weightItem;
                            delete e->arrowItem;
                            logStep("Removed edge " + QString::number(e->fromId) + " -> " +
                                    QString::number(e->toId), "Edge removed from graph");
                            delete e;
                            m_edges.removeAt(i);
                            emit graphChanged();
                            break;
                        }
                    }
                }
                break;
            }

            case ModeEditEdge: {
                Edge* selectedEdge = nullptr;
                double minDistance = 15.0;

                for (Edge* e : m_edges) {
                    Vertex* from = m_vertices.value(e->fromId);
                    Vertex* to = m_vertices.value(e->toId);
                    if (!from || !to) continue;

                    QPointF p1 = from->pos();
                    QPointF p2 = to->pos();
                    QPointF clickPos = scenePos;
                    QPointF lineVec = p2 - p1;
                    double lineLen = sqrt(lineVec.x() * lineVec.x() + lineVec.y() * lineVec.y());

                    if (lineLen < 0.001) continue;

                    QPointF clickVec = clickPos - p1;
                    double t = (clickVec.x() * lineVec.x() + clickVec.y() * lineVec.y()) / (lineLen * lineLen);
                    t = qMax(0.0, qMin(1.0, t));

                    QPointF closestPoint = p1 + t * lineVec;
                    double distance = sqrt(pow(clickPos.x() - closestPoint.x(), 2) +
                                           pow(clickPos.y() - closestPoint.y(), 2));

                    if (distance < minDistance) {
                        minDistance = distance;
                        selectedEdge = e;
                    }
                }

                if (selectedEdge) {
                    bool ok;
                    int newWeight = QInputDialog::getInt(nullptr, "Edit Edge Weight",
                        QString("Enter new weight for edge %1 -> %2:").arg(selectedEdge->fromId).arg(selectedEdge->toId),
                        selectedEdge->weight, 1, 9999, 1, &ok);
                    if (ok && newWeight != selectedEdge->weight) {
                        selectedEdge->weight = newWeight;
                        selectedEdge->weightItem->setPlainText(QString::number(newWeight));
                        logStep("Updated edge " + QString::number(selectedEdge->fromId) + " -> " +
                                QString::number(selectedEdge->toId) + " weight to " + QString::number(newWeight),
                                "Edge weight changed");
                        emit graphChanged();
                    }
                }
                break;
            }

            case ModeMove:
                break;
        }
    }

    QGraphicsView::mousePressEvent(event);
}

void GraphWidget::setMode(InteractionMode mode) {
    m_mode = mode;
    m_tempEdgeSource = nullptr;
    resetColors();
    if (m_mode == ModeMove) {
        setDragMode(QGraphicsView::ScrollHandDrag);
        viewport()->setCursor(Qt::OpenHandCursor);
    } else {
        setDragMode(QGraphicsView::NoDrag);
        if (m_mode == ModeAddVertex)
            viewport()->setCursor(Qt::CrossCursor);
        else if (m_mode == ModeRemove || m_mode == ModeDeleteEdge)
            viewport()->setCursor(Qt::PointingHandCursor);
        else if (m_mode == ModeEditEdge)
            viewport()->setCursor(Qt::PointingHandCursor);
        else
            viewport()->setCursor(Qt::ArrowCursor);
    }
}

void GraphWidget::setDirectedMode(bool directed) {
    m_directedMode = directed;
    updateEdgesGeometry();
    emit graphChanged();
}

void GraphWidget::addVertex(const QPointF& pos) {
    Vertex* v = new Vertex(m_nextVertexId, pos);
    m_scene->addItem(v);
    m_vertices[m_nextVertexId] = v;
    m_nextVertexId++;
    logStep("Added vertex " + QString::number(m_nextVertexId - 1),
            "New vertex created on the scene");
}

void GraphWidget::addEdge(int fromId, int toId, int weight) {
    if (fromId == toId) return;
    if (findEdge(fromId, toId)) return;

    Vertex* from = m_vertices.value(fromId);
    Vertex* to = m_vertices.value(toId);
    if (!from || !to) return;

    Edge* edge = new Edge;
    edge->fromId = fromId;
    edge->toId = toId;
    edge->weight = weight;
    edge->directed = m_directedMode;

    QLineF line(from->pos(), to->pos());
    edge->lineItem = new QGraphicsLineItem(line);
    edge->lineItem->setPen(QPen(Qt::white, 2));
    edge->lineItem->setZValue(-1);

    edge->weightItem = new QGraphicsTextItem(QString::number(weight));
    edge->weightItem->setDefaultTextColor(Qt::cyan);
    edge->weightItem->setFont(QFont("Arial", 9, QFont::Bold));
    edge->weightItem->setZValue(-1);

    edge->arrowItem = nullptr;

    m_scene->addItem(edge->lineItem);
    m_scene->addItem(edge->weightItem);
    m_edges.append(edge);

    updateEdgesGeometry();
    logStep("Added edge " + QString::number(fromId) + " -> " + QString::number(toId) +
            " (weight: " + QString::number(weight) + ", " +
            (edge->directed ? "directed" : "undirected") + ")",
            "New edge added to the graph");
}

void GraphWidget::editEdgeWeight(int fromId, int toId, int newWeight)
{
    Edge* e = findEdge(fromId, toId);
    if (e) {
        e->weight = newWeight;
        e->weightItem->setPlainText(QString::number(newWeight));
        updateEdgesGeometry();
        logStep("Updated edge " + QString::number(fromId) + " -> " +
                QString::number(toId) + " weight to " + QString::number(newWeight),
                "Edge weight changed");
        emit graphChanged();
    }
}

void GraphWidget::removeItemAt(const QPointF& pos) {
    QGraphicsItem* item = m_scene->itemAt(pos, QTransform());
    if (!item) return;

    Vertex* v = dynamic_cast<Vertex*>(item);
    if (v) {
        int vid = v->getId();
        for (int i = m_edges.size() - 1; i >= 0; --i) {
            Edge* e = m_edges[i];
            if (e->fromId == vid || e->toId == vid) {
                m_scene->removeItem(e->lineItem);
                if (e->weightItem) m_scene->removeItem(e->weightItem);
                if (e->arrowItem) m_scene->removeItem(e->arrowItem);
                delete e->lineItem;
                delete e->weightItem;
                delete e->arrowItem;
                delete e;
                m_edges.removeAt(i);
            }
        }
        m_scene->removeItem(v);
        m_vertices.remove(vid);
        delete v;
        logStep("Removed vertex " + QString::number(vid), "Vertex and all its edges removed");
        return;
    }

    QGraphicsLineItem* line = dynamic_cast<QGraphicsLineItem*>(item);
    if (line) {
        for (int i = 0; i < m_edges.size(); ++i) {
            if (m_edges[i]->lineItem == line) {
                Edge* e = m_edges[i];
                m_scene->removeItem(e->lineItem);
                if (e->weightItem) m_scene->removeItem(e->weightItem);
                if (e->arrowItem) m_scene->removeItem(e->arrowItem);
                delete e->lineItem;
                delete e->weightItem;
                delete e->arrowItem;
                logStep("Removed edge " + QString::number(e->fromId) + " -> " +
                        QString::number(e->toId), "Edge removed from graph");
                delete e;
                m_edges.removeAt(i);
                break;
            }
        }
    }
}

void GraphWidget::clearGraph() {
    resetAlgorithm();
    m_scene->clear();
    m_vertices.clear();
    m_edges.clear();
    m_nextVertexId = 1;
    m_startVertexId = -1;
    m_endVertexId = -1;
    logStep("Graph cleared", "All vertices and edges removed");
}

void GraphWidget::createDefaultGraph() {
    clearGraph();
    m_directedMode = false;

    addVertex(QPointF(150, 250));
    addVertex(QPointF(300, 150));
    addVertex(QPointF(450, 200));
    addVertex(QPointF(350, 350));
    addVertex(QPointF(550, 350));
    addVertex(QPointF(650, 250));

    addEdge(1, 2, 4);
    addEdge(1, 4, 7);
    addEdge(2, 3, 2);
    addEdge(2, 4, 1);
    addEdge(3, 4, 5);
    addEdge(3, 6, 8);
    addEdge(4, 5, 3);
    addEdge(5, 6, 2);
    addEdge(5, 2, 6);

    setStartVertex(1);
    setEndVertex(6);

    logStep("Default graph created", "Sample graph with 6 vertices and 9 edges loaded");
}

void GraphWidget::updateEdgesGeometry() {
    for (Edge* e : m_edges) {
        Vertex* from = m_vertices.value(e->fromId);
        Vertex* to = m_vertices.value(e->toId);
        if (!from || !to) continue;

        QPointF p1 = from->pos();
        QPointF p2 = to->pos();
        QLineF line(p1, p2);
        double angle = line.angle();
        double offset = 22;

        QPointF start(p1.x() + offset * qCos(qDegreesToRadians(angle)),
                      p1.y() + offset * qSin(qDegreesToRadians(angle)));
        QPointF end(p2.x() - offset * qCos(qDegreesToRadians(angle)),
                    p2.y() - offset * qSin(qDegreesToRadians(angle)));

        e->lineItem->setLine(QLineF(start, end));

        QPointF mid = (start + end) / 2;
        e->weightItem->setPos(mid);

        updateArrow(e);
    }
}

void GraphWidget::updateArrow(Edge* edge) {
    Vertex* from = m_vertices.value(edge->fromId);
    Vertex* to = m_vertices.value(edge->toId);
    if (!from || !to) return;

    if (edge->arrowItem) {
        m_scene->removeItem(edge->arrowItem);
        delete edge->arrowItem;
        edge->arrowItem = nullptr;
    }

    if (!edge->directed) {
        return;
    }

    QPointF fromPos = from->pos();
    QPointF toPos = to->pos();

    QPointF direction = toPos - fromPos;
    double length = sqrt(direction.x() * direction.x() + direction.y() * direction.y());
    if (length < 0.001) return;

    QPointF dirNorm = direction / length;
    double offset = 22;
    QPointF start = fromPos + dirNorm * offset;
    QPointF end = toPos - dirNorm * offset;
    double t = 0.7;
    QPointF arrowPos = start + (end - start) * t;
    double arrowSize = 12;
    QPointF perp(-dirNorm.y(), dirNorm.x());
    QPointF tip = arrowPos;
    QPointF left = tip - dirNorm * arrowSize + perp * (arrowSize / 2);
    QPointF right = tip - dirNorm * arrowSize - perp * (arrowSize / 2);

    QPainterPath arrowPath;
    arrowPath.moveTo(tip);
    arrowPath.lineTo(left);
    arrowPath.lineTo(right);
    arrowPath.lineTo(tip);
    arrowPath.closeSubpath();

    edge->arrowItem = new QGraphicsPathItem(arrowPath);
    edge->arrowItem->setPen(QPen(Qt::white, 1.5));
    edge->arrowItem->setBrush(QBrush(Qt::white));
    edge->arrowItem->setZValue(1);
    m_scene->addItem(edge->arrowItem);
}

Edge* GraphWidget::findEdge(int from, int to) const {
    for (Edge* e : m_edges) {
        if (e->directed) {
            if (e->fromId == from && e->toId == to) return e;
        } else {
            if ((e->fromId == from && e->toId == to) ||
                (e->fromId == to && e->toId == from)) return e;
        }
    }
    return nullptr;
}

QList<Edge*> GraphWidget::getIncidentEdges(int vertexId) const {
    QList<Edge*> edges;
    for (Edge* e : m_edges) {
        if (e->directed) {
            if (e->fromId == vertexId) {
                edges.append(e);
            }
        } else {
            if (e->fromId == vertexId || e->toId == vertexId) {
                edges.append(e);
            }
        }
    }
    return edges;
}

Vertex* GraphWidget::findVertex(int id) const {
    return m_vertices.value(id, nullptr);
}

void GraphWidget::highlightVertex(int id, const QColor& color) {
    if (Vertex* v = findVertex(id)) v->setColor(color);
}

void GraphWidget::highlightEdge(int from, int to, const QColor& color) {
    Edge* e = findEdge(from, to);
    if (e && e->lineItem) e->lineItem->setPen(QPen(color, 3));
}

void GraphWidget::resetColors() {
    for (Vertex* v : m_vertices) {
        v->resetAppearance();
    }
    for (Edge* e : m_edges) {
        e->lineItem->setPen(QPen(Qt::white, 2));
        if (e->arrowItem) {
            e->arrowItem->setPen(QPen(Qt::white, 1.5));
            e->arrowItem->setBrush(QBrush(Qt::white));
        }
    }
}

void GraphWidget::clearHighlights() {
    resetColors();
}

void GraphWidget::logStep(const QString& structures, const QString& explanation) {
    addAnimationStep(structures, explanation);
}

void GraphWidget::addAnimationStep(const QString& structures, const QString& explanation) {
    m_animationSteps.append(qMakePair(structures, explanation));
    if (!m_isRunning || m_isPaused) {
        emit stateUpdated(structures, explanation);
    }
}

void GraphWidget::restoreStepState(int stepIndex) {
    if (stepIndex < 0 || stepIndex >= m_animationSteps.size()) return;
    resetColors();
    const auto& step = m_animationSteps[stepIndex];
    emit stateUpdated(step.first, step.second);
}

void GraphWidget::highlightPath(const QList<int>& path, const QColor& color)
{
    if (path.size() < 2) return;

    for (int vertexId : path) {
        if (Vertex* v = findVertex(vertexId)) {
            v->setColor(color);
            v->setText(QString::number(vertexId));
        }
    }

    for (int i = 0; i < path.size() - 1; i++) {
        int from = path[i];
        int to = path[i + 1];

        Edge* e = findEdge(from, to);
        if (e && e->lineItem) {
            e->lineItem->setPen(QPen(color, 4));
            if (e->arrowItem) {
                e->arrowItem->setPen(QPen(color, 2));
                e->arrowItem->setBrush(QBrush(color));
            }
        }
    }

    m_scene->update();
    QCoreApplication::processEvents();
}

void GraphWidget::initBFS() {
    if (m_startVertexId == -1) {
        logStep("Error", "Start vertex not set");
        m_isRunning = false;
        return;
    }

    resetColors();
    m_visited.clear();
    m_parent.clear();
    m_bfsQueue.clear();

    m_bfsQueue.enqueue(m_startVertexId);
    m_visited.insert(m_startVertexId);
    m_parent[m_startVertexId] = -1;
    highlightVertex(m_startVertexId, Qt::green);

    if (m_endVertexId != -1 && m_startVertexId == m_endVertexId) {
        QString result = "Start vertex is the same as target vertex. Path found!";
        addAnimationStep(result, "Target vertex reached immediately!");
        highlightVertex(m_startVertexId, Qt::red);
        emit algorithmFinished(result, true);
        m_isRunning = false;
        return;
    }

    addAnimationStep("Queue: [" + QString::number(m_startVertexId) + "]\nVisited: {" +
                     QString::number(m_startVertexId) + "}",
                     "Starting BFS from vertex " + QString::number(m_startVertexId));
}

bool GraphWidget::executeBFSStep() {
    if (m_bfsQueue.isEmpty()) {
        QString result = "BFS completed. Target vertex " + QString::number(m_endVertexId) + " NOT found!";
        addAnimationStep(result, "Target vertex is not reachable from start vertex");
        emit algorithmFinished(result, false);
        return false;
    }

    int current = m_bfsQueue.dequeue();
    highlightVertex(current, Qt::yellow);
    addAnimationStep("Processing vertex: " + QString::number(current),
                     "Currently exploring vertex " + QString::number(current));

    if (m_endVertexId != -1 && current == m_endVertexId) {
        QList<int> path;
        int cur = current;
        while (cur != -1) {
            path.prepend(cur);
            cur = m_parent[cur];
        }

        highlightPath(path, Qt::red);

        QString pathStr;
        for (int p : path) pathStr += QString::number(p) + " -> ";
        pathStr.chop(4);

        QString result = "BFS completed. Target vertex " + QString::number(m_endVertexId) + " found!\nPath: " + pathStr;
        addAnimationStep(result, "Target vertex reached! Path found.");
        emit algorithmFinished(result, true);
        return false;
    }

    QList<int> newNeighbors;
    QList<Edge*> edges = getIncidentEdges(current);

    for (Edge* e : edges) {
        int neighbor = (e->fromId == current) ? e->toId : e->fromId;
        if (!m_visited.contains(neighbor)) {
            newNeighbors.append(neighbor);
            m_visited.insert(neighbor);
            m_parent[neighbor] = current;
            m_bfsQueue.enqueue(neighbor);
        }
    }

    if (!newNeighbors.isEmpty()) {
        QString neighborsStr;
        for (int neighbor : newNeighbors) {
            highlightVertex(neighbor, Qt::green);
            highlightEdge(current, neighbor, Qt::blue);
            neighborsStr += QString::number(neighbor) + " ";
        }

        addAnimationStep("Vertex " + QString::number(current) + " found neighbors: " + neighborsStr,
                         "New vertices discovered from vertex " + QString::number(current));
    } else {
        addAnimationStep("Vertex " + QString::number(current) + " has no new neighbors",
                         "No undiscovered vertices adjacent to vertex " + QString::number(current));
    }

    highlightVertex(current, Qt::cyan);

    QString queueStr;
    for (int v : m_bfsQueue) queueStr += QString::number(v) + " ";
    addAnimationStep("Queue: [" + queueStr + "]",
                     "Vertices waiting to be processed");

    return true;
}

void GraphWidget::initDFS() {
    if (m_startVertexId == -1) {
        logStep("Error", "Start vertex not set. Please select a start vertex from the dropdown.");
        m_isRunning = false;
        return;
    }

    resetColors();
    m_visited.clear();
    m_parent.clear();
    m_dfsStack.clear();

    m_dfsStack.push(m_startVertexId);
    m_visited.insert(m_startVertexId);
    m_parent[m_startVertexId] = -1;
    highlightVertex(m_startVertexId, Qt::green);
    m_scene->update();
    QCoreApplication::processEvents();

    if (m_endVertexId != -1 && m_startVertexId == m_endVertexId) {
        QString result = "Start vertex " + QString::number(m_startVertexId) +
                         " is the same as target vertex. Path found!";
        addAnimationStep(result, "Target vertex reached immediately!");
        highlightVertex(m_startVertexId, Qt::red);
        emit algorithmFinished(result, true);
        m_isRunning = false;
        return;
    }

    QString targetInfo = "";
    if (m_endVertexId != -1) {
        targetInfo = " to find vertex " + QString::number(m_endVertexId);
    } else {
        targetInfo = " (no target specified, will visit all vertices)";
    }

    addAnimationStep("Stack: [" + QString::number(m_startVertexId) + "]\nVisited: {" +
                     QString::number(m_startVertexId) + "}",
                     "Starting DFS from vertex " + QString::number(m_startVertexId) + targetInfo);
}

bool GraphWidget::executeDFSStep() {
    if (m_dfsStack.isEmpty()) {
        QString result = "DFS completed. Target vertex " + QString::number(m_endVertexId) + " NOT found!";
        addAnimationStep(result, "Target vertex is not reachable from start vertex");
        emit algorithmFinished(result, false);
        return false;
    }

    int current = m_dfsStack.pop();
    highlightVertex(current, Qt::yellow);
    m_scene->update();
    QCoreApplication::processEvents();

    addAnimationStep("Processing vertex: " + QString::number(current),
                     "Currently exploring vertex " + QString::number(current));

    if (m_endVertexId != -1 && current == m_endVertexId) {
        QList<int> path;
        int cur = current;
        while (cur != -1) {
            path.prepend(cur);
            cur = m_parent[cur];
        }

        highlightPath(path, Qt::red);

        QString pathStr;
        for (int p : path) pathStr += QString::number(p) + " -> ";
        pathStr.chop(4);

        QString result = "DFS completed. Target vertex " + QString::number(m_endVertexId) + " found!\nPath: " + pathStr;
        addAnimationStep(result, "Target vertex reached! Path found.");
        emit algorithmFinished(result, true);
        return false;
    }

    QList<Edge*> edges = getIncidentEdges(current);
    int nextNeighbor = -1;
    QList<int> neighbors;
    for (Edge* e : edges) {
        int neighbor = (e->fromId == current) ? e->toId : e->fromId;
        if (!m_visited.contains(neighbor)) {
            neighbors.append(neighbor);
        }
    }
    std::sort(neighbors.begin(), neighbors.end());

    if (!neighbors.isEmpty()) {
        nextNeighbor = neighbors.first();
    }

    if (nextNeighbor != -1) {
        m_visited.insert(nextNeighbor);
        m_parent[nextNeighbor] = current;
        m_dfsStack.push(current);
        m_dfsStack.push(nextNeighbor);
        highlightVertex(nextNeighbor, Qt::green);
        highlightEdge(current, nextNeighbor, Qt::blue);
        m_scene->update();
        QCoreApplication::processEvents();

        addAnimationStep("Moving from vertex " + QString::number(current) +
                         " to vertex " + QString::number(nextNeighbor),
                         "DFS goes deeper to vertex " + QString::number(nextNeighbor));

        QString stackStr;
        QStack<int> tempStack = m_dfsStack;
        QList<int> stackList;
        while (!tempStack.isEmpty()) {
            stackList.prepend(tempStack.pop());
        }
        for (int v : stackList) {
            stackStr += QString::number(v) + " ";
        }
        addAnimationStep("Stack: [" + stackStr + "]",
                         "Vertices waiting to be processed (top is next)");
    } else {
        addAnimationStep("Backtracking from vertex " + QString::number(current),
                         "No unvisited neighbors, returning to previous vertex");

        QString stackStr;
        QStack<int> tempStack = m_dfsStack;
        QList<int> stackList;
        while (!tempStack.isEmpty()) {
            stackList.prepend(tempStack.pop());
        }
        for (int v : stackList) {
            stackStr += QString::number(v) + " ";
        }
        addAnimationStep("Stack: [" + stackStr + "]",
                         "Backtracking complete");
    }

    highlightVertex(current, Qt::cyan);
    m_scene->update();
    QCoreApplication::processEvents();

    return true;
}

void GraphWidget::initDijkstra() {
    if (m_startVertexId == -1) {
        logStep("Error", "Start vertex not set");
        m_isRunning = false;
        return;
    }

    m_dist.clear();
    m_processed.clear();
    m_parent.clear();

    for (int id : m_vertices.keys()) {
        m_dist[id] = INF;
    }
    m_dist[m_startVertexId] = 0;
    m_parent[m_startVertexId] = -1;

    highlightVertex(m_startVertexId, Qt::green);
    findVertex(m_startVertexId)->setDistance(0);

    QString distStr;
    for (int id : m_vertices.keys()) {
        distStr += QString::number(id) + ":" + (m_dist[id] == INF ? "INF" : QString::number(m_dist[id])) + " ";
    }

    addAnimationStep("Initialization:\n" + distStr +
                     "\nProcessed: {}",
                     "Dijkstra's algorithm. Distance to start vertex = 0, others = INF");
}

bool GraphWidget::executeDijkstraStep() {
    int current = -1;
    int minDist = INF;
    for (int id : m_vertices.keys()) {
        if (!m_processed.contains(id) && m_dist[id] < minDist) {
            minDist = m_dist[id];
            current = id;
        }
    }

    if (current == -1 || m_dist[current] == INF) {
        QString result;
        bool success = false;
        if (m_endVertexId != -1 && m_dist[m_endVertexId] != INF) {
            result = "Dijkstra's algorithm completed. Distance to vertex " + QString::number(m_endVertexId) +
                     ": " + QString::number(m_dist[m_endVertexId]);
            success = true;
        } else if (m_endVertexId != -1) {
            result = "Dijkstra's algorithm completed. Target vertex " + QString::number(m_endVertexId) + " is NOT reachable!";
            success = false;
        } else {
            result = "Dijkstra's algorithm completed. All distances calculated.";
            success = true;
        }
        addAnimationStep(result, success ? "Shortest paths found" : "Target vertex unreachable");
        reconstructAndHighlightPath();
        emit algorithmFinished(result, success);
        return false;
    }

    m_processed.insert(current);
    highlightVertex(current, Qt::yellow);

    QList<Edge*> edges = getIncidentEdges(current);
    bool distanceUpdated = false;

    for (Edge* e : edges) {
        int neighbor = (e->fromId == current) ? e->toId : e->fromId;
        if (!m_processed.contains(neighbor)) {
            int newDist = m_dist[current] + e->weight;
            if (newDist < m_dist[neighbor]) {
                m_dist[neighbor] = newDist;
                m_parent[neighbor] = current;
                findVertex(neighbor)->setDistance(newDist);
                highlightEdge(current, neighbor, Qt::blue);
                distanceUpdated = true;
            }
        }
    }

    QString distStr;
    for (int id : m_vertices.keys()) {
        distStr += QString::number(id) + ":" + (m_dist[id] == INF ? "INF" : QString::number(m_dist[id])) + " ";
    }
    QString processedStr;
    for (int id : m_processed) processedStr += QString::number(id) + " ";

    if (distanceUpdated) {
        addAnimationStep("Processed vertex: " + QString::number(current) +
                         "\nDistances: " + distStr +
                         "\nProcessed: {" + processedStr + "}",
                         "Vertex " + QString::number(current) + " processed. Distances updated");
    } else {
        addAnimationStep("Processed vertex: " + QString::number(current) +
                         "\nDistances: " + distStr +
                         "\nProcessed: {" + processedStr + "}",
                         "Vertex " + QString::number(current) + " processed. No changes");
    }

    highlightVertex(current, Qt::cyan);
    return true;
}

void GraphWidget::initAStar() {
    if (m_startVertexId == -1 || m_endVertexId == -1) {
        logStep("Error", "Start and/or end vertices not set");
        m_isRunning = false;
        return;
    }

    Vertex* end = findVertex(m_endVertexId);
    if (end) {
        QPointF endPos = end->pos();
        for (Vertex* v : m_vertices) {
            QPointF delta = v->pos() - endPos;
            m_heuristic[v->getId()] = static_cast<int>(sqrt(delta.x() * delta.x() + delta.y() * delta.y()) / 5);
        }
    }

    m_dist.clear();
    m_processed.clear();
    m_parent.clear();

    for (int id : m_vertices.keys()) {
        m_dist[id] = INF;
    }
    m_dist[m_startVertexId] = 0;
    m_parent[m_startVertexId] = -1;

    highlightVertex(m_startVertexId, Qt::green);
    findVertex(m_startVertexId)->setDistance(0);

    addAnimationStep("A* initialization\nStart: " + QString::number(m_startVertexId) +
                     "\nGoal: " + QString::number(m_endVertexId),
                     "A* algorithm with heuristic (Euclidean distance)");
}

bool GraphWidget::executeAStarStep() {
    int current = -1;
    int minF = INF;
    for (int id : m_vertices.keys()) {
        if (!m_processed.contains(id) && m_dist[id] < INF) {
            int f = m_dist[id] + m_heuristic.value(id, 0);
            if (f < minF) {
                minF = f;
                current = id;
            }
        }
    }

    if (current == -1) {
        QString result = "A* completed. Path to vertex " + QString::number(m_endVertexId) + " NOT found!";
        addAnimationStep(result, "Target vertex unreachable from start vertex");
        reconstructAndHighlightPath();
        emit algorithmFinished(result, false);
        return false;
    }

    if (current == m_endVertexId) {
        QList<int> path;
        int cur = current;
        while (cur != -1) {
            path.prepend(cur);
            cur = m_parent[cur];
        }
        QString pathStr;
        for (int p : path) pathStr += QString::number(p) + " -> ";
        pathStr.chop(4);

        QString result = "A* completed. Path found to vertex " + QString::number(m_endVertexId) +
                         "!\nPath: " + pathStr + "\nPath length: " + QString::number(m_dist[current]);
        addAnimationStep(result, "A* completed. Path to goal found");
        highlightPath(path, Qt::red);
        emit algorithmFinished(result, true);
        return false;
    }

    m_processed.insert(current);
    highlightVertex(current, Qt::yellow);

    QList<Edge*> edges = getIncidentEdges(current);
    bool distanceUpdated = false;

    for (Edge* e : edges) {
        int neighbor = (e->fromId == current) ? e->toId : e->fromId;
        if (!m_processed.contains(neighbor)) {
            int newDist = m_dist[current] + e->weight;
            if (newDist < m_dist[neighbor]) {
                m_dist[neighbor] = newDist;
                m_parent[neighbor] = current;
                findVertex(neighbor)->setDistance(newDist);
                highlightEdge(current, neighbor, Qt::blue);
                distanceUpdated = true;
            }
        }
    }

    if (distanceUpdated) {
        addAnimationStep("Processed vertex: " + QString::number(current) +
                         "\nDistance from start: " + QString::number(m_dist[current]) +
                         "\nHeuristic to goal: " + QString::number(m_heuristic.value(current, 0)) +
                         "\nEstimate f = " + QString::number(m_dist[current] + m_heuristic.value(current, 0)),
                         "Vertex " + QString::number(current) + " processed. Distances updated");
    } else {
        addAnimationStep("Processed vertex: " + QString::number(current) +
                         "\nDistance from start: " + QString::number(m_dist[current]) +
                         "\nHeuristic to goal: " + QString::number(m_heuristic.value(current, 0)) +
                         "\nEstimate f = " + QString::number(m_dist[current] + m_heuristic.value(current, 0)),
                         "Vertex " + QString::number(current) + " processed. No distance improvements");
    }

    highlightVertex(current, Qt::cyan);
    return true;
}

void GraphWidget::initBellmanFord() {
    if (m_startVertexId == -1) {
        logStep("Error", "Start vertex not set");
        m_isRunning = false;
        return;
    }

    m_bfDist.clear();
    m_parent.clear();
    m_edgeList = m_edges;
    m_bfIteration = 0;

    for (int id : m_vertices.keys()) {
        m_bfDist[id] = INF;
    }
    m_bfDist[m_startVertexId] = 0;
    m_parent[m_startVertexId] = -1;

    highlightVertex(m_startVertexId, Qt::green);
    findVertex(m_startVertexId)->setDistance(0);

    addAnimationStep("Bellman-Ford. Iteration 0 of " + QString::number(m_vertices.size() - 1) +
                     "\nDistances: start=0, others=INF",
                     "Bellman-Ford algorithm. Finds shortest paths with negative weights");
}

bool GraphWidget::executeBellmanFordStep() {
    int n = m_vertices.size();

    if (m_bfIteration >= n - 1) {
        for (Edge* e : m_edgeList) {
            if (m_bfDist[e->fromId] < INF &&
                m_bfDist[e->toId] > m_bfDist[e->fromId] + e->weight) {
                QString result = "Bellman-Ford: Negative cycle detected!";
                addAnimationStep(result, "Graph contains a cycle with negative total weight");
                emit algorithmFinished(result, false);
                return false;
            }
        }

        QString result;
        bool success = false;
        if (m_endVertexId != -1 && m_bfDist[m_endVertexId] != INF) {
            result = "Bellman-Ford completed. Distance to goal: " + QString::number(m_bfDist[m_endVertexId]);
            success = true;
        } else if (m_endVertexId != -1) {
            result = "Bellman-Ford completed. Target vertex " + QString::number(m_endVertexId) + " is NOT reachable!";
            success = false;
        } else {
            result = "Bellman-Ford completed. All shortest paths found.";
            success = true;
        }
        addAnimationStep(result, success ? "Shortest paths calculated" : "Target vertex unreachable");
        reconstructAndHighlightPath();
        emit algorithmFinished(result, success);
        return false;
    }

    bool distanceUpdated = false;
    m_bfIteration++;

    for (Edge* e : m_edgeList) {
        if (m_bfDist[e->fromId] < INF &&
            m_bfDist[e->toId] > m_bfDist[e->fromId] + e->weight) {
            m_bfDist[e->toId] = m_bfDist[e->fromId] + e->weight;
            m_parent[e->toId] = e->fromId;
            findVertex(e->toId)->setDistance(m_bfDist[e->toId]);
            highlightEdge(e->fromId, e->toId, Qt::blue);
            distanceUpdated = true;
        }
    }

    QString distStr;
    for (int id : m_vertices.keys()) {
        distStr += QString::number(id) + ":" + (m_bfDist[id] == INF ? "INF" : QString::number(m_bfDist[id])) + " ";
    }

    if (distanceUpdated) {
        addAnimationStep("Iteration " + QString::number(m_bfIteration) + " of " + QString::number(n - 1) +
                         "\nDistances updated:\n" + distStr,
                         "Relaxation iteration: improved distances found");
    } else {
        addAnimationStep("Iteration " + QString::number(m_bfIteration) + " of " + QString::number(n - 1) +
                         "\nDistances unchanged:\n" + distStr,
                         "Relaxation iteration: no improvements, can finish early");
    }

    return true;
}

void GraphWidget::initPrim() {
    if (m_startVertexId == -1) {
        if (!m_vertices.isEmpty()) {
            m_startVertexId = m_vertices.firstKey();
        } else {
            logStep("Error", "No vertices in graph");
            m_isRunning = false;
            return;
        }
    }

    m_primInTree.clear();
    m_primKey.clear();
    m_primParent.clear();

    for (int id : m_vertices.keys()) {
        m_primKey[id] = INF;
    }
    m_primKey[m_startVertexId] = 0;
    m_primParent[m_startVertexId] = -1;

    highlightVertex(m_startVertexId, Qt::green);

    addAnimationStep("Prim's algorithm. Starting from vertex " + QString::number(m_startVertexId),
                     "Building Minimum Spanning Tree using Prim's algorithm");
}

bool GraphWidget::executePrimStep() {
    int current = -1;
    int minKey = INF;
    for (int id : m_vertices.keys()) {
        if (!m_primInTree.contains(id) && m_primKey[id] < minKey) {
            minKey = m_primKey[id];
            current = id;
        }
    }

    if (current == -1) {
        int totalWeight = 0;
        for (int id : m_primInTree) {
            if (m_primParent[id] != -1) {
                Edge* e = findEdge(m_primParent[id], id);
                if (e) totalWeight += e->weight;
            }
        }
        bool success = (m_primInTree.size() == m_vertices.size());
        QString result;
        if (success) {
            result = "MST built successfully. Tree weight: " + QString::number(totalWeight);
        } else {
            result = "Prim's algorithm completed. Graph is not connected! MST covers only " +
                     QString::number(m_primInTree.size()) + " of " + QString::number(m_vertices.size()) + " vertices.";
        }
        addAnimationStep(result, success ? "Minimum Spanning Tree completed" : "Graph is disconnected - partial MST");
        emit algorithmFinished(result, success);
        return false;
    }

    m_primInTree.insert(current);
    highlightVertex(current, Qt::yellow);

    if (m_primParent[current] != -1) {
        highlightEdge(m_primParent[current], current, Qt::red);
    }

    QList<Edge*> edges = getIncidentEdges(current);

    for (Edge* e : edges) {
        int neighbor = (e->fromId == current) ? e->toId : e->fromId;
        if (!m_primInTree.contains(neighbor) && e->weight < m_primKey[neighbor]) {
            m_primKey[neighbor] = e->weight;
            m_primParent[neighbor] = current;
            highlightEdge(current, neighbor, Qt::blue);
        }
    }

    addAnimationStep("Added vertex " + QString::number(current) +
                     " (key=" + QString::number(minKey) + ")" +
                     "\nIn tree: " + QString::number(m_primInTree.size()) + " vertices",
                     "Vertex " + QString::number(current) + " added to spanning tree");

    highlightVertex(current, Qt::cyan);
    return true;
}

void GraphWidget::initKruskal() {
    m_sortedEdges = m_edges;
    std::sort(m_sortedEdges.begin(), m_sortedEdges.end(),
              [](Edge* a, Edge* b) { return a->weight < b->weight; });

    m_kruskalParent.clear();
    for (int id : m_vertices.keys()) {
        m_kruskalParent[id] = id;
    }

    m_mstEdges.clear();
    m_kruskalIndex = 0;

    addAnimationStep("Kruskal's algorithm. Edges sorted by weight",
                     "Building Minimum Spanning Tree using Kruskal's algorithm");
}

static int findKruskalParent(QMap<int, int>& parent, int x) {
    if (parent[x] != x) parent[x] = findKruskalParent(parent, parent[x]);
    return parent[x];
}

bool GraphWidget::executeKruskalStep() {
    if (m_kruskalIndex >= m_sortedEdges.size() || m_mstEdges.size() >= m_vertices.size() - 1) {
        int totalWeight = 0;
        for (Edge* e : m_mstEdges) totalWeight += e->weight;
        bool success = (m_mstEdges.size() == m_vertices.size() - 1) || (m_vertices.size() <= 1);
        QString result;
        if (success) {
            result = "MST built successfully. Tree weight: " + QString::number(totalWeight);
        } else {
            result = "Kruskal's algorithm completed. Graph is not connected! MST covers only " +
                     QString::number(m_mstEdges.size() + 1) + " of " + QString::number(m_vertices.size()) + " vertices.";
        }
        addAnimationStep(result, success ? "Minimum Spanning Tree completed" : "Graph is disconnected - partial MST");
        emit algorithmFinished(result, success);
        return false;
    }

    Edge* e = m_sortedEdges[m_kruskalIndex];
    m_kruskalIndex++;

    int root1 = findKruskalParent(m_kruskalParent, e->fromId);
    int root2 = findKruskalParent(m_kruskalParent, e->toId);

    if (root1 != root2) {
        m_kruskalParent[root1] = root2;
        m_mstEdges.append(e);
        highlightEdge(e->fromId, e->toId, Qt::red);
        addAnimationStep("Added edge " + QString::number(e->fromId) + "->" +
                         QString::number(e->toId) + " (weight=" + QString::number(e->weight) + ")",
                         "Edge added to MST (does not create cycle)");
    } else {
        highlightEdge(e->fromId, e->toId, Qt::gray);
        addAnimationStep("Skipped edge " + QString::number(e->fromId) + "->" +
                         QString::number(e->toId) + " (weight=" + QString::number(e->weight) + ")",
                         "Edge skipped (would create cycle)");
    }

    return true;
}

void GraphWidget::initTopologicalSort() {
    if (!m_directedMode) {
        logStep("Error", "Topological sort works only for directed graphs");
        m_isRunning = false;
        return;
    }

    m_inDegree.clear();
    m_topologicalQueue.clear();
    m_topologicalOrder.clear();

    for (int id : m_vertices.keys()) {
        m_inDegree[id] = 0;
    }

    for (Edge* e : m_edges) {
        if (e->directed) {
            m_inDegree[e->toId]++;
        }
    }

    for (int id : m_vertices.keys()) {
        if (m_inDegree[id] == 0) {
            m_topologicalQueue.enqueue(id);
            highlightVertex(id, Qt::green);
        }
    }

    addAnimationStep("Topological sort\nVertices with zero in-degree: " +
                     QString::number(m_topologicalQueue.size()),
                     "Starting topological sort of DAG");
}

bool GraphWidget::executeTopologicalSortStep() {
    if (m_topologicalQueue.isEmpty()) {
        if (m_topologicalOrder.size() < m_vertices.size()) {
            QString result = "Topological sort failed: Cycle detected in graph!";
            addAnimationStep(result, "Graph contains a cycle, topological sort impossible");
            emit algorithmFinished(result, false);
        } else {
            QString orderStr;
            for (int v : m_topologicalOrder) orderStr += QString::number(v) + " -> ";
            orderStr.chop(4);
            QString result = "Topological sort completed successfully.\nOrder: " + orderStr;
            addAnimationStep(result, "Topological sort completed - DAG verified");
            emit algorithmFinished(result, true);
        }
        return false;
    }

    int current = m_topologicalQueue.dequeue();
    m_topologicalOrder.append(current);
    highlightVertex(current, Qt::yellow);

    QList<Edge*> edges = getIncidentEdges(current);
    for (Edge* e : edges) {
        if (e->directed && e->fromId == current) {
            m_inDegree[e->toId]--;
            if (m_inDegree[e->toId] == 0) {
                m_topologicalQueue.enqueue(e->toId);
                highlightVertex(e->toId, Qt::green);
            }
            highlightEdge(current, e->toId, Qt::blue);
        }
    }

    QString queueStr;
    for (int v : m_topologicalQueue) queueStr += QString::number(v) + " ";
    QString orderStr;
    for (int v : m_topologicalOrder) orderStr += QString::number(v) + " ";

    addAnimationStep("Removed vertex " + QString::number(current) +
                     "\nQueue: [" + queueStr + "]" +
                     "\nOrder: [" + orderStr + "]",
                     "Vertex " + QString::number(current) + " removed. In-degrees updated");

    highlightVertex(current, Qt::cyan);
    return true;
}

void GraphWidget::initSCC() {
    if (!m_directedMode) {
        logStep("Error", "SCC search works only for directed graphs");
        m_isRunning = false;
        return;
    }

    int maxId = m_nextVertexId;
    m_adjMatrix = QVector<QVector<int>>(maxId, QVector<int>(maxId, 0));
    m_revAdjMatrix = QVector<QVector<int>>(maxId, QVector<int>(maxId, 0));

    for (Edge* e : m_edges) {
        if (e->directed) {
            m_adjMatrix[e->fromId][e->toId] = 1;
            m_revAdjMatrix[e->toId][e->fromId] = 1;
        }
    }

    m_sccVisited.clear();
    m_sccOrder.clear();
    m_sccComponents.clear();
    m_sccPhase = 0;
    m_sccStack.clear();

    addAnimationStep("Finding Strongly Connected Components (Kosaraju's algorithm)",
                     "First pass: DFS to get vertex order");
}

static void dfsSCC1(int v, QSet<int>& visited, QList<int>& order,
                    const QVector<QVector<int>>& adj) {
    visited.insert(v);
    for (int u = 0; u < adj.size(); u++) {
        if (adj[v][u] && !visited.contains(u)) {
            dfsSCC1(u, visited, order, adj);
        }
    }
    order.append(v);
}

static void dfsSCC2(int v, QSet<int>& visited, QList<int>& component,
                    const QVector<QVector<int>>& revAdj) {
    visited.insert(v);
    component.append(v);
    for (int u = 0; u < revAdj.size(); u++) {
        if (revAdj[v][u] && !visited.contains(u)) {
            dfsSCC2(u, visited, component, revAdj);
        }
    }
}

bool GraphWidget::executeSCCStep() {
    if (m_sccPhase == 0) {
        bool foundNew = false;
        for (int id : m_vertices.keys()) {
            if (!m_sccVisited.contains(id)) {
                dfsSCC1(id, m_sccVisited, m_sccOrder, m_adjMatrix);
                foundNew = true;
                break;
            }
        }

        if (!foundNew) {
            m_sccPhase = 1;
            m_sccVisited.clear();
            addAnimationStep("First pass completed. Order: " +
                             QString::number(m_sccOrder.size()) + " vertices",
                             "Second pass: processing vertices in reverse order");
            return true;
        }

        addAnimationStep("First pass DFS... Visited " + QString::number(m_sccVisited.size()) +
                         " vertices", "Performing depth-first search");
        return true;

    } else if (m_sccPhase == 1) {
        if (m_sccOrder.isEmpty()) {
            QString result = "SCC search completed. Components found: " + QString::number(m_sccComponents.size());
            for (int i = 0; i < m_sccComponents.size(); i++) {
                QString compStr;
                for (int v : m_sccComponents[i]) compStr += QString::number(v) + " ";
                addAnimationStep("Component " + QString::number(i+1) + ": {" + compStr + "}",
                                 "Strongly connected component detected");
            }
            addAnimationStep(result, "SCC search completed successfully");
            emit algorithmFinished(result, true);
            return false;
        }

        int v = m_sccOrder.takeLast();
        if (!m_sccVisited.contains(v)) {
            QList<int> component;
            dfsSCC2(v, m_sccVisited, component, m_revAdjMatrix);
            m_sccComponents.append(component);

            QColor compColor = QColor::fromHsv((m_sccComponents.size() * 50) % 360, 200, 200);
            for (int cv : component) {
                highlightVertex(cv, compColor);
            }
            addAnimationStep("Found SCC: {" + QString::number(component.size()) + " vertices}",
                             "Strongly connected component detected");
        }
        return true;
    }

    return false;
}

void GraphWidget::startAlgorithm(Algorithm algo) {
    if (m_isRunning && !m_isPaused) return;

    resetAlgorithm();
    m_currentAlgorithm = algo;
    m_isRunning = true;
    m_isPaused = false;
    m_animationSteps.clear();
    m_currentStepIndex = -1;

    resetColors();

    switch (algo) {
        case AlgoBFS: initBFS(); break;
        case AlgoDFS: initDFS(); break;
        case AlgoDijkstra: initDijkstra(); break;
        case AlgoAStar: initAStar(); break;
        case AlgoBellmanFord: initBellmanFord(); break;
        case AlgoPrim: initPrim(); break;
        case AlgoKruskal: initKruskal(); break;
        case AlgoTopologicalSort: initTopologicalSort(); break;
        case AlgoSCC: initSCC(); break;
    }

    m_animationTimer->start(m_animationSpeedMs);
}

void GraphWidget::pauseAlgorithm() {
    if (!m_isRunning) return;
    m_isPaused = true;
    m_animationTimer->stop();
}

void GraphWidget::stepForward() {
    if (!m_isRunning) {
        startAlgorithm(m_currentAlgorithm);
        m_isPaused = true;
        m_animationTimer->stop();
        return;
    }

    if (m_isPaused || m_isRunning) {
        bool continueExecution = true;

        switch (m_currentAlgorithm) {
            case AlgoBFS: continueExecution = executeBFSStep(); break;
            case AlgoDFS: continueExecution = executeDFSStep(); break;
            case AlgoDijkstra: continueExecution = executeDijkstraStep(); break;
            case AlgoAStar: continueExecution = executeAStarStep(); break;
            case AlgoBellmanFord: continueExecution = executeBellmanFordStep(); break;
            case AlgoPrim: continueExecution = executePrimStep(); break;
            case AlgoKruskal: continueExecution = executeKruskalStep(); break;
            case AlgoTopologicalSort: continueExecution = executeTopologicalSortStep(); break;
            case AlgoSCC: continueExecution = executeSCCStep(); break;
        }

        if (!continueExecution) {
            m_isRunning = false;
            m_animationTimer->stop();
        }
    }
}

void GraphWidget::stepBackward() {
    if (m_currentStepIndex > 0) {
        m_currentStepIndex--;
        restoreStepState(m_currentStepIndex);
    }
}

void GraphWidget::resetAlgorithm()
{
    m_animationTimer->stop();
    m_isRunning = false;
    m_isPaused = false;
    m_currentStepIndex = -1;
    resetColors();

    m_bfsQueue.clear();
    m_dfsStack.clear();
    m_visited.clear();
    m_dist.clear();
    m_processed.clear();
    m_bfDist.clear();
    m_primInTree.clear();
    m_primKey.clear();
    m_mstEdges.clear();
    m_topologicalOrder.clear();
    m_sccComponents.clear();
    m_sccOrder.clear();
    m_sccVisited.clear();
    m_heuristic.clear();

    m_bfIteration = 0;
    m_kruskalIndex = 0;
    m_sccPhase = 0;

    for (Vertex* v : m_vertices) {
        v->setDistance(INF);
        v->setText(QString::number(v->getId()));
    }

    updateEdgesGeometry();

    addAnimationStep("Algorithm reset", "Graph state restored");
}

void GraphWidget::setAnimationSpeed(int speedMs) {
    m_animationSpeedMs = speedMs;
    if (m_animationTimer->isActive())
        m_animationTimer->start(m_animationSpeedMs);
}

void GraphWidget::animateStep() {
    if (m_isPaused) return;
    stepForward();
}

void GraphWidget::setStartVertex(int id) {
    if (m_vertices.contains(id)) {
        if (m_startVertexId != -1) {
            Vertex* oldStart = findVertex(m_startVertexId);
            if (oldStart) oldStart->setText(QString::number(m_startVertexId));
        }
        m_startVertexId = id;
        Vertex* v = findVertex(id);
        v->setText(QString::number(id));
        logStep("Start vertex: " + QString::number(id),
                "Start vertex set for algorithms");
    }
}

void GraphWidget::setEndVertex(int id) {
    if (m_vertices.contains(id)) {
        if (m_endVertexId != -1) {
            Vertex* oldEnd = findVertex(m_endVertexId);
            if (oldEnd) oldEnd->setText(QString::number(m_endVertexId));
        }
        m_endVertexId = id;
        Vertex* v = findVertex(id);
        v->setText(QString::number(id));
        logStep("End vertex: " + QString::number(id),
                "End vertex set for algorithms");
    }
}

void GraphWidget::resetAlgorithmState() {
    resetAlgorithm();
}

void GraphWidget::reconstructAndHighlightPath()
{
    if (m_endVertexId == -1) return;

    int currentDist = INF;
    switch (m_currentAlgorithm) {
        case AlgoDijkstra:
            currentDist = m_dist.value(m_endVertexId, INF);
            break;
        case AlgoAStar:
            currentDist = m_dist.value(m_endVertexId, INF);
            break;
        case AlgoBellmanFord:
            currentDist = m_bfDist.value(m_endVertexId, INF);
            break;
        default:
            return;
    }

    if (currentDist == INF) {
        return;
    }

    QList<int> path;
    int cur = m_endVertexId;
    while (cur != -1 && m_parent.contains(cur)) {
        path.prepend(cur);
        cur = m_parent[cur];
        if (cur == -1) break;
        if (path.contains(cur)) {
            break;
        }
    }

    if (!path.isEmpty() && path.first() == m_startVertexId) {
        highlightPath(path, Qt::red);

        QString pathStr;
        for (int p : path) pathStr += QString::number(p) + " -> ";
        pathStr.chop(4);

        addAnimationStep("Final shortest path found!\nPath: " + pathStr +
                         "\nTotal distance: " + QString::number(currentDist),
                         "Shortest path highlighted in red");
    }
}

QJsonObject GraphWidget::toJson() const {
    QJsonObject obj;
    QJsonArray verticesArr;
    for (Vertex* v : m_vertices) {
        QJsonObject vObj;
        vObj["id"] = v->getId();
        vObj["x"] = v->pos().x();
        vObj["y"] = v->pos().y();
        verticesArr.append(vObj);
    }
    obj["vertices"] = verticesArr;

    QJsonArray edgesArr;
    for (Edge* e : m_edges) {
        QJsonObject eObj;
        eObj["from"] = e->fromId;
        eObj["to"] = e->toId;
        eObj["weight"] = e->weight;
        eObj["directed"] = e->directed;
        edgesArr.append(eObj);
    }
    obj["edges"] = edgesArr;
    obj["directed"] = m_directedMode;
    obj["nextId"] = m_nextVertexId;
    obj["startVertex"] = m_startVertexId;
    obj["endVertex"] = m_endVertexId;

    return obj;
}

bool GraphWidget::fromJson(const QJsonObject& json) {
    clearGraph();

    QJsonArray verticesArr = json["vertices"].toArray();
    for (const QJsonValue& val : verticesArr) {
        QJsonObject vObj = val.toObject();
        int id = vObj["id"].toInt();
        qreal x = vObj["x"].toDouble();
        qreal y = vObj["y"].toDouble();
        Vertex* v = new Vertex(id, QPointF(x, y));
        m_scene->addItem(v);
        m_vertices[id] = v;
        if (id >= m_nextVertexId) m_nextVertexId = id + 1;
    }

    QJsonArray edgesArr = json["edges"].toArray();
    for (const QJsonValue& val : edgesArr) {
        QJsonObject eObj = val.toObject();
        int from = eObj["from"].toInt();
        int to = eObj["to"].toInt();
        int weight = eObj["weight"].toInt();
        bool directed = eObj["directed"].toBool();

        if (m_vertices.contains(from) && m_vertices.contains(to)) {
            bool currentDirected = m_directedMode;
            m_directedMode = directed;
            addEdge(from, to, weight);
            m_directedMode = currentDirected;
        }
    }

    if (json.contains("directed")) {
        m_directedMode = json["directed"].toBool();
    }
    if (json.contains("startVertex")) {
        m_startVertexId = json["startVertex"].toInt();
        if (m_vertices.contains(m_startVertexId))
            findVertex(m_startVertexId)->setText(QString::number(m_startVertexId));
    }
    if (json.contains("endVertex")) {
        m_endVertexId = json["endVertex"].toInt();
        if (m_vertices.contains(m_endVertexId))
            findVertex(m_endVertexId)->setText(QString::number(m_endVertexId));
    }

    updateEdgesGeometry();
    return true;
}

void GraphWidget::updateGraphAppearance()
{
    updateEdgesGeometry();
    update();
}
