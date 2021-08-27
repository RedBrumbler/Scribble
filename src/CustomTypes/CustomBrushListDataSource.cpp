#include "CustomTypes/CustomBrushListDataSource.hpp"

#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Texture2D.hpp"
#include "UnityEngine/SpriteMeshType.hpp"
#include "UnityEngine/Rect.hpp"
#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/UI/Image.hpp"

#include "TMPro/TextMeshProUGUI.hpp"
#include "questui/shared/ArrayUtil.hpp"

DEFINE_TYPE(QuestUI, CustomListTableData);

using namespace HMUI;
using namespace TMPro;
using namespace UnityEngine;
using namespace UnityEngine::UI;

namespace QuestUI 
{
    void CustomListTableData::ctor()
    {
        INVOKE_CTOR();
        set_listStyle(CustomListTableData::ListStyle::List);
        expandCell = false;
        reuseIdentifier = il2cpp_utils::newcsstr("QuestUIListTableCell");
        tableView = nullptr;
    }
    
    void CustomListTableData::dtor()
    {
        Finalize();
    }

    void CustomListTableData::set_listStyle(CustomListTableData::ListStyle value)
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

    CustomListTableData::ListStyle CustomListTableData::get_listStyle()
    {
        return listStyle;
    }

    GlobalNamespace::LevelListTableCell* CustomListTableData::GetTableCell()
    {
        auto tableCell = reinterpret_cast<GlobalNamespace::LevelListTableCell*>(tableView->DequeueReusableCellForIdentifier(reuseIdentifier));
        if (!tableCell)
        {
            if (!songListTableCellInstance)
                songListTableCellInstance = ArrayUtil::First(Resources::FindObjectsOfTypeAll<GlobalNamespace::LevelListTableCell*>(), [](auto x){ return to_utf8(csstrtostr(x->get_name())) == "LevelListTableCell"; });

            tableCell = Instantiate(songListTableCellInstance);
        }

        //tableCell.SetField("_beatmapCharacteristicImages", new Image[0]);
        tableCell->notOwned = false;

        tableCell->set_reuseIdentifier(reuseIdentifier);
        return tableCell;   
    }

    GlobalNamespace::LevelPackTableCell* CustomListTableData::GetLevelPackTableCell()
    {
        auto tableCell = reinterpret_cast<GlobalNamespace::LevelPackTableCell*>(tableView->DequeueReusableCellForIdentifier(reuseIdentifier));
        if (!tableCell)
        {
            if (!levelPackTableCellInstance)
                levelPackTableCellInstance = ArrayUtil::First(Resources::FindObjectsOfTypeAll<GlobalNamespace::LevelPackTableCell*>(), [](auto x){ return to_utf8(csstrtostr(x->get_name())) == "AnnotatedBeatmapLevelCollectionTableCell"; });
            tableCell = Instantiate(levelPackTableCellInstance);
        }

        tableCell->set_reuseIdentifier(reuseIdentifier);
        return tableCell;
    }

    GlobalNamespace::SimpleTextTableCell* CustomListTableData::GetSimpleTextTableCell()
    {
        auto tableCell = reinterpret_cast<GlobalNamespace::SimpleTextTableCell*>(tableView->DequeueReusableCellForIdentifier(reuseIdentifier));
        if (!tableCell)
        {
            if (!simpleTextTableCellInstance)
                simpleTextTableCellInstance = ArrayUtil::First(Resources::FindObjectsOfTypeAll<GlobalNamespace::SimpleTextTableCell*>(), [](auto x){ return to_utf8(csstrtostr(x->get_name())) == "SimpleTextTableCell"; });
            tableCell = Instantiate(simpleTextTableCellInstance);
        }

        tableCell->set_reuseIdentifier(reuseIdentifier);
        return tableCell;
    }

    HMUI::TableCell* CustomListTableData::CellForIdx(HMUI::TableView* tableView, int idx)
    {
        switch (listStyle)
        {
            case ListStyle::List: {
                auto tableCell = GetTableCell();

                TextMeshProUGUI* nameText = tableCell->songNameText;
                TextMeshProUGUI* authorText = tableCell->songAuthorText;
                tableCell->songBpmText->get_gameObject()->SetActive(false);
                tableCell->songDurationText->get_gameObject()->SetActive(false);
                tableCell->favoritesBadgeImage->get_gameObject()->SetActive(false);
                static auto BpmIcon = il2cpp_utils::newcsstr<il2cpp_utils::CreationType::Manual>("BpmIcon");
                tableCell->get_transform()->Find(BpmIcon)->get_gameObject()->SetActive(false);
                
                if (expandCell)
                {
                    nameText->get_rectTransform()->set_anchorMax(Vector2(2, 0.5f));
                    authorText->get_rectTransform()->set_anchorMax(Vector2(2, 0.5f));
                }

                auto& cellInfo = data[idx];
                nameText->set_text(cellInfo.get_text());
                authorText->set_text(cellInfo.get_subText());
                tableCell->coverImage->set_sprite(cellInfo.get_icon());
                tableCell->coverImage->set_color(cellInfo.color);
                
                return tableCell;
            }
            case ListStyle::Box: {
                auto cell = GetLevelPackTableCell();
                cell->set_showNewRibbon(false);
                auto& cellInfo = data[idx];
                cell->infoText->set_text(cellInfo.get_combinedText());
                auto packCoverImage = cell->coverImage;

                packCoverImage->set_sprite(cellInfo.get_icon());
                packCoverImage->set_color(cellInfo.color);

                return cell;
            }
            case ListStyle::Simple: {
                auto simpleCell = GetSimpleTextTableCell();
                simpleCell->text->set_richText(true);
                simpleCell->text->set_enableWordWrapping(true);
                simpleCell->set_text(data[idx].get_text());

                return simpleCell;
            }
        }

        return nullptr;        
    }

    float CustomListTableData::CellSize()
    {
        return cellSize;
    }

    int CustomListTableData::NumberOfCells()
    {
        return data.size();
    }

    Il2CppString* CustomListTableData::CustomCellInfo::get_text() 
    { 
        return il2cpp_utils::newcsstr(text); 
    }

    Il2CppString* CustomListTableData::CustomCellInfo::get_subText() 
    { 
        return il2cpp_utils::newcsstr(subText); 
    }

    Il2CppString* CustomListTableData::CustomCellInfo::get_combinedText()
    {
        return il2cpp_utils::newcsstr(string_format("%s\n%s", text.c_str(), subText.c_str())); 
    }

    UnityEngine::Sprite* CustomListTableData::CustomCellInfo::get_icon()
    {
        if (icon) return icon;
        auto texture = Texture2D::get_blackTexture();
        return Sprite::Create(texture, UnityEngine::Rect(0.0f, 0.0f, (float)texture->get_width(), (float)texture->get_height()), Vector2(0.5f,0.5f), 1024.0f, 1u, SpriteMeshType::FullRect, Vector4(0.0f, 0.0f, 0.0f, 0.0f), false);
    }
}