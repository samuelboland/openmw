#include "itemlistwidget.hpp"

#include <MyGUI_FactoryManager.h>
#include <MyGUI_ImageBox.h>
#include <MyGUI_RenderManager.h>
#include <MyGUI_TextBox.h>
#include <MyGUI_LanguageManager.h>
#include <MyGUI_Colour.h>

#include "../mwbase/world.hpp"
#include "../mwbase/environment.hpp"
#include "../mwbase/windowmanager.hpp"
#include "../mwbase/mechanicsmanager.hpp"
#include "../mwbase/windowmanager.hpp"

#include "../mwgui/quickkeysmenu.hpp"

#include "../mwmechanics/weapontype.hpp"
#include "../mwmechanics/actorutil.hpp"

#include "../mwworld/player.hpp"
#include "../mwworld/class.hpp"
#include "../mwworld/inventorystore.hpp"
#include "../mwworld/actionequip.hpp"
#include "../mwworld/esmstore.hpp"

#include "../mwbase/environment.hpp"
#include "../mwbase/world.hpp"
#include "../mwbase/mechanicsmanager.hpp"

#include "itemview.hpp"

namespace MWGui
{
    const std::unordered_map<ESM::Weapon::Type, const std::string> ItemListWidget::mWeapType{
        {ESM::Weapon::ShortBladeOneHand, "Short Blade"},
        {ESM::Weapon::LongBladeOneHand , "Long Blade"},
        {ESM::Weapon::LongBladeTwoHand , "Long Blade (2H)"},
        {ESM::Weapon::BluntOneHand     , "Blunt"},
        {ESM::Weapon::BluntTwoClose    , "Blunt (2H)"},
        {ESM::Weapon::BluntTwoWide     , "Blunt (2H)"},
        {ESM::Weapon::SpearTwoWide     , "Spear"},
        {ESM::Weapon::AxeOneHand       , "Axe"},
        {ESM::Weapon::AxeTwoHand       , "Axe (2H)"},
        {ESM::Weapon::MarksmanBow      , "Bow"},
        {ESM::Weapon::MarksmanCrossbow , "Crossbow"},
        {ESM::Weapon::MarksmanThrown   , "Thrown"},
        {ESM::Weapon::Arrow            , "Arrow"},
        {ESM::Weapon::Bolt             , "Bolt"}
    };

    const std::string ItemListWidget::mBlank = "-";

    ItemListWidget::ItemListWidget()
        : mIcon(nullptr)
        , mName(nullptr)
        , mValue(nullptr)
        , mWeight(nullptr)
        , mRatio(nullptr)
        , mArmorClass(nullptr)
        , mWeaponType(nullptr)
        , mItem(nullptr)
    {
        setNeedKeyFocus(true);  
    }

    void ItemListWidget::setItem(const ItemStack &item,int category)
    {
        mName->setSize(0.50 * getWidth(), mName->getHeight());
        const std::string typeName = item.mBase.getClass().getTypeName();

        // all filter views show these columns 
        createIcon(item, typeName);
        createName(item, typeName); 
        createWeight(item, typeName);
        createValue(item, typeName);
        createRatio(item, typeName);

        Gui::ButtonGroup group;

        group.push_back(mName);
        group.push_back(mWeight);
        group.push_back(mValue);
        group.push_back(mRatio);

        if (category == MWGui::SortFilterItemModel::Category_Weapon)
        {
            createWeaponType(item, typeName);
            group.push_back(mWeaponType);
        }
        else if (category == MWGui::SortFilterItemModel::Category_Armor)
        {
            createArmorClass(item, typeName);
            group.push_back(mArmorClass);
        }
        Gui::SharedStateButton::createButtonGroup(group);

        // hidden widget still take up space :(
        for (size_t i = 0; i < mItem->getChildCount(); i++)
        {
            MyGUI::Widget* child = mItem->getChildAt(i); 
            if (child == nullptr) continue; 
            if (!child->getVisible())
                child->setSize(0,0);
        }

        if (item.mType == ItemStack::Type_Barter)
            mName->setTextColour(MyGUI::Colour::parse(MyGUI::LanguageManager::getInstance().replaceTags("#{fontcolour=normal_over}")));

        int x = mName->getTextSize().width+mIcon->getParent()->getWidth()+8;
        int size = 16;
        if (item.mFlags & ItemStack::Flag_Enchanted)
        {
            MyGUI::ImageBox* t = mItem->getParent()->createWidget<MyGUI::ImageBox>("ImageBox",
                MyGUI::IntCoord(x,10,size,size), MyGUI::Align::Left | MyGUI::Align::VCenter);
            t->setImageTexture("textures\\ui\\enchanted.dds");
            t->setNeedMouseFocus(false);
            x += size;
        }
        if (item.mFlags & ItemStack::Flag_Bound)
        {
            MyGUI::ImageBox* t = mItem->getParent()->createWidget<MyGUI::ImageBox>("ImageBox",
                MyGUI::IntCoord(x,10,size,size), MyGUI::Align::Left | MyGUI::Align::VCenter);
            t->setImageTexture("textures\\ui\\bound.dds");
            t->setNeedMouseFocus(false);
            x+= size;
        }
        if (MWBase::Environment::get().getWindowManager()->getQuickKeysMenu()->isAssigned(item.mBase))
        {
            MyGUI::ImageBox* t = mItem->getParent()->createWidget<MyGUI::ImageBox>("ImageBox",
                MyGUI::IntCoord(x,10,size,size), MyGUI::Align::Left | MyGUI::Align::VCenter);
            t->setImageTexture("textures\\ui\\favorite.dds");
            t->setNeedMouseFocus(false);
            x+= size;
        }
        
        auto stolenItems = MWBase::Environment::get().getMechanicsManager()->getStolenItemOwners(item.mBase.getCellRef().getRefId());

        if (!stolenItems.empty())
        {
            const MWWorld::Ptr& player = MWMechanics::getPlayer();
            bool isContainer = item.mBase.getContainerStore() != &player.getClass().getContainerStore(player);

            if (!isContainer)
            {
                MyGUI::ImageBox* t = mItem->getParent()->createWidget<MyGUI::ImageBox>("ImageBox",
                    MyGUI::IntCoord(x,10,size,size), MyGUI::Align::Left | MyGUI::Align::VCenter);
                t->setImageTexture("textures\\ui\\stolen.dds");
                t->setNeedMouseFocus(false);
                x+= size;            
            }
        }

        if (item.mType == ItemStack::Type_Equipped)
        {
            MyGUI::ImageBox* t = mItem->getParent()->createWidget<MyGUI::ImageBox>("ImageBox",
                MyGUI::IntCoord(x,10,size,size), MyGUI::Align::Left | MyGUI::Align::VCenter);
            t->setImageTexture("textures\\ui\\equipped.dds");
            t->setNeedMouseFocus(false);
            x+= size;
        }   

    }

