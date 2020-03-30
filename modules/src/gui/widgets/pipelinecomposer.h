#ifndef CTL_PIPELINECOMPOSER_H
#define CTL_PIPELINECOMPOSER_H

#include <memory>

#include <QWidget>
#include <QListWidget>
#include <QGridLayout>

namespace Ui {
class PipelineComposer;
}

namespace CTL
{
class ProjectionPipeline;
class ProjectorExtension;
class AbstractProjector;
}

namespace CTL {
namespace gui {

class PipelineComposer : public QWidget
{
    Q_OBJECT

public:
    static constexpr int ProjectorTypeOffset = 1000;

    explicit PipelineComposer(QWidget *parent = 0);
    ~PipelineComposer();

    std::unique_ptr<ProjectionPipeline> pipeline() const;

    static std::unique_ptr<ProjectionPipeline> fromDialog();

private slots:
    void on__LW_projectorProto_itemDoubleClicked(QListWidgetItem *item);

private:
    Ui::PipelineComposer *ui;

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
