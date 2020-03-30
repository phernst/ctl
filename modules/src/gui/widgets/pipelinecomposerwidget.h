#ifndef CTL_PIPELINECOMPOSERWIDGET_H
#define CTL_PIPELINECOMPOSERWIDGET_H

#include <memory>

#include <QWidget>
#include <QListWidget>
#include <QGridLayout>

namespace Ui {
class PipelineComposerWidget;
}

namespace CTL
{
class ProjectionPipeline;
class ProjectorExtension;
class AbstractProjector;
}

namespace CTL {
namespace gui {

class PipelineComposerWidget : public QWidget
{
    Q_OBJECT

public:
    static constexpr int ProjectorTypeOffset = 1000;

    explicit PipelineComposerWidget(QWidget *parent = 0);
    ~PipelineComposerWidget();

    std::unique_ptr<ProjectionPipeline> pipeline() const;

    static std::unique_ptr<ProjectionPipeline> fromDialog();

private slots:
    void on__LW_projectorProto_itemDoubleClicked(QListWidgetItem *item);

private:
    Ui::PipelineComposerWidget *ui;

    void initializeExtensionPrototypes();
    void initializeProjectorPrototypes();
    std::unique_ptr<ProjectorExtension> createExtension(int type) const;
    std::unique_ptr<AbstractProjector> createProjector(int type) const;
};

class ExtensionConfigWidget : public QWidget
{
    Q_OBJECT

public:
    ExtensionConfigWidget(QWidget* parent = nullptr);


public slots:
    void updateInterface(QListWidgetItem* item);

private:
    QListWidgetItem* _currentItem;
    QGridLayout* _layout;

    void clearLayout();
    static QVariant parsedInputWidget(QWidget* widget);
    void setExtensionObject(QListWidgetItem* item);

private slots:
    void somethingChanged();
};

} // namespace gui
} // namespace CTL

#endif // CTL_PIPELINECOMPOSERWIDGET_H
