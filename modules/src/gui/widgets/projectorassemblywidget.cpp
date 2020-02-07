#include "projectorassemblywidget.h"
#include "ui_projectorassemblywidget.h"

static QString firstLine()
{
    return QStringLiteral("auto myProjector = CTL::makeProjector<CTL::OCL::RayCasterProjector>()");
}

ProjectorAssemblyWidget::ProjectorAssemblyWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ProjectorAssemblyWidget)
{
    ui->setupUi(this);

    initExtensionList();

    ui->codeViewer->setText(firstLine() + ";\n");

    connect(ui->pipelineList->model(), SIGNAL(rowsInserted(const QModelIndex&, int, int)), SLOT(updateViewer()));
    connect(ui->pipelineList->model(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)), SLOT(updateViewer()));
    connect(ui->pipelineList->model(), &QAbstractItemModel::rowsMoved, this, &ProjectorAssemblyWidget::updateViewer);

    connect(ui->pipelineList, &QListWidget::itemClicked, this, &ProjectorAssemblyWidget::extensionItemClicked);

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

QString ProjectorAssemblyWidget::codeString()
{
    QString theCode = firstLine();

    for(auto row = 0, count = ui->pipelineList->count(); row < count; ++row)
    {
        const auto item = ui->pipelineList->item(row);
        theCode += " |\n                   "
                   "CTL::makeExtension<CTL::" + item->text() + ">()";
    }
    theCode += ";\n\n-----------\n\n";

    return theCode;
}

void ProjectorAssemblyWidget::setExtensionPrototypes(const QVector<QListWidgetItem*>& prototypes)
{
    ui->extensionList->clear();
    uint row = 0;
    for(const auto item : prototypes)
        ui->extensionList->insertItem(row++, item);
}

QVector<QListWidgetItem*> ProjectorAssemblyWidget::extensions()
{
    const auto nbItems = ui->pipelineList->count();
    QVector<QListWidgetItem*> ret(nbItems);

    for(auto row = 0; row < nbItems; ++row)
        ret[row] = (ui->pipelineList->item(row));

    return ret;
}

void ProjectorAssemblyWidget::on_pipelineList_itemDoubleClicked(QListWidgetItem* item)
{
    delete item;
    if(!ui->pipelineList->selectedItems().isEmpty())
        emit extensionItemClicked(ui->pipelineList->selectedItems().first());
    else
        emit extensionItemClicked(nullptr);
}

void ProjectorAssemblyWidget::on_extensionList_itemDoubleClicked(QListWidgetItem* item)
{
    auto newItem = new QListWidgetItem(item->text(), ui->pipelineList, item->type());
    newItem->setData(Qt::UserRole, item->data(Qt::UserRole));
}

void ProjectorAssemblyWidget::updateViewer()
{
    QString text = codeString();
    std::vector<Extension> extensions;

    for(auto row = 0, count = ui->pipelineList->count(); row < count; ++row)
        extensions.push_back(static_cast<Extension>(ui->pipelineList->item(row)->type() - QListWidgetItem::UserType));

    const auto report = reportPhysicalCompatibility(extensions);
    text += compatibilityReport2String(extensions, report);

    ui->codeViewer->setText(text);
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


