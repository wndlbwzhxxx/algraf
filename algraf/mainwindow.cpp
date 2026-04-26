#include "mainwindow.h"
#include <QMenuBar>
#include <QMenu>
#include <QStatusBar>
#include <QToolBar>
#include <QSplitter>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QSlider>
#include <QTextEdit>
#include <QTabWidget>
#include <QCheckBox>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QInputDialog>
#include <QMessageBox>
#include <QScrollBar>
#include <QTimer>
#include <QDialog>
#include <QDialogButtonBox>
#include <QApplication>

// Algorithm descriptions dialog
class AlgorithmInfoDialog : public QDialog
{
public:
    AlgorithmInfoDialog(QWidget *parent = nullptr) : QDialog(parent)
    {
        setWindowTitle("Algorithms - Documentation");
        setMinimumSize(600, 500);

        QVBoxLayout *layout = new QVBoxLayout(this);

        QTextEdit *textEdit = new QTextEdit(this);
        textEdit->setReadOnly(true);

        QString descriptions =
            "<h2>Graph Algorithms Documentation</h2>\n\n"

            "<h3 style='color:#007acc;'>1. BFS (Breadth-First Search)</h3>"
            "<p><b>Type:</b> Unweighted Graph Search<br>"
            "<b>Time Complexity:</b> O(V + E)<br>"
            "<b>Description:</b> BFS explores vertices level by level, starting from a source vertex. "
            "It uses a queue data structure and guarantees the shortest path in terms of number of edges. "
            "BFS is ideal for finding the shortest path in unweighted graphs and checking connectivity.</p>"

            "<h3 style='color:#007acc;'>2. DFS (Depth-First Search)</h3>"
            "<p><b>Type:</b> Graph Traversal<br>"
            "<b>Time Complexity:</b> O(V + E)<br>"
            "<b>Description:</b> DFS explores as far as possible along each branch before backtracking. "
            "It uses a stack (or recursion) and is useful for topological sorting, cycle detection, "
            "and solving puzzles like mazes.</p>"

            "<h3 style='color:#007acc;'>3. Dijkstra's Algorithm</h3>"
            "<p><b>Type:</b> Shortest Path (Positive Weights)<br>"
            "<b>Time Complexity:</b> O((V + E) log V) with priority queue<br>"
            "<b>Description:</b> Finds the shortest path from a source vertex to all other vertices "
            "in a graph with non-negative edge weights. It uses a greedy approach, always selecting "
            "the vertex with the smallest known distance.</p>"

            "<h3 style='color:#007acc;'>4. A* Algorithm</h3>"
            "<p><b>Type:</b> Heuristic-based Shortest Path<br>"
            "<b>Time Complexity:</b> O(E) in best case, O(b^d) in worst case<br>"
            "<b>Description:</b> An extension of Dijkstra's algorithm that uses a heuristic function "
            "to guide the search towards the goal. A* is commonly used in pathfinding for games and "
            "GPS navigation. It uses the formula: f(n) = g(n) + h(n), where g(n) is the cost from start "
            "and h(n) is the estimated cost to goal (Euclidean distance in this implementation).</p>"

            "<h3 style='color:#007acc;'>5. Bellman-Ford Algorithm</h3>"
            "<p><b>Type:</b> Shortest Path (Negative Weights Allowed)<br>"
            "<b>Time Complexity:</b> O(V × E)<br>"
            "<b>Description:</b> Finds shortest paths from a source vertex even when negative edge weights "
            "are present. It can detect negative cycles in the graph. The algorithm relaxes all edges "
            "(V-1) times, where V is the number of vertices.</p>"

            "<h3 style='color:#007acc;'>6. Prim's Algorithm (MST)</h3>"
            "<p><b>Type:</b> Minimum Spanning Tree<br>"
            "<b>Time Complexity:</b> O((V + E) log V) with priority queue<br>"
            "<b>Description:</b> Builds a Minimum Spanning Tree by growing a single tree from a start vertex. "
            "At each step, it adds the cheapest edge connecting a vertex in the tree to a vertex outside the tree. "
            "Prim's algorithm works best for dense graphs.</p>"

            "<h3 style='color:#007acc;'>7. Kruskal's Algorithm (MST)</h3>"
            "<p><b>Type:</b> Minimum Spanning Tree<br>"
            "<b>Time Complexity:</b> O(E log E) or O(E log V)<br>"
            "<b>Description:</b> Builds an MST by sorting all edges by weight and adding them one by one, "
            "skipping edges that would create cycles. It uses a Union-Find data structure for cycle detection. "
            "Kruskal's algorithm works best for sparse graphs.</p>"

            "<h3 style='color:#007acc;'>8. Topological Sort</h3>"
            "<p><b>Type:</b> DAG Linear Ordering<br>"
            "<b>Time Complexity:</b> O(V + E)<br>"
            "<b>Description:</b> Orders vertices in a Directed Acyclic Graph (DAG) such that for every directed "
            "edge u → v, u comes before v. This is useful for scheduling tasks, resolving dependencies, "
            "and determining compilation order. Requires the graph to be directed and acyclic.</p>"

            "<h3 style='color:#007acc;'>9. Kosaraju's Algorithm (SCC)</h3>"
            "<p><b>Type:</b> Strongly Connected Components<br>"
            "<b>Time Complexity:</b> O(V + E)<br>"
            "<b>Description:</b> Finds all strongly connected components in a directed graph. "
            "It performs two DFS passes: first on the original graph to get finishing times, "
            "then on the reversed graph. Vertices that reach each other in both directions form an SCC.</p>\n\n"

            "<hr>\n"
            "<h3>How to Use:</h3>"
            "<ul>"
            "<li><b>Add Vertex:</b> Click 'Add Vertex Mode' then click on canvas</li>"
            "<li><b>Add Edge:</b> Click 'Add Edge Mode', then click first vertex and second vertex</li>"
            "<li><b>Delete:</b> Use delete mode to remove vertices or edges</li>"
            "<li><b>Edit Edge Weight:</b> Use edit mode and click on any edge</li>"
            "<li><b>Move Vertices:</b> Use move mode to drag vertices around</li>"
            "<li><b>Run Algorithm:</b> Select algorithm, set start/goal vertices, click Start</li>"
            "</ul>";

        textEdit->setHtml(descriptions);
        layout->addWidget(textEdit);

        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::accept);
        layout->addWidget(buttonBox);
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), currentTheme("dark")
{
    setWindowTitle("Algraf - Graph Algorithm Visualizer");
    resize(1400, 900);

    setStyleSheet(R"(
        QMainWindow {
            background-color: #1e1e1e;
        }
        QMenuBar {
            background-color: #2d2d2d;
            color: #ffffff;
            border-bottom: 1px solid #3d3d3d;
        }
        QMenuBar::item {
            background-color: transparent;
            padding: 6px 12px;
        }
        QMenuBar::item:selected {
            background-color: #3d3d3d;
        }
        QMenu {
            background-color: #2d2d2d;
            color: #ffffff;
            border: 1px solid #3d3d3d;
        }
        QMenu::item {
            padding: 6px 24px;
        }
        QMenu::item:selected {
            background-color: #007acc;
        }
        QToolBar {
            background-color: #2d2d2d;
            border: none;
            border-bottom: 1px solid #3d3d3d;
            spacing: 8px;
            padding: 4px;
        }
        QToolBar QToolButton {
            background-color: #3d3d3d;
            color: #ffffff;
            border: none;
            border-radius: 4px;
            padding: 6px 12px;
        }
        QToolBar QToolButton:hover {
            background-color: #007acc;
        }
        QGroupBox {
            color: #ffffff;
            border: 1px solid #3d3d3d;
            border-radius: 6px;
            margin-top: 12px;
            font-weight: bold;
            background-color: #252525;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 12px;
            padding: 0 6px;
            color: #007acc;
        }
        QLabel {
            color: #d4d4d4;
        }
        QPushButton {
            background-color: #3d3d3d;
            color: #ffffff;
            border: none;
            border-radius: 4px;
            padding: 8px 12px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #007acc;
        }
        QPushButton:pressed {
            background-color: #005a9e;
        }
        QPushButton.active-mode {
            background-color: #007acc;
            border: 2px solid #ffcc00;
        }
        QComboBox {
            background-color: #3d3d3d;
            color: #ffffff;
            border: 1px solid #4d4d4d;
            border-radius: 4px;
            padding: 6px;
        }
        QComboBox::drop-down {
            border: none;
        }
        QComboBox QAbstractItemView {
            background-color: #3d3d3d;
            color: #ffffff;
            selection-background-color: #007acc;
        }
        QLineEdit {
            background-color: #3d3d3d;
            color: #ffffff;
            border: 1px solid #4d4d4d;
            border-radius: 4px;
            padding: 4px;
        }
        QSlider::groove:horizontal {
            height: 4px;
            background-color: #3d3d3d;
            border-radius: 2px;
        }
        QSlider::handle:horizontal {
            background-color: #007acc;
            width: 12px;
            height: 12px;
            margin: -4px 0;
            border-radius: 6px;
        }
        QSlider::sub-page:horizontal {
            background-color: #007acc;
            border-radius: 2px;
        }
        QTextEdit {
            background-color: #1e1e1e;
            color: #d4d4d4;
            border: 1px solid #3d3d3d;
            border-radius: 4px;
            font-family: 'Consolas', 'Monaco', monospace;
            font-size: 11px;
        }
        QTabWidget::pane {
            border: 1px solid #3d3d3d;
            border-radius: 4px;
            background-color: #252525;
        }
        QTabBar::tab {
            background-color: #3d3d3d;
            color: #d4d4d4;
            padding: 8px 16px;
            margin-right: 2px;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
        }
        QTabBar::tab:selected {
            background-color: #007acc;
            color: #ffffff;
        }
        QTabBar::tab:hover {
            background-color: #4d4d4d;
        }
    )");

    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal, this);

    // Left Panel
    QWidget *leftPanel = new QWidget();
    leftPanel->setMaximumWidth(280);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setSpacing(12);

    QLabel *logoLabel = new QLabel("Algraf");
    logoLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #007acc; padding: 8px;");
    logoLabel->setAlignment(Qt::AlignCenter);
    leftLayout->addWidget(logoLabel);

    QTabWidget *toolsTabs = new QTabWidget();

    // Vertices tab
    QWidget *vertexTab = new QWidget();
    QVBoxLayout *vertexLayout = new QVBoxLayout(vertexTab);

    addVertexBtn = new QPushButton("➕ Add Vertex Mode");
    vertexLayout->addWidget(addVertexBtn);

    moveBtn = new QPushButton("✋ Move Mode");
    vertexLayout->addWidget(moveBtn);

    deleteBtn = new QPushButton("🗑 Delete Vertex/Edge Mode");
    deleteBtn->setStyleSheet("background-color: #8b0000;");
    vertexLayout->addWidget(deleteBtn);

    vertexLayout->addStretch();
    toolsTabs->addTab(vertexTab, "Vertices");

    // Edges tab
    QWidget *edgeTab = new QWidget();
    QVBoxLayout *edgeLayout = new QVBoxLayout(edgeTab);

    edgeTypeCombo = new QComboBox();
    edgeTypeCombo->addItem("Directed");
    edgeTypeCombo->addItem("Undirected");
    edgeLayout->addWidget(edgeTypeCombo);

    QHBoxLayout *weightLayout = new QHBoxLayout();
    weightLayout->addWidget(new QLabel("Default Weight:"));
    weightEdit = new QLineEdit("1");
    weightEdit->setMaximumWidth(60);
    weightLayout->addWidget(weightEdit);
    weightLayout->addStretch();
    edgeLayout->addLayout(weightLayout);

    addEdgeBtn = new QPushButton("🔗 Add Edge Mode");
    edgeLayout->addWidget(addEdgeBtn);

    deleteEdgeBtn = new QPushButton("❌ Delete Edge Mode");
    deleteEdgeBtn->setStyleSheet("background-color: #8b0000;");
    edgeLayout->addWidget(deleteEdgeBtn);

    editEdgeBtn = new QPushButton("✏️ Edit Edge Weight Mode");
    edgeLayout->addWidget(editEdgeBtn);

    edgeLayout->addStretch();
    toolsTabs->addTab(edgeTab, "Edges");

    leftLayout->addWidget(toolsTabs);

    QPushButton *clearGraphBtn = new QPushButton("Clear Graph");
    clearGraphBtn->setStyleSheet("background-color: #8b0000;");
    leftLayout->addWidget(clearGraphBtn);

    leftLayout->addStretch();

    // Center Panel
    QWidget *centerPanel = new QWidget();
    QVBoxLayout *centerLayout = new QVBoxLayout(centerPanel);
    centerLayout->setContentsMargins(0, 0, 0, 0);

    QToolBar *canvasToolbar = new QToolBar();
    canvasToolbar->setIconSize(QSize(24, 24));

    QAction *zoomInAction = canvasToolbar->addAction("Zoom In");
    QAction *zoomOutAction = canvasToolbar->addAction("Zoom Out");
    QAction *fitViewAction = canvasToolbar->addAction("Fit View");

    connect(zoomInAction, &QAction::triggered, this, &MainWindow::onZoomIn);
    connect(zoomOutAction, &QAction::triggered, this, &MainWindow::onZoomOut);
    connect(fitViewAction, &QAction::triggered, this, &MainWindow::onFitView);

    centerLayout->addWidget(canvasToolbar);

    m_graphWidget = new GraphWidget(this);
    m_graphWidget->setMinimumHeight(500);
    m_graphWidget->setStyleSheet("background-color: #1a1a1a; border: 1px solid #3d3d3d; border-radius: 8px;");
    centerLayout->addWidget(m_graphWidget);
    m_graphWidget->createDefaultGraph();

    connect(m_graphWidget, &GraphWidget::graphChanged, this, &MainWindow::updateGraphInfo);
    connect(m_graphWidget, &GraphWidget::algorithmFinished,
            this, &MainWindow::onAlgorithmFinished);

    // Right Panel
    QWidget *rightPanel = new QWidget();
    rightPanel->setMaximumWidth(320);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setSpacing(12);

    QGroupBox *graphInfoGroup = new QGroupBox("Graph Information");
    QVBoxLayout *graphInfoLayout = new QVBoxLayout(graphInfoGroup);

    verticesCountLabel = new QLabel("Vertices: 0");
    edgesCountLabel = new QLabel("Edges: 0");
    directedLabel = new QLabel("Type: Undirected");

    graphInfoLayout->addWidget(verticesCountLabel);
    graphInfoLayout->addWidget(edgesCountLabel);
    graphInfoLayout->addWidget(directedLabel);

    rightLayout->addWidget(graphInfoGroup);

    QGroupBox *algoGroup = new QGroupBox("Algorithm Settings");
    QVBoxLayout *algoLayout = new QVBoxLayout(algoGroup);

    algoLayout->addWidget(new QLabel("Algorithm:"));
    algorithmCombo = new QComboBox();
    algorithmCombo->addItem("BFS (Breadth-First Search)");
    algorithmCombo->addItem("DFS (Depth-First Search)");
    algorithmCombo->addItem("Dijkstra");
    algorithmCombo->addItem("A*");
    algorithmCombo->addItem("Bellman-Ford");
    algorithmCombo->addItem("Prim (MST)");
    algorithmCombo->addItem("Kruskal (MST)");
    algorithmCombo->addItem("Topological Sort");
    algorithmCombo->addItem("Find SCC");
    algoLayout->addWidget(algorithmCombo);

    QHBoxLayout *startLayout = new QHBoxLayout();
    startLayout->addWidget(new QLabel("Start:"));
    startVertexCombo = new QComboBox();
    startVertexCombo->setEditable(true);
    startLayout->addWidget(startVertexCombo);
    algoLayout->addLayout(startLayout);

    QHBoxLayout *targetLayout = new QHBoxLayout();
    targetLayout->addWidget(new QLabel("Goal:"));
    targetVertexCombo = new QComboBox();
    targetVertexCombo->setEditable(true);
    targetLayout->addWidget(targetVertexCombo);
    algoLayout->addLayout(targetLayout);

    rightLayout->addWidget(algoGroup);

    QGroupBox *animationGroup = new QGroupBox("Animation Control");
    QVBoxLayout *animationLayout = new QVBoxLayout(animationGroup);

    QHBoxLayout *controlButtons = new QHBoxLayout();
    startBtn = new QPushButton("Start");
    startBtn->setStyleSheet("background-color: #007acc;");
    pauseBtn = new QPushButton("Pause");
    resetBtn = new QPushButton("Reset");

    controlButtons->addWidget(startBtn);
    controlButtons->addWidget(pauseBtn);
    controlButtons->addWidget(resetBtn);
    animationLayout->addLayout(controlButtons);

    QHBoxLayout *stepButtons = new QHBoxLayout();
    stepBackwardBtn = new QPushButton("Step Back");
    stepForwardBtn = new QPushButton("Step Forward");
    stepButtons->addWidget(stepBackwardBtn);
    stepButtons->addWidget(stepForwardBtn);
    animationLayout->addLayout(stepButtons);

    animationLayout->addWidget(new QLabel("Animation Speed (steps/sec):"));
    speedSlider = new QSlider(Qt::Horizontal);
    speedSlider->setRange(0, 100);
    speedSlider->setValue(20);
    animationLayout->addWidget(speedSlider);

    speedLabel = new QLabel("1 step/second");
    speedLabel->setAlignment(Qt::AlignCenter);
    animationLayout->addWidget(speedLabel);

    rightLayout->addWidget(animationGroup);
    rightLayout->addStretch();

    mainSplitter->addWidget(leftPanel);
    mainSplitter->addWidget(centerPanel);
    mainSplitter->addWidget(rightPanel);

    mainSplitter->setStretchFactor(0, 0);
    mainSplitter->setStretchFactor(1, 1);
    mainSplitter->setStretchFactor(2, 0);

    // Bottom Panel
    QWidget *bottomWidget = new QWidget();
    bottomWidget->setMaximumHeight(180);
    QVBoxLayout *bottomLayout = new QVBoxLayout(bottomWidget);
    bottomLayout->setSpacing(8);

    QSplitter *bottomSplitter = new QSplitter(Qt::Horizontal);

    QGroupBox *stateGroup = new QGroupBox("Algorithm State");
    QVBoxLayout *stateLayout = new QVBoxLayout(stateGroup);
    stateText = new QTextEdit();
    stateText->setReadOnly(true);
    stateText->setPlainText("Waiting for algorithm start...");
    stateLayout->addWidget(stateText);
    bottomSplitter->addWidget(stateGroup);

    QGroupBox *explanationGroup = new QGroupBox("Step Explanation");
    QVBoxLayout *explanationLayout = new QVBoxLayout(explanationGroup);
    explanationText = new QTextEdit();
    explanationText->setReadOnly(true);
    explanationText->setPlainText("Select an algorithm and click 'Start' to begin visualization.");
    explanationLayout->addWidget(explanationText);
    bottomSplitter->addWidget(explanationGroup);

    bottomSplitter->setStretchFactor(0, 1);
    bottomSplitter->setStretchFactor(1, 1);

    bottomLayout->addWidget(bottomSplitter);

    QWidget *mainWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(mainSplitter, 1);
    mainLayout->addWidget(bottomWidget, 0);
    setCentralWidget(mainWidget);

    // Menu Bar
    QMenuBar *menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    // File menu
    QMenu *fileMenu = menuBar->addMenu("File");
    fileMenu->addAction("New Graph", this, &MainWindow::onNewGraph, QKeySequence::New);
    fileMenu->addAction("Open...", this, &MainWindow::onLoadGraph, QKeySequence::Open);
    fileMenu->addAction("Save", this, &MainWindow::onSaveGraph, QKeySequence::Save);
    fileMenu->addAction("Save As...", this, &MainWindow::onSaveGraphAs, QKeySequence::SaveAs);
    fileMenu->addSeparator();
    fileMenu->addAction("Exit", this, &QWidget::close, QKeySequence::Quit);

    // Edit menu
    QMenu *editMenu = menuBar->addMenu("Edit");

    QMenu *themeMenu = editMenu->addMenu("Theme");
    QAction *darkThemeAction = themeMenu->addAction("Dark Theme");
    QAction *lightThemeAction = themeMenu->addAction("Light Theme");
    connect(darkThemeAction, &QAction::triggered, this, &MainWindow::onDarkTheme);
    connect(lightThemeAction, &QAction::triggered, this, &MainWindow::onLightTheme);

    editMenu->addSeparator();
    editMenu->addAction("Clear Graph", this, &MainWindow::onNewGraph, QKeySequence(Qt::CTRL | Qt::Key_D));

    // View menu
    QMenu *viewMenu = menuBar->addMenu("View");
    viewMenu->addAction("Zoom In", this, &MainWindow::onZoomIn, QKeySequence::ZoomIn);
    viewMenu->addAction("Zoom Out", this, &MainWindow::onZoomOut, QKeySequence::ZoomOut);
    viewMenu->addAction("Fit View", this, &MainWindow::onFitView, QKeySequence(Qt::CTRL | Qt::Key_0));

    // Algorithms menu
    QMenu *algorithmsMenu = menuBar->addMenu("Algorithms");
    QAction *algorithmsInfoAction = algorithmsMenu->addAction("Algorithm Documentation");
    algorithmsInfoAction->setShortcut(QKeySequence(Qt::Key_F1));
    connect(algorithmsInfoAction, &QAction::triggered, [this]() {
        AlgorithmInfoDialog dialog(this);
        dialog.exec();
    });

    algorithmsMenu->addSeparator();

    // Add quick algorithm access
    QAction *bfsAction = algorithmsMenu->addAction("Run BFS");
    connect(bfsAction, &QAction::triggered, [this]() {
        algorithmCombo->setCurrentIndex(0);
        onStartAlgorithm();
    });

    QAction *dfsAction = algorithmsMenu->addAction("Run DFS");
    connect(dfsAction, &QAction::triggered, [this]() {
        algorithmCombo->setCurrentIndex(1);
        onStartAlgorithm();
    });

    QAction *dijkstraAction = algorithmsMenu->addAction("Run Dijkstra");
    connect(dijkstraAction, &QAction::triggered, [this]() {
        algorithmCombo->setCurrentIndex(2);
        onStartAlgorithm();
    });

    QAction *astarAction = algorithmsMenu->addAction("Run A*");
    connect(astarAction, &QAction::triggered, [this]() {
        algorithmCombo->setCurrentIndex(3);
        onStartAlgorithm();
    });

    algorithmsMenu->addSeparator();

    QAction *primAction = algorithmsMenu->addAction("Run Prim (MST)");
    connect(primAction, &QAction::triggered, [this]() {
        algorithmCombo->setCurrentIndex(5);
        onStartAlgorithm();
    });

    QAction *kruskalAction = algorithmsMenu->addAction("Run Kruskal (MST)");
    connect(kruskalAction, &QAction::triggered, [this]() {
        algorithmCombo->setCurrentIndex(6);
        onStartAlgorithm();
    });

    algorithmsMenu->addSeparator();

    QAction *topoAction = algorithmsMenu->addAction("Run Topological Sort");
    connect(topoAction, &QAction::triggered, [this]() {
        algorithmCombo->setCurrentIndex(7);
        onStartAlgorithm();
    });

    QAction *sccAction = algorithmsMenu->addAction("Find SCC");
    connect(sccAction, &QAction::triggered, [this]() {
        algorithmCombo->setCurrentIndex(8);
        onStartAlgorithm();
    });

    // Help menu
    QMenu *helpMenu = menuBar->addMenu("Help");
    helpMenu->addAction("About", this, &MainWindow::onAbout);
    helpMenu->addAction("About Qt", qApp, &QApplication::aboutQt);

    statusBar()->showMessage("Ready");

    setupConnections();
    connect(clearGraphBtn, &QPushButton::clicked, this, &MainWindow::onNewGraph);
    connect(edgeTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this](int index) {
                m_graphWidget->setDirectedMode(index == 0);
                updateGraphInfo();
                m_graphWidget->updateEdgesGeometry();
            });
    connect(weightEdit, &QLineEdit::textChanged, this, &MainWindow::onEdgeWeightChanged);

    onMoveMode();

    updateVertexCombos();
    updateGraphInfo();

    connect(m_graphWidget, &GraphWidget::stateUpdated,
            [this](const QString& structures, const QString& explanation) {
        stateText->append(structures);
        explanationText->append(explanation);
        updateVertexCombos();
        updateGraphInfo();
    });
}

