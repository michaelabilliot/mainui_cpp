/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "Framework.h"
#include "Bitmap.h"
#include "YesNoMessageBox.h"
#include "keydefs.h"
#include "PlayerIntroduceDialog.h"
#include "Action.h"
#include "Field.h"
#include "CofMultiplayerAddress.h"

#define ART_BANNER			"gfx/shell/head_multi"

class CMenuMultiplayer : public CMenuFramework
{
public:
	CMenuMultiplayer() : CMenuFramework( "CMenuMultiplayer" ) { }

	void Show() override
	{
		CMenuBaseWindow *previous = uiStatic.menu.Current();
		CMenuFramework::Show();
		if (cofMenu) {
			if (previous && previous != this) previous->DisableTransition();
			EngFuncs::KEY_SetDest(KEY_MENU);
			DisableTransition();
			SetCursorToItem(address);
			return;
		}

		if( EngFuncs::GetCvarFloat( "menu_mp_firsttime2" ) && EngFuncs::GetCvarFloat( "cl_nopred" ) )
		{
			EngFuncs::CvarSetValue( "cl_nopred", 0.0f );
		}

		if( !UI::Names::CheckIsNameValid( EngFuncs::GetCvarString( "name" ) ) )
		{
			UI_PlayerIntroduceDialog_Show( this );
		}
	}
	void Draw() override
	{
		if (cofMenu) UI_FillRect(GetRenderPosition(), GetRenderSize(), 0xFF121212);
		CMenuFramework::Draw();
	}
	bool KeyDown(int key) override
	{
		if (cofMenu && UI::Key::IsEnter(key) && ItemAtCursor() == &address) {
			Join();
			return true;
		}
		return CMenuFramework::KeyDown(key);
	}

private:
	void _Init() override;
	bool cofMenu = false;
	CMenuField address, playerName, password;
	CMenuAction title, help, hostHelp, error, connect, host, manhunt, lan, back;
	char errorText[160] = {};
	void CofInit();
	void Join();
	void HostCampaign() { Host("cof_campaign_01"); }
	void HostManhunt() { Host("cof_manhunt_campaign"); }
	void Host(const char *map);
	bool SavePlayer();
	void Error(const char *message) {
		Q_strncpy(errorText, message, sizeof(errorText));
		error.szName = errorText;
	}
};

void CMenuMultiplayer::CofInit()
{
	EngFuncs::CvarRegister("ui_cof_address", "", FCVAR_ARCHIVE);
	title.SetNameAndStatus("MULTIPLAYER / CO-OP", nullptr);
	title.SetRect(80, 72, 850, 48);
	title.charSize = 32;
	title.iFlags = QMF_INACTIVE;
	AddItem(title);
	help.SetNameAndStatus("Use your host's Radmin / Hamachi IPv4 address to join.", nullptr);
	help.SetRect(80, 128, 870, 30);
	help.charSize = 16;
	help.iFlags = QMF_INACTIVE;
	help.colorBase = 0xFFBDBDBD;
	AddItem(help);
	playerName.SetNameAndStatus("Player name", nullptr);
	playerName.LinkCvar("name");
	playerName.SetRect(80, 205, 560, 36);
	playerName.iMaxLength = 31;
	playerName.eTextAlignment = QM_LEFT;
	AddItem(playerName);
	address.SetNameAndStatus("Host IP address (optional :port)", "Example: 192.168.193.226 or 192.168.193.226:27015");
	address.LinkCvar("ui_cof_address");
	address.SetRect(80, 295, 560, 36);
	address.iMaxLength = 63;
	address.eTextAlignment = QM_LEFT;
	AddItem(address);
	password.SetNameAndStatus("Password (leave empty for an open game)", nullptr);
	password.SetRect(80, 385, 560, 36);
	password.iMaxLength = 63;
	password.bHideInput = true;
	password.eTextAlignment = QM_LEFT;
	AddItem(password);
	connect.SetNameAndStatus("CONNECT", "Join the host at the address above");
	connect.SetRect(680, 295, 260, 36);
	connect.onReleased = VoidCb(&CMenuMultiplayer::Join);
	host.SetNameAndStatus("HOST CAMPAIGN", "Start the native four-player campaign lobby");
	host.SetRect(80, 490, 390, 42);
	host.onReleased = VoidCb(&CMenuMultiplayer::HostCampaign);
	manhunt.SetNameAndStatus("HOST MANHUNT", "Start the native four-player Manhunt lobby");
	manhunt.SetRect(510, 490, 390, 42);
	manhunt.onReleased = VoidCb(&CMenuMultiplayer::HostManhunt);
	lan.SetNameAndStatus("BROWSE LAN", "Search for Xash servers on your local network");
	lan.SetRect(80, 610, 390, 42);
	lan.onReleased = UI_LanGame_Menu;
	back.SetNameAndStatus("BACK", nullptr);
	back.SetRect(510, 610, 390, 42);
	back.onReleased = VoidCb(&CMenuMultiplayer::Hide);
	CMenuAction *buttons[] = { &connect, &host, &manhunt, &lan, &back };
	for (auto button : buttons) {
		button->SetBackground(0xFF292929, 0xFF454545);
		button->charSize = 20;
		button->colorBase = 0xFFDEDEDE;
		button->colorFocus = 0xFFFFFFFF;
		AddItem(*button);
	}
	hostHelp.SetNameAndStatus("Host: 4 players / UDP 27015. Share your VPN IPv4 address with friends.", nullptr);
	hostHelp.SetRect(80, 550, 870, 28);
	hostHelp.charSize = 16;
	hostHelp.iFlags = QMF_INACTIVE;
	hostHelp.colorBase = 0xFFBDBDBD;
	AddItem(hostHelp);
	error.SetNameAndStatus(errorText, nullptr);
	error.SetRect(80, 690, 870, 32);
	error.charSize = 16;
	error.colorBase = 0xFFFFB0A0;
	error.iFlags = QMF_INACTIVE;
	AddItem(error);
}