    void ItemListWidget::registerComponents()
    {
        MyGUI::FactoryManager::getInstance().registerFactory<ItemListWidget>("Widget");
    }

    void ItemListWidget::initialiseOverride()
    {
        assignWidget(mIcon, "Icon"); 
        assignWidget(mName, "Name");
        assignWidget(mValue, "Value");
        assignWidget(mWeight, "Weight");
        assignWidget(mRatio, "Ratio");
        assignWidget(mArmorClass, "ArmorClass");
        assignWidget(mWeaponType, "WeaponType");
        assignWidget(mItem, "Item");
        
        Base::initialiseOverride();
    }

    void ItemListWidget::onMouseSetFocus(MyGUI::Widget *_old) 
    {
        MWBase::Environment::get().getWindowManager()->setKeyFocusWidget(this);
        mName->setStateSelected(true);
        Base::onMouseSetFocus(_old);
    }

    void ItemListWidget::onMouseLostFocus(MyGUI::Widget *_new)
    {
        MWBase::Environment::get().getWindowManager()->setKeyFocusWidget(nullptr);
        mName->setStateSelected(false);
        Base::onMouseLostFocus(_new);
    }

    void ItemListWidget::createIcon(const ItemStack& item, const std::string& typeName)
    {
        std::string invIcon = item.mBase.getClass().getInventoryIcon(item.mBase);
        if (invIcon.empty())
            invIcon = "default icon.tga";
        mIcon->setImageTexture(MWBase::Environment::get().getWindowManager()->correctIconPath(invIcon));
        mIcon->setVisible(true);
    }

    void ItemListWidget::createName(const ItemStack& item, const std::string& typeName)
    {
        auto countSuffix = (item.mCount <= 1) ? "" : " (" + MyGUI::utility::toString(item.mCount) + ")";

        mName->setCaption(item.mBase.getClass().getName(item.mBase) + countSuffix);
        mName->setVisible(true);
    }

    void ItemListWidget::createWeight(const ItemStack& item, const std::string& typeName)
    {
        float weight = item.mBase.getClass().getWeight(item.mBase);
        if (!std::isnormal(weight) && weight != 0)
        {
            mWeight->setCaption("-");
            return; 
        }
        
        mWeight->setCaption(MyGUI::utility::toString(item.mBase.getClass().getWeight(item.mBase)*item.mCount));
        mWeight->setVisible(true);
    }

    void ItemListWidget::createValue(const ItemStack& item, const std::string& typeName)
    {
        float value = item.mBase.getClass().getValue(item.mBase);
        if (value <= 0 || (item.mFlags & ItemStack::Flag_Bound))
            mValue->setCaption("-");
        else 
            mValue->setCaption(MyGUI::utility::toString(value));
        mValue->setVisible(true);
    }

    void ItemListWidget::createRatio(const ItemStack& item, const std::string& typeName)
    {
        if (item.mBase.isEmpty())
            mRatio->setCaption("-");
        else
        {
            float v = item.mBase.getClass().getValue(item.mBase);
            float w = item.mBase.getClass().getWeight(item.mBase);

            if (w > 0)
                mRatio->setCaption(MyGUI::utility::toString(std::roundf(v/w)));
        }
        mRatio->setVisible(true);
    }

    void ItemListWidget::createArmorClass(const ItemStack& item, const std::string& typeName)
    {
        if (typeName == typeid(ESM::Armor).name())
        {
            std::string typeText = "-";
            const MWWorld::LiveCellRef<ESM::Armor> *ref = item.mBase.get<ESM::Armor>();
            if (ref->mBase->mData.mWeight == 0)
                typeText = "-";
            else
            {
                int armorType = item.mBase.getClass().getEquipmentSkill(item.mBase);
                if (armorType == ESM::Skill::LightArmor)
                    typeText = "#{sLight}";
                else if (armorType == ESM::Skill::MediumArmor)
                    typeText = "#{sMedium}";
                else
                    typeText = "#{sHeavy}";
            }
            mArmorClass->setCaptionWithReplacing(typeText);
        }
        mArmorClass->setVisible(true);
    }

    void ItemListWidget::createWeaponType(const ItemStack& item, const std::string& typeName)
    {
        if (typeName == typeid(ESM::Weapon).name())
        {            
            auto typeIt = mWeapType.find(static_cast<ESM::Weapon::Type>(item.mBase.get<ESM::Weapon>()->mBase->mData.mType));
            if (typeIt != mWeapType.end())
                mWeaponType->setCaption(typeIt->second);
            else 
                mWeaponType->setCaption("-");
        }
        mWeaponType->setVisible(true);
    }
}