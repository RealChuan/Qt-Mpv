#include "playlistview.hpp"

PlayListView::PlayListView(QWidget *parent)
    : QListView(parent)
{
    setAlternatingRowColors(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
}

PlayListView::~PlayListView() = default;

auto PlayListView::selectedAllIndexs() const -> QModelIndexList
{
    return selectedIndexes();
}