// Добавьте эту функцию в класс MainWindow (перед setTheme)

void MainWindow::applyButtonStyles()
{
    // Применяем стили для активной кнопки в зависимости от текущей темы
    QString activeStyle = (currentTheme == "dark") ?
        "background-color: #007acc; border: 2px solid #ffcc00;" :
        "background-color: #007acc; border: 2px solid #ffaa00; color: white;";

    QString inactiveStyle = (currentTheme == "dark") ?
        "background-color: #3d3d3d; color: #ffffff;" :
        "background-color: #e0e0e0; color: #000000;";

    QString deleteActiveStyle = (currentTheme == "dark") ?
        "background-color: #8b0000; border: 2px solid #ffcc00; color: #ffffff;" :
        "background-color: #c0392b; border: 2px solid #ffaa00; color: #ffffff;";

    QString deleteInactiveStyle = (currentTheme == "dark") ?
        "background-color: #8b0000; color: #ffffff;" :
        "background-color: #c0392b; color: #ffffff;";

    // Определяем какая кнопка сейчас активна
    QPushButton* activeButton = nullptr;
    if (addVertexBtn->styleSheet().contains("border: 2px solid #ffcc00") ||
        addVertexBtn->styleSheet().contains("border: 2px solid #ffaa00")) {
        activeButton = addVertexBtn;
    } else if (addEdgeBtn->styleSheet().contains("border: 2px solid #ffcc00") ||
               addEdgeBtn->styleSheet().contains("border: 2px solid #ffaa00")) {
        activeButton = addEdgeBtn;
    } else if (deleteEdgeBtn->styleSheet().contains("border: 2px solid #ffcc00") ||
               deleteEdgeBtn->styleSheet().contains("border: 2px solid #ffaa00")) {
        activeButton = deleteEdgeBtn;
    } else if (editEdgeBtn->styleSheet().contains("border: 2px solid #ffcc00") ||
               editEdgeBtn->styleSheet().contains("border: 2px solid #ffaa00")) {
        activeButton = editEdgeBtn;
    } else if (deleteBtn->styleSheet().contains("border: 2px solid #ffcc00") ||
               deleteBtn->styleSheet().contains("border: 2px solid #ffaa00")) {
        activeButton = deleteBtn;
    } else if (moveBtn->styleSheet().contains("border: 2px solid #ffcc00") ||
               moveBtn->styleSheet().contains("border: 2px solid #ffaa00")) {
        activeButton = moveBtn;
    }

    // Применяем стили
    addVertexBtn->setStyleSheet((activeButton == addVertexBtn) ? activeStyle : inactiveStyle);
    addEdgeBtn->setStyleSheet((activeButton == addEdgeBtn) ? activeStyle : inactiveStyle);
    deleteEdgeBtn->setStyleSheet((activeButton == deleteEdgeBtn) ? deleteActiveStyle : deleteInactiveStyle);
    editEdgeBtn->setStyleSheet((activeButton == editEdgeBtn) ? activeStyle : inactiveStyle);
    deleteBtn->setStyleSheet((activeButton == deleteBtn) ? deleteActiveStyle : deleteInactiveStyle);
    moveBtn->setStyleSheet((activeButton == moveBtn) ? activeStyle : inactiveStyle);

    // Стиль для кнопки Clear Graph
    QPushButton* clearGraphBtn = findChild<QPushButton*>("clearGraphBtn");
    if (clearGraphBtn) {
        clearGraphBtn->setStyleSheet((currentTheme == "dark") ?
            "background-color: #8b0000; color: #ffffff; border: none; border-radius: 4px; padding: 8px 12px; font-weight: bold;" :
            "background-color: #c0392b; color: #ffffff; border: none; border-radius: 4px; padding: 8px 12px; font-weight: bold;");
    }
}

