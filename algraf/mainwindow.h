#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMessageBox>
#include <QTimer>
#include "graphwidget.h"

QT_BEGIN_NAMESPACE
class QPushButton;
class QComboBox;
class QLineEdit;
class QSlider;
class QTextEdit;
class QGroupBox;
class QLabel;
class QAction;
class QToolBar;
class QSplitter;
class QTabWidget;
class QCheckBox;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onNewGraph();
    void onLoadGraph();
    void onSaveGraph();
    void onSaveGraphAs();
    void onAlgorithmChanged();
    void onStartAlgorithm();
    void onPauseAlgorithm();
    void onResetAlgorithm();
    void onStepForward();
    void onStepBackward();
    void onSpeedChanged(int value);
    void onAddVertexMode();
    void onDeleteMode();
    void onAddEdgeMode();
    void onDeleteEdgeMode();
    void onEditEdgeMode();
    void onMoveMode();
    void onAbout();
    void onZoomIn();
    void onZoomOut();
    void onFitView();
    void onEdgeWeightChanged();
    void onAlgorithmFinished(const QString& result, bool success);

    // New slots for theme
    void onLightTheme();
    void onDarkTheme();

private:
    void setupConnections();
    void updateVertexCombos();
    void updateModeButtons(QPushButton* activeButton);
    void updateGraphInfo();
    void applyButtonStyles();
    void setTheme(const QString& theme);  // "light" or "dark"

    GraphWidget *m_graphWidget;

    QPushButton *addVertexBtn;
    QPushButton *deleteBtn;
    QPushButton *addEdgeBtn;
    QPushButton *deleteEdgeBtn;
    QPushButton *editEdgeBtn;
    QPushButton *moveBtn;
    QComboBox *edgeTypeCombo;
    QLineEdit *weightEdit;

    QComboBox *algorithmCombo;
    QComboBox *startVertexCombo;
    QComboBox *targetVertexCombo;

    QPushButton *startBtn;
    QPushButton *pauseBtn;
    QPushButton *resetBtn;
    QPushButton *stepForwardBtn;
    QPushButton *stepBackwardBtn;
    QSlider *speedSlider;
    QLabel *speedLabel;

    QTextEdit *stateText;
    QTextEdit *explanationText;

    QLabel *verticesCountLabel;
    QLabel *edgesCountLabel;
    QLabel *directedLabel;

    QString currentTheme;
};

#endif // MAINWINDOW_H
