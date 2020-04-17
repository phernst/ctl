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

/*!
 * \class PipelineComposer
 *
 * \brief The PipelineComposer class provides a user interface to put together a simulation pipeline
 * consisting of a projector and an arbitrary number of extensions.
 *
 * This class can be used to compose the pipeline used to create projections from volume data with
 * the CTL. The currently composed pipeline can be obtained using pipeline(). Note that in order to
 * be functional, a pipeline must contain a projector and an arbitrary number of extensions (zero
 * included).
 *
 * The following IO operations are supported by this widget:
 *
 * - Adding/replacing the projector:
 *    - Double-click a projector type from the list of "Available projectors" on the left.
 * - Managing the extension list:
 *    - Adding/removing: Double-click an extension type from the list of "Available extensions" on
 * the left to append it to the pipeline or double click an extension in the current list of
 * "Selected extensions" to remove it from the pipeline.
 *    - Moving extension positions: Left click (and hold) an extension in the current list of
 * "Selected extensions" and drag it to the position within the list where you want to move it to.
 *
 * For convenience, the fromDialog() method can be used to achieve a one-line solution, creating a
 * dialog that will return the composed pipeline once accepted ("OK") by the user.
 *
 * ![Example of a PipelineComposer::fromDialog() window.](gui/PipelineComposer_withBoxes.png)
 *
 * To compose your pipeline, set the projector method that should be used in the simulations by
 * double-clicking it in the list of available projectors (see Figure: red box, top), and add all
 * extensions that shall be applied (same procedure, i.e. double-click it in the corresponding list;
 * Figure: red box, bottom). You can always review your currently composed pipeline in the lists in
 * the middle of the widget (Figure: green box). To change the projection method, simply double-click
 * another projector type in the list and it will replace the previously selected one. Extensions
 * can be removed from the pipeline by double-clicking them in the list of "Selected extensions"
 * (Figure: green box, bottom). The position of an extension within the pipeline can be change by
 * dragging (i.e. left-click it, hold mouse button, and move) it to the desired position in the
 * list. Extensions on the top of the list are closest to the forward projector, while extensions on
 * the bottom will be last in the pipeline. The individual settings of the projector and each of the
 * selected extensions can be inspected and modified by left-clicking the corresponding element in
 * the list (Figure: green box). The settings will the appear in the box on the right of the widget
 * (Figure: violet box), where they can be reviewed and modified.
 * The text box on the bottom (Figure: blue box) provides a source code example that creates the
 * currently composed pipeline (note: individual parameter settings are not included!).
 * Additionally, it shows an evaluation of the order of extensions in the pipeline with respect to
 * whether or not it is physically meaningful and/or computationally efficient (Figure: blue box,
 * bottom).
 *
 * Example: create projections from a ball phantom with a pipeline from the composer dialog
 * \code
 * auto volume = VoxelVolume<float>::ball(50.0f, 0.5f, 0.02f);
 *
 * auto system = CTsystemBuilder::createFromBlueprint(blueprints::GenericCarmCT());
 * AcquisitionSetup setup(system, 100);
 * setup.applyPreparationProtocol(protocols::ShortScanTrajectory(600.0f));
 *
 * auto projector = gui::PipelineComposer::fromDialog();
 *
 * if(!projector) // check if user canceled the dialog
 *     return;
 *
 * projector->configure(setup);
 * auto projections = projector->project(volume);
 *
 * gui::ProjectionViewer::plot(projections);
 * \endcode
 */

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
    void setProjector(QListWidgetItem *item);

private:
    Ui::PipelineComposer *ui;

    void initializeExtensionPrototypes();
    void initializeProjectorPrototypes();
    std::unique_ptr<ProjectorExtension> createExtension(int type) const;
    std::unique_ptr<AbstractProjector> createProjector(int type) const;
};

namespace details
{

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

} // namespace details;

} // namespace gui
} // namespace CTL

#endif // CTL_PIPELINECOMPOSERWIDGET_H