void MainWindow::setTheme(const QString& theme)
{
    currentTheme = theme;

    if (theme == "light") {
        QString lightStyle = R"(
            QMainWindow {
                background-color: #f5f5f5;
            }
            QMenuBar {
                background-color: #e0e0e0;
                color: #000000;
                border-bottom: 1px solid #c0c0c0;
            }
            QMenuBar::item {
                background-color: transparent;
                padding: 6px 12px;
            }
            QMenuBar::item:selected {
                background-color: #007acc;
                color: white;
            }
            QMenu {
                background-color: #e0e0e0;
                color: #000000;
                border: 1px solid #c0c0c0;
            }
            QMenu::item {
                padding: 6px 24px;
            }
            QMenu::item:selected {
                background-color: #007acc;
                color: white;
            }
            QToolBar {
                background-color: #e0e0e0;
                border: none;
                border-bottom: 1px solid #c0c0c0;
                spacing: 8px;
                padding: 4px;
            }
            QToolBar QToolButton {
                background-color: #d0d0d0;
                color: #000000;
                border: none;
                border-radius: 4px;
                padding: 6px 12px;
            }
            QToolBar QToolButton:hover {
                background-color: #007acc;
                color: white;
            }
            QGroupBox {
                color: #000000;
                border: 1px solid #c0c0c0;
                border-radius: 6px;
                margin-top: 12px;
                font-weight: bold;
                background-color: #f0f0f0;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 12px;
                padding: 0 6px;
                color: #007acc;
            }
            QLabel {
                color: #000000;
            }
            QPushButton {
                background-color: #d0d0d0;
                color: #000000;
                border: none;
                border-radius: 4px;
                padding: 8px 12px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #007acc;
                color: white;
            }
            QPushButton:pressed {
                background-color: #005a9e;
            }
            QComboBox {
                background-color: white;
                color: #000000;
                border: 1px solid #c0c0c0;
                border-radius: 4px;
                padding: 6px;
            }
            QComboBox::drop-down {
                border: none;
            }
            QComboBox QAbstractItemView {
                background-color: white;
                color: #000000;
                selection-background-color: #007acc;
                selection-color: white;
            }
            QLineEdit {
                background-color: white;
                color: #000000;
                border: 1px solid #c0c0c0;
                border-radius: 4px;
                padding: 4px;
            }
            QSlider::groove:horizontal {
                height: 4px;
                background-color: #d0d0d0;
                border-radius: 2px;
            }
            QSlider::handle:horizontal {
                background-color: #007acc;
                width: 12px;
                height: 12px;
                margin: -4px 0;
                border-radius: 6px;
            }
            QSlider::sub-page:horizontal {
                background-color: #007acc;
                border-radius: 2px;
            }
            QTextEdit {
                background-color: white;
                color: #000000;
                border: 1px solid #c0c0c0;
                border-radius: 4px;
                font-family: 'Consolas', 'Monaco', monospace;
                font-size: 11px;
            }
            QTabWidget::pane {
                border: 1px solid #c0c0c0;
                border-radius: 4px;
                background-color: #f0f0f0;
            }
            QTabBar::tab {
                background-color: #d0d0d0;
                color: #000000;
                padding: 8px 16px;
                margin-right: 2px;
                border-top-left-radius: 4px;
                border-top-right-radius: 4px;
            }
            QTabBar::tab:selected {
                background-color: #007acc;
                color: white;
            }
            QTabBar::tab:hover {
                background-color: #b0b0b0;
            }
            QStatusBar {
                background-color: #e0e0e0;
                color: #000000;
            }
            QScrollBar:vertical {
                background-color: #f0f0f0;
                width: 12px;
            }
            QScrollBar::handle:vertical {
                background-color: #c0c0c0;
                border-radius: 6px;
                min-height: 20px;
            }
            QScrollBar::handle:vertical:hover {
                background-color: #a0a0a0;
            }
        )";
        this->setStyleSheet(lightStyle);

        // Меняем фон графической сцены на светлый
        m_graphWidget->setStyleSheet("background-color: #fafafa; border: 1px solid #c0c0c0; border-radius: 8px;");

        // Меняем фон самой сцены через QGraphicsScene
        QGraphicsScene* scene = m_graphWidget->scene();
        if (scene) {
            scene->setBackgroundBrush(QColor(250, 250, 250));  // Светлый фон
        }

        // Обновляем цвета вершин и ребер для лучшей видимости на светлом фоне
        QMap<int, Vertex*> vertices = m_graphWidget->getVertices();
        for (Vertex* v : vertices) {
            v->setColor(QColor(0, 150, 200));  // Синий цвет для вершин
            // Обновляем цвета текста
            v->resetAppearance();
        }

        // Обновляем цвета ребер
        QList<Edge*> edges = m_graphWidget->getEdges();
        for (Edge* e : edges) {
            e->lineItem->setPen(QPen(Qt::darkGray, 2));  // Темно-серые линии на светлом фоне
            if (e->arrowItem) {
                e->arrowItem->setPen(QPen(Qt::darkGray, 1.5));
                e->arrowItem->setBrush(QBrush(Qt::darkGray));
            }
            e->weightItem->setDefaultTextColor(Qt::darkBlue);  // Темно-синий текст веса
        }

    } else {
        QString darkStyle = R"(
            QMainWindow {
                background-color: #1e1e1e;
            }
            QMenuBar {
                background-color: #2d2d2d;
                color: #ffffff;
                border-bottom: 1px solid #3d3d3d;
            }
            QMenuBar::item {
                background-color: transparent;
                padding: 6px 12px;
            }
            QMenuBar::item:selected {
                background-color: #3d3d3d;
            }
            QMenu {
                background-color: #2d2d2d;
                color: #ffffff;
                border: 1px solid #3d3d3d;
            }
            QMenu::item {
                padding: 6px 24px;
            }
            QMenu::item:selected {
                background-color: #007acc;
            }
            QToolBar {
                background-color: #2d2d2d;
                border: none;
                border-bottom: 1px solid #3d3d3d;
                spacing: 8px;
                padding: 4px;
            }
            QToolBar QToolButton {
                background-color: #3d3d3d;
                color: #ffffff;
                border: none;
                border-radius: 4px;
                padding: 6px 12px;
            }
            QToolBar QToolButton:hover {
                background-color: #007acc;
            }
            QGroupBox {
                color: #ffffff;
                border: 1px solid #3d3d3d;
                border-radius: 6px;
                margin-top: 12px;
                font-weight: bold;
                background-color: #252525;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 12px;
                padding: 0 6px;
                color: #007acc;
            }
            QLabel {
                color: #d4d4d4;
            }
            QPushButton {
                background-color: #3d3d3d;
                color: #ffffff;
                border: none;
                border-radius: 4px;
                padding: 8px 12px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #007acc;
            }
            QPushButton:pressed {
                background-color: #005a9e;
            }
            QComboBox {
                background-color: #3d3d3d;
                color: #ffffff;
                border: 1px solid #4d4d4d;
                border-radius: 4px;
                padding: 6px;
            }
            QComboBox::drop-down {
                border: none;
            }
            QComboBox QAbstractItemView {
                background-color: #3d3d3d;
                color: #ffffff;
                selection-background-color: #007acc;
            }
            QLineEdit {
                background-color: #3d3d3d;
                color: #ffffff;
                border: 1px solid #4d4d4d;
                border-radius: 4px;
                padding: 4px;
            }
            QSlider::groove:horizontal {
                height: 4px;
                background-color: #3d3d3d;
                border-radius: 2px;
            }
            QSlider::handle:horizontal {
                background-color: #007acc;
                width: 12px;
                height: 12px;
                margin: -4px 0;
                border-radius: 6px;
            }
            QSlider::sub-page:horizontal {
                background-color: #007acc;
                border-radius: 2px;
            }
            QTextEdit {
                background-color: #1e1e1e;
                color: #d4d4d4;
                border: 1px solid #3d3d3d;
                border-radius: 4px;
                font-family: 'Consolas', 'Monaco', monospace;
                font-size: 11px;
            }
            QTabWidget::pane {
                border: 1px solid #3d3d3d;
                border-radius: 4px;
                background-color: #252525;
            }
            QTabBar::tab {
                background-color: #3d3d3d;
                color: #d4d4d4;
                padding: 8px 16px;
                margin-right: 2px;
                border-top-left-radius: 4px;
                border-top-right-radius: 4px;
            }
            QTabBar::tab:selected {
                background-color: #007acc;
                color: #ffffff;
            }
            QTabBar::tab:hover {
                background-color: #4d4d4d;
            }
            QStatusBar {
                background-color: #2d2d2d;
                color: #ffffff;
            }
            QScrollBar:vertical {
                background-color: #2d2d2d;
                width: 12px;
            }
            QScrollBar::handle:vertical {
                background-color: #4d4d4d;
                border-radius: 6px;
                min-height: 20px;
            }
            QScrollBar::handle:vertical:hover {
                background-color: #5d5d5d;
            }
        )";
        this->setStyleSheet(darkStyle);

        // Меняем фон графической сцены на темный
        m_graphWidget->setStyleSheet("background-color: #1a1a1a; border: 1px solid #3d3d3d; border-radius: 8px;");

        // Меняем фон самой сцены через QGraphicsScene
        QGraphicsScene* scene = m_graphWidget->scene();
        if (scene) {
            scene->setBackgroundBrush(QColor(30, 30, 40));  // Темный фон
        }

        // Обновляем цвета вершин и ребер для темного фона
        QMap<int, Vertex*> vertices = m_graphWidget->getVertices();
        for (Vertex* v : vertices) {
            v->setColor(QColor(0, 150, 200));  // Синий цвет для вершин
            v->resetAppearance();
        }

        // Обновляем цвета ребер
        QList<Edge*> edges = m_graphWidget->getEdges();
        for (Edge* e : edges) {
            e->lineItem->setPen(QPen(Qt::white, 2));  // Белые линии на темном фоне
            if (e->arrowItem) {
                e->arrowItem->setPen(QPen(Qt::white, 1.5));
                e->arrowItem->setBrush(QBrush(Qt::white));
            }
            e->weightItem->setDefaultTextColor(Qt::cyan);  // Бирюзовый текст веса
        }
    }

    // Обновляем отображение графа
    m_graphWidget->updateEdgesGeometry();
    m_graphWidget->update();

    // Восстанавливаем стили кнопок после смены темы
    applyButtonStyles();
}

