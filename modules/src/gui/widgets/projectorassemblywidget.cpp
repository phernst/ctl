#include "projectorassemblywidget.h"
#include "ui_projectorassemblywidget.h"
#include <QDebug>

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

    auto extType = 0;
    for(const auto name : extensionNames())
        new QListWidgetItem(name, ui->extensionList, extType++);

    ui->codeViewer->setText(firstLine() + ";\n");
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
}

void ProjectorAssemblyWidget::on_extensionList_itemDoubleClicked(QListWidgetItem* item)
{
    new QListWidgetItem(item->text(), ui->pipelineList, item->type());
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
        extensions.push_back(static_cast<Extension>(item->type()));
    }
    theCode += ";\n\n-----------\n\n";

    const auto report = reportPhysicalCompatibility(extensions);
    theCode += compatibilityReport2String(extensions, report);

    ui->codeViewer->setText(theCode);
}

void ProjectorAssemblyWidget::on_pushButton_clicked() { updateViewer(); }