bool CMenuMultiplayer::SavePlayer()
{
	if (!UI::Names::CheckIsNameValid(playerName.GetBuffer())) {
		Error("Enter a player name before connecting or hosting.");
		SetCursorToItem(playerName);
		return false;
	}
	playerName.WriteCvar();
	address.WriteCvar();
	Error("");
	return true;
}

void CMenuMultiplayer::Join()
{
	char endpoint[64];
	if (!CofMultiplayer::Address(address.GetBuffer(), endpoint, sizeof(endpoint))) {
		Error("Enter a valid IPv4 address, for example 192.168.193.226:27015.");
		SetCursorToItem(address);
		return;
	}
	if (!SavePlayer()) return;
	EngFuncs::CvarSetString("password", password.GetBuffer());
	EngFuncs::CvarSetValue("cl_nopred", 0.0f);
	// Endpoint contains only normalized digits, dots and one colon.
	EngFuncs::ClientCmdF(false, "connect %s\n", endpoint);
}

void CMenuMultiplayer::Host(const char *map)
{
	if (!EngFuncs::CheckGameDll() || !EngFuncs::IsMapValid(map)) {
		Error("The native server DLL or selected co-op campaign map is missing.");
		return;
	}
	if (!SavePlayer()) return;
	EngFuncs::CvarSetString("sv_password", password.GetBuffer());
	EngFuncs::CvarSetString("hostname", "Cry of Fear Co-op");
	// Hamachi/Radmin use 25.x/26.x, which this engine's LAN filter rejects.
	// A private, unlisted server still accepts a direct connection over the VPN.
	EngFuncs::CvarSetValue("sv_lan", 0.0f);
	EngFuncs::CvarSetValue("public", 0.0f);
	EngFuncs::CvarSetValue("sv_nat", 0.0f);
	EngFuncs::CvarSetValue("skill", 2.0f);
	EngFuncs::CvarSetValue("pausable", 0.0f);
	EngFuncs::CvarSetValue("cl_nopred", 0.0f);
	// This DLL selects CHalfLifeMultiplay from deathmatch, then sets sv_coop itself.
	// Reapply after disconnect; map startup applies the maxplayers latch.
	EngFuncs::ClientCmdF(false,
		"disconnect\nwait\nwait\nwait\nmaxplayers 4\ncoop 0\ndeathmatch 1\nteamplay 0\nmap %s\n", map);
}

/*
=================
CMenuMultiplayer::Init
=================
*/
void CMenuMultiplayer::_Init( void )
{
	cofMenu = !stricmp(gMenu.m_gameinfo.gamefolder, "CryOfFear");
	if (cofMenu) { CofInit(); return; }
	banner.SetPicture( ART_BANNER );
	AddItem( banner );

	AddButton( L( "Internet game" ), L( "View list of a game internet servers and join the one of your choice" ), PC_INET_GAME, UI_InternetGames_Menu, QMF_NOTIFY );
	// AddButton( L( "Spectate game" ), L( "Spectate internet games" ), PC_SPECTATE_GAMES, NoopCb, QMF_GRAYED | QMF_NOTIFY );
	AddButton( L( "LAN game" ), L( "Set up the game on the local area network" ), PC_LAN_GAME, UI_LanGame_Menu, QMF_NOTIFY );
	AddButton( L( "GameUI_GameMenu_Customize" ), L( "Choose your player name, and select visual options for your character" ), PC_CUSTOMIZE, UI_PlayerSetup_Menu, QMF_NOTIFY );
	AddButton( L( "Controls" ), L( "Change keyboard and mouse settings" ), PC_CONTROLS, UI_Controls_Menu, QMF_NOTIFY );
	AddButton( L( "Done" ), L( "Go back to the Main menu" ), PC_DONE, VoidCb( &CMenuMultiplayer::Hide ), QMF_NOTIFY );
}

ADD_MENU( menu_multiplayer, CMenuMultiplayer, UI_MultiPlayer_Menu );
