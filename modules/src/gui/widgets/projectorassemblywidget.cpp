#include "projectorassemblywidget.h"
#include "ui_projectorassemblywidget.h"
#include <QDebug>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>

static QString firstLine()
{
    return QStringLiteral("auto myProjector = CTL::makeProjector<CTL::OCL::RayCasterProjector>()");
}

ProjectorAssemblyWidget::ProjectorAssemblyWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ProjectorAssemblyWidget)
{
    ui->setupUi(this);
    ui->pipelineList->setDragDropMode(QAbstractItemView::DragDropMode::InternalMove);

    initExtensionList();

    ui->codeViewer->setText(firstLine() + ";\n");

    connect(ui->pipelineList->model(), SIGNAL(rowsInserted(const QModelIndex&, int, int)), SLOT(updateViewer()));
    connect(ui->pipelineList->model(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)), SLOT(updateViewer()));
    connect(ui->pipelineList->model(), &QAbstractItemModel::rowsMoved, this, &ProjectorAssemblyWidget::updateViewer);

    connect(ui->pipelineList, &QListWidget::itemClicked, ui->widget, &ExtensionConfigWidget::updateInterface);

}

ProjectorAssemblyWidget::~ProjectorAssemblyWidget() { delete ui; }

const ProjectorAssemblyWidget::ExtensionNames& ProjectorAssemblyWidget::extensionNames()
{
    static constexpr ExtensionNames ret{
        "ArealFocalSpotExtension",
        "PoissonNoiseExtension",
        "SpectralEffectsExtension",
        "DetectorSaturationExtension",
    };
    return ret;
}

const ProjectorAssemblyWidget::CompatibilityMatrix& ProjectorAssemblyWidget::compatibilityMatrix()
{
    using Physical = PhysicalCompatibility;
    static constexpr CompatibilityMatrix ret {
         // SecondExtenion: ArealFocalSpot    PoissonNoise      SpectralEffects   DetectorSaturation
    /*AFS*/ FirstExtension{ Physical::Approx, Physical::True,   Physical::True,   Physical::True  },
    /*PN*/  FirstExtension{ Physical::Ineff,  Physical::Approx, Physical::True,   Physical::True  },
    /*SE*/  FirstExtension{ Physical::Ineff,  Physical::Approx, Physical::Ineff,  Physical::True  },
    /*DS*/  FirstExtension{ Physical::False,  Physical::False,  Physical::False,  Physical::False },
    };
    return ret;
}

ProjectorAssemblyWidget::CompatibilityReport ProjectorAssemblyWidget::reportPhysicalCompatibility(
    const std::vector<Extension>& extensions)
{
    CompatibilityReport ret(extensions.size(), { Extension(0), PhysicalCompatibility::True });
    const auto& cM = compatibilityMatrix();
    for(size_t firstExt = 0, end = extensions.size(); firstExt < end; ++firstExt)
        for(size_t secondExt = firstExt + 1; secondExt < end; ++secondExt)
        {
            const auto firstExtType = extensions[firstExt];
            const auto secondExtType = extensions[secondExt];
            const auto curCompatibility = cM[firstExtType][secondExtType];
            const auto oldCompatibility = ret[firstExt].second;
            const auto newCompatibility = std::min(curCompatibility, oldCompatibility);
            if(newCompatibility != oldCompatibility)
                ret[firstExt] = std::make_pair(secondExtType, newCompatibility);
        }
    return ret;
}

QString ProjectorAssemblyWidget::compatibilityReport2String(
    const std::vector<Extension>& extensions,
    const ProjectorAssemblyWidget::CompatibilityReport& report)
{
    QString ret = QStringLiteral("Physical evaluation:\n");
    const auto& nameLU = extensionNames();
    auto extNb = 0u;
    for(const auto& ext : extensions)
    {
        ret += "- " + QString(nameLU[ext]) + ": ";
        const auto currentExtType = report[extNb];
        switch (currentExtType.second) {
        case PhysicalCompatibility::True:
            ret += "ok";
            break;
        case PhysicalCompatibility::False:
            ret += "unphysical before ";
            ret += nameLU[currentExtType.first];
            break;
        case PhysicalCompatibility::Approx:
            ret += "approximation before ";
            ret += nameLU[currentExtType.first];
            break;
        case PhysicalCompatibility::Ineff:
            ret += "correct but inefficient before ";
            ret += nameLU[currentExtType.first];
            break;
        }
        ret += "\n";

        ++extNb;
    }
    return ret;
}

void ProjectorAssemblyWidget::on_pipelineList_itemDoubleClicked(QListWidgetItem* item)
{
    delete item;
    if(!ui->pipelineList->selectedItems().isEmpty())
        ui->widget->updateInterface(ui->pipelineList->selectedItems().first());
    else
        ui->widget->updateInterface(nullptr);
}

void ProjectorAssemblyWidget::on_extensionList_itemDoubleClicked(QListWidgetItem* item)
{
    auto newItem = new QListWidgetItem(item->text(), ui->pipelineList, item->type());
    newItem->setData(Qt::UserRole, item->data(Qt::UserRole));
}

void ProjectorAssemblyWidget::updateViewer()
{
    QString theCode = firstLine();
    std::vector<Extension> extensions;

    for(auto row = 0, count = ui->pipelineList->count(); row < count; ++row)
    {
        const auto item = ui->pipelineList->item(row);
        theCode += " |\n                   "
                   "CTL::makeExtension<CTL::" + item->text() + ">()";
        extensions.push_back(static_cast<Extension>(item->type() - QListWidgetItem::UserType));
    }
    theCode += ";\n\n-----------\n\n";

    const auto report = reportPhysicalCompatibility(extensions);
    theCode += compatibilityReport2String(extensions, report);

    ui->codeViewer->setText(theCode);
}

void ProjectorAssemblyWidget::initExtensionList()
{
    const auto nbExt = extensionNames().size();

    for(uint ext = 0; ext < nbExt; ++ext)
    {
        auto newItem = new QListWidgetItem(extensionNames().at(ext),
                                           ui->extensionList,
                                           int(QListWidgetItem::UserType) + ext);
        QVariantMap initialData;
        switch (ext) {
        case ArealFocalSpotExtension:
            initialData.insert("Discretization X", 1);
            initialData.insert("Discretization Y", 1);
            break;
        case PoissonNoiseExtension:
            initialData.insert("Use Fixed Seed", false);
            initialData.insert("Fixed Seed", 42);
            break;
        case SpectralEffectsExtension:
            initialData.insert("Energy Bin Width", 10.0);
            break;
        case DetectorSaturationExtension:
            break;
        }

        newItem->setData(Qt::UserRole, initialData);
    }
}

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
        else if(i.value().type() == QVariant::Int)
        {
            auto widget = new QSpinBox;
            widget->setValue(i.value().toInt());
            _layout->addWidget(widget, row, 1);
            connect(widget, SIGNAL(valueChanged(int)), this, SLOT(somethingChanged()));
        }
        else if(i.value().type() == QVariant::Double)
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


