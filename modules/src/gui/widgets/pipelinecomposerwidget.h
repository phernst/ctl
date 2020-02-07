#ifndef PIPELINECOMPOSERWIDGET_H
#define PIPELINECOMPOSERWIDGET_H

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

class PipelineComposerWidget : public QWidget
{
    Q_OBJECT

public:
    static constexpr int ProjectorTypeOffset = 1000;

    explicit PipelineComposerWidget(QWidget *parent = 0);
    ~PipelineComposerWidget();

    std::unique_ptr<CTL::ProjectionPipeline> pipeline() const;

    static std::unique_ptr<CTL::ProjectionPipeline> fromDialog();

private slots:
    void on__LW_projectorProto_itemDoubleClicked(QListWidgetItem *item);

private:
    Ui::PipelineComposerWidget *ui;

    void initializeExtensionPrototypes();
    void initializeProjectorPrototypes();
    std::unique_ptr<CTL::ProjectorExtension> createExtension(int type) const;
    std::unique_ptr<CTL::AbstractProjector> createProjector(int type) const;
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


#endif // PIPELINECOMPOSERWIDGET_H
