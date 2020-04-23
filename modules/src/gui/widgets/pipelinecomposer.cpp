#include "pipelinecomposer.h"
#include "ui_pipelinecomposer.h"

#include "projectors/projectionpipeline.h"
// projectors
#include "projectors/raycasterprojector.h"
// extensions
#include "projectors/arealfocalspotextension.h"
#include "projectors/detectorsaturationextension.h"
#include "projectors/poissonnoiseextension.h"
#include "projectors/spectraleffectsextension.h"
#include "projectors/dynamicprojectorextension.h"


#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>

namespace CTL {
namespace gui {

/*!
 * Constructs a PipelineComposer with the given \a parent.
 */
PipelineComposer::PipelineComposer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PipelineComposer)
{
    ui->setupUi(this);

    connect(ui->_LW_projectorProto, &QListWidget::itemDoubleClicked,
            this, &PipelineComposer::setProjector);
    connect(ui->_w_extensions, &ExtensionChainWidget::extensionItemClicked,
            ui->_W_propertyManager, &details::ExtensionConfigWidget::updateInterface);
    connect(ui->_LW_selectedProjector, &QListWidget::itemClicked,
            ui->_W_propertyManager, &details::ExtensionConfigWidget::updateInterface);

    initializeExtensionPrototypes();
    initializeProjectorPrototypes();

    setWindowTitle("Pipeline composer");
}

/*!
 * Destroys this instance.
 */
PipelineComposer::~PipelineComposer()
{
    delete ui;
}

/*!
 * Returns the currently composed pipeline.
 *
 * \sa ProjectionPipeline.
 */
std::unique_ptr<ProjectionPipeline> PipelineComposer::pipeline() const
{
    std::unique_ptr<ProjectionPipeline> pipe(new ProjectionPipeline);

    auto projectorItem = ui->_LW_selectedProjector->item(0);
    int projectorType = -1;
    if(projectorItem)
        projectorType = int(projectorItem->type()) - int(QListWidgetItem::UserType) - ProjectorTypeOffset;

    auto projector = createProjector(projectorType);
    if(projector)
        projector->setParameter(projectorItem->data(Qt::UserRole));

    pipe->setProjector(std::move(projector));

    auto extensions = ui->_w_extensions->extensions();

    for(const auto& ext : extensions)
    {
        auto newExtension = createExtension(ext->type() - int(QListWidgetItem::UserType));
        if(!newExtension) continue;

        newExtension->setParameter(ext->data(Qt::UserRole));
        pipe->appendExtension(std::move(newExtension));
    }

    return pipe;
}

/*!
 * Creates a dialog window to compose the pipeline and return the composed object once the dialog is
 * accepted by the user by clicking "OK".
 *
 * Returns a \c nullptr if the dialog is cancelled.
 */
std::unique_ptr<ProjectionPipeline> PipelineComposer::fromDialog()
{
    QDialog dialog;
    auto layout = new QVBoxLayout;
    auto composer = new PipelineComposer(&dialog);
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), &dialog, SLOT(accept()));
    connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), &dialog, SLOT(reject()));
    layout->addWidget(composer);
    layout->addWidget(buttonBox);
    dialog.setLayout(layout);
    dialog.setWindowTitle("Pipeline composer");
    dialog.resize(1000,600);

    if(dialog.exec())
        return composer->pipeline();
    else
        return nullptr;
}

void PipelineComposer::initializeExtensionPrototypes()
{
    QVector<QListWidgetItem*> prototypes;

    QStringList extensionNames{ "ArealFocalSpotExtension",
                                "PoissonNoiseExtension",
                                "SpectralEffectsExtension",
                                "DetectorSaturationExtension",
                                "DynamicProjectorExtension"};

    QVector<CTL::ProjectorExtension*> dummyExtensions;
    dummyExtensions.append(new ArealFocalSpotExtension);
    dummyExtensions.append(new PoissonNoiseExtension);
    dummyExtensions.append(new SpectralEffectsExtension);
    dummyExtensions.append(new DetectorSaturationExtension);
    dummyExtensions.append(new DynamicProjectorExtension);

    for(int ext = 0; ext < extensionNames.size(); ++ext)
    {
        auto newItem = new QListWidgetItem(extensionNames.at(ext),
                                           nullptr,
                                           int(QListWidgetItem::UserType) + ext);

        QVariant initialData = dummyExtensions.at(ext)->parameter();
        qInfo() << initialData;

        newItem->setData(Qt::UserRole, initialData);
        prototypes.append(newItem);
    }

    for(auto& dummy : dummyExtensions)
        delete dummy;

    ui->_w_extensions->setExtensionPrototypes(prototypes);
}

void PipelineComposer::initializeProjectorPrototypes()
{
    QStringList projectorNames{ "RayCasterProjector" };

    QVector<AbstractProjector*> dummyProjectors;
    dummyProjectors.append(new OCL::RayCasterProjector);

    for(int proj = 0; proj < projectorNames.size(); ++proj)
    {
        qInfo() << proj;
        auto newItem = new QListWidgetItem(projectorNames.at(proj),
                                           nullptr,
                                           int(QListWidgetItem::UserType) + ProjectorTypeOffset + proj);

        QVariant initialData = dummyProjectors.at(proj)->parameter();
        qInfo() << proj;

        newItem->setData(Qt::UserRole, initialData);

        ui->_LW_projectorProto->insertItem(proj, newItem);
    }

    for(auto& dummy : dummyProjectors)
        delete dummy;
}