void MainWindow::onLightTheme()
{
    setTheme("light");
    statusBar()->showMessage("Light theme applied");
}

void MainWindow::onDarkTheme()
{
    setTheme("dark");
    statusBar()->showMessage("Dark theme applied");
}

void MainWindow::updateModeButtons(QPushButton* activeButton)
{
    QString activeStyle = "background-color: #007acc; border: 2px solid #ffcc00;";
    QString inactiveStyle = "background-color: #3d3d3d;";
    QString deleteActiveStyle = "background-color: #8b0000; border: 2px solid #ffcc00;";
    QString deleteInactiveStyle = "background-color: #8b0000;";

    addVertexBtn->setStyleSheet((activeButton == addVertexBtn) ? activeStyle : inactiveStyle);
    addEdgeBtn->setStyleSheet((activeButton == addEdgeBtn) ? activeStyle : inactiveStyle);
    deleteEdgeBtn->setStyleSheet((activeButton == deleteEdgeBtn) ? deleteActiveStyle : deleteInactiveStyle);
    editEdgeBtn->setStyleSheet((activeButton == editEdgeBtn) ? activeStyle : inactiveStyle);
    deleteBtn->setStyleSheet((activeButton == deleteBtn) ? deleteActiveStyle : deleteInactiveStyle);
    moveBtn->setStyleSheet((activeButton == moveBtn) ? activeStyle : inactiveStyle);
}

