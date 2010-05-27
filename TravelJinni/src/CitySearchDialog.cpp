/*
  TravelJinni - Openstreetmap offline viewer
  Copyright (C) 2009  Tim Teulings

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "CitySearchDialog.h"

#include <Lum/Object.h>
#include <Lum/Panel.h>
#include <Lum/String.h>
#include <Lum/Table.h>

#include <osmscout/Database.h>

CitySearchDialog::CitySearchDialog(DatabaseTask* databaseTask)
 : databaseTask(databaseTask),
   okAction(new Lum::Model::Action()),
   regionName(new Lum::Model::String(L"")),
   searchTimerAction(new Lum::Model::Action()),
   regionsModel(new RegionsModel(regions)),
   regionSelection(new Lum::Model::SingleLineSelection()),
   hasResult(false)
{
  Observe(okAction);
  Observe(GetClosedAction());
  Observe(regionName);
  Observe(searchTimerAction);
  Observe(regionSelection);

  okAction->Disable();
}

Lum::Object* CitySearchDialog::GetContent()
{
  Lum::Panel            *panel;
  Lum::String           *string;
  Lum::Table            *table;
  Lum::Model::HeaderRef headerModel;

  panel=Lum::VPanel::Create(true,true);

  string=Lum::String::Create(regionName,25,true,false);

  panel->Add(string);

  panel->AddSpace();

  headerModel=new Lum::Model::HeaderImpl();
  headerModel->AddColumn(L"Name",Lum::Base::Size::stdCharWidth,25,true);

  table=new Lum::Table();
  table->SetFlex(true,true);
  table->SetMinWidth(Lum::Base::Size::stdCharWidth,60);
  table->SetMinHeight(Lum::Base::Size::stdCharHeight,6);
  table->SetShowHeader(true);
  table->GetTableView()->SetAutoFitColumns(true);
  table->GetTableView()->SetAutoVSize(true);
  table->SetModel(regionsModel);
  table->SetTablePainter(new RegionsModelPainter());
  table->SetHeaderModel(headerModel);
  table->SetSelection(regionSelection);
  table->SetDoubleClickAction(okAction);
  panel->Add(table);

  regionsModel->SetEmptyText(L"- no search criteria -");

  return panel;
}

void CitySearchDialog::GetActions(std::vector<Lum::Dlg::ActionInfo>& actions)
{
  Lum::Dlg::ActionDialog::CreateActionInfosOkCancel(actions,okAction,GetClosedAction());
}

void CitySearchDialog::FetchAdminRegions()
{
  bool limitReached=true;

  regionsModel->Off();

  if (!regionName->Empty()) {
    databaseTask->GetMatchingAdminRegions(regionName->Get(),regions,50,limitReached);

    if (limitReached) {
      regions.clear();
      regionsModel->SetEmptyText(L"- too many hits -");
    }
    else if (regions.size()==0) {
      regionsModel->SetEmptyText(L"- no matches -");
    }
    else {
      regionsModel->SetEmptyText(L"");
    }
  }
  else {
    regions.clear();
    regionsModel->SetEmptyText(L"- no search criteria -");
  }

  regionsModel->On();
}

void CitySearchDialog::Resync(Lum::Base::Model* model, const Lum::Base::ResyncMsg& msg)
{
  if (model==GetClosedAction() &&  GetClosedAction()->IsFinished()) {
    Exit();
  }
  else if (model==searchTimerAction && searchTimerAction->IsFinished()) {
    FetchAdminRegions();
  }
  else if (model==regionName) {
    if (regionName->Empty()) {
      FetchAdminRegions();
    }
    else {
      Lum::OS::display->RemoveTimer(searchTimerAction);
      Lum::OS::display->AddTimer(1,150000,searchTimerAction);
    }
  }
  else if (model==regionSelection) {
    if (regionSelection->HasSelection()) {
      okAction->Enable();
    }
    else {
      okAction->Disable();
    }
  }
  else if (model==okAction && okAction->IsEnabled() && okAction->IsFinished()) {
    assert(regionSelection->HasSelection());

    result=regionsModel->GetEntry(regionSelection->GetLine());
    hasResult=true;
    Exit();
  }
  else {
    Dialog::Resync(model,msg);
  }
}

bool CitySearchDialog::HasResult() const
{
  return hasResult;
}

const osmscout::AdminRegion& CitySearchDialog::GetResult() const
{
  return result;
}


