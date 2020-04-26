#include "quickloot.hpp"

#include <MyGUI_Gui.h>
#include <MyGUI_RenderManager.h>
#include <MyGUI_InputManager.h>
#include <MyGUI_ImageBox.h>

#include <components/settings/settings.hpp>
#include <components/widgets/box.hpp>

#include "../mwbase/world.hpp"
#include "../mwbase/environment.hpp"
#include "../mwbase/windowmanager.hpp"
#include "../mwbase/mechanicsmanager.hpp"

#include "../mwworld/class.hpp"
#include "../mwworld/esmstore.hpp"
#include "../mwmechanics/spellcasting.hpp"
#include "../mwmechanics/actorutil.hpp"
#include "../mwmechanics/creaturestats.hpp"

#include "inventorywindow.hpp"

#include "inventoryitemmodel.hpp"
#include "pickpocketitemmodel.hpp"
#include "containeritemmodel.hpp"

#include "itemmodel.hpp"

namespace MWGui
{
    QuickLoot::QuickLoot() :
        Layout("openmw_quickloot.layout")
        , mFocusToolTipX(0.0)
        , mFocusToolTipY(0.0)
        , mHorizontalScrollIndex(0)
        , mDelay(0.0)
        , mRemainingDelay(0.0)
        , mLastMouseX(0)
        , mLastMouseY(0)
        , mEnabled(true)
        , mFrameDuration(0.f)
        , mQuickLoot(nullptr)
        , mModel(nullptr)
        , mSortModel(nullptr)
        , mLastIndex(0)
    {
        getWidget(mQuickLoot, "QuickLoot");
        getWidget(mLabel, "Label");

        setVisibleAll(false);

        mQuickLoot->eventKeyButtonPressed += MyGUI::newDelegate(this, &QuickLoot::onKeyButtonPressed);
        mQuickLoot->eventItemClicked += MyGUI::newDelegate(this, &QuickLoot::onItemSelected);

        mQuickLoot->disableHeader(true);

        // turn off mouse focus so that getMouseFocusWidget returns the correct widget,
        // even if the mouse is over the tooltip
        mQuickLoot->setNeedKeyFocus(true);
        mQuickLoot->setNeedMouseFocus(true);
        mMainWidget->setNeedMouseFocus(false);
        mMainWidget->setNeedKeyFocus(false);

        // same delay as tooltips, for now 
        mDelay = Settings::Manager::getFloat("tooltip delay", "GUI");
        mRemainingDelay = mDelay;
    }

    void QuickLoot::notifyMouseWheel(int rel)
    {
        if (rel < 0)
            mLastIndex++;
        else 
            mLastIndex = std::max(0, mLastIndex - 1);
    }

    // TODO: add take all shortcut 
    void QuickLoot::onItemSelected(int index)
    {
        if (!mModel) return;

        const ItemStack& item = mModel->getItem(mSortModel->mapToSource(index));
        std::string sound = item.mBase.getClass().getDownSoundId(item.mBase);
        MWBase::Environment::get().getWindowManager()->playSound(sound);

        MWWorld::Ptr object = item.mBase;
        int count = 1;
        bool shift = MyGUI::InputManager::getInstance().isShiftPressed();

        if (shift)
            count = item.mCount;

        if (!mModel->onTakeItem(item.mBase,count))
            return;

        mModel->moveItem(item,count,MWBase::Environment::get().getWindowManager()->getInventoryWindow()->getModel());
        MWBase::Environment::get().getWindowManager()->getInventoryWindow()->updateItemView();
        mQuickLoot->update();
        mQuickLoot->forceItemFocused(index);
    }

    void QuickLoot::onKeyButtonPressed(MyGUI::Widget* sender, MyGUI::KeyCode key)
    {
    }

    void QuickLoot::setEnabled(bool enabled)
    {
        mEnabled = enabled;
    }

    void QuickLoot::onFrame(float frameDuration)
    {
        mFrameDuration = frameDuration;
    }

    void QuickLoot::setVisibleAll(bool visible)
    {
        mMainWidget->setVisible(visible);
        for (int i = 0; i < mMainWidget->getChildCount(); i++)
            mMainWidget->getChildAt(i)->setVisible(visible);
    }

