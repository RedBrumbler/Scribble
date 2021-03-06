#include "CustomTypes/CustomBrushListDataSource.hpp"

#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Texture2D.hpp"
#include "UnityEngine/SpriteMeshType.hpp"
#include "UnityEngine/Rect.hpp"
#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/UI/Image.hpp"
#include "HMUI/ImageView.hpp"

#include "TMPro/TextMeshProUGUI.hpp"
#include "questui/shared/ArrayUtil.hpp"

DEFINE_TYPE(Scribble, CustomBrushListDataSource);

using namespace HMUI;
using namespace TMPro;
using namespace QuestUI;
using namespace UnityEngine;
using namespace UnityEngine::UI;

namespace Scribble 
{
    void CustomBrushListDataSource::ctor()
    {
        INVOKE_CTOR();
        set_listStyle(CustomBrushListDataSource::ListStyle::List);
        expandCell = false;
        tableView = nullptr;
    }
    
    void CustomBrushListDataSource::dtor()
    {
        Finalize();
    }

    void CustomBrushListDataSource::set_listStyle(CustomBrushListDataSource::ListStyle value)
    {
        this->listStyle = value;
        switch (value)
        {
            case ListStyle::List:
                cellSize = 8.5f;
                break;
            case ListStyle::Box:
                cellSize = tableView->get_tableType() == TableView::TableType::Horizontal ? 30.0f : 35.0f;
                break;
            case ListStyle::Simple:
                cellSize = 8.0f;
                break;
        }
    }

    CustomBrushListDataSource::ListStyle CustomBrushListDataSource::get_listStyle()
    {
        return listStyle;
    }

    GlobalNamespace::LevelListTableCell* CustomBrushListDataSource::GetTableCell()
    {
        auto tableCell = reinterpret_cast<GlobalNamespace::LevelListTableCell*>(tableView->DequeueReusableCellForIdentifier("ScribbleCustomBrushListTableCell"));
        if (!tableCell)
        {
            if (!songListTableCellInstance)
                songListTableCellInstance = ArrayUtil::First(Resources::FindObjectsOfTypeAll<GlobalNamespace::LevelListTableCell*>(), [](auto x){ return x->get_name() == "LevelListTableCell"; });

            tableCell = Instantiate(songListTableCellInstance);
        }

        //tableCell.SetField("_beatmapCharacteristicImages", new Image[0]);
        tableCell->dyn__notOwned() = false;

        tableCell->set_reuseIdentifier("ScribbleCustomBrushListTableCell");
        return tableCell;   
    }

    GlobalNamespace::LevelPackTableCell* CustomBrushListDataSource::GetLevelPackTableCell()
    {
        auto tableCell = reinterpret_cast<GlobalNamespace::LevelPackTableCell*>(tableView->DequeueReusableCellForIdentifier("ScribbleCustomBrushListTableCell"));
        if (!tableCell)
        {
            if (!levelPackTableCellInstance)
                levelPackTableCellInstance = ArrayUtil::First(Resources::FindObjectsOfTypeAll<GlobalNamespace::LevelPackTableCell*>(), [](auto x){ return x->get_name() == "AnnotatedBeatmapLevelCollectionTableCell"; });
            tableCell = Instantiate(levelPackTableCellInstance);
        }
        return tableCell;
    }

    GlobalNamespace::SimpleTextTableCell* CustomBrushListDataSource::GetSimpleTextTableCell()
    {
        auto tableCell = reinterpret_cast<GlobalNamespace::SimpleTextTableCell*>(tableView->DequeueReusableCellForIdentifier("ScribbleCustomBrushListTableCell"));
        if (!tableCell)
        {
            if (!simpleTextTableCellInstance)
                simpleTextTableCellInstance = ArrayUtil::First(Resources::FindObjectsOfTypeAll<GlobalNamespace::SimpleTextTableCell*>(), [](auto x){ return x->get_name() == "SimpleTextTableCell"; });
            tableCell = Instantiate(simpleTextTableCellInstance);
        }

        tableCell->set_reuseIdentifier("ScribbleCustomBrushListTableCell");
        return tableCell;
    }

    HMUI::TableCell* CustomBrushListDataSource::CellForIdx(HMUI::TableView* tableView, int idx)
    {
        switch (listStyle)
        {
            case ListStyle::List: {
                auto tableCell = GetTableCell();

                TextMeshProUGUI* nameText = tableCell->dyn__songNameText();
                TextMeshProUGUI* authorText = tableCell->dyn__songAuthorText();
                tableCell->dyn__songBpmText()->get_gameObject()->SetActive(false);
                tableCell->dyn__songDurationText()->get_gameObject()->SetActive(false);
                tableCell->dyn__favoritesBadgeImage()->get_gameObject()->SetActive(false);
                tableCell->get_transform()->Find("BpmIcon")->get_gameObject()->SetActive(false);
                
                if (expandCell)
                {
                    nameText->get_rectTransform()->set_anchorMax(Vector2(2, 0.5f));
                    authorText->get_rectTransform()->set_anchorMax(Vector2(2, 0.5f));
                }

                auto& cellInfo = data[idx];
                nameText->set_text(cellInfo.get_text());
                authorText->set_text(cellInfo.get_subText());
                tableCell->dyn__coverImage()->set_sprite(cellInfo.get_icon());
                tableCell->dyn__coverImage()->set_color(cellInfo.color);

                return tableCell;
            }
            case ListStyle::Box: {
                auto cell = GetLevelPackTableCell();
                cell->set_showNewRibbon(false);
                auto& cellInfo = data[idx];
                cell->dyn__infoText()->set_text(cellInfo.get_combinedText());
                auto packCoverImage = cell->dyn__coverImage();

                packCoverImage->set_sprite(cellInfo.get_icon());
                packCoverImage->set_color(cellInfo.color);

                return reinterpret_cast<TableCell*>(cell);
            }
            case ListStyle::Simple: {
                auto simpleCell = GetSimpleTextTableCell();
                simpleCell->dyn__text()->set_richText(true);
                simpleCell->dyn__text()->set_enableWordWrapping(true);
                simpleCell->set_text(data[idx].get_text());

                return simpleCell;
            }
        }

        return nullptr;        
    }

    float CustomBrushListDataSource::CellSize()
    {
        return cellSize;
    }

    int CustomBrushListDataSource::NumberOfCells()
    {
        return data.size();
    }

    StringW CustomBrushListDataSource::CustomCellInfo::get_text() 
    { 
        return text;
    }

    StringW CustomBrushListDataSource::CustomCellInfo::get_subText() 
    { 
        return subText;
    }

    StringW CustomBrushListDataSource::CustomCellInfo::get_combinedText()
    {
        return string_format("%s\n%s", text.c_str(), subText.c_str());
    }

    UnityEngine::Sprite* CustomBrushListDataSource::CustomCellInfo::get_icon()
    {
        if (icon) return icon;
        auto texture = Texture2D::get_blackTexture();
        return Sprite::Create(texture, UnityEngine::Rect(0.0f, 0.0f, (float)texture->get_width(), (float)texture->get_height()), Vector2(0.5f,0.5f), 1024.0f, 1u, SpriteMeshType::FullRect, Vector4(0.0f, 0.0f, 0.0f, 0.0f), false);
    }
}