void MainWindow::setupConnections()
{
    connect(startBtn, &QPushButton::clicked, this, &MainWindow::onStartAlgorithm);
    connect(pauseBtn, &QPushButton::clicked, this, &MainWindow::onPauseAlgorithm);
    connect(resetBtn, &QPushButton::clicked, this, &MainWindow::onResetAlgorithm);
    connect(stepForwardBtn, &QPushButton::clicked, this, &MainWindow::onStepForward);
    connect(stepBackwardBtn, &QPushButton::clicked, this, &MainWindow::onStepBackward);
    connect(speedSlider, &QSlider::valueChanged, this, &MainWindow::onSpeedChanged);
    connect(algorithmCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onAlgorithmChanged);
    connect(addVertexBtn, &QPushButton::clicked, this, &MainWindow::onAddVertexMode);
    connect(deleteBtn, &QPushButton::clicked, this, &MainWindow::onDeleteMode);
    connect(addEdgeBtn, &QPushButton::clicked, this, &MainWindow::onAddEdgeMode);
    connect(deleteEdgeBtn, &QPushButton::clicked, this, &MainWindow::onDeleteEdgeMode);
    connect(editEdgeBtn, &QPushButton::clicked, this, &MainWindow::onEditEdgeMode);
    connect(moveBtn, &QPushButton::clicked, this, &MainWindow::onMoveMode);
}