    void QuickLoot::update(float frameDuration)
    {
        const MyGUI::IntSize &viewSize = MyGUI::RenderManager::getInstance().getViewSize();

        if (!mEnabled)
        {
            return;
        }

        MWBase::WindowManager *winMgr = MWBase::Environment::get().getWindowManager();
        bool guiMode = winMgr->isGuiMode();

        if (guiMode)
        {
            setVisibleAll(false);
            return;
        }
        
        if (!mFocusObject.isEmpty())
        {

            mQuickLoot->setModel(nullptr);
            mModel = nullptr;
            mSortModel = nullptr;
            mQuickLoot->update();

            bool loot = mFocusObject.getClass().isActor() && mFocusObject.getClass().getCreatureStats(mFocusObject).isDead();
            bool sneaking = MWBase::Environment::get().getMechanicsManager()->isSneaking(MWMechanics::getPlayer());

            mQuickLoot->getParent()->changeWidgetSkin("HUD_Box_NoTransp");

            if (mFocusObject.getClass().hasInventoryStore(mFocusObject))
            {
                if (mFocusObject.getClass().isNpc() && !loot && sneaking)
                {
                    mModel = new PickpocketItemModel(mFocusObject, new InventoryItemModel(mFocusObject),
                        !mFocusObject.getClass().getCreatureStats(mFocusObject).getKnockedDown());
                    mQuickLoot->getParent()->changeWidgetSkin("HUD_Box_NoTransp_Owned");
                }
                else if (loot)
                {
                    mModel = new InventoryItemModel(mFocusObject);
                }
                else 
                {
                    mModel = nullptr;
                    setVisibleAll(false);
                }
            }
            else 
            {
                mModel = new ContainerItemModel(mFocusObject);
            }

            if (mModel)
            {
                mSortModel = new SortFilterItemModel(mModel);
                mSortModel->setCategory(SortFilterItemModel::Category_Simple);
                mQuickLoot->setModel(mSortModel);
                mQuickLoot->update();
            }
            MWBase::Environment::get().getWindowManager()->setKeyFocusWidget(mQuickLoot);
           
            if (mQuickLoot->getModel())
                mLastIndex = mQuickLoot->forceItemFocused(mLastIndex);

            if (mModel && mModel->getItemCount())
            {
                mLabel->setCaption(mFocusObject.getClass().getName(mFocusObject));
                setVisibleAll(true);
            }
            else 
            {
                setVisibleAll(false);
            }
        }

        MyGUI::IntSize tooltipSize = MyGUI::IntSize(mMainWidget->getWidth(),64 + mQuickLoot->requestListSize());
        setCoord(viewSize.width*0.7 - tooltipSize.width/2,
                viewSize.height*0.6 - tooltipSize.height/2,
                tooltipSize.width,
                tooltipSize.height);
    }

    void QuickLoot::position(MyGUI::IntPoint& position, MyGUI::IntSize size, MyGUI::IntSize viewportSize)
    {
        position += MyGUI::IntPoint(0, 32)
        - MyGUI::IntPoint(static_cast<int>(MyGUI::InputManager::getInstance().getMousePosition().left / float(viewportSize.width) * size.width), 0);

        if ((position.left + size.width) > viewportSize.width)
        {
            position.left = viewportSize.width - size.width;
        }
        if ((position.top + size.height) > viewportSize.height)
        {
            position.top = MyGUI::InputManager::getInstance().getMousePosition().top - size.height - 8;
        }
    }

    void QuickLoot::clear()
    {
        mFocusObject = MWWorld::Ptr();

        for (unsigned int i=0; i < mMainWidget->getChildCount(); ++i)
        {
            mMainWidget->getChildAt(i)->setVisible(false);
        }
    }

    // TODO: this never shows when we reload a save 
    void QuickLoot::setFocusObject(const MWWorld::Ptr& focus)
    {
        if (focus.isEmpty() || 
            (focus.getTypeName() != typeid(ESM::Container).name() && !focus.getClass().hasInventoryStore(focus)))
        {
            setVisibleAll(false);
            return;
        }
        
        mLastFocusObject = mFocusObject;
        mFocusObject = focus;

        update(mFrameDuration);
    }

    void QuickLoot::setFocusObjectScreenCoords(float min_x, float min_y, float max_x, float max_y)
    {
        mFocusToolTipX = (min_x + max_x) / 2;
        mFocusToolTipY = min_y;
    }

    void QuickLoot::setDelay(float delay)
    {
        mDelay = delay;
        mRemainingDelay = mDelay;
    }

}
