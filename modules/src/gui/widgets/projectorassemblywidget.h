#ifndef PROJECTORASSEMBLYWIDGET_H
#define PROJECTORASSEMBLYWIDGET_H

#include <QWidget>
#include <array>

QT_BEGIN_NAMESPACE
namespace Ui { class ProjectorAssemblyWidget; }
QT_END_NAMESPACE

class ExtensionConfigWidget;
class QListWidgetItem;

class ProjectorAssemblyWidget : public QWidget
{
    Q_OBJECT

public:
    ProjectorAssemblyWidget(QWidget *parent = nullptr);
    ~ProjectorAssemblyWidget();

    enum Extension
    {
        ArealFocalSpotExtension,
        PoissonNoiseExtension,
        SpectralEffectsExtension,
        DetectorSaturationExtension,
        Count
    };

    enum class PhysicalCompatibility
    {
        False,  //!< unphysical
        Approx, //!< physical approximation
        Ineff,  //!< physically correct but inefficient to compute
        True    //!< physically correct
    };

    using FirstExtension = std::array<PhysicalCompatibility, Extension::Count>;
    using CompatibilityMatrix = std::array<FirstExtension, Extension::Count>;
    using ExtensionNames = std::array<const char*, Extension::Count>;
    using CompatibilityReport = std::vector<std::pair<Extension, PhysicalCompatibility>>;

    static const ExtensionNames& extensionNames();
    static const CompatibilityMatrix& compatibilityMatrix();
    static CompatibilityReport reportPhysicalCompatibility(const std::vector<Extension>& extensions);
    static QString compatibilityReport2String(const std::vector<Extension>& extensions,
                                              const CompatibilityReport& report);
    QString codeString();

    void setExtensionPrototypes(const QVector<QListWidgetItem*>& prototypes);
    QVector<QListWidgetItem*> extensions();

signals:
    void extensionItemClicked(QListWidgetItem* item);

private slots:
    void on_extensionList_itemDoubleClicked(QListWidgetItem* item);
    void on_pipelineList_itemDoubleClicked(QListWidgetItem* item);
    void updateViewer();

private:
    Ui::ProjectorAssemblyWidget* ui;

    void initExtensionList();
};

#endif // PROJECTORASSEMBLYWIDGET_H