std::unique_ptr<ProjectorExtension> PipelineComposer::createExtension(int type) const
{
    std::unique_ptr<ProjectorExtension> ret;
    switch (type)
    {
    case ExtensionChainWidget::ArealFocalSpotExtension:
        ret.reset(new ArealFocalSpotExtension);
        break;
    case ExtensionChainWidget::PoissonNoiseExtension:
        ret.reset(new PoissonNoiseExtension);
        break;
    case ExtensionChainWidget::SpectralEffectsExtension:
        ret.reset(new SpectralEffectsExtension);
        break;
    case ExtensionChainWidget::DetectorSaturationExtension:
        ret.reset(new DetectorSaturationExtension);
        break;
    case ExtensionChainWidget::DynamicProjectorExtension:
        ret.reset(new DynamicProjectorExtension);
        break;
    }

    return ret;
}

std::unique_ptr<AbstractProjector> PipelineComposer::createProjector(int type) const
{
    std::unique_ptr<AbstractProjector> ret;
    switch (type)
    {
    case 0:
        ret.reset(new OCL::RayCasterProjector);
        break;
    }

    return ret;
}

void PipelineComposer::setProjector(QListWidgetItem* item)
{
    ui->_LW_selectedProjector->clear();
    auto newItem = new QListWidgetItem(item->text(), ui->_LW_selectedProjector, item->type());
    newItem->setData(Qt::UserRole, item->data(Qt::UserRole));
    ui->_W_propertyManager->updateInterface(newItem);
}

namespace details {

ExtensionConfigWidget::ExtensionConfigWidget(QWidget* parent)
    : QWidget(parent)
    , _layout(new QGridLayout)
{
    this->setLayout(_layout);
}

void ExtensionConfigWidget::setExtensionObject(QListWidgetItem *item)
{
    _currentItem = item;
}

void ExtensionConfigWidget::updateInterface(QListWidgetItem* item)
{
    clearLayout();
    if(!item) return;

    setExtensionObject(item);
    qInfo() << "update";

    QVariantMap dataMap = _currentItem->data(Qt::UserRole).toMap();

    int row = 0;
    QVariantMap::const_iterator i = dataMap.constBegin();
    while (i != dataMap.constEnd())
    {
        _layout->addWidget(new QLabel(i.key()), row, 0);
        if(i.value().type() == QVariant::Bool)
        {
            auto widget = new QCheckBox("enable");
            widget->setChecked(i.value().toBool());
            _layout->addWidget(widget, row, 1);
            connect(widget, SIGNAL(toggled(bool)), this, SLOT(somethingChanged()));
        }
        else if(i.value().type() == QVariant::Int || i.value().type() == QVariant::UInt)
        {
            auto widget = new QSpinBox;
            widget->setValue(i.value().toInt());
            _layout->addWidget(widget, row, 1);
            connect(widget, SIGNAL(valueChanged(int)), this, SLOT(somethingChanged()));
        }
        else if(i.value().type() == QVariant::Double || i.value().type() == 38)
        {
            auto widget = new QDoubleSpinBox;
            widget->setValue(i.value().toDouble());
            _layout->addWidget(widget, row, 1);
            connect(widget, SIGNAL(valueChanged(double)), this, SLOT(somethingChanged()));
        }

        ++row;
        ++i;
    }

}

void ExtensionConfigWidget::clearLayout()
{
    const auto nbItems = _layout->count();
    const auto nbRows = _layout->rowCount();
    qInfo() << "clear" << nbRows << nbItems;
    for(int item = nbItems-1; item >=0 ; --item)
    {
        qInfo() << "clear item " << item;
        QWidget* w = _layout->itemAt(item)->widget();
        if(w != NULL)
        {
            _layout->removeWidget(w);
            delete w;
        }
    }
}

void ExtensionConfigWidget::somethingChanged()
{
    const auto nbItems = _layout->count();
    const auto nbRows = nbItems / 2;

    QVariantMap map;
    for (int row = 0; row < nbRows; ++row)
    {
        auto name = static_cast<QLabel*>(_layout->itemAtPosition(row, 0)->widget())->text();
        map.insert(name, parsedInputWidget(_layout->itemAtPosition(row, 1)->widget()));
    }

    _currentItem->setData(Qt::UserRole, map);
}

QVariant ExtensionConfigWidget::parsedInputWidget(QWidget *widget)
{
    QVariant ret;

    if(dynamic_cast<QCheckBox*>(widget))
        ret = dynamic_cast<QCheckBox*>(widget)->isChecked();
    else if(dynamic_cast<QSpinBox*>(widget))
        ret = dynamic_cast<QSpinBox*>(widget)->value();
    else if(dynamic_cast<QDoubleSpinBox*>(widget))
        ret = dynamic_cast<QDoubleSpinBox*>(widget)->value();

    return ret;
}

} // namespace details

} // namespace gui
} // namespace CTL