void MainWindow::updateVertexCombos()
{
    QString currentStart = startVertexCombo->currentText();
    QString currentTarget = targetVertexCombo->currentText();

    startVertexCombo->clear();
    targetVertexCombo->clear();

    QMap<int, Vertex*> vertices = m_graphWidget->getVertices();
    for (int id : vertices.keys()) {
        startVertexCombo->addItem(QString::number(id));
        targetVertexCombo->addItem(QString::number(id));
    }

    int startIndex = startVertexCombo->findText(currentStart);
    if (startIndex >= 0) {
        startVertexCombo->setCurrentIndex(startIndex);
    } else if (startVertexCombo->count() > 0) {
        startVertexCombo->setCurrentIndex(0);
    }

    int targetIndex = targetVertexCombo->findText(currentTarget);
    if (targetIndex >= 0) {
        targetVertexCombo->setCurrentIndex(targetIndex);
    } else if (targetVertexCombo->count() > 0) {
        targetVertexCombo->setCurrentIndex(0);
    }
}

void MainWindow::updateGraphInfo()
{
    verticesCountLabel->setText("Vertices: " + QString::number(m_graphWidget->getVertices().size()));
    edgesCountLabel->setText("Edges: " + QString::number(m_graphWidget->getEdges().size()));
    directedLabel->setText("Type: " + QString(m_graphWidget->isDirectedMode() ? "Directed" : "Undirected"));
}

