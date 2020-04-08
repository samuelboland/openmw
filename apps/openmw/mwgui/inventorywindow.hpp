#ifndef MGUI_Inventory_H
#define MGUI_Inventory_H

#include "windowpinnablebase.hpp"
#include "mode.hpp"

#include <components/widgets/imagebutton.hpp>

#include "../mwworld/ptr.hpp"
#include "../mwrender/characterpreview.hpp"

namespace osg
{
    class Group;
}

namespace Resource
{
    class ResourceSystem;
}

namespace MWGui
{
    namespace Widgets
    {
        class MWDynamicStat;
    }

    class ItemView;
    class SortFilterItemModel;
    class TradeItemModel;
    class DragAndDrop;
    class ItemModel;

    class InventoryWindow : public WindowPinnableBase
    {
        public:
            InventoryWindow(DragAndDrop* dragAndDrop, osg::Group* parent, Resource::ResourceSystem* resourceSystem);

            virtual void onOpen();

            /// start trading, disables item drag&drop
            void setTrading(bool trading);

            void onFrame(float dt);

            void pickUpObject (MWWorld::Ptr object);

            SortFilterItemModel* getSortFilterModel();
            TradeItemModel* getTradeModel();
            ItemModel* getModel();

            void updateItemView();

            void updatePlayer();

            void clear();

            void useItem(const MWWorld::Ptr& ptr, bool force=false);

            void setGuiMode(GuiMode mode);

            /// Cycle to previous/next weapon
            void cycle(bool next);

        protected:
            virtual void onTitleDoubleClicked();

        private:
            DragAndDrop* mDragAndDrop;

            int mSelectedItem;

            MWWorld::Ptr mPtr;

            MWGui::ItemView* mItemView;
            SortFilterItemModel* mSortModel;
            TradeItemModel* mTradeModel;

            MyGUI::TextBox* mPlayerGold; 
            MyGUI::TextBox* mArmorRating;
            MyGUI::TextBox* mEncumbranceBar;

            Gui::ImageButton* mAllButton;
            Gui::ImageButton* mWeaponButton;
            Gui::ImageButton* mArmorButton;
            Gui::ImageButton* mClothButton;
            Gui::ImageButton* mPotionButton;
            Gui::ImageButton* mIngredientButton;
            Gui::ImageButton* mBookButton;
            Gui::ImageButton* mToolButton;
            Gui::ImageButton* mMagicButton;
            Gui::ImageButton* mMiscButton;
            
            MyGUI::EditBox* mFilterEdit;

            GuiMode mGuiMode;

            int mLastXSize;
            int mLastYSize;

            bool mTrading;
            float mScaleFactor;
            float mUpdateTimer;

            void toggleMaximized();
            
            void onHeaderClicked(int sort);
            void onItemSelected(int index);
            void onItemSelectedFromSourceModel(int index);

            void onBackgroundSelected();

            std::string getModeSetting() const;

            void sellItem(MyGUI::Widget* sender, int count);
            void dragItem(MyGUI::Widget* sender, int count);

            void onWindowResize(MyGUI::Window* _sender);
            void onFilterChanged(MyGUI::Widget* _sender);
            void onNameFilterChanged(MyGUI::EditBox* _sender);
            void onPinToggled();

            void updateEncumbranceBar();
            void notifyContentChanged();
            void dirtyPreview();
            void updateArmorRating();
            void updatePlayerGold();

            /// Unequips count items from mSelectedItem, if it is equipped, and then updates mSelectedItem in case the items were re-stacked
            void ensureSelectedItemUnequipped(int count);
    };
}

#endif // Inventory_H
