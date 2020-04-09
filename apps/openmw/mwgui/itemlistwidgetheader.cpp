#include "itemlistwidgetheader.hpp"

#include <MyGUI_Gui.h>
#include <MyGUI_FactoryManager.h>
#include <MyGUI_RenderManager.h>
#include <MyGUI_Button.h>

#include "../mwbase/windowmanager.hpp"
#include "../mwbase/environment.hpp"

#include "inventorywindow.hpp"
#include "sortfilteritemmodel.hpp"

namespace MWGui
{
    ItemListWidgetHeader::ItemListWidgetHeader()
        : mName(nullptr)
        , mValue(nullptr)
        , mWeight(nullptr)
        , mRatio(nullptr)
        , mArmorClass(nullptr)
        , mWeaponType(nullptr)
    {

    }

    void ItemListWidgetHeader::registerComponents()
    {
        MyGUI::FactoryManager::getInstance().registerFactory<ItemListWidgetHeader>("Widget");
    }

    void ItemListWidgetHeader::initialiseOverride()
    {
        assignWidget(mName, "Name");
        assignWidget(mValue, "Value");
        assignWidget(mWeight, "Weight");
        assignWidget(mRatio, "Ratio");
        assignWidget(mArmorClass, "ArmorClass");
        assignWidget(mWeaponType, "WeaponType");

        if (mName)
        {
            mName->setUserData(MWGui::SortFilterItemModel::Sort_Name);
            mName->eventMouseButtonClick += MyGUI::newDelegate(this, &ItemListWidgetHeader::onSortClicked);
        }
        if (mValue)
        {
            mValue->setUserData(MWGui::SortFilterItemModel::Sort_Value);
            mValue->eventMouseButtonClick += MyGUI::newDelegate(this, &ItemListWidgetHeader::onSortClicked);
        }
        if (mWeight)
        {
            mWeight->setUserData(MWGui::SortFilterItemModel::Sort_Weight);
            mWeight->eventMouseButtonClick += MyGUI::newDelegate(this, &ItemListWidgetHeader::onSortClicked);
        }
        if (mRatio)
        {
            mRatio->setUserData(MWGui::SortFilterItemModel::Sort_Ratio);
            mRatio->eventMouseButtonClick += MyGUI::newDelegate(this, &ItemListWidgetHeader::onSortClicked);
        }
        if (mArmorClass)
        {
            mArmorClass->setUserData(MWGui::SortFilterItemModel::Sort_ArmorType);
            mArmorClass->eventMouseButtonClick += MyGUI::newDelegate(this, &ItemListWidgetHeader::onSortClicked);
        }
        if (mWeaponType)
        {
            mWeaponType->setUserData(MWGui::SortFilterItemModel::Sort_WeaponType);
            mWeaponType->eventMouseButtonClick += MyGUI::newDelegate(this, &ItemListWidgetHeader::onSortClicked);
        }

        Base::initialiseOverride();
    }

    void ItemListWidgetHeader::onSortClicked(MyGUI::Widget* sender)
    {
        eventItemClicked(*sender->getUserData<int>());
    }
}