void MainWindow::onNewGraph()
{
    m_graphWidget->clearGraph();
    updateVertexCombos();
    updateGraphInfo();
    statusBar()->showMessage("New graph created");

    startVertexCombo->clear();
    targetVertexCombo->clear();
    startVertexCombo->addItem("1");
    targetVertexCombo->addItem("1");
}

void MainWindow::onLoadGraph()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load Graph", "", "JSON (*.json)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (m_graphWidget->fromJson(doc.object())) {
            statusBar()->showMessage("Graph loaded from " + fileName);
            updateVertexCombos();
            updateGraphInfo();
        } else {
            statusBar()->showMessage("Failed to load graph");
        }
        file.close();
    }
}

void MainWindow::onSaveGraph()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save Graph", "", "JSON (*.json)");
    if (fileName.isEmpty()) return;

    QJsonObject json = m_graphWidget->toJson();
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(json).toJson());
        file.close();
        statusBar()->showMessage("Graph saved to " + fileName);
    }
}

void MainWindow::onSaveGraphAs()
{
    onSaveGraph();
}

void MainWindow::onAlgorithmChanged()
{
    int index = algorithmCombo->currentIndex();
    bool needsStartTarget = (index == 0 || index == 1 || index == 2 || index == 3 || index == 4);

    startVertexCombo->setEnabled(needsStartTarget);
    targetVertexCombo->setEnabled(index == 2 || index == 3 || index == 4);

    statusBar()->showMessage("Selected algorithm: " + algorithmCombo->currentText());
}

void MainWindow::onStartAlgorithm()
{
    int index = algorithmCombo->currentIndex();
    Algorithm algo = static_cast<Algorithm>(index);

    bool startOk;
    int startId = startVertexCombo->currentText().toInt(&startOk);
    if (startOk && startId > 0) {
        m_graphWidget->setStartVertex(startId);
    } else {
        statusBar()->showMessage("Please enter a valid start vertex ID");
        return;
    }

    bool needsEnd = (index == 2 || index == 3 || index == 4);
    if (needsEnd) {
        bool endOk;
        int endId = targetVertexCombo->currentText().toInt(&endOk);
        if (endOk && endId > 0) {
            m_graphWidget->setEndVertex(endId);
        } else {
            statusBar()->showMessage("Please enter a valid end vertex ID");
            return;
        }
    }

    stateText->clear();
    explanationText->clear();

    m_graphWidget->startAlgorithm(algo);
    statusBar()->showMessage("Running: " + algorithmCombo->currentText());
}

