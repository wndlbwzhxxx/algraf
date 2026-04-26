#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QGraphicsPathItem>
#include <QMap>
#include <QSet>
#include <QVector>
#include <QQueue>
#include <QStack>
#include <QTimer>
#include <QPen>
#include <QBrush>
#include <QPointF>
#include <QPainterPath>
#include <QPair>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>
#include <QWidget>
#include <QMouseEvent>
#include <QInputDialog>
#include <QCoreApplication>

enum InteractionMode {
    ModeAddVertex,
    ModeAddEdge,
    ModeRemove,
    ModeMove,
    ModeDeleteEdge,
    ModeEditEdge
};

enum Algorithm {
    AlgoBFS,
    AlgoDFS,
    AlgoDijkstra,
    AlgoAStar,
    AlgoBellmanFord,
    AlgoPrim,
    AlgoKruskal,
    AlgoTopologicalSort,
    AlgoSCC
};

struct Edge {
    int fromId;
    int toId;
    int weight;
    bool directed;
    QGraphicsLineItem* lineItem;
    QGraphicsTextItem* weightItem;
    QGraphicsPathItem* arrowItem;
};

class Vertex : public QGraphicsEllipseItem {
public:
    Vertex(int id, const QPointF& pos, QGraphicsItem* parent = nullptr);
    int getId() const { return m_id; }
    void setColor(const QColor& color);
    void setText(const QString& text);
    void updateLabel();
    void setDistance(int dist);
    void resetAppearance();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

private:
    int m_id;
    QGraphicsTextItem* m_label;
    QGraphicsTextItem* m_distanceLabel;
    QPointF m_dragStart;
    bool m_dragging;
    QColor m_originalColor;
};

class GraphWidget : public QGraphicsView {
    Q_OBJECT

public:
    explicit GraphWidget(QWidget* parent = nullptr);
    ~GraphWidget();

    void setMode(InteractionMode mode);
    void setDirectedMode(bool directed);
    bool isDirectedMode() const { return m_directedMode; }

    void addVertex(const QPointF& pos);
    void addEdge(int fromId, int toId, int weight);
    void editEdgeWeight(int fromId, int toId, int newWeight);
    void removeItemAt(const QPointF& pos);
    void clearGraph();
    void updateEdgesGeometry();
    void updateGraphAppearance();

    void startAlgorithm(Algorithm algo);
    void pauseAlgorithm();
    void stepForward();
    void stepBackward();
    void resetAlgorithm();
    void setAnimationSpeed(int speedMs);

    QJsonObject toJson() const;
    bool fromJson(const QJsonObject& json);

    void setStartVertex(int id);
    void setEndVertex(int id);
    int getStartVertex() const { return m_startVertexId; }
    int getEndVertex() const { return m_endVertexId; }

    QMap<int, Vertex*> getVertices() const { return m_vertices; }
    QList<Edge*> getEdges() const { return m_edges; }

    void setTempEdgeWeight(int weight) { m_tempEdgeWeight = weight; }
    int getTempEdgeWeight() const { return m_tempEdgeWeight; }

    void createDefaultGraph();

signals:
    void stateUpdated(const QString& structures, const QString& explanation);
    void algorithmFinished(const QString& result, bool success); // success = true если успешно
    void stepExecuted(int currentStep, int totalSteps);
    void graphChanged();

protected:
    void mousePressEvent(QMouseEvent* event) override;

private slots:
    void animateStep();

private:
    Vertex* findVertex(int id) const;
    Edge* findEdge(int from, int to) const;
    QList<Edge*> getIncidentEdges(int vertexId) const;
    void updateArrow(Edge* edge);
    void highlightVertex(int id, const QColor& color);
    void highlightEdge(int from, int to, const QColor& color);
    void highlightPath(const QList<int>& path, const QColor& color);
    void resetColors();
    void clearHighlights();
    void logStep(const QString& structures, const QString& explanation);
    void addAnimationStep(const QString& structures, const QString& explanation);
    void restoreStepState(int stepIndex);

    void initBFS();
    bool executeBFSStep();
    void initDFS();
    bool executeDFSStep();
    void initDijkstra();
    bool executeDijkstraStep();
    void initAStar();
    bool executeAStarStep();
    void initBellmanFord();
    bool executeBellmanFordStep();
    void initPrim();
    bool executePrimStep();
    void initKruskal();
    bool executeKruskalStep();
    void initTopologicalSort();
    bool executeTopologicalSortStep();
    void initSCC();
    bool executeSCCStep();
    void reconstructAndHighlightPath();
    void resetAlgorithmState();

    QGraphicsScene* m_scene;
    QMap<int, Vertex*> m_vertices;
    QList<Edge*> m_edges;
    int m_nextVertexId;

    InteractionMode m_mode;
    bool m_directedMode;
    Vertex* m_tempEdgeSource;
    int m_tempEdgeWeight;

    Algorithm m_currentAlgorithm;
    QTimer* m_animationTimer;
    int m_animationSpeedMs;
    bool m_isRunning;
    bool m_isPaused;
    QVector<QPair<QString, QString>> m_animationSteps;
    int m_currentStepIndex;

    int m_startVertexId;
    int m_endVertexId;

    QQueue<int> m_bfsQueue;
    QStack<int> m_dfsStack;
    QSet<int> m_visited;
    QMap<int, int> m_parent;
    QMap<int, int> m_dist;
    QSet<int> m_processed;
    QMap<int, int> m_heuristic;
    QMap<int, int> m_bfDist;
    QList<Edge*> m_edgeList;
    int m_bfIteration;
    QSet<int> m_primInTree;
    QMap<int, int> m_primKey;
    QMap<int, int> m_primParent;
    QList<Edge*> m_sortedEdges;
    QMap<int, int> m_kruskalParent;
    QList<Edge*> m_mstEdges;
    int m_kruskalIndex;
    QMap<int, int> m_inDegree;
    QQueue<int> m_topologicalQueue;
    QList<int> m_topologicalOrder;
    QStack<int> m_sccStack;
    QSet<int> m_sccVisited;
    QList<QList<int>> m_sccComponents;
    QVector<QVector<int>> m_adjMatrix;
    QVector<QVector<int>> m_revAdjMatrix;
    int m_sccPhase;
    QList<int> m_sccOrder;
};

#endif // GRAPHWIDGET_H
