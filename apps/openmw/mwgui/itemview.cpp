#include "itemview.hpp"

#include <cmath>

#include <MyGUI_FactoryManager.h>
#include <MyGUI_Gui.h>
#include <MyGUI_ImageBox.h>
#include <MyGUI_TextBox.h>
#include <MyGUI_ScrollView.h>
#include <MyGUI_Button.h>

#include "../mwworld/class.hpp"
#include "../mwworld/inventorystore.hpp"

#include "../mwbase/inputmanager.hpp"
#include "../mwbase/windowmanager.hpp"
#include "../mwbase/environment.hpp"

#include "../mwmechanics/actorutil.hpp"

#include "inventorywindow.hpp"
#include "itemlistwidget.hpp"
#include "itemlistwidgetheader.hpp"
#include "itemmodel.hpp"
#include "itemwidget.hpp"

namespace MWGui
{

ItemView::ItemView()
    : mModel(nullptr)
    , mScrollView(nullptr)
    , mHeader(nullptr)
{
}

ItemView::~ItemView()
{
    delete mModel;
}

void ItemView::setModel(ItemModel *model)
{
    if (mModel == model)
        return;

    delete mModel;
    mModel = model;

    update();
}

void ItemView::initialiseOverride()
{
    Base::initialiseOverride();

    assignWidget(mScrollView, "ScrollView");
    if (mScrollView == nullptr)
        throw std::runtime_error("Item view needs a scroll view");
    assignWidget(mHeader, "Header"); 
    if (mHeader == nullptr)
        throw std::runtime_error("Item view needs a header view");
    mScrollView->setCanvasAlign(MyGUI::Align::Left | MyGUI::Align::Top);
}

void ItemView::layoutWidgets()
{
    if (!mScrollView->getChildCount())
        return;

    MyGUI::Widget* dragArea = mScrollView->getChildAt(0);
    int y = 0;

    mHeader->setSize(MyGUI::IntSize(mScrollView->getWidth()-50,36));

    unsigned int count = dragArea->getChildCount();
    int h = (count) ? dragArea->getChildAt(0)->getHeight() : 0; 

    for (unsigned int i=0; i<count; ++i)
    {
        dragArea->getChildAt(i)->setPosition(0,y);
        y += h;
    }

    MyGUI::IntSize size = MyGUI::IntSize(mScrollView->getSize().width,std::max(mScrollView->getSize().height, y));

    // Canvas size must be expressed with VScroll disabled, otherwise MyGUI would expand the scroll area when the scrollbar is hidden
    mScrollView->setVisibleVScroll(false);
    mScrollView->setVisibleHScroll(false);
    mScrollView->setCanvasSize(size);
    mScrollView->setVisibleVScroll(true);
    mScrollView->setVisibleHScroll(true);
    dragArea->setSize(size);
}

void ItemView::update()
{
    if (!mModel)
        return;

    mModel->update();
    
    while (mScrollView->getChildCount())
        MyGUI::Gui::getInstance().destroyWidget(mScrollView->getChildAt(0));
    while (mHeader->getChildCount())
        MyGUI::Gui::getInstance().destroyWidget(mHeader->getChildAt(0)); 

    MyGUI::Widget* dragArea = mScrollView->createWidget<MyGUI::Widget>("",0,0,mScrollView->getWidth(),mScrollView->getHeight(),
                                                                       MyGUI::Align::Stretch);
    dragArea->setNeedMouseFocus(true);
    dragArea->eventMouseButtonClick += MyGUI::newDelegate(this, &ItemView::onSelectedBackground);
    dragArea->eventMouseWheel += MyGUI::newDelegate(this, &ItemView::onMouseWheelMoved);
    
    int category = dynamic_cast<MWGui::SortFilterItemModel*>(mModel)->getCategory();  

    if (category == MWGui::SortFilterItemModel::Category_Weapon)
        mHeader->changeWidgetSkin("MW_ItemListHeader_Weapon");
    else if (category == MWGui::SortFilterItemModel::Category_Armor)
        mHeader->changeWidgetSkin("MW_ItemListHeader_Armor");
    else 
        mHeader->changeWidgetSkin("MW_ItemListHeader_All");

    for (ItemModel::ModelIndex i=0; i < static_cast<int>(mModel->getItemCount()); ++i)
    {
        const ItemStack& item = mModel->getItem(i);
        ItemListWidget* itemWidget = dragArea->createWidget<ItemListWidget>("MW_ItemList",
            MyGUI::IntCoord(0,0,mScrollView->getWidth()-50, 35), MyGUI::Align::HStretch | MyGUI::Align::Top);

        itemWidget->setNeedKeyFocus(true);
        itemWidget->setNeedMouseFocus(true);
        itemWidget->setItem(item, category);
        itemWidget->setUserString("ToolTipType", "ItemModelIndex");
        itemWidget->setUserData(std::make_pair(i,mModel)); 
        itemWidget->eventMouseButtonClick += MyGUI::newDelegate(this, &ItemView::onSelectedItem);
        itemWidget->eventMouseWheel += MyGUI::newDelegate(this, &ItemView::onMouseWheelMoved);
        itemWidget->eventKeyButtonPressed += MyGUI::newDelegate(this, &ItemView::onKeyButtonPressed);
    }
    layoutWidgets();
}
void ItemView::resetScrollBars()
{
    mScrollView->setViewOffset(MyGUI::IntPoint(0, 0));
}

void ItemView::onSelectedItem(MyGUI::Widget *sender)
{
    ItemModel::ModelIndex index = (*sender->getUserData<std::pair<ItemModel::ModelIndex, ItemModel*> >()).first;
    eventItemClicked(index);
}

void ItemView::onKeyButtonPressed(MyGUI::Widget *sender, MyGUI::KeyCode key, MyGUI::Char character)
{
    ItemModel::ModelIndex index = (*sender->getUserData<std::pair<ItemModel::ModelIndex, ItemModel*> >()).first;
    eventKeyButtonPressed(sender, key);
}

void ItemView::onSelectedBackground(MyGUI::Widget *sender)
{
    eventBackgroundClicked();
}

void ItemView::onMouseWheelMoved(MyGUI::Widget *_sender, int _rel)
{
    if (mScrollView->getViewOffset().top + _rel*0.3f > 0)
        mScrollView->setViewOffset(MyGUI::IntPoint(0, 0));
    else
        mScrollView->setViewOffset(MyGUI::IntPoint(0,static_cast<int>(mScrollView->getViewOffset().top + _rel*0.3f)));
}

void ItemView::setSize(const MyGUI::IntSize &_value)
{
    bool changed = (_value.width != getWidth() || _value.height != getHeight());
    Base::setSize(_value);
    if (changed)
        layoutWidgets();
}

void ItemView::setCoord(const MyGUI::IntCoord &_value)
{
    bool changed = (_value.width != getWidth() || _value.height != getHeight());
    Base::setCoord(_value);
    if (changed)
        layoutWidgets();
}

void ItemView::registerComponents()
{
    MyGUI::FactoryManager::getInstance().registerFactory<MWGui::ItemView>("Widget");
}

}