void MainWindow::onPauseAlgorithm()
{
    m_graphWidget->pauseAlgorithm();
    statusBar()->showMessage("Algorithm paused");
}

void MainWindow::onResetAlgorithm()
{
    m_graphWidget->resetAlgorithm();
    stateText->clear();
    explanationText->clear();
    statusBar()->showMessage("Algorithm reset");

    bool startOk;
    int startId = startVertexCombo->currentText().toInt(&startOk);
    if (startOk && startId > 0) {
        m_graphWidget->setStartVertex(startId);
    }

    bool endOk;
    int endId = targetVertexCombo->currentText().toInt(&endOk);
    if (endOk && endId > 0) {
        m_graphWidget->setEndVertex(endId);
    }
}

void MainWindow::onStepForward()
{
    m_graphWidget->stepForward();
}

void MainWindow::onStepBackward()
{
    m_graphWidget->stepBackward();
}

void MainWindow::onSpeedChanged(int value)
{
    double stepsPerSec = 0.1 + (value / 100.0) * 9.9;
    m_graphWidget->setAnimationSpeed(static_cast<int>(stepsPerSec * 100));
    speedLabel->setText(QString::number(stepsPerSec, 'f', 1) + " steps/second");
    statusBar()->showMessage("Animation speed: " + QString::number(stepsPerSec, 'f', 1) + " steps/sec");
}

void MainWindow::onAddVertexMode()
{
    m_graphWidget->setMode(ModeAddVertex);
    updateModeButtons(addVertexBtn);
    statusBar()->showMessage("Add vertex mode: click on canvas to add vertices");
}

void MainWindow::onDeleteMode()
{
    m_graphWidget->setMode(ModeRemove);
    updateModeButtons(deleteBtn);
    statusBar()->showMessage("Delete mode: click on vertex or edge to delete");
}

void MainWindow::onAddEdgeMode()
{
    m_graphWidget->setMode(ModeAddEdge);
    int weight = weightEdit->text().toInt();
    if (weight < 1) weight = 1;
    m_graphWidget->setTempEdgeWeight(weight);
    updateModeButtons(addEdgeBtn);
    statusBar()->showMessage("Add edge mode: click on first vertex, then on second vertex");
}

void MainWindow::onDeleteEdgeMode()
{
    m_graphWidget->setMode(ModeDeleteEdge);
    updateModeButtons(deleteEdgeBtn);
    statusBar()->showMessage("Delete edge mode: click on an edge to delete it");
}

void MainWindow::onEditEdgeMode()
{
    m_graphWidget->setMode(ModeEditEdge);
    updateModeButtons(editEdgeBtn);
    statusBar()->showMessage("Edit edge weight mode: click on an edge to change its weight");
}

void MainWindow::onEdgeWeightChanged()
{
    int weight = weightEdit->text().toInt();
    if (weight < 1) weight = 1;
    m_graphWidget->setTempEdgeWeight(weight);
}

void MainWindow::onMoveMode()
{
    m_graphWidget->setMode(ModeMove);
    updateModeButtons(moveBtn);
    statusBar()->showMessage("Move mode: drag vertices to reposition them");
}

void MainWindow::onZoomIn()
{
    m_graphWidget->scale(1.2, 1.2);
}

void MainWindow::onZoomOut()
{
    m_graphWidget->scale(0.8, 0.8);
}

void MainWindow::onFitView()
{
    m_graphWidget->fitInView(m_graphWidget->sceneRect(), Qt::KeepAspectRatio);
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "About Algraf",
                       "<h2>Algraf - Graph Algorithm Visualizer</h2>"
                       "<p>Version 2.0</p>"
                       "<p>Interactive visualization of graph algorithms:</p>"
                       "<ul>"
                       "<li>BFS (Breadth-First Search)</li>"
                       "<li>DFS (Depth-First Search)</li>"
                       "<li>Dijkstra's Algorithm</li>"
                       "<li>A* Algorithm</li>"
                       "<li>Bellman-Ford Algorithm</li>"
                       "<li>Prim's Algorithm (MST)</li>"
                       "<li>Kruskal's Algorithm (MST)</li>"
                       "<li>Topological Sort</li>"
                       "<li>Strongly Connected Components (Kosaraju)</li>"
                       "</ul>"
                       "<p><b>New Features:</b></p>"
                       "<ul>"
                       "<li>Algorithm documentation in Algorithms menu (F1)</li>"
                       "<li>Light/Dark theme switching in Edit menu</li>"
                       "<li>Quick algorithm access from menu bar</li>"
                       "</ul>"
                       "<p>Copyright 2024</p>");
}

void MainWindow::onAlgorithmFinished(const QString& result, bool success)
{
    if (success) {
        statusBar()->setStyleSheet("QStatusBar { background-color: #1e7e34; color: white; }");
        statusBar()->showMessage("Success: " + result);
        QTimer::singleShot(3000, [this]() {
            statusBar()->setStyleSheet("");
            statusBar()->showMessage("Ready");
        });
    } else {
        statusBar()->setStyleSheet("QStatusBar { background-color: #c0392b; color: white; }");
        statusBar()->showMessage("Error: " + result);
        QTimer::singleShot(3000, [this]() {
            statusBar()->setStyleSheet("");
            statusBar()->showMessage("Ready");
        });
    }

    if (!success && (result.contains("NOT found") || result.contains("unreachable") ||
                     result.contains("not connected") || result.contains("Cycle detected") ||
                     result.contains("Negative cycle"))) {
        QMessageBox::warning(this, "Algorithm Result",
                             "<b>Algorithm could not complete successfully</b><br><br>" + result);
    }

    stateText->append(success ? "[SUCCESS] " + result : "[ERROR] " + result);
    explanationText->append(success ? "Algorithm finished successfully." : "Algorithm finished with issues.");

    stateText->verticalScrollBar()->setValue(stateText->verticalScrollBar()->maximum());
    explanationText->verticalScrollBar()->setValue(explanationText->verticalScrollBar()->maximum());
}

MainWindow::~MainWindow()
{
}
