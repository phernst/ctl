#include "extensionchainwidget.h"
#include "ui_extensionchainwidget.h"

static QString firstLine()
{
    return QStringLiteral("auto myProjector = CTL::makeProjector<CTL::OCL::RayCasterProjector>()");
}

namespace CTL {
namespace gui {

ExtensionChainWidget::ExtensionChainWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ExtensionChainWidget)
{
    ui->setupUi(this);

    initExtensionList();

    ui->codeViewer->setText(firstLine() + ";\n");

    connect(ui->pipelineList->model(), SIGNAL(rowsInserted(const QModelIndex&, int, int)), SLOT(updateViewer()));
    connect(ui->pipelineList->model(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)), SLOT(updateViewer()));
    connect(ui->pipelineList->model(), &QAbstractItemModel::rowsMoved, this, &ExtensionChainWidget::updateViewer);

    connect(ui->pipelineList, &QListWidget::itemClicked, this, &ExtensionChainWidget::extensionItemClicked);

}

ExtensionChainWidget::~ExtensionChainWidget() { delete ui; }

const ExtensionChainWidget::ExtensionNames& ExtensionChainWidget::extensionNames()
{
    static constexpr ExtensionNames ret{
        "ArealFocalSpotExtension",
        "PoissonNoiseExtension",
        "SpectralEffectsExtension",
        "DetectorSaturationExtension",
        "DynamicProjectorExtension",
    };
    return ret;
}

const ExtensionChainWidget::CompatibilityMatrix& ExtensionChainWidget::compatibilityMatrix()
{
    using Physical = PhysicalCompatibility;
    static constexpr CompatibilityMatrix ret {
         // SecondExtenion: ArealFocalSpot    PoissonNoise      SpectralEffects   DetectorSaturation  DynamicProjector
    /*AFS*/ FirstExtension{ Physical::Approx, Physical::True,   Physical::True,   Physical::True,     Physical::True  },
    /*PN*/  FirstExtension{ Physical::Ineff,  Physical::Approx, Physical::True,   Physical::True,     Physical::True  },
    /*SE*/  FirstExtension{ Physical::Ineff,  Physical::Approx, Physical::Ineff,  Physical::True,     Physical::True  },
    /*DS*/  FirstExtension{ Physical::False,  Physical::False,  Physical::False,  Physical::False,    Physical::True  },
    /*DP*/  FirstExtension{ Physical::Ineff,  Physical::True,   Physical::Undef, Physical::True,     Physical::False },
    };
    return ret;
}

ExtensionChainWidget::CompatibilityReport ExtensionChainWidget::reportPhysicalCompatibility(
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

QString ExtensionChainWidget::compatibilityReport2String(
    const std::vector<Extension>& extensions,
    const ExtensionChainWidget::CompatibilityReport& report)
{
    QString ret = QStringLiteral("Physical evaluation:\n");
    const auto& nameLU = extensionNames();
    auto extNb = 0u;
    for(const auto& ext : extensions)
    {
        ret += QStringLiteral("- ") + QString(nameLU[ext]) + QStringLiteral(": ");
        const auto currentExtType = report[extNb];
        switch (currentExtType.second) {
        case PhysicalCompatibility::True:
            ret += QStringLiteral("ok");
            break;
        case PhysicalCompatibility::False:
            ret += QStringLiteral("unphysical before ");
            ret += nameLU[currentExtType.first];
            break;
        case PhysicalCompatibility::Approx:
            ret += QStringLiteral("approximation before ");
            ret += nameLU[currentExtType.first];
            break;
        case PhysicalCompatibility::Ineff:
            ret += QStringLiteral("correct but inefficient before ");
            ret += nameLU[currentExtType.first];
            break;
        case PhysicalCompatibility::Undef:
            ret += QStringLiteral("has undefined outcome before ");
            ret += nameLU[currentExtType.first];
            ret += QStringLiteral("\nCorrectness of the result may depend on the context.");
            break;
        }
        ret += QStringLiteral("\n");

        ++extNb;
    }
    return ret;
}

QString ExtensionChainWidget::codeString()
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

void ExtensionChainWidget::setExtensionPrototypes(const QVector<QListWidgetItem*>& prototypes)
{
    ui->extensionList->clear();
    uint row = 0;
    for(const auto item : prototypes)
        ui->extensionList->insertItem(row++, item);
}

QVector<QListWidgetItem*> ExtensionChainWidget::extensions()
{
    const auto nbItems = ui->pipelineList->count();
    QVector<QListWidgetItem*> ret(nbItems);

    for(auto row = 0; row < nbItems; ++row)
        ret[row] = (ui->pipelineList->item(row));

    return ret;
}

void ExtensionChainWidget::on_pipelineList_itemDoubleClicked(QListWidgetItem* item)
{
    delete item;
    if(!ui->pipelineList->selectedItems().isEmpty())
        emit extensionItemClicked(ui->pipelineList->selectedItems().first());
    else
        emit extensionItemClicked(nullptr);
}

void ExtensionChainWidget::on_extensionList_itemDoubleClicked(QListWidgetItem* item)
{
    auto newItem = new QListWidgetItem(item->text(), ui->pipelineList, item->type());
    newItem->setData(Qt::UserRole, item->data(Qt::UserRole));
}

void ExtensionChainWidget::updateViewer()
{
    QString text = codeString();
    std::vector<Extension> extensions;

    for(auto row = 0, count = ui->pipelineList->count(); row < count; ++row)
        extensions.push_back(static_cast<Extension>(ui->pipelineList->item(row)->type() - QListWidgetItem::UserType));

    const auto report = reportPhysicalCompatibility(extensions);
    text += compatibilityReport2String(extensions, report);

    ui->codeViewer->setText(text);
}

void ExtensionChainWidget::initExtensionList()
{
    const auto nbExt = extensionNames().size();

    for(uint ext = 0; ext < nbExt; ++ext)
    {
        new QListWidgetItem(extensionNames().at(ext),
                            ui->extensionList,
                            int(QListWidgetItem::UserType) + ext);
    }
}

} // namespace gui
} // namespace CTL
