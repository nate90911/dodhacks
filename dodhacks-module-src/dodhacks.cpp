
#include "amxxmodule.h"

#ifndef __linux__
#include <detours.h>
#else
#include <subhook.h>
#include <sys/mman.h>
#endif

#include <Memory.h>

enum : unsigned char
{
	SIG_PLAYERSPAWN = false,
	SIG_GIVENAMEDITEM,
	SIG_DROPPLAYERITEM,
	SIG_GIVEAMMO,
	SIG_SETWAVETIME,
	SIG_GETWAVETIME,
	SIG_REMOVEPLAYERITEM,
	SIG_ADDPLAYERITEM,
	SIG_REMOVEALLITEMS,
	SIG_SETBODYGROUP,
	SIG_DESTROYITEM,
	SIG_SUBREMOVE,
	SIG_PACKWEAPON,
	SIG_WPNBOXKILL,
	SIG_WPNBOXACTIVATETHINK,
	SIG_CREATE,
	SIG_INSTALLGAMERULES,
	SIG_UTILREMOVE,

	SIG_PATCHFG42,
	SIG_PATCHENFIELD,

	SIG_ORIGFG42_BYTE,
	SIG_PATCHFG42_BYTE,

	SIG_ORIGENFIELD_BYTE,
	SIG_PATCHENFIELD_BYTE,

	SIG_OFFS_ALLIESAREBRIT,
	SIG_OFFS_ALLIESAREPARA,
	SIG_OFFS_AXISAREPARA,
	SIG_OFFS_ALLIESRESPAWNFACTOR,
	SIG_OFFS_AXISRESPAWNFACTOR,
	SIG_OFFS_ALLIESINFINITELIVES,
	SIG_OFFS_AXISINFINITELIVES,

	SIG_OFFS_ITEMSCOPE,
	SIG_OFFS_APPLYITEMSCOPE,

	SIG_OFFS_THINKFUNC_PFN,
	SIG_OFFS_THINKFUNC_DELTA,
};

enum : unsigned char
{
	DOD_FA_PLAYERSPAWN = false,
	DOD_FA_GIVENAMEDITEM,
	DOD_FA_DROPPLAYERITEM,
	DOD_FA_GIVEAMMO,
	DOD_FA_SETWAVETIME,
	DOD_FA_GETWAVETIME,
	DOD_FA_REMOVEPLAYERITEM,
	DOD_FA_ADDPLAYERITEM,
	DOD_FA_REMOVEALLITEMS,
	DOD_FA_SETBODYGROUP,
	DOD_FA_DESTROYITEM,
	DOD_FA_SUBREMOVE,
	DOD_FA_PACKWEAPON,
	DOD_FA_WPNBOXKILL,
	DOD_FA_WPNBOXACTIVATETHINK,
	DOD_FA_CREATE,
	DOD_FA_INSTALLGAMERULES,
	DOD_FA_UTILREMOVE,
};

typedef void(__thiscall* DoD_PlayerSpawn_Type) (::size_t CDoDTeamPlay, ::size_t CBasePlayer);
typedef ::edict_t* (__thiscall* DoD_GiveNamedItem_Type) (::size_t CBasePlayer, const char* pItem);
typedef void(__thiscall* DoD_DropPlayerItem_Type) (::size_t CBasePlayer, char* pItem, bool Force);
typedef int(__thiscall* DoD_GiveAmmo_Type) (::size_t CBasePlayer, int Ammo, const char* pName, int Max);
typedef void(__thiscall* DoD_SetWaveTime_Type) (::size_t CDoDTeamPlay, int Team, float Time);
typedef float(__thiscall* DoD_GetWaveTime_Type) (::size_t CDoDTeamPlay, int Team);
typedef int(__thiscall* DoD_RemovePlayerItem_Type) (::size_t CBasePlayer, ::size_t CBasePlayerItem);
typedef int(__thiscall* DoD_AddPlayerItem_Type) (::size_t CBasePlayer, ::size_t CBasePlayerItem);
typedef void(__thiscall* DoD_RemoveAllItems_Type) (::size_t CBasePlayer, int RemoveSuit);
typedef void(__thiscall* DoD_SetBodygroup_Type) (::size_t CBasePlayer, int Group, int Value);
typedef void(__thiscall* DoD_DestroyItem_Type) (::size_t CBasePlayerItem);
typedef void(__thiscall* DoD_SubRemove_Type) (::size_t CBaseEntity);
typedef int(__thiscall* DoD_PackWeapon_Type) (::size_t CWeaponBox, ::size_t CBasePlayerItem);
typedef void(__thiscall* DoD_WpnBoxKill_Type) (::size_t CWeaponBox);
typedef void(__thiscall* DoD_WpnBoxActivateThink_Type) (::size_t CWeaponBox);

typedef ::size_t(*DoD_Create_Type) (char* pItem, const ::Vector* pOrigin, const ::Vector* pAngles, ::edict_t* pOwner);
typedef ::size_t(*DoD_InstallGameRules_Type) ();
typedef void (*DoD_UtilRemove_Type) (::size_t CBaseEntity);

struct AllocatedString
{
	::SourceHook::String Buffer;
	::size_t Index;
};

struct SignatureData
{
	::SourceHook::String Symbol;
	::SourceHook::CVector < unsigned char > Signature;
	bool IsSymbol;
	::size_t Offs;
};

struct CustomKeyValue_Del
{
	::SourceHook::String Class;
	::SourceHook::String Key;
	::SourceHook::String Value;
	::SourceHook::String Map;
};

struct CustomKeyValue_Add : CustomKeyValue_Del
{
	bool Added;
};

::size_t g_CDoDTeamPlay = false;

::DoD_PlayerSpawn_Type DoD_PlayerSpawn = NULL;
::DoD_GiveNamedItem_Type DoD_GiveNamedItem = NULL;
::DoD_DropPlayerItem_Type DoD_DropPlayerItem = NULL;
::DoD_GiveAmmo_Type DoD_GiveAmmo = NULL;
::DoD_SetWaveTime_Type DoD_SetWaveTime = NULL;
::DoD_GetWaveTime_Type DoD_GetWaveTime = NULL;
::DoD_RemovePlayerItem_Type DoD_RemovePlayerItem = NULL;
::DoD_AddPlayerItem_Type DoD_AddPlayerItem = NULL;
::DoD_RemoveAllItems_Type DoD_RemoveAllItems = NULL;
::DoD_SetBodygroup_Type DoD_SetBodygroup = NULL;
::DoD_DestroyItem_Type DoD_DestroyItem = NULL;
::DoD_SubRemove_Type DoD_SubRemove = NULL;
::DoD_PackWeapon_Type DoD_PackWeapon = NULL;
::DoD_InstallGameRules_Type DoD_InstallGameRules = NULL;
::DoD_UtilRemove_Type DoD_UtilRemove = NULL;
::DoD_WpnBoxKill_Type DoD_WpnBoxKill = NULL;
::DoD_WpnBoxActivateThink_Type DoD_WpnBoxActivateThink = NULL;
::DoD_Create_Type DoD_Create = NULL;

bool g_DoDPlayerSpawn_Hook = false;
bool g_DoDGiveNamedItem_Hook = false;
bool g_DoDDropPlayerItem_Hook = false;
bool g_DoDGiveAmmo_Hook = false;
bool g_DoDSetWaveTime_Hook = false;
bool g_DoDGetWaveTime_Hook = false;
bool g_DoDRemovePlayerItem_Hook = false;
bool g_DoDAddPlayerItem_Hook = false;
bool g_DoDRemoveAllItems_Hook = false;
bool g_DoDSetBodygroup_Hook = false;
bool g_DoDDestroyItem_Hook = false;
bool g_DoDSubRemove_Hook = false;
bool g_DoDPackWeapon_Hook = false;
bool g_DoDInstallGameRules_Hook = false;
bool g_DoDUtilRemove_Hook = false;
bool g_DoDWpnBoxKill_Hook = false;
bool g_DoDWpnBoxActivateThink_Hook = false;
bool g_DoDCreate_Hook = false;

#ifdef __linux__
::subhook_t g_pDoDPlayerSpawn = NULL;
::subhook_t g_pDoDGiveNamedItem = NULL;
::subhook_t g_pDoDDropPlayerItem = NULL;
::subhook_t g_pDoDGiveAmmo = NULL;
::subhook_t g_pDoDSetWaveTime = NULL;
::subhook_t g_pDoDGetWaveTime = NULL;
::subhook_t g_pDoDRemovePlayerItem = NULL;
::subhook_t g_pDoDAddPlayerItem = NULL;
::subhook_t g_pDoDRemoveAllItems = NULL;
::subhook_t g_pDoDSetBodygroup = NULL;
::subhook_t g_pDoDDestroyItem = NULL;
::subhook_t g_pDoDSubRemove = NULL;
::subhook_t g_pDoDPackWeapon = NULL;
::subhook_t g_pDoDInstallGameRules = NULL;
::subhook_t g_pDoDUtilRemove = NULL;
::subhook_t g_pDoDWpnBoxKill = NULL;
::subhook_t g_pDoDWpnBoxActivateThink = NULL;
::subhook_t g_pDoDCreate = NULL;
#endif

void* g_pDoDPlayerSpawn_Addr = NULL;
void* g_pDoDGiveNamedItem_Addr = NULL;
void* g_pDoDDropPlayerItem_Addr = NULL;
void* g_pDoDGiveAmmo_Addr = NULL;
void* g_pDoDSetWaveTime_Addr = NULL;
void* g_pDoDGetWaveTime_Addr = NULL;
void* g_pDoDRemovePlayerItem_Addr = NULL;
void* g_pDoDAddPlayerItem_Addr = NULL;
void* g_pDoDRemoveAllItems_Addr = NULL;
void* g_pDoDSetBodygroup_Addr = NULL;
void* g_pDoDDestroyItem_Addr = NULL;
void* g_pDoDSubRemove_Addr = NULL;
void* g_pDoDPackWeapon_Addr = NULL;
void* g_pDoDInstallGameRules_Addr = NULL;
void* g_pDoDUtilRemove_Addr = NULL;
void* g_pDoDWpnBoxKill_Addr = NULL;
void* g_pDoDWpnBoxActivateThink_Addr = NULL;
void* g_pDoDCreate_Addr = NULL;

int g_fwPlayerSpawn = false;
int g_fwGiveNamedItem = false;
int g_fwDropPlayerItem = false;
int g_fwAddPlayerItem = false;
int g_fwRemovePlayerItem = false;
int g_fwGetWaveTime = false;
int g_fwSetWaveTime = false;
int g_fwRemoveAllItems = false;
int g_fwGiveAmmo = false;
int g_fwSetBodygroup = false;
int g_fwDestroyItem = false;
int g_fwSubRemove = false;
int g_fwPackWeapon = false;
int g_fwInstallGameRules = false;
int g_fwUtilRemove = false;
int g_fwWpnBoxKill = false;
int g_fwWpnBoxActivateThink = false;
int g_fwCreate = false;

int g_fwPlayerSpawn_Post = false;
int g_fwGiveNamedItem_Post = false;
int g_fwDropPlayerItem_Post = false;
int g_fwAddPlayerItem_Post = false;
int g_fwRemovePlayerItem_Post = false;
int g_fwGetWaveTime_Post = false;
int g_fwSetWaveTime_Post = false;
int g_fwRemoveAllItems_Post = false;
int g_fwGiveAmmo_Post = false;
int g_fwSetBodygroup_Post = false;
int g_fwDestroyItem_Post = false;
int g_fwSubRemove_Post = false;
int g_fwPackWeapon_Post = false;
int g_fwInstallGameRules_Post = false;
int g_fwUtilRemove_Post = false;
int g_fwWpnBoxKill_Post = false;
int g_fwWpnBoxActivateThink_Post = false;
int g_fwCreate_Post = false;

unsigned char* g_pAutoScopeFG42Addr = NULL;
unsigned char* g_pAutoScopeEnfieldAddr = NULL;

::SourceHook::CVector < ::SignatureData > g_Sigs;
::SourceHook::CVector < ::AllocatedString > g_Strings;
::SourceHook::CVector < ::CustomKeyValue_Add > g_CustomKeyValues_Add;
::SourceHook::CVector < ::CustomKeyValue_Del > g_CustomKeyValues_Del;

::size_t setupString(const char* pString)
{
	for (const auto& String : ::g_Strings)
	{
		if (false == String.Buffer.icmp(pString))
			return String.Index;
	}

	::AllocatedString String;
	String.Buffer = pString;
	String.Index = (*::g_engfuncs.pfnAllocString) (pString);
	::g_Strings.push_back(String);
	return String.Index;
}

int baseToIndex(::size_t CBase)
{
	if (false == CBase)
		return -1;

	::entvars_t* pVars = *(::entvars_t**)(CBase + 4);
	if (!pVars)
		return -1;

	::edict_t* pEntity = pVars->pContainingEntity;
	if (!pEntity)
		return -1;

	return ::ENTINDEX(pEntity);
}

void ServerDeactivate_Post()
{
	::g_Strings.clear();
	::g_CustomKeyValues_Add.clear();
	::g_CustomKeyValues_Del.clear();

	RETURN_META(::MRES_IGNORED);
}

void DispatchKeyValue(::edict_t* pEntity, ::KeyValueData* pKvData)
{
	if (!pEntity)
	{
		RETURN_META(::MRES_IGNORED);
	}
	::KeyValueData keyValData;
	::SourceHook::String Map = STRING(::gpGlobals->mapname);
	::SourceHook::String Class = STRING(pEntity->v.classname);
	for (auto& cusKeyVal : ::g_CustomKeyValues_Add)
	{ /// Add/ update the desired key values to map.
		if (cusKeyVal.Added || cusKeyVal.Class.icmp(Class) || (false == cusKeyVal.Map.empty() && cusKeyVal.Map.icmp(Map)))
			continue;
		keyValData.szClassName = cusKeyVal.Class.c_str();
		keyValData.szKeyName = cusKeyVal.Key.c_str();
		keyValData.szValue = cusKeyVal.Value.c_str();
		keyValData.fHandled = false;
		::gpGamedllFuncs->dllapi_table->pfnKeyValue(pEntity, &keyValData);
		if (false == keyValData.fHandled)
		{
			MF_Log("*** Failed adding K: '%s' & V: '%s' to '%s'! ***\n",
				keyValData.szKeyName, keyValData.szValue, keyValData.szClassName);
		}
		cusKeyVal.Added = true;
	}
	if (pKvData && pKvData->szKeyName && *pKvData->szKeyName)
	{
		for (const auto& cusKeyVal : ::g_CustomKeyValues_Add)
		{ /// If this key value has already been inserted, do not insert it again on the same map.
			if (cusKeyVal.Class.icmp(Class) || (false == cusKeyVal.Map.empty() && cusKeyVal.Map.icmp(Map)) ||
				cusKeyVal.Key.icmp(pKvData->szKeyName))
			{ /// Has nothing to do with what we are looking for.
				continue;
			}
			pKvData->fHandled = true;
			RETURN_META(::MRES_SUPERCEDE);
		}
		for (const auto& cusKeyVal : ::g_CustomKeyValues_Del)
		{ /// If this is a key value that the user wants to remove, filter it out.
			if (cusKeyVal.Class.icmp(Class) || (false == cusKeyVal.Map.empty() && cusKeyVal.Map.icmp(Map)) ||
				cusKeyVal.Key.icmp(pKvData->szKeyName))
			{ /// Has nothing to do with what we are looking for.
				continue;
			}
			if (!pKvData->szValue || false == *pKvData->szValue || cusKeyVal.Value.empty() ||
				false == cusKeyVal.Value.icmp(pKvData->szValue))
			{ /// If there's no value, delete it because it matched the key. If the values matches, delete as well.
				RETURN_META(::MRES_SUPERCEDE);
			}
		}
	}
	RETURN_META(::MRES_IGNORED);
}

::cell DoD_PlayerSpawn_Native(::tagAMX* pAmx, ::cell* pParam)
{
	if (!::g_pDoDPlayerSpawn_Addr)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Signature for ::DoD_PlayerSpawn not found!");
		return false;
	}

	if (false == ::g_CDoDTeamPlay)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "::CDoDTeamPlay (Game Rules) pointer is null at the moment!");
		return false;
	}

	auto Player = pParam[1];
	if (Player < 1 || Player > ::gpGlobals->maxClients)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid player index %d!", Player);
		return false;
	}

	if (!::g_fn_IsPlayerValid(Player))
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Player %d is invalid!", Player);
		return false;
	}

	auto pPlayer = ::g_fn_GetPlayerEdict(Player);
	if (!pPlayer)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Player %d has no edict!", Player);
		return false;
	}

	auto pPlayerBase = pPlayer->pvPrivateData;
	if (!pPlayerBase)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Player %d has no private data!", Player);
		return false;
	}

	if (pParam[2])
		((::DoD_PlayerSpawn_Type) ::g_pDoDPlayerSpawn_Addr) (::g_CDoDTeamPlay, (::size_t)pPlayerBase);
	else
	{
		if (::g_DoDPlayerSpawn_Hook)
			::DoD_PlayerSpawn(::g_CDoDTeamPlay, (::size_t)pPlayerBase);
		else
			((::DoD_PlayerSpawn_Type) ::g_pDoDPlayerSpawn_Addr) (::g_CDoDTeamPlay, (::size_t)pPlayerBase);
	}
	return true;
}

::cell DoD_RemoveAllItems_Native(::tagAMX* pAmx, ::cell* pParam)
{
	if (!::g_pDoDRemoveAllItems_Addr)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Signature for ::DoD_RemoveAllItems not found!");
		return false;
	}

	auto Player = pParam[1];
	if (Player < 1 || Player > ::gpGlobals->maxClients)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid player index %d!", Player);
		return false;
	}

	if (!::g_fn_IsPlayerValid(Player))
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Player %d is invalid!", Player);
		return false;
	}
	if (!::g_fn_IsPlayerIngame(Player))
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Player %d is not in-game!", Player);
		return false;
	}

	auto pPlayer = ::g_fn_GetPlayerEdict(Player);
	if (!pPlayer)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Player %d has no edict!", Player);
		return false;
	}

	auto pPlayerBase = pPlayer->pvPrivateData;
	if (!pPlayerBase)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Player %d has no private data!", Player);
		return false;
	}

	if (pParam[3])
		((::DoD_RemoveAllItems_Type) ::g_pDoDRemoveAllItems_Addr) ((::size_t)pPlayerBase, pParam[2]);
	else
	{
		if (::g_DoDRemoveAllItems_Hook)
			::DoD_RemoveAllItems((::size_t)pPlayerBase, pParam[2]);
		else
			((::DoD_RemoveAllItems_Type) ::g_pDoDRemoveAllItems_Addr) ((::size_t)pPlayerBase, pParam[2]);
	}
	return true;
}

::cell DoD_SubRemove_Native(::tagAMX* pAmx, ::cell* pParam)
{
	if (!::g_pDoDSubRemove_Addr)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Signature for ::DoD_SubRemove not found!");
		return false;
	}

	auto Entity = pParam[1];
	if (Entity < 0 || Entity > ::gpGlobals->maxEntities)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid entity index %d!", Entity);
		return false;
	}

	auto pEntity = ::INDEXENT(Entity);
	if (!pEntity)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Entity %d has no edict!", Entity);
		return false;
	}

	auto pEntityBase = pEntity->pvPrivateData;
	if (!pEntityBase)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Entity %d has no private data!", Entity);
		return false;
	}

	if (pParam[2])
		((::DoD_SubRemove_Type) ::g_pDoDSubRemove_Addr) ((::size_t)pEntityBase);
	else
	{
		if (::g_DoDSubRemove_Hook)
			::DoD_SubRemove((::size_t)pEntityBase);
		else
			((::DoD_SubRemove_Type) ::g_pDoDSubRemove_Addr) ((::size_t)pEntityBase);
	}
	return true;
}

::cell DoD_PackWeapon_Native(::tagAMX* pAmx, ::cell* pParam)
{
	auto pRes = ::g_fn_GetAmxAddr(pAmx, pParam[3]);
	if (pRes)
		*pRes = false;

	if (!::g_pDoDPackWeapon_Addr)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Signature for ::DoD_PackWeapon not found!");
		return false;
	}

	auto Entity = pParam[1];
	if (Entity <= ::gpGlobals->maxClients || Entity > ::gpGlobals->maxEntities)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid weaponbox entity index %d!", Entity);
		return false;
	}

	auto Weapon = pParam[2];
	if (Weapon <= ::gpGlobals->maxClients || Weapon > ::gpGlobals->maxEntities)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid weapon entity index %d!", Weapon);
		return false;
	}

	auto pEntity = ::INDEXENT(Entity);
	if (!pEntity)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Entity (weaponbox) %d has no edict!", Entity);
		return false;
	}

	auto pWeapon = ::INDEXENT(Weapon);
	if (!pWeapon)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Entity (weapon) %d has no edict!", Weapon);
		return false;
	}

	auto pEntityBase = pEntity->pvPrivateData;
	if (!pEntityBase)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Entity (weaponbox) %d has no private data!", Entity);
		return false;
	}

	auto pWeaponBase = pWeapon->pvPrivateData;
	if (!pWeaponBase)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Entity (weapon) %d has no private data!", Weapon);
		return false;
	}

	if (pRes)
	{
		if (pParam[4])
			*pRes = ((::DoD_PackWeapon_Type) ::g_pDoDPackWeapon_Addr) ((::size_t)pEntityBase, (::size_t)pWeaponBase);
		else
		{
			if (::g_DoDPackWeapon_Hook)
				*pRes = ::DoD_PackWeapon((::size_t)pEntityBase, (::size_t)pWeaponBase);
			else
				*pRes = ((::DoD_PackWeapon_Type) ::g_pDoDPackWeapon_Addr) ((::size_t)pEntityBase, (::size_t)pWeaponBase);
		}
	}
	else
	{
		if (pParam[4])
			((::DoD_PackWeapon_Type) ::g_pDoDPackWeapon_Addr) ((::size_t)pEntityBase, (::size_t)pWeaponBase);
		else
		{
			if (::g_DoDPackWeapon_Hook)
				::DoD_PackWeapon((::size_t)pEntityBase, (::size_t)pWeaponBase);
			else
				((::DoD_PackWeapon_Type) ::g_pDoDPackWeapon_Addr) ((::size_t)pEntityBase, (::size_t)pWeaponBase);
		}
	}
	return true;
}

::cell DoD_DestroyItem_Native(::tagAMX* pAmx, ::cell* pParam)
{
	if (!::g_pDoDDestroyItem_Addr)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Signature for ::DoD_DestroyItem not found!");
		return false;
	}

	auto Entity = pParam[1];
	if (Entity <= gpGlobals->maxClients || Entity > ::gpGlobals->maxEntities)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid item entity index %d!", Entity);
		return false;
	}

	auto pEntity = ::INDEXENT(Entity);
	if (!pEntity)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Entity %d has no edict!", Entity);
		return false;
	}

	auto pEntityBase = pEntity->pvPrivateData;
	if (!pEntityBase)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Entity %d has no private data!", Entity);
		return false;
	}

	if (pParam[2])
		((::DoD_DestroyItem_Type) ::g_pDoDDestroyItem_Addr) ((::size_t)pEntityBase);
	else
	{
		if (::g_DoDDestroyItem_Hook)
			::DoD_DestroyItem((::size_t)pEntityBase);
		else
			((::DoD_DestroyItem_Type) ::g_pDoDDestroyItem_Addr) ((::size_t)pEntityBase);
	}
	return true;
}

::cell DoD_UtilRemove_Native(::tagAMX* pAmx, ::cell* pParam)
{
	if (!::g_pDoDUtilRemove_Addr)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Signature for ::DoD_UtilRemove not found!");
		return false;
	}

	auto Entity = pParam[1];
	if (Entity < 0 || Entity > ::gpGlobals->maxEntities)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid entity index %d!", Entity);
		return false;
	}

	auto pEntity = ::INDEXENT(Entity);
	if (!pEntity)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Entity %d has no edict!", Entity);
		return false;
	}

	auto pEntityBase = pEntity->pvPrivateData;
	if (!pEntityBase)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Entity %d has no private data!", Entity);
		return false;
	}

	if (pParam[2])
		((::DoD_UtilRemove_Type) ::g_pDoDUtilRemove_Addr) ((::size_t)pEntityBase);
	else
	{
		if (::g_DoDUtilRemove_Hook)
			::DoD_UtilRemove((::size_t)pEntityBase);
		else
			((::DoD_UtilRemove_Type) ::g_pDoDUtilRemove_Addr) ((::size_t)pEntityBase);
	}
	return true;
}

::cell DoD_WpnBoxKill_Native(::tagAMX* pAmx, ::cell* pParam)
{
	if (!::g_pDoDWpnBoxKill_Addr)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Signature for ::DoD_WpnBoxKill not found!");
		return false;
	}

	auto Entity = pParam[1];
	if (Entity <= ::gpGlobals->maxClients || Entity > ::gpGlobals->maxEntities)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid weaponbox entity index %d!", Entity);
		return false;
	}

	auto pEntity = ::INDEXENT(Entity);
	if (!pEntity)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Entity %d has no edict!", Entity);
		return false;
	}

	auto pEntityBase = pEntity->pvPrivateData;
	if (!pEntityBase)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Entity %d has no private data!", Entity);
		return false;
	}

	if (pParam[2])
		((::DoD_WpnBoxKill_Type) ::g_pDoDWpnBoxKill_Addr) ((::size_t)pEntityBase);
	else
	{
		if (::g_DoDWpnBoxKill_Hook)
			::DoD_WpnBoxKill((::size_t)pEntityBase);
		else
			((::DoD_WpnBoxKill_Type) ::g_pDoDWpnBoxKill_Addr) ((::size_t)pEntityBase);
	}
	return true;
}

::cell DoD_WpnBoxActivateThink_Native(::tagAMX* pAmx, ::cell* pParam)
{
	if (!::g_pDoDWpnBoxActivateThink_Addr)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Signature for ::DoD_WpnBoxActivateThink not found!");
		return false;
	}

	auto Entity = pParam[1];
	if (Entity <= ::gpGlobals->maxClients || Entity > ::gpGlobals->maxEntities)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid weaponbox entity index %d!", Entity);
		return false;
	}

	auto pEntity = ::INDEXENT(Entity);
	if (!pEntity)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Entity %d has no edict!", Entity);
		return false;
	}

	auto pEntityBase = pEntity->pvPrivateData;
	if (!pEntityBase)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Entity %d has no private data!", Entity);
		return false;
	}

	if (pParam[2])
		((::DoD_WpnBoxActivateThink_Type) ::g_pDoDWpnBoxActivateThink_Addr) ((::size_t)pEntityBase);
	else
	{
		if (::g_DoDWpnBoxActivateThink_Hook)
			::DoD_WpnBoxActivateThink((::size_t)pEntityBase);
		else
			((::DoD_WpnBoxActivateThink_Type) ::g_pDoDWpnBoxActivateThink_Addr) ((::size_t)pEntityBase);
	}
	return true;
}

::cell DoD_Create_Native(::tagAMX* pAmx, ::cell* pParam)
{
	auto pItemRes = ::g_fn_GetAmxAddr(pAmx, pParam[5]);
	if (pItemRes)
		*pItemRes = -1;

	if (!::g_pDoDCreate_Addr)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Signature for ::DoD_Create not found!");
		return false;
	}

	int Len;
	auto pName = ::g_fn_GetAmxString(pAmx, pParam[1], false, &Len);
	if (Len < 1 || !pName || false == *pName)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid item name string (null or empty)!");
		return false;
	}

	::size_t Item;
	::cell* pOrigin = ::g_fn_GetAmxAddr(pAmx, pParam[2]);
	::cell* pAngles = ::g_fn_GetAmxAddr(pAmx, pParam[3]);
	::cell Owner = pParam[4];
	::edict_t* pOwner = ((Owner < 0 || Owner > ::gpGlobals->maxEntities) ? NULL : ::INDEXENT(Owner));
	::Vector Origin = !pOrigin ?
		::Vector(false, false, false) :
		::Vector(::g_fn_CellToReal(pOrigin[0]), ::g_fn_CellToReal(pOrigin[1]), ::g_fn_CellToReal(pOrigin[2]));
	::Vector Angles = !pAngles ?
		::Vector(false, false, false) :
		::Vector(::g_fn_CellToReal(pAngles[0]), ::g_fn_CellToReal(pAngles[1]), ::g_fn_CellToReal(pAngles[2]));

	if (pParam[6])
		Item = ((::DoD_Create_Type) ::g_pDoDCreate_Addr) ((char*)STRING(::setupString(pName)), &Origin, &Angles, pOwner);
	else
	{
		if (::g_DoDCreate_Hook)
			Item = ::DoD_Create((char*)STRING(::setupString(pName)), &Origin, &Angles, pOwner);
		else
			Item = ((::DoD_Create_Type) ::g_pDoDCreate_Addr) ((char*)STRING(::setupString(pName)), &Origin, &Angles, pOwner);
	}

	if (pItemRes)
		*pItemRes = ::baseToIndex(Item);
	return true;
}

::cell DoD_GiveNamedItem_Native(::tagAMX* pAmx, ::cell* pParam)
{
	auto pItemRes = ::g_fn_GetAmxAddr(pAmx, pParam[3]);
	if (pItemRes)
		*pItemRes = -1;

	if (!::g_pDoDGiveNamedItem_Addr)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Signature for ::DoD_GiveNamedItem not found!");
		return false;
	}

	auto Player = pParam[1];
	if (Player < 1 || Player > ::gpGlobals->maxClients)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid player index %d!", Player);
		return false;
	}

	int Len;
	auto pItem = ::g_fn_GetAmxString(pAmx, pParam[2], false, &Len);
	if (Len < 1 || !pItem || false == *pItem)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid item name string (null or empty)!");
		return false;
	}

	if (!::g_fn_IsPlayerValid(Player))
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Player %d is invalid!", Player);
		return false;
	}
	if (!::g_fn_IsPlayerIngame(Player))
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Player %d is not in-game!", Player);
		return false;
	}
	if (!::g_fn_IsPlayerAlive(Player))
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Player %d is not alive!", Player);
		return false;
	}

	auto pPlayer = ::g_fn_GetPlayerEdict(Player);
	if (!pPlayer)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Player %d has no edict!", Player);
		return false;
	}

	auto pPlayerBase = pPlayer->pvPrivateData;
	if (!pPlayerBase)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Player %d has no private data!", Player);
		return false;
	}

	::edict_t* pEntity;
	if (false == ::_stricmp("weapon_scopedfg42", pItem))
	{
		if (pParam[4])
			pEntity = ((::DoD_GiveNamedItem_Type) ::g_pDoDGiveNamedItem_Addr) ((::size_t)pPlayerBase, STRING(::setupString("weapon_fg42")));
		else
		{
			if (::g_DoDGiveNamedItem_Hook)
				pEntity = ::DoD_GiveNamedItem((::size_t)pPlayerBase, STRING(::setupString("weapon_fg42")));
			else
				pEntity = ((::DoD_GiveNamedItem_Type) ::g_pDoDGiveNamedItem_Addr) ((::size_t)pPlayerBase, STRING(::setupString("weapon_fg42")));
		}
		if (pEntity)
		{
			auto pItemBase = (::size_t*)pEntity->pvPrivateData;
			if (pItemBase && !(*(::size_t*)((unsigned char*)pItemBase + ::g_Sigs[::SIG_OFFS_ITEMSCOPE].Offs) & 1u))
			{
				*(::size_t*)((unsigned char*)pItemBase + ::g_Sigs[::SIG_OFFS_ITEMSCOPE].Offs) |= 1u;
				(*(void(__thiscall**) (::size_t*)) (*pItemBase + ::g_Sigs[::SIG_OFFS_APPLYITEMSCOPE].Offs)) (pItemBase);
			}
		}
	}
	else if (false == ::_stricmp("weapon_scopedenfield", pItem))
	{
		if (pParam[4])
			pEntity = ((::DoD_GiveNamedItem_Type) ::g_pDoDGiveNamedItem_Addr) ((::size_t)pPlayerBase, STRING(::setupString("weapon_enfield")));
		else
		{
			if (::g_DoDGiveNamedItem_Hook)
				pEntity = ::DoD_GiveNamedItem((::size_t)pPlayerBase, STRING(::setupString("weapon_enfield")));
			else
				pEntity = ((::DoD_GiveNamedItem_Type) ::g_pDoDGiveNamedItem_Addr) ((::size_t)pPlayerBase, STRING(::setupString("weapon_enfield")));
		}
		if (pEntity)
		{
			auto pItemBase = (::size_t*)pEntity->pvPrivateData;
			if (pItemBase && !(*(::size_t*)((unsigned char*)pItemBase + ::g_Sigs[::SIG_OFFS_ITEMSCOPE].Offs) & 1u))
			{
				*(::size_t*)((unsigned char*)pItemBase + ::g_Sigs[::SIG_OFFS_ITEMSCOPE].Offs) |= 1u;
				(*(void(__thiscall**) (::size_t*)) (*pItemBase + ::g_Sigs[::SIG_OFFS_APPLYITEMSCOPE].Offs)) (pItemBase);
			}
		}
	}
	else
	{
		if (pParam[4])
			pEntity = ((::DoD_GiveNamedItem_Type) ::g_pDoDGiveNamedItem_Addr) ((::size_t)pPlayerBase, STRING(::setupString(pItem)));
		else
		{
			if (::g_DoDGiveNamedItem_Hook)
				pEntity = ::DoD_GiveNamedItem((::size_t)pPlayerBase, STRING(::setupString(pItem)));
			else
				pEntity = ((::DoD_GiveNamedItem_Type) ::g_pDoDGiveNamedItem_Addr) ((::size_t)pPlayerBase, STRING(::setupString(pItem)));
		}
	}

	if (pItemRes && pEntity)
		*pItemRes = ::cell(::ENTINDEX(pEntity));
	return true;
}

::cell DoD_DropPlayerItem_Native(::tagAMX* pAmx, ::cell* pParam)
{
	if (!::g_pDoDDropPlayerItem_Addr)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Signature for ::DoD_DropPlayerItem not found!");
		return false;
	}

	auto Player = pParam[1];
	if (Player < 1 || Player > ::gpGlobals->maxClients)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid player index %d!", Player);
		return false;
	}

	int Len;
	auto pItem = ::g_fn_GetAmxString(pAmx, pParam[2], false, &Len);
	if (!pItem)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid item name string (null)!");
		return false;
	}

	if (!::g_fn_IsPlayerValid(Player))
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Player %d is invalid!", Player);
		return false;
	}
	if (!::g_fn_IsPlayerIngame(Player))
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Player %d is not in-game!", Player);
		return false;
	}

	auto pPlayer = ::g_fn_GetPlayerEdict(Player);
	if (!pPlayer)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Player %d has no edict!", Player);
		return false;
	}

	auto pPlayerBase = pPlayer->pvPrivateData;
	if (!pPlayerBase)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Player %d has no private data!", Player);
		return false;
	}

	if (pParam[4])
		((::DoD_DropPlayerItem_Type) ::g_pDoDDropPlayerItem_Addr) ((::size_t)pPlayerBase, pItem, (bool)pParam[3]);
	else
	{
		if (::g_DoDDropPlayerItem_Hook)
			::DoD_DropPlayerItem((::size_t)pPlayerBase, pItem, (bool)pParam[3]);
		else
			((::DoD_DropPlayerItem_Type) ::g_pDoDDropPlayerItem_Addr) ((::size_t)pPlayerBase, pItem, (bool)pParam[3]);
	}
	return true;
}

::cell DoD_SetBodygroup_Native(::tagAMX* pAmx, ::cell* pParam)
{
	if (!::g_pDoDSetBodygroup_Addr)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Signature for ::DoD_SetBodygroup not found!");
		return false;
	}

	auto Entity = pParam[1];
	if (Entity < 0 || Entity > ::gpGlobals->maxEntities)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid entity index %d!", Entity);
		return false;
	}

	auto pEntity = ::INDEXENT(Entity);
	if (!pEntity)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Entity %d has no edict!", Entity);
		return false;
	}

	auto pEntityBase = pEntity->pvPrivateData;
	if (!pEntityBase)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Entity %d has no private data!", Entity);
		return false;
	}

	if (pParam[4])
		((::DoD_SetBodygroup_Type) ::g_pDoDSetBodygroup_Addr) ((::size_t)pEntityBase, pParam[2], pParam[3]);
	else
	{
		if (::g_DoDSetBodygroup_Hook)
			::DoD_SetBodygroup((::size_t)pEntityBase, pParam[2], pParam[3]);
		else
			((::DoD_SetBodygroup_Type) ::g_pDoDSetBodygroup_Addr) ((::size_t)pEntityBase, pParam[2], pParam[3]);
	}
	return true;
}

::cell DoD_GiveAmmo_Native(::tagAMX* pAmx, ::cell* pParam)
{
	auto pAmmoRes = ::g_fn_GetAmxAddr(pAmx, pParam[5]);
	if (pAmmoRes)
		*pAmmoRes = -1;

	if (!::g_pDoDGiveAmmo_Addr)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Signature for ::DoD_GiveAmmo not found!");
		return false;
	}

	auto Player = pParam[1];
	if (Player < 1 || Player > ::gpGlobals->maxClients)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid player index %d!", Player);
		return false;
	}

	int Len;
	auto pName = ::g_fn_GetAmxString(pAmx, pParam[3], false, &Len);
	if (Len < 1 || !pName || false == *pName)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid ammo name string (null or empty)!");
		return false;
	}

	if (!::g_fn_IsPlayerValid(Player))
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Player %d is invalid!", Player);
		return false;
	}
	if (!::g_fn_IsPlayerIngame(Player))
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Player %d is not in-game!", Player);
		return false;
	}
	if (!::g_fn_IsPlayerAlive(Player))
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Player %d is not alive!", Player);
		return false;
	}

	auto pPlayer = ::g_fn_GetPlayerEdict(Player);
	if (!pPlayer)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Player %d has no edict!", Player);
		return false;
	}

	auto pPlayerBase = pPlayer->pvPrivateData;
	if (!pPlayerBase)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Player %d has no private data!", Player);
		return false;
	}

	if (pParam[6])
	{
		if (pAmmoRes)
			*pAmmoRes = ((::DoD_GiveAmmo_Type) ::g_pDoDGiveAmmo_Addr) ((::size_t)pPlayerBase, pParam[2], pName, pParam[4]);
		else
			((::DoD_GiveAmmo_Type) ::g_pDoDGiveAmmo_Addr) ((::size_t)pPlayerBase, pParam[2], pName, pParam[4]);
	}
	else
	{
		if (::g_DoDGiveAmmo_Hook)
		{
			if (pAmmoRes)
				*pAmmoRes = ::DoD_GiveAmmo((::size_t)pPlayerBase, pParam[2], pName, pParam[4]);
			else
				::DoD_GiveAmmo((::size_t)pPlayerBase, pParam[2], pName, pParam[4]);
		}
		else
		{
			if (pAmmoRes)
				*pAmmoRes = ((::DoD_GiveAmmo_Type) ::g_pDoDGiveAmmo_Addr) ((::size_t)pPlayerBase, pParam[2], pName, pParam[4]);
			else
				((::DoD_GiveAmmo_Type) ::g_pDoDGiveAmmo_Addr) ((::size_t)pPlayerBase, pParam[2], pName, pParam[4]);
		}
	}
	return true;
}

::cell DoD_AddHealthIfWounded_Native(::tagAMX* pAmx, ::cell* pParam)
{
	auto pAdded = ::g_fn_GetAmxAddr(pAmx, pParam[3]);
	if (pAdded)
		*pAdded = false;

	auto Player = pParam[1];
	if (Player < 1 || Player > ::gpGlobals->maxClients)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid player index %d!", Player);
		return false;
	}

	if (!::g_fn_IsPlayerValid(Player))
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Player %d is invalid!", Player);
		return false;
	}
	if (!::g_fn_IsPlayerIngame(Player))
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Player %d is not in-game!", Player);
		return false;
	}
	if (!::g_fn_IsPlayerAlive(Player))
		return false;

	auto pPlayer = ::g_fn_GetPlayerEdict(Player);
	if (!pPlayer)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Player %d has no edict!", Player);
		return false;
	}

	if (pPlayer->v.health + (float)pParam[2] >= pPlayer->v.max_health)
	{
		if (pAdded)
			*pAdded = ::cell(pPlayer->v.max_health - pPlayer->v.health);
		pPlayer->v.health = pPlayer->v.max_health;
	}
	else
	{
		pPlayer->v.health += (float)pParam[2];
		if (pAdded)
			*pAdded = pParam[2];
	}
	return true;
}

::cell DoD_IsPlayerFullHealth_Native(::tagAMX* pAmx, ::cell* pParam)
{
	auto Player = pParam[1];
	if (Player < 1 || Player > ::gpGlobals->maxClients)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid player index %d!", Player);
		return false;
	}

	if (!::g_fn_IsPlayerValid(Player))
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Player %d is invalid!", Player);
		return false;
	}
	if (!::g_fn_IsPlayerIngame(Player))
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Player %d is not in-game!", Player);
		return false;
	}
	if (!::g_fn_IsPlayerAlive(Player))
		return false;

	auto pPlayer = ::g_fn_GetPlayerEdict(Player);
	if (!pPlayer)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Player %d has no edict!", Player);
		return false;
	}

	return pPlayer->v.health == pPlayer->v.max_health;
}

::cell DoD_RemovePlayerItem_Native(::tagAMX* pAmx, ::cell* pParam)
{
	auto pRes = ::g_fn_GetAmxAddr(pAmx, pParam[3]);
	if (pRes)
		*pRes = false;

	if (!::g_pDoDRemovePlayerItem_Addr)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Signature for ::DoD_RemovePlayerItem not found!");
		return false;
	}

	auto Player = pParam[1];
	if (Player < 1 || Player > ::gpGlobals->maxClients)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid player index %d!", Player);
		return false;
	}

	if (!::g_fn_IsPlayerValid(Player))
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Player %d is invalid!", Player);
		return false;
	}
	if (!::g_fn_IsPlayerIngame(Player))
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Player %d is not in-game!", Player);
		return false;
	}

	auto pPlayer = ::g_fn_GetPlayerEdict(Player);
	if (!pPlayer)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Player %d has no edict!", Player);
		return false;
	}

	auto pPlayerBase = pPlayer->pvPrivateData;
	if (!pPlayerBase)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Player %d has no private data!", Player);
		return false;
	}

	auto Item = pParam[2];
	if (Item <= ::gpGlobals->maxClients || Item > ::gpGlobals->maxEntities)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid item entity index %d!", Item);
		return false;
	}

	auto pItem = ::INDEXENT(Item);
	if (!pItem)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Item %d has no edict!", Item);
		return false;
	}

	auto pItemBase = pItem->pvPrivateData;
	if (!pItemBase)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Item %d has no private data!", Item);
		return false;
	}

	if (pRes)
	{
		if (pParam[4] || false == ::g_DoDRemovePlayerItem_Hook)
			*pRes = ((::DoD_RemovePlayerItem_Type) ::g_pDoDRemovePlayerItem_Addr) ((::size_t)pPlayerBase, (::size_t)pItemBase);
		else
			*pRes = ::DoD_RemovePlayerItem((::size_t)pPlayerBase, (::size_t)pItemBase);
	}
	else
	{
		if (pParam[4] || false == ::g_DoDRemovePlayerItem_Hook)
			((::DoD_RemovePlayerItem_Type) ::g_pDoDRemovePlayerItem_Addr) ((::size_t)pPlayerBase, (::size_t)pItemBase);
		else
			::DoD_RemovePlayerItem((::size_t)pPlayerBase, (::size_t)pItemBase);
	}
	return true;
}

::cell DoD_AddPlayerItem_Native(::tagAMX* pAmx, ::cell* pParam)
{
	auto pRes = ::g_fn_GetAmxAddr(pAmx, pParam[3]);
	if (pRes)
		*pRes = false;

	if (!::g_pDoDAddPlayerItem_Addr)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Signature for ::DoD_AddPlayerItem not found!");
		return false;
	}

	auto Player = pParam[1];
	if (Player < 1 || Player > ::gpGlobals->maxClients)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid player index %d!", Player);
		return false;
	}

	if (!::g_fn_IsPlayerValid(Player))
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Player %d is invalid!", Player);
		return false;
	}
	if (!::g_fn_IsPlayerIngame(Player))
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Player %d is not in-game!", Player);
		return false;
	}

	auto pPlayer = ::g_fn_GetPlayerEdict(Player);
	if (!pPlayer)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Player %d has no edict!", Player);
		return false;
	}

	auto pPlayerBase = pPlayer->pvPrivateData;
	if (!pPlayerBase)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Player %d has no private data!", Player);
		return false;
	}

	auto Item = pParam[2];
	if (Item <= ::gpGlobals->maxClients || Item > ::gpGlobals->maxEntities)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid item entity index %d!", Item);
		return false;
	}

	auto pItem = ::INDEXENT(Item);
	if (!pItem)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Item %d has no edict!", Item);
		return false;
	}

	auto pItemBase = pItem->pvPrivateData;
	if (!pItemBase)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Item %d has no private data!", Item);
		return false;
	}

	if (pRes)
	{
		if (pParam[4] || false == ::g_DoDAddPlayerItem_Hook)
			*pRes = ((::DoD_AddPlayerItem_Type) ::g_pDoDAddPlayerItem_Addr) ((::size_t)pPlayerBase, (::size_t)pItemBase);
		else
			*pRes = ::DoD_AddPlayerItem((::size_t)pPlayerBase, (::size_t)pItemBase);
	}
	else
	{
		if (pParam[4] || false == ::g_DoDAddPlayerItem_Hook)
			((::DoD_AddPlayerItem_Type) ::g_pDoDAddPlayerItem_Addr) ((::size_t)pPlayerBase, (::size_t)pItemBase);
		else
			::DoD_AddPlayerItem((::size_t)pPlayerBase, (::size_t)pItemBase);
	}
	return true;
}

::cell DoD_HasScope_Native(::tagAMX* pAmx, ::cell* pParam)
{
	auto Item = pParam[1];
	if (Item <= ::gpGlobals->maxClients || Item > ::gpGlobals->maxEntities)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid item entity index %d!", Item);
		return false;
	}

	auto pItem = ::INDEXENT(Item);
	if (!pItem)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Item %d has no edict!", Item);
		return false;
	}

	auto pItemBase = (::size_t*)pItem->pvPrivateData;
	if (!pItemBase)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Item %d has no private data!", Item);
		return false;
	}

	return *(::size_t*)((unsigned char*)pItemBase + ::g_Sigs[::SIG_OFFS_ITEMSCOPE].Offs) & 1u;
}

::cell DoD_AddScope_Native(::tagAMX* pAmx, ::cell* pParam)
{
	auto Item = pParam[1];
	if (Item <= ::gpGlobals->maxClients || Item > ::gpGlobals->maxEntities)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid item entity index %d!", Item);
		return false;
	}

	auto pItem = ::INDEXENT(Item);
	if (!pItem)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Item %d has no edict!", Item);
		return false;
	}

	auto pItemBase = (::size_t*)pItem->pvPrivateData;
	if (!pItemBase)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Item %d has no private data!", Item);
		return false;
	}

	if (!(*(::size_t*)((unsigned char*)pItemBase + ::g_Sigs[::SIG_OFFS_ITEMSCOPE].Offs) & 1u))
	{
		*(::size_t*)((unsigned char*)pItemBase + ::g_Sigs[::SIG_OFFS_ITEMSCOPE].Offs) |= 1u;
		if (pParam[2])
			(*(void(__thiscall**) (::size_t*)) (*pItemBase + ::g_Sigs[::SIG_OFFS_APPLYITEMSCOPE].Offs)) (pItemBase);
	}
	return true;
}

::cell DoD_DeployItem_Native(::tagAMX* pAmx, ::cell* pParam)
{
	auto Item = pParam[1];
	if (Item <= ::gpGlobals->maxClients || Item > ::gpGlobals->maxEntities)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid item entity index %d!", Item);
		return false;
	}

	auto pItem = ::INDEXENT(Item);
	if (!pItem)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Item %d has no edict!", Item);
		return false;
	}

	auto pItemBase = (::size_t*)pItem->pvPrivateData;
	if (!pItemBase)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Item %d has no private data!", Item);
		return false;
	}

	(*(void(__thiscall**) (::size_t*)) (*pItemBase + ::g_Sigs[::SIG_OFFS_APPLYITEMSCOPE].Offs)) (pItemBase);
	return true;
}

::cell DoD_SetEntityThinkFunc_Native(::tagAMX* pAmx, ::cell* pParam)
{
	auto Func = pParam[2];
	if (Func < 1)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid function address %d!", Func);
		return false;
	}

	auto Entity = pParam[1];
	if (Entity < 0 || Entity > ::gpGlobals->maxEntities)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid entity index %d!", Entity);
		return false;
	}

	auto Time = ::g_fn_CellToReal(pParam[7]);
	if (Time < 0.f)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid think time %f!", Time);
		return false;
	}

	auto pEntity = ::INDEXENT(Entity);
	if (!pEntity)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Entity %d has no edict!", Entity);
		return false;
	}

	auto pEntityBase = (::size_t*)pEntity->pvPrivateData;
	if (!pEntityBase)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Entity %d has no private data!", Entity);
		return false;
	}

	*(::size_t*)((unsigned char*)pEntityBase + ::g_Sigs[::SIG_OFFS_THINKFUNC_PFN].Offs) = (::size_t)Func;
	if (pParam[3])
		*(int*)((unsigned char*)pEntityBase + ::g_Sigs[::SIG_OFFS_THINKFUNC_DELTA].Offs) = (int)pParam[4];
	if (pParam[5])
		pEntity->v.nextthink = (pParam[6] ? (::gpGlobals->time + Time) : Time);
	return true;
}

::cell DoD_GetEntityThinkFunc_Native(::tagAMX* pAmx, ::cell* pParam)
{
	auto pDelta = ::g_fn_GetAmxAddr(pAmx, pParam[2]);
	if (pDelta)
		*pDelta = false;

	auto Entity = pParam[1];
	if (Entity < 0 || Entity > ::gpGlobals->maxEntities)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid entity index %d!", Entity);
		return false;
	}

	auto pEntity = ::INDEXENT(Entity);
	if (!pEntity)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Entity %d has no edict!", Entity);
		return false;
	}

	auto pEntityBase = (::size_t*)pEntity->pvPrivateData;
	if (!pEntityBase)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Entity %d has no private data!", Entity);
		return false;
	}

	if (pDelta)
		*pDelta = ::cell(*(int*)((unsigned char*)pEntityBase + ::g_Sigs[::SIG_OFFS_THINKFUNC_DELTA].Offs));
	return ::cell(*(::size_t*)((unsigned char*)pEntityBase + ::g_Sigs[::SIG_OFFS_THINKFUNC_PFN].Offs));
}

::cell DoD_SetWaveTime_Native(::tagAMX* pAmx, ::cell* pParam)
{
	if (!::g_pDoDSetWaveTime_Addr)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Signature for ::DoD_SetWaveTime not found!");
		return false;
	}

	if (false == ::g_CDoDTeamPlay)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "::CDoDTeamPlay (Game Rules) pointer is null at the moment!");
		return false;
	}

	auto Time = ::g_fn_CellToReal(pParam[2]);

	if (pParam[3])
		((::DoD_SetWaveTime_Type) ::g_pDoDSetWaveTime_Addr) (::g_CDoDTeamPlay, pParam[1], Time);
	else
	{
		if (::g_DoDSetWaveTime_Hook)
			::DoD_SetWaveTime(::g_CDoDTeamPlay, pParam[1], Time);
		else
			((::DoD_SetWaveTime_Type) ::g_pDoDSetWaveTime_Addr) (::g_CDoDTeamPlay, pParam[1], Time);
	}
	return true;
}

::cell DoD_GetWaveTime_Native(::tagAMX* pAmx, ::cell* pParam)
{
	if (!::g_pDoDGetWaveTime_Addr)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Signature for ::DoD_GetWaveTime not found!");
		return false;
	}

	if (false == ::g_CDoDTeamPlay)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "::CDoDTeamPlay (Game Rules) pointer is null at the moment!");
		return false;
	}

	float Time;
	if (pParam[2])
		Time = ((::DoD_GetWaveTime_Type) ::g_pDoDGetWaveTime_Addr) (::g_CDoDTeamPlay, pParam[1]);
	else
	{
		if (::g_DoDGetWaveTime_Hook)
			Time = ::DoD_GetWaveTime(::g_CDoDTeamPlay, pParam[1]);
		else
			Time = ((::DoD_GetWaveTime_Type) ::g_pDoDGetWaveTime_Addr) (::g_CDoDTeamPlay, pParam[1]);
	}
	return ::g_fn_RealToCell(Time);
}

::cell DoD_AreAlliesBritish_Native(::tagAMX* pAmx, ::cell* pParam)
{
	if (false == ::g_CDoDTeamPlay)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "::CDoDTeamPlay (Game Rules) pointer is null at the moment!");
		return false;
	}

	auto pAddr = (unsigned char*)(::g_CDoDTeamPlay);
	return *(pAddr + ::g_Sigs[::SIG_OFFS_ALLIESAREBRIT].Offs);
}

::cell DoD_AreAlliesParatroopers_Native(::tagAMX* pAmx, ::cell* pParam)
{
	if (false == ::g_CDoDTeamPlay)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "::CDoDTeamPlay (Game Rules) pointer is null at the moment!");
		return false;
	}

	auto pAddr = (unsigned char*)(::g_CDoDTeamPlay);
	return *(pAddr + ::g_Sigs[::SIG_OFFS_ALLIESAREPARA].Offs);
}

::cell DoD_AreAxisParatroopers_Native(::tagAMX* pAmx, ::cell* pParam)
{
	if (false == ::g_CDoDTeamPlay)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "::CDoDTeamPlay (Game Rules) pointer is null at the moment!");
		return false;
	}

	auto pAddr = (unsigned char*)(::g_CDoDTeamPlay);
	return *(pAddr + ::g_Sigs[::SIG_OFFS_AXISAREPARA].Offs);
}

::cell DoD_HaveAxisInfiniteLives_Native(::tagAMX* pAmx, ::cell* pParam)
{
	if (false == ::g_CDoDTeamPlay)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "::CDoDTeamPlay (Game Rules) pointer is null at the moment!");
		return false;
	}

	auto pAddr = (unsigned char*)(::g_CDoDTeamPlay);
	return *(pAddr + ::g_Sigs[::SIG_OFFS_AXISINFINITELIVES].Offs);
}

::cell DoD_HaveAlliesInfiniteLives_Native(::tagAMX* pAmx, ::cell* pParam)
{
	if (false == ::g_CDoDTeamPlay)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "::CDoDTeamPlay (Game Rules) pointer is null at the moment!");
		return false;
	}

	auto pAddr = (unsigned char*)(::g_CDoDTeamPlay);
	return *(pAddr + ::g_Sigs[::SIG_OFFS_ALLIESINFINITELIVES].Offs);
}

::cell DoD_GetAxisRespawnFactor_Native(::tagAMX* pAmx, ::cell* pParam)
{
	if (false == ::g_CDoDTeamPlay)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "::CDoDTeamPlay (Game Rules) pointer is null at the moment!");
		return false;
	}

	auto pAddr = (float*)(::g_CDoDTeamPlay + ::g_Sigs[::SIG_OFFS_AXISRESPAWNFACTOR].Offs);
	return ::g_fn_RealToCell(*pAddr);
}

::cell DoD_GetAlliesRespawnFactor_Native(::tagAMX* pAmx, ::cell* pParam)
{
	if (false == ::g_CDoDTeamPlay)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "::CDoDTeamPlay (Game Rules) pointer is null at the moment!");
		return false;
	}

	auto pAddr = (float*)(::g_CDoDTeamPlay + ::g_Sigs[::SIG_OFFS_ALLIESRESPAWNFACTOR].Offs);
	return ::g_fn_RealToCell(*pAddr);
}

::cell DoD_ReadGameRulesBool_Native(::tagAMX* pAmx, ::cell* pParam)
{
	if (false == ::g_CDoDTeamPlay)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "::CDoDTeamPlay (Game Rules) pointer is null at the moment!");
		return false;
	}

	auto pAddr = (unsigned char*)(::g_CDoDTeamPlay);
	return *(pAddr + pParam[1]);
}

::cell DoD_ReadGameRulesInt_Native(::tagAMX* pAmx, ::cell* pParam)
{
	if (false == ::g_CDoDTeamPlay)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "::CDoDTeamPlay (Game Rules) pointer is null at the moment!");
		return false;
	}

	auto pAddr = (::size_t*)(::g_CDoDTeamPlay + pParam[1]);
	return (::cell)*pAddr;
}

::cell DoD_ReadGameRulesFloat_Native(::tagAMX* pAmx, ::cell* pParam)
{
	if (false == ::g_CDoDTeamPlay)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "::CDoDTeamPlay (Game Rules) pointer is null at the moment!");
		return false;
	}

	auto pAddr = (float*)(::g_CDoDTeamPlay + pParam[1]);
	return ::g_fn_RealToCell(*pAddr);
}

::cell DoD_ReadGameRulesStr_Native(::tagAMX* pAmx, ::cell* pParam)
{
	if (false == ::g_CDoDTeamPlay)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "::CDoDTeamPlay (Game Rules) pointer is null at the moment!");
		return false;
	}

	::SourceHook::String Buffer;
	auto pAddr = (unsigned char*)(::g_CDoDTeamPlay);
	for (::size_t Iter = false; Iter < (::size_t)pParam[2]; Iter++)
		Buffer.append(*(pAddr + pParam[1] + Iter));

	if (pParam[5])
		return ::g_fn_SetAmxStringUTF8Char(pAmx, pParam[3], Buffer.c_str(), Buffer.size(), pParam[4]);
	return ::g_fn_SetAmxString(pAmx, pParam[3], Buffer.c_str(), pParam[4]);
}

::cell DoD_IsWeaponPrimary_Native(::tagAMX* pAmx, ::cell* pParam)
{
	static const char* Weapons[] =
	{
		"weapon_thompson", "weapon_sten", "weapon_mp40", "weapon_greasegun",
		"weapon_spring", "weapon_scopedkar", "weapon_mp44", "weapon_fg42",
		"weapon_scopedfg42", "weapon_bren", "weapon_bar", "weapon_kar",
		"weapon_garand", "weapon_enfield", "weapon_scopedenfield", "weapon_mg34",
		"weapon_mg42", "weapon_m1carbine", "weapon_k43", "weapon_30cal",
		"weapon_piat", "weapon_bazooka", "weapon_pschreck",

		"thompson", "sten", "mp40", "greasegun",
		"spring", "scopedkar", "mp44", "fg42",
		"scopedfg42", "bren", "bar", "kar",
		"garand", "enfield", "scopedenfield", "mg34",
		"mg42", "m1carbine", "k43", "30cal",
		"piat", "bazooka", "pschreck",
	};

	int Len;
	auto pName = ::g_fn_GetAmxString(pAmx, pParam[1], false, &Len);
	if (Len < 1 || !pName || false == *pName)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid weapon name string (null or empty)!");
		return false;
	}

	for (const auto& Weapon : Weapons)
	{
		if (false == ::_stricmp(Weapon, pName))
			return true;
	}
	return false;
}

::cell DoD_IsWeaponKnife_Native(::tagAMX* pAmx, ::cell* pParam)
{
	static const char* Weapons[] =
	{
		"weapon_spade", "weapon_amerknife", "weapon_gerknife",
		"spade", "amerknife", "gerknife",
	};

	int Len;
	auto pName = ::g_fn_GetAmxString(pAmx, pParam[1], false, &Len);
	if (Len < 1 || !pName || false == *pName)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid weapon name string (null or empty)!");
		return false;
	}

	for (const auto& Weapon : Weapons)
	{
		if (false == ::_stricmp(Weapon, pName))
			return true;
	}
	return false;
}

::cell DoD_IsWeaponSecondary_Native(::tagAMX* pAmx, ::cell* pParam)
{
	static const char* Weapons[] =
	{
		"weapon_webley", "weapon_colt", "weapon_luger",
		"webley", "colt", "luger",
	};

	int Len;
	auto pName = ::g_fn_GetAmxString(pAmx, pParam[1], false, &Len);
	if (Len < 1 || !pName || false == *pName)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid weapon name string (null or empty)!");
		return false;
	}

	for (const auto& Weapon : Weapons)
	{
		if (false == ::_stricmp(Weapon, pName))
			return true;
	}
	return false;
}

::cell DoD_IsWeaponGrenade_Native(::tagAMX* pAmx, ::cell* pParam)
{
	static const char* Weapons[] =
	{
		"weapon_handgrenade", "weapon_handgrenade_ex",
		"weapon_stickgrenade", "weapon_stickgrenade_ex",
		"handgrenade", "handgrenade_ex",
		"stickgrenade", "stickgrenade_ex",
	};

	int Len;
	auto pName = ::g_fn_GetAmxString(pAmx, pParam[1], false, &Len);
	if (Len < 1 || !pName || false == *pName)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid weapon name string (null or empty)!");
		return false;
	}

	for (const auto& Weapon : Weapons)
	{
		if (false == ::_stricmp(Weapon, pName))
			return true;
	}
	return false;
}

::cell DoD_AddKeyValDel_Native(::tagAMX* pAmx, ::cell* pParam)
{
	int Len;
	auto pString = ::g_fn_GetAmxString(pAmx, pParam[1], false, &Len);
	if (!pString)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid map name string (null)!");
		return false;
	}

	::CustomKeyValue_Del keyValData;
	keyValData.Map = pString;

	pString = ::g_fn_GetAmxString(pAmx, pParam[2], false, &Len);
	if (Len < 1 || !pString || false == *pString)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid entity class name string (null or empty)!");
		return false;
	}
	keyValData.Class = pString;

	pString = ::g_fn_GetAmxString(pAmx, pParam[3], false, &Len);
	if (Len < 1 || !pString || false == *pString)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid entity key name string (null or empty)!");
		return false;
	}
	keyValData.Key = pString;

	pString = ::g_fn_GetAmxString(pAmx, pParam[4], false, &Len);
	if (!pString)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid entity value name string (null)!");
		return false;
	}
	keyValData.Value = pString;

	::g_CustomKeyValues_Del.push_back(keyValData);
	return true;
}

::cell DoD_AddKeyValAdd_Native(::tagAMX* pAmx, ::cell* pParam)
{
	int Len;
	auto pString = ::g_fn_GetAmxString(pAmx, pParam[1], false, &Len);
	if (!pString)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid map name string (null)!");
		return false;
	}

	::CustomKeyValue_Add keyValData;
	keyValData.Added = false;
	keyValData.Map = pString;

	pString = ::g_fn_GetAmxString(pAmx, pParam[2], false, &Len);
	if (Len < 1 || !pString || false == *pString)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid entity class name string (null or empty)!");
		return false;
	}
	keyValData.Class = pString;

	pString = ::g_fn_GetAmxString(pAmx, pParam[3], false, &Len);
	if (Len < 1 || !pString || false == *pString)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid entity key name string (null or empty)!");
		return false;
	}
	keyValData.Key = pString;

	pString = ::g_fn_GetAmxString(pAmx, pParam[4], false, &Len);
	if (Len < 1 || !pString || false == *pString)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid entity value name string (null or empty)!");
		return false;
	}
	keyValData.Value = pString;

	::g_CustomKeyValues_Add.push_back(keyValData);
	return true;
}

::cell DoD_DisableAutoScoping_Native(::tagAMX* pAmx, ::cell* pParam)
{
	if (!::g_pAutoScopeFG42Addr || !::g_pAutoScopeEnfieldAddr)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Auto-scoping signature(s) not found!");
		return false;
	}

#ifndef __linux__
	unsigned long origProtection_FG42 = false;
	::VirtualProtect(::g_pAutoScopeFG42Addr, true, PAGE_EXECUTE_READWRITE, &origProtection_FG42);
	*::g_pAutoScopeFG42Addr = ::g_Sigs[::SIG_PATCHFG42_BYTE].Signature[0];
	unsigned long oldProtection_FG42 = false;
	::VirtualProtect(::g_pAutoScopeFG42Addr, true, origProtection_FG42, &oldProtection_FG42);

	unsigned long origProtection_Enfield = false;
	::VirtualProtect(::g_pAutoScopeEnfieldAddr, true, PAGE_EXECUTE_READWRITE, &origProtection_Enfield);
	*::g_pAutoScopeEnfieldAddr = ::g_Sigs[::SIG_PATCHENFIELD_BYTE].Signature[0];
	unsigned long oldProtection_Enfield = false;
	::VirtualProtect(::g_pAutoScopeEnfieldAddr, true, origProtection_Enfield, &oldProtection_Enfield);
#else
	::mprotect(::g_pAutoScopeFG42Addr, true, PROT_READ | PROT_WRITE | PROT_EXEC);
	*::g_pAutoScopeFG42Addr = ::g_Sigs[::SIG_PATCHFG42_BYTE].Signature[0];
	::mprotect(::g_pAutoScopeFG42Addr, true, PROT_READ | PROT_EXEC);

	::mprotect(::g_pAutoScopeEnfieldAddr, true, PROT_READ | PROT_WRITE | PROT_EXEC);
	*::g_pAutoScopeEnfieldAddr = ::g_Sigs[::SIG_PATCHENFIELD_BYTE].Signature[0];
	::mprotect(::g_pAutoScopeEnfieldAddr, true, PROT_READ | PROT_EXEC);
#endif

	return true;
}

::cell DoD_EnableAutoScoping_Native(::tagAMX* pAmx, ::cell* pParam)
{
	if (!::g_pAutoScopeFG42Addr || !::g_pAutoScopeEnfieldAddr)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Auto-scoping signature(s) not found!");
		return false;
	}

#ifndef __linux__
	unsigned long origProtection_FG42 = false;
	::VirtualProtect(::g_pAutoScopeFG42Addr, true, PAGE_EXECUTE_READWRITE, &origProtection_FG42);
	*::g_pAutoScopeFG42Addr = ::g_Sigs[::SIG_ORIGFG42_BYTE].Signature[0];
	unsigned long oldProtection_FG42 = false;
	::VirtualProtect(::g_pAutoScopeFG42Addr, true, origProtection_FG42, &oldProtection_FG42);

	unsigned long origProtection_Enfield = false;
	::VirtualProtect(::g_pAutoScopeEnfieldAddr, true, PAGE_EXECUTE_READWRITE, &origProtection_Enfield);
	*::g_pAutoScopeEnfieldAddr = ::g_Sigs[::SIG_ORIGENFIELD_BYTE].Signature[0];
	unsigned long oldProtection_Enfield = false;
	::VirtualProtect(::g_pAutoScopeEnfieldAddr, true, origProtection_Enfield, &oldProtection_Enfield);
#else
	::mprotect(::g_pAutoScopeFG42Addr, true, PROT_READ | PROT_WRITE | PROT_EXEC);
	*::g_pAutoScopeFG42Addr = ::g_Sigs[::SIG_ORIGFG42_BYTE].Signature[0];
	::mprotect(::g_pAutoScopeFG42Addr, true, PROT_READ | PROT_EXEC);

	::mprotect(::g_pAutoScopeEnfieldAddr, true, PROT_READ | PROT_WRITE | PROT_EXEC);
	*::g_pAutoScopeEnfieldAddr = ::g_Sigs[::SIG_ORIGENFIELD_BYTE].Signature[0];
	::mprotect(::g_pAutoScopeEnfieldAddr, true, PROT_READ | PROT_EXEC);
#endif

	return true;
}

::cell DoD_IsAutoScopingEnabled_Native(::tagAMX* pAmx, ::cell* pParam)
{
	if (!::g_pAutoScopeFG42Addr || !::g_pAutoScopeEnfieldAddr)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Auto-scoping signature(s) not found!");
		return false;
	}

	return *::g_pAutoScopeFG42Addr != ::g_Sigs[::SIG_PATCHFG42_BYTE].Signature[0];
}

::cell DoD_AreGameRulesReady_Native(::tagAMX* pAmx, ::cell* pParam)
{
	return false != ::g_CDoDTeamPlay;
}

::cell DoD_InstallGameRules_Native(::tagAMX* pAmx, ::cell* pParam)
{
	if (!::g_pDoDInstallGameRules_Addr)
	{
		::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Signature for ::DoD_InstallGameRules not found!");
		return false;
	}

	if (pParam[1] || false == ::g_DoDInstallGameRules_Hook)
		return (::cell)((::DoD_InstallGameRules_Type) ::g_pDoDInstallGameRules_Addr) ();
	return (::cell) ::DoD_InstallGameRules();
}

::cell DoD_GetFunctionAddress_Native(::tagAMX* pAmx, ::cell* pParam)
{
	if (true == (bool)pParam[2])
	{
		switch (pParam[1])
		{
		case ::DOD_FA_PLAYERSPAWN: return (::cell) ::g_pDoDPlayerSpawn_Addr;
		case ::DOD_FA_GIVENAMEDITEM: return (::cell) ::g_pDoDGiveNamedItem_Addr;
		case ::DOD_FA_DROPPLAYERITEM: return (::cell) ::g_pDoDDropPlayerItem_Addr;
		case ::DOD_FA_GIVEAMMO: return (::cell) ::g_pDoDGiveAmmo_Addr;
		case ::DOD_FA_SETWAVETIME: return (::cell) ::g_pDoDSetWaveTime_Addr;
		case ::DOD_FA_GETWAVETIME: return (::cell) ::g_pDoDGetWaveTime_Addr;
		case ::DOD_FA_REMOVEPLAYERITEM: return (::cell) ::g_pDoDRemovePlayerItem_Addr;
		case ::DOD_FA_ADDPLAYERITEM: return (::cell) ::g_pDoDAddPlayerItem_Addr;
		case ::DOD_FA_REMOVEALLITEMS: return (::cell) ::g_pDoDRemoveAllItems_Addr;
		case ::DOD_FA_SETBODYGROUP: return (::cell) ::g_pDoDSetBodygroup_Addr;
		case ::DOD_FA_DESTROYITEM: return (::cell) ::g_pDoDDestroyItem_Addr;
		case ::DOD_FA_SUBREMOVE: return (::cell) ::g_pDoDSubRemove_Addr;
		case ::DOD_FA_PACKWEAPON: return (::cell) ::g_pDoDPackWeapon_Addr;
		case ::DOD_FA_INSTALLGAMERULES: return (::cell) ::g_pDoDInstallGameRules_Addr;
		case ::DOD_FA_UTILREMOVE: return (::cell) ::g_pDoDUtilRemove_Addr;
		case ::DOD_FA_CREATE: return (::cell) ::g_pDoDCreate_Addr;
		case ::DOD_FA_WPNBOXKILL: return (::cell) ::g_pDoDWpnBoxKill_Addr;
		case ::DOD_FA_WPNBOXACTIVATETHINK: return (::cell) ::g_pDoDWpnBoxActivateThink_Addr;
		}
	}

	switch (pParam[1])
	{
	case ::DOD_FA_PLAYERSPAWN: return ::cell(::g_DoDPlayerSpawn_Hook ? ::DoD_PlayerSpawn : ::g_pDoDPlayerSpawn_Addr);
	case ::DOD_FA_GIVENAMEDITEM: return ::cell(::g_DoDGiveNamedItem_Hook ? ::DoD_GiveNamedItem : ::g_pDoDGiveNamedItem_Addr);
	case ::DOD_FA_DROPPLAYERITEM: return ::cell(::g_DoDDropPlayerItem_Hook ? ::DoD_DropPlayerItem : ::g_pDoDDropPlayerItem_Addr);
	case ::DOD_FA_GIVEAMMO: return ::cell(::g_DoDGiveAmmo_Hook ? ::DoD_GiveAmmo : ::g_pDoDGiveAmmo_Addr);
	case ::DOD_FA_SETWAVETIME: return ::cell(::g_DoDSetWaveTime_Hook ? ::DoD_SetWaveTime : ::g_pDoDSetWaveTime_Addr);
	case ::DOD_FA_GETWAVETIME: return ::cell(::g_DoDGetWaveTime_Hook ? ::DoD_GetWaveTime : ::g_pDoDGetWaveTime_Addr);
	case ::DOD_FA_REMOVEPLAYERITEM: return ::cell(::g_DoDRemovePlayerItem_Hook ? ::DoD_RemovePlayerItem : ::g_pDoDRemovePlayerItem_Addr);
	case ::DOD_FA_ADDPLAYERITEM: return ::cell(::g_DoDAddPlayerItem_Hook ? ::DoD_AddPlayerItem : ::g_pDoDAddPlayerItem_Addr);
	case ::DOD_FA_REMOVEALLITEMS: return ::cell(::g_DoDRemoveAllItems_Hook ? ::DoD_RemoveAllItems : ::g_pDoDRemoveAllItems_Addr);
	case ::DOD_FA_SETBODYGROUP: return ::cell(::g_DoDSetBodygroup_Hook ? ::DoD_SetBodygroup : ::g_pDoDSetBodygroup_Addr);
	case ::DOD_FA_DESTROYITEM: return ::cell(::g_DoDDestroyItem_Hook ? ::DoD_DestroyItem : ::g_pDoDDestroyItem_Addr);
	case ::DOD_FA_SUBREMOVE: return ::cell(::g_DoDSubRemove_Hook ? ::DoD_SubRemove : ::g_pDoDSubRemove_Addr);
	case ::DOD_FA_PACKWEAPON: return ::cell(::g_DoDPackWeapon_Hook ? ::DoD_PackWeapon : ::g_pDoDPackWeapon_Addr);
	case ::DOD_FA_INSTALLGAMERULES: return ::cell(::g_DoDInstallGameRules_Hook ? ::DoD_InstallGameRules : ::g_pDoDInstallGameRules_Addr);
	case ::DOD_FA_UTILREMOVE: return ::cell(::g_DoDUtilRemove_Hook ? ::DoD_UtilRemove : ::g_pDoDUtilRemove_Addr);
	case ::DOD_FA_CREATE: return ::cell(::g_DoDCreate_Hook ? ::DoD_Create : ::g_pDoDCreate_Addr);
	case ::DOD_FA_WPNBOXKILL: return ::cell(::g_DoDWpnBoxKill_Hook ? ::DoD_WpnBoxKill : ::g_pDoDWpnBoxKill_Addr);
	case ::DOD_FA_WPNBOXACTIVATETHINK: return ::cell(::g_DoDWpnBoxActivateThink_Hook ? ::DoD_WpnBoxActivateThink : ::g_pDoDWpnBoxActivateThink_Addr);
	}

	::MF_LogError(pAmx, ::AMX_ERR_NATIVE, "Invalid function %d!", pParam[1]);
	return false;
}

void __fastcall DoD_PlayerSpawn_Hook(::size_t CDoDTeamPlay, FASTCALL_PARAM::size_t CBasePlayer)
{
	if (false != CDoDTeamPlay)
		::g_CDoDTeamPlay = CDoDTeamPlay;

	if (false == CBasePlayer)
	{
		::DoD_PlayerSpawn(CDoDTeamPlay, CBasePlayer);
		return;
	}

	auto pPlayerVars = *(::entvars_t**)(CBasePlayer + 4);
	if (!pPlayerVars)
	{
		::DoD_PlayerSpawn(CDoDTeamPlay, CBasePlayer);
		return;
	}

	auto pPlayer = pPlayerVars->pContainingEntity;
	if (!pPlayer)
	{
		::DoD_PlayerSpawn(CDoDTeamPlay, CBasePlayer);
		return;
	}

	auto Player = ::ENTINDEX(pPlayer);
	if (::g_fn_ExecuteForward(::g_fwPlayerSpawn, CDoDTeamPlay, &Player))
		return;

	::DoD_PlayerSpawn(CDoDTeamPlay, CBasePlayer);
	::g_fn_ExecuteForward(::g_fwPlayerSpawn_Post, CDoDTeamPlay, Player);
}

void __fastcall DoD_SetBodygroup_Hook(::size_t CBasePlayer, FASTCALL_PARAM int Group, int Value)
{
	if (false == CBasePlayer)
	{
		::DoD_SetBodygroup(CBasePlayer, Group, Value);
		return;
	}

	auto pPlayerVars = *(::entvars_t**)(CBasePlayer + 4);
	if (!pPlayerVars)
	{
		::DoD_SetBodygroup(CBasePlayer, Group, Value);
		return;
	}

	auto pPlayer = pPlayerVars->pContainingEntity;
	if (!pPlayer)
	{
		::DoD_SetBodygroup(CBasePlayer, Group, Value);
		return;
	}

	auto Player = ::ENTINDEX(pPlayer);
	if (::g_fn_ExecuteForward(::g_fwSetBodygroup, Player, &Group, &Value))
		return;

	::DoD_SetBodygroup(CBasePlayer, Group, Value);
	::g_fn_ExecuteForward(::g_fwSetBodygroup_Post, Player, Group, Value);
}

void __fastcall DoD_SubRemove_Hook(::size_t CBaseEntity FASTCALL_PARAM_ALONE)
{
	if (false == CBaseEntity)
	{
		::DoD_SubRemove(CBaseEntity);
		return;
	}

	auto pEntityVars = *(::entvars_t**)(CBaseEntity + 4);
	if (!pEntityVars)
	{
		::DoD_SubRemove(CBaseEntity);
		return;
	}

	auto pEntity = pEntityVars->pContainingEntity;
	if (!pEntity)
	{
		::DoD_SubRemove(CBaseEntity);
		return;
	}

	auto Entity = ::ENTINDEX(pEntity);
	if (::g_fn_ExecuteForward(::g_fwSubRemove, Entity))
		return;

	::DoD_SubRemove(CBaseEntity);
	::g_fn_ExecuteForward(::g_fwSubRemove_Post, Entity);
}

int __fastcall DoD_PackWeapon_Hook(::size_t CWeaponBox, FASTCALL_PARAM::size_t CBasePlayerItem)
{
	if (false == CWeaponBox || false == CBasePlayerItem)
		return ::DoD_PackWeapon(CWeaponBox, CBasePlayerItem);

	auto pEntityVars = *(::entvars_t**)(CWeaponBox + 4);
	if (!pEntityVars)
		return ::DoD_PackWeapon(CWeaponBox, CBasePlayerItem);

	auto pWeaponVars = *(::entvars_t**)(CBasePlayerItem + 4);
	if (!pWeaponVars)
		return ::DoD_PackWeapon(CWeaponBox, CBasePlayerItem);

	auto pEntity = pEntityVars->pContainingEntity;
	if (!pEntity)
		return ::DoD_PackWeapon(CWeaponBox, CBasePlayerItem);

	auto pWeapon = pWeaponVars->pContainingEntity;
	if (!pWeapon)
		return ::DoD_PackWeapon(CWeaponBox, CBasePlayerItem);

	auto Entity = ::ENTINDEX(pEntity), Weapon = ::ENTINDEX(pWeapon);
	if (::g_fn_ExecuteForward(::g_fwPackWeapon, Entity, &Weapon))
		return false;

	auto Res = ::DoD_PackWeapon(CWeaponBox, CBasePlayerItem);
	::g_fn_ExecuteForward(::g_fwPackWeapon_Post, Entity, Weapon, Res);
	return Res;
}

void DoD_UtilRemove_Hook(::size_t CBaseEntity)
{
	if (false == CBaseEntity)
	{
		::DoD_UtilRemove(CBaseEntity);
		return;
	}

	auto pEntityVars = *(::entvars_t**)(CBaseEntity + 4);
	if (!pEntityVars)
	{
		::DoD_UtilRemove(CBaseEntity);
		return;
	}

	auto pEntity = pEntityVars->pContainingEntity;
	if (!pEntity)
	{
		::DoD_UtilRemove(CBaseEntity);
		return;
	}

	auto Entity = ::ENTINDEX(pEntity);
	if (::g_fn_ExecuteForward(::g_fwUtilRemove, &Entity))
		return;

	::DoD_UtilRemove(CBaseEntity);
	::g_fn_ExecuteForward(::g_fwUtilRemove_Post, Entity);
}

void __fastcall DoD_WpnBoxKill_Hook(::size_t CWeaponBox FASTCALL_PARAM_ALONE)
{
	if (false == CWeaponBox)
	{
		::DoD_WpnBoxKill(CWeaponBox);
		return;
	}

	auto pEntityVars = *(::entvars_t**)(CWeaponBox + 4);
	if (!pEntityVars)
	{
		::DoD_WpnBoxKill(CWeaponBox);
		return;
	}

	auto pEntity = pEntityVars->pContainingEntity;
	if (!pEntity)
	{
		::DoD_WpnBoxKill(CWeaponBox);
		return;
	}

	auto Entity = ::ENTINDEX(pEntity);
	if (::g_fn_ExecuteForward(::g_fwWpnBoxKill, Entity))
		return;

	::DoD_WpnBoxKill(CWeaponBox);
	::g_fn_ExecuteForward(::g_fwWpnBoxKill_Post, Entity);
}

void __fastcall DoD_WpnBoxActivateThink_Hook(::size_t CWeaponBox FASTCALL_PARAM_ALONE)
{
	if (false == CWeaponBox)
	{
		::DoD_WpnBoxActivateThink(CWeaponBox);
		return;
	}

	auto pEntityVars = *(::entvars_t**)(CWeaponBox + 4);
	if (!pEntityVars)
	{
		::DoD_WpnBoxActivateThink(CWeaponBox);
		return;
	}

	auto pEntity = pEntityVars->pContainingEntity;
	if (!pEntity)
	{
		::DoD_WpnBoxActivateThink(CWeaponBox);
		return;
	}

	auto Entity = ::ENTINDEX(pEntity);
	if (::g_fn_ExecuteForward(::g_fwWpnBoxActivateThink, Entity))
		return;

	::DoD_WpnBoxActivateThink(CWeaponBox);
	::g_fn_ExecuteForward(::g_fwWpnBoxActivateThink_Post, Entity);
}

::size_t DoD_Create_Hook(char* pName, const ::Vector* pOrigin, const ::Vector* pAngles, ::edict_t* pOwner)
{
	if (!pName)
	{
		static bool Logged = false;
		if (false == Logged)
		{
			Logged = true;
			::MF_Log("*** Warning, ::DoD_Create_Hook called by game with a null item name! Contact the author to enhance this module! ***");
		}
		return ::DoD_Create(pName, pOrigin, pAngles, pOwner);
	}
	if (false == *pName)
	{
		static bool Logged = false;
		if (false == Logged)
		{
			Logged = true;
			::MF_Log("*** Warning, ::DoD_Create_Hook called by game with an empty item name! Contact the author to enhance this module! ***");
		}
		return ::DoD_Create(pName, pOrigin, pAngles, pOwner);
	}
	if (!pOrigin)
	{
		static bool Logged = false;
		if (false == Logged)
		{
			Logged = true;
			::MF_Log("*** Warning, ::DoD_Create_Hook called by game with a null origin parameter! Contact the author to enhance this module! ***");
		}
		return ::DoD_Create(pName, pOrigin, pAngles, pOwner);
	}
	if (!pAngles)
	{
		static bool Logged = false;
		if (false == Logged)
		{
			Logged = true;
			::MF_Log("*** Warning, ::DoD_Create_Hook called by game with a null angles parameter! Contact the author to enhance this module! ***");
		}
		return ::DoD_Create(pName, pOrigin, pAngles, pOwner);
	}

	static char Buffer[64];
#ifndef __linux__
	::strncpy_s(Buffer, sizeof Buffer, pName, _TRUNCATE);
#else
	::snprintf(Buffer, sizeof Buffer, pName);
#endif
	::Vector vecOrigin = *pOrigin;
	static ::cell Origin[3];
	Origin[0] = ::g_fn_RealToCell(vecOrigin.x);
	Origin[1] = ::g_fn_RealToCell(vecOrigin.y);
	Origin[2] = ::g_fn_RealToCell(vecOrigin.z);
	::Vector vecAngles = *pAngles;
	static ::cell Angles[3];
	Angles[0] = ::g_fn_RealToCell(vecAngles.x);
	Angles[1] = ::g_fn_RealToCell(vecAngles.y);
	Angles[2] = ::g_fn_RealToCell(vecAngles.z);
	::cell Owner = ::cell(pOwner ? ::ENTINDEX(pOwner) : -1);
	::cell amxOrigin = ::g_fn_PrepareCellArrayA(Origin, ARRAYSIZE(Origin), true);
	::cell amxAngles = ::g_fn_PrepareCellArrayA(Angles, ARRAYSIZE(Angles), true);
	if (::g_fn_ExecuteForward(::g_fwCreate, Buffer, sizeof Buffer, amxOrigin, amxAngles, &Owner) || false == Buffer[0])
		return false;

	::Vector vecNewOrigin = ::Vector(::g_fn_CellToReal(Origin[0]), ::g_fn_CellToReal(Origin[1]), ::g_fn_CellToReal(Origin[2]));
	::Vector vecNewAngles = ::Vector(::g_fn_CellToReal(Angles[0]), ::g_fn_CellToReal(Angles[1]), ::g_fn_CellToReal(Angles[2]));
	::edict_t* pNewOwner = ((Owner < 0 || Owner > ::gpGlobals->maxEntities) ? NULL : ::INDEXENT(Owner));
	::size_t Item = ::DoD_Create((char*)STRING(::setupString(Buffer)), &vecNewOrigin, &vecNewAngles, pNewOwner);
	static ::cell NewOrigin[3];
	NewOrigin[0] = ::g_fn_RealToCell(vecNewOrigin.x);
	NewOrigin[1] = ::g_fn_RealToCell(vecNewOrigin.y);
	NewOrigin[2] = ::g_fn_RealToCell(vecNewOrigin.z);
	static ::cell NewAngles[3];
	NewAngles[0] = ::g_fn_RealToCell(vecNewAngles.x);
	NewAngles[1] = ::g_fn_RealToCell(vecNewAngles.y);
	NewAngles[2] = ::g_fn_RealToCell(vecNewAngles.z);
	::cell amxNewOrigin = ::g_fn_PrepareCellArrayA(NewOrigin, ARRAYSIZE(NewOrigin), false);
	::cell amxNewAngles = ::g_fn_PrepareCellArrayA(NewAngles, ARRAYSIZE(NewAngles), false);
	::cell ItemIndex = ::baseToIndex(Item);
	::g_fn_ExecuteForward(::g_fwCreate_Post, Buffer, amxNewOrigin, amxNewAngles, Owner, ItemIndex);
	return Item;
}

void __fastcall DoD_DestroyItem_Hook(::size_t CBasePlayerItem FASTCALL_PARAM_ALONE)
{
	if (false == CBasePlayerItem)
	{
		::DoD_DestroyItem(CBasePlayerItem);
		return;
	}

	auto pEntityVars = *(::entvars_t**)(CBasePlayerItem + 4);
	if (!pEntityVars)
	{
		::DoD_DestroyItem(CBasePlayerItem);
		return;
	}

	auto pEntity = pEntityVars->pContainingEntity;
	if (!pEntity)
	{
		::DoD_DestroyItem(CBasePlayerItem);
		return;
	}

	auto Entity = ::ENTINDEX(pEntity);
	if (::g_fn_ExecuteForward(::g_fwDestroyItem, Entity))
		return;

	::DoD_DestroyItem(CBasePlayerItem);
	::g_fn_ExecuteForward(::g_fwDestroyItem_Post, Entity);
}

::edict_t* __fastcall DoD_GiveNamedItem_Hook(::size_t CBasePlayer, FASTCALL_PARAM const char* pItem)
{
	if (false == CBasePlayer)
		return ::DoD_GiveNamedItem(CBasePlayer, pItem);

	if (!pItem)
	{
		static bool Logged = false;
		if (false == Logged)
		{
			Logged = true;
			::MF_Log("*** Warning, ::DoD_GiveNamedItem_Hook called by game with a null item name! Contact the author to enhance this module! ***");
		}
		return ::DoD_GiveNamedItem(CBasePlayer, pItem);
	}
	if (false == *pItem)
	{
		static bool Logged = false;
		if (false == Logged)
		{
			Logged = true;
			::MF_Log("*** Warning, ::DoD_GiveNamedItem_Hook called by game with an empty item name! Contact the author to enhance this module! ***");
		}
		return ::DoD_GiveNamedItem(CBasePlayer, pItem);
	}

	auto pPlayerVars = *(::entvars_t**)(CBasePlayer + 4);
	if (!pPlayerVars)
		return ::DoD_GiveNamedItem(CBasePlayer, pItem);

	auto pPlayer = pPlayerVars->pContainingEntity;
	if (!pPlayer)
		return ::DoD_GiveNamedItem(CBasePlayer, pItem);

	auto Player = ::ENTINDEX(pPlayer);
	static char Buffer[64];
#ifndef __linux__
	::strncpy_s(Buffer, sizeof Buffer, pItem, _TRUNCATE);
#else
	::snprintf(Buffer, sizeof Buffer, pItem);
#endif
	if (::g_fn_ExecuteForward(::g_fwGiveNamedItem, Player, Buffer, sizeof Buffer) || false == Buffer[0])
		return NULL;

	::edict_t* pEntity;
	if (false == ::_stricmp("weapon_scopedfg42", Buffer))
	{
		pEntity = ::DoD_GiveNamedItem(CBasePlayer, STRING(::setupString("weapon_fg42")));
		if (pEntity)
		{
			auto pItemBase = (::size_t*)pEntity->pvPrivateData;
			if (pItemBase && !(*(::size_t*)((unsigned char*)pItemBase + ::g_Sigs[::SIG_OFFS_ITEMSCOPE].Offs) & 1u))
			{
				*(::size_t*)((unsigned char*)pItemBase + ::g_Sigs[::SIG_OFFS_ITEMSCOPE].Offs) |= 1u;
				(*(void(__thiscall**) (::size_t*)) (*pItemBase + ::g_Sigs[::SIG_OFFS_APPLYITEMSCOPE].Offs)) (pItemBase);
			}
		}
	}
	else if (false == ::_stricmp("weapon_scopedenfield", Buffer))
	{
		pEntity = ::DoD_GiveNamedItem(CBasePlayer, STRING(::setupString("weapon_enfield")));
		if (pEntity)
		{
			auto pItemBase = (::size_t*)pEntity->pvPrivateData;
			if (pItemBase && !(*(::size_t*)((unsigned char*)pItemBase + ::g_Sigs[::SIG_OFFS_ITEMSCOPE].Offs) & 1u))
			{
				*(::size_t*)((unsigned char*)pItemBase + ::g_Sigs[::SIG_OFFS_ITEMSCOPE].Offs) |= 1u;
				(*(void(__thiscall**) (::size_t*)) (*pItemBase + ::g_Sigs[::SIG_OFFS_APPLYITEMSCOPE].Offs)) (pItemBase);
			}
		}
	}
	else
		pEntity = ::DoD_GiveNamedItem(CBasePlayer, STRING(::setupString(Buffer)));

	::g_fn_ExecuteForward(::g_fwGiveNamedItem_Post, Player, Buffer, pEntity ? ::ENTINDEX(pEntity) : -1);
	return pEntity;
}

int __fastcall DoD_GiveAmmo_Hook(::size_t CBasePlayer, FASTCALL_PARAM int Ammo, const char* pName, int Max)
{
	if (false == CBasePlayer)
		return ::DoD_GiveAmmo(CBasePlayer, Ammo, pName, Max);

	if (!pName)
	{
		static bool Logged = false;
		if (false == Logged)
		{
			Logged = true;
			::MF_Log("*** Warning, ::DoD_GiveAmmo_Hook called by game with a null ammo name! Contact the author to enhance this module! ***");
		}
		return ::DoD_GiveAmmo(CBasePlayer, Ammo, pName, Max);
	}
	if (false == *pName)
	{
		static bool Logged = false;
		if (false == Logged)
		{
			Logged = true;
			::MF_Log("*** Warning, ::DoD_GiveAmmo_Hook called by game with an empty ammo name! Contact the author to enhance this module! ***");
		}
		return ::DoD_GiveAmmo(CBasePlayer, Ammo, pName, Max);
	}

	auto pPlayerVars = *(::entvars_t**)(CBasePlayer + 4);
	if (!pPlayerVars)
		return ::DoD_GiveAmmo(CBasePlayer, Ammo, pName, Max);

	auto pPlayer = pPlayerVars->pContainingEntity;
	if (!pPlayer)
		return ::DoD_GiveAmmo(CBasePlayer, Ammo, pName, Max);

	auto Player = ::ENTINDEX(pPlayer);
	static char Buffer[64];
#ifndef __linux__
	::strncpy_s(Buffer, sizeof Buffer, pName, _TRUNCATE);
#else
	::snprintf(Buffer, sizeof Buffer, pName);
#endif
	if (::g_fn_ExecuteForward(::g_fwGiveAmmo, Player, &Ammo, Buffer, sizeof Buffer, &Max) || false == Buffer[0])
		return -1;

	auto Res = ::DoD_GiveAmmo(CBasePlayer, Ammo, Buffer, Max);
	::g_fn_ExecuteForward(::g_fwGiveAmmo_Post, Player, Ammo, Buffer, Max, Res);
	return Res;
}

void __fastcall DoD_DropPlayerItem_Hook(::size_t CBasePlayer, FASTCALL_PARAM char* pItem, bool Force)
{
	if (false == CBasePlayer)
	{
		::DoD_DropPlayerItem(CBasePlayer, pItem, Force);
		return;
	}

	if (!pItem)
	{
		static bool Logged = false;
		if (false == Logged)
		{
			Logged = true;
			::MF_Log("*** Warning, ::DoD_DropPlayerItem_Hook called by game with a null item name! Contact the author to enhance this module! ***");
		}
		return ::DoD_DropPlayerItem(CBasePlayer, pItem, Force);
	}

	auto pPlayerVars = *(::entvars_t**)(CBasePlayer + 4);
	if (!pPlayerVars)
	{
		::DoD_DropPlayerItem(CBasePlayer, pItem, Force);
		return;
	}

	auto pPlayer = pPlayerVars->pContainingEntity;
	if (!pPlayer)
	{
		::DoD_DropPlayerItem(CBasePlayer, pItem, Force);
		return;
	}

	auto Player = ::ENTINDEX(pPlayer);
	static char Buffer[64];
#ifndef __linux__
	::strncpy_s(Buffer, sizeof Buffer, pItem, _TRUNCATE);
#else
	::snprintf(Buffer, sizeof Buffer, pItem);
#endif
	if (::g_fn_ExecuteForward(::g_fwDropPlayerItem, Player, Buffer, sizeof Buffer, &Force))
		return;

	::DoD_DropPlayerItem(CBasePlayer, Buffer, Force);
	::g_fn_ExecuteForward(::g_fwDropPlayerItem_Post, Player, Buffer, Force);
}

int __fastcall DoD_RemovePlayerItem_Hook(::size_t CBasePlayer, FASTCALL_PARAM::size_t CBasePlayerItem)
{
	if (false == CBasePlayer || false == CBasePlayerItem)
		return ::DoD_RemovePlayerItem(CBasePlayer, CBasePlayerItem);

	auto pPlayerVars = *(::entvars_t**)(CBasePlayer + 4);
	if (!pPlayerVars)
		return ::DoD_RemovePlayerItem(CBasePlayer, CBasePlayerItem);

	auto pItemVars = *(::entvars_t**)(CBasePlayerItem + 4);
	if (!pItemVars)
		return ::DoD_RemovePlayerItem(CBasePlayer, CBasePlayerItem);

	auto pPlayer = pPlayerVars->pContainingEntity;
	if (!pPlayer)
		return ::DoD_RemovePlayerItem(CBasePlayer, CBasePlayerItem);

	auto pItem = pItemVars->pContainingEntity;
	if (!pItem)
		return ::DoD_RemovePlayerItem(CBasePlayer, CBasePlayerItem);

	auto Player = ::ENTINDEX(pPlayer);
	auto Item = ::ENTINDEX(pItem);
	if (::g_fn_ExecuteForward(::g_fwRemovePlayerItem, Player, &Item))
		return false;

	auto Res = ::DoD_RemovePlayerItem(CBasePlayer, CBasePlayerItem);
	::g_fn_ExecuteForward(::g_fwRemovePlayerItem_Post, Player, Item, Res);
	return Res;
}

int __fastcall DoD_AddPlayerItem_Hook(::size_t CBasePlayer, FASTCALL_PARAM::size_t CBasePlayerItem)
{
	if (false == CBasePlayer || false == CBasePlayerItem)
		return ::DoD_AddPlayerItem(CBasePlayer, CBasePlayerItem);

	auto pPlayerVars = *(::entvars_t**)(CBasePlayer + 4);
	if (!pPlayerVars)
		return ::DoD_AddPlayerItem(CBasePlayer, CBasePlayerItem);

	auto pItemVars = *(::entvars_t**)(CBasePlayerItem + 4);
	if (!pItemVars)
		return ::DoD_AddPlayerItem(CBasePlayer, CBasePlayerItem);

	auto pPlayer = pPlayerVars->pContainingEntity;
	if (!pPlayer)
		return ::DoD_AddPlayerItem(CBasePlayer, CBasePlayerItem);

	auto pItem = pItemVars->pContainingEntity;
	if (!pItem)
		return ::DoD_AddPlayerItem(CBasePlayer, CBasePlayerItem);

	auto Player = ::ENTINDEX(pPlayer);
	auto Item = ::ENTINDEX(pItem);
	if (::g_fn_ExecuteForward(::g_fwAddPlayerItem, Player, &Item))
		return false;

	auto Res = ::DoD_AddPlayerItem(CBasePlayer, CBasePlayerItem);
	::g_fn_ExecuteForward(::g_fwAddPlayerItem_Post, Player, Item, Res);
	return Res;
}

void __fastcall DoD_RemoveAllItems_Hook(::size_t CBasePlayer, FASTCALL_PARAM int RemoveSuit)
{
	if (false == CBasePlayer)
	{
		::DoD_RemoveAllItems(CBasePlayer, RemoveSuit);
		return;
	}

	auto pPlayerVars = *(::entvars_t**)(CBasePlayer + 4);
	if (!pPlayerVars)
	{
		::DoD_RemoveAllItems(CBasePlayer, RemoveSuit);
		return;
	}

	auto pPlayer = pPlayerVars->pContainingEntity;
	if (!pPlayer)
	{
		::DoD_RemoveAllItems(CBasePlayer, RemoveSuit);
		return;
	}

	auto Player = ::ENTINDEX(pPlayer);
	if (::g_fn_ExecuteForward(::g_fwRemoveAllItems, Player, &RemoveSuit))
		return;

	::DoD_RemoveAllItems(CBasePlayer, RemoveSuit);
	::g_fn_ExecuteForward(::g_fwRemoveAllItems_Post, Player, RemoveSuit);
}

float __fastcall DoD_GetWaveTime_Hook(::size_t CDoDTeamPlay, FASTCALL_PARAM int Team)
{
	if (false != CDoDTeamPlay)
		::g_CDoDTeamPlay = CDoDTeamPlay;

	float Time = false;
	if (::g_fn_ExecuteForward(::g_fwGetWaveTime, CDoDTeamPlay, &Team, &Time))
		return Time;

	Time = ::DoD_GetWaveTime(CDoDTeamPlay, Team);
	::g_fn_ExecuteForward(::g_fwGetWaveTime_Post, CDoDTeamPlay, Team, Time);
	return Time;
}

void __fastcall DoD_SetWaveTime_Hook(::size_t CDoDTeamPlay, FASTCALL_PARAM int Team, float Time)
{
	if (false != CDoDTeamPlay)
		::g_CDoDTeamPlay = CDoDTeamPlay;

	if (::g_fn_ExecuteForward(::g_fwSetWaveTime, CDoDTeamPlay, &Team, &Time))
		return;

	::DoD_SetWaveTime(CDoDTeamPlay, Team, Time);
	::g_fn_ExecuteForward(::g_fwSetWaveTime_Post, CDoDTeamPlay, Team, Time);
}

::size_t DoD_InstallGameRules_Hook()
{
	if (::g_fn_ExecuteForward(::g_fwInstallGameRules))
		return false;

	::g_CDoDTeamPlay = ::DoD_InstallGameRules();
	::g_fn_ExecuteForward(::g_fwInstallGameRules_Post, ::g_CDoDTeamPlay);
	return ::g_CDoDTeamPlay;
}

::AMX_NATIVE_INFO DoDHacks_Natives[] =
{
	{ "DoD_PlayerSpawn", ::DoD_PlayerSpawn_Native, },
	{ "DoD_GiveNamedItem", ::DoD_GiveNamedItem_Native, },
	{ "DoD_DropPlayerItem", ::DoD_DropPlayerItem_Native, },
	{ "DoD_RemoveAllItems", ::DoD_RemoveAllItems_Native, },
	{ "DoD_GiveAmmo", ::DoD_GiveAmmo_Native, },
	{ "DoD_AddHealthIfWounded", ::DoD_AddHealthIfWounded_Native, },
	{ "DoD_IsPlayerFullHealth", ::DoD_IsPlayerFullHealth_Native, },
	{ "DoD_InstallGameRules", ::DoD_InstallGameRules_Native, },
	{ "DoD_DeployItem", ::DoD_DeployItem_Native, },
	{ "DoD_SetBodygroup", ::DoD_SetBodygroup_Native, },
	{ "DoD_SubRemove", ::DoD_SubRemove_Native, },
	{ "DoD_UtilRemove", ::DoD_UtilRemove_Native, },
	{ "DoD_DestroyItem", ::DoD_DestroyItem_Native, },
	{ "DoD_PackWeapon", ::DoD_PackWeapon_Native, },
	{ "DoD_WpnBoxKill", ::DoD_WpnBoxKill_Native, },
	{ "DoD_WpnBoxActivateThink", ::DoD_WpnBoxActivateThink_Native, },
	{ "DoD_Create", ::DoD_Create_Native, },
	{ "DoD_GetFunctionAddress", ::DoD_GetFunctionAddress_Native, },
	{ "DoD_SetEntityThinkFunc", ::DoD_SetEntityThinkFunc_Native, },
	{ "DoD_GetEntityThinkFunc", ::DoD_GetEntityThinkFunc_Native, },

	{ "DoD_SetWaveTime", ::DoD_SetWaveTime_Native, },
	{ "DoD_GetWaveTime", ::DoD_GetWaveTime_Native, },

	{ "DoD_IsWeaponPrimary", ::DoD_IsWeaponPrimary_Native, },
	{ "DoD_IsWeaponSecondary", ::DoD_IsWeaponSecondary_Native, },
	{ "DoD_IsWeaponKnife", ::DoD_IsWeaponKnife_Native, },
	{ "DoD_IsWeaponGrenade", ::DoD_IsWeaponGrenade_Native, },

	{ "DoD_DisableAutoScoping", ::DoD_DisableAutoScoping_Native, },
	{ "DoD_EnableAutoScoping", ::DoD_EnableAutoScoping_Native, },
	{ "DoD_IsAutoScopingEnabled", ::DoD_IsAutoScopingEnabled_Native, },

	{ "DoD_HasScope", ::DoD_HasScope_Native, },
	{ "DoD_AddScope", ::DoD_AddScope_Native, },

	{ "DoD_AddKeyValDel", ::DoD_AddKeyValDel_Native, },
	{ "DoD_AddKeyValAdd", ::DoD_AddKeyValAdd_Native, },

	{ "DoD_RemovePlayerItem", ::DoD_RemovePlayerItem_Native, },
	{ "DoD_AddPlayerItem", ::DoD_AddPlayerItem_Native, },

	{ "DoD_AreAlliesBritish", ::DoD_AreAlliesBritish_Native, },
	{ "DoD_AreAlliesParatroopers", ::DoD_AreAlliesParatroopers_Native, },
	{ "DoD_AreAxisParatroopers", ::DoD_AreAxisParatroopers_Native, },
	{ "DoD_HaveAlliesInfiniteLives", ::DoD_HaveAlliesInfiniteLives_Native, },
	{ "DoD_HaveAxisInfiniteLives", ::DoD_HaveAxisInfiniteLives_Native, },
	{ "DoD_GetAlliesRespawnFactor", ::DoD_GetAlliesRespawnFactor_Native, },
	{ "DoD_GetAxisRespawnFactor", ::DoD_GetAxisRespawnFactor_Native, },

	{ "DoD_ReadGameRulesBool", ::DoD_ReadGameRulesBool_Native, },
	{ "DoD_ReadGameRulesFloat", ::DoD_ReadGameRulesFloat_Native, },
	{ "DoD_ReadGameRulesInt", ::DoD_ReadGameRulesInt_Native, },
	{ "DoD_ReadGameRulesStr", ::DoD_ReadGameRulesStr_Native, },

	{ "DoD_AreGameRulesReady", ::DoD_AreGameRulesReady_Native, },

	{ NULL, NULL, },
};

bool ReadConfig(bool ForLinux)
{
	char Buffer[256];
	::g_fn_BuildPathnameR(Buffer, sizeof Buffer, "%s/dod_hacks_signatures.ini", ::g_fn_GetLocalInfo("amxx_configsdir", "addons/amxmodx/configs"));
#ifndef __linux__
	::_iobuf* pConfig;
	::fopen_s(&pConfig, Buffer, "r");
#else
	::FILE* pConfig = ::fopen(Buffer, "r");
#endif
	if (!pConfig)
	{
		::MF_Log("Unable to open '%s'!", Buffer);
		return false;
	}

	::SourceHook::String Line;
	::SignatureData SigData;
	bool Linux;
	while (::fgets(Buffer, sizeof Buffer, pConfig))
	{
		Line = Buffer;
		Line.trim();
		if (Line.empty() || ';' == Line[0] || '/' == Line[0])
		{
			continue;
		}
		else if (false == Line.icmp("[Linux]"))
		{
			Linux = true;
			continue;
		}
		else if (false == Line.icmp("[Windows]"))
		{
			Linux = false;
			continue;
		}
		if (Linux != ForLinux)
		{
			continue;
		}
		if (false == Line.icmpn("Sig:", 4))
		{
			SigData.IsSymbol = false;
			::vectorizeSignature(Line, SigData.Signature);
		}
		else if (false == Line.icmpn("OffsDec:", 8))
		{
			Line.erase(false, 8);
			Line.trim();
			SigData.Offs = ::strtoull(Line.c_str(), NULL, int(10));
		}
		else if (false == Line.icmpn("OffsHex:", 8))
		{
			Line.erase(false, 8);
			Line.trim();
			SigData.Offs = ::strtoull(Line.c_str(), NULL, int(16));
		}
		else
		{
			SigData.IsSymbol = true;
			SigData.Symbol = Line;
		}
		::g_Sigs.push_back(SigData);
	}
	::fclose(pConfig);
	return true;
}

void OnAmxxAttach()
{
#ifndef __linux__
	if (!::ReadConfig(false))
	{
		::g_fn_AddNatives(::DoDHacks_Natives);
		return;
	}

	auto pDoD = ::GetModuleHandleA("dod.dll");
	if (!pDoD)
	{
		::MF_Log("::GetModuleHandleA failed!");
		::g_fn_AddNatives(::DoDHacks_Natives);
		return;
	}

	::_MEMORY_BASIC_INFORMATION memInfo{ };
	if (false == ::VirtualQuery(pDoD, &memInfo, sizeof memInfo) || !memInfo.AllocationBase)
	{
		::MF_Log("::VirtualQuery failed!");
		::g_fn_AddNatives(::DoDHacks_Natives);
		return;
	}

	::size_t Addr = false;
	auto pImgDosHdr = (::_IMAGE_DOS_HEADER*)memInfo.AllocationBase;
	auto pImgNtHdr = (::_IMAGE_NT_HEADERS*)((::size_t)pImgDosHdr + (::size_t)pImgDosHdr->e_lfanew);

	if (::g_Sigs[::SIG_PLAYERSPAWN].IsSymbol == false)
	{
		if (::findInMemory((unsigned char*)memInfo.AllocationBase, pImgNtHdr->OptionalHeader.SizeOfImage, ::g_Sigs[::SIG_PLAYERSPAWN].Signature, &Addr, true))
			::g_pDoDPlayerSpawn_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_PlayerSpawn signature not found!");
	}
	else if (!(::g_pDoDPlayerSpawn_Addr = (void*) ::GetProcAddress(pDoD, ::g_Sigs[::SIG_PLAYERSPAWN].Symbol.c_str())))
		::MF_Log("::DoD_PlayerSpawn symbol not found!");

	if (::g_Sigs[::SIG_GIVENAMEDITEM].IsSymbol == false)
	{
		if (::findInMemory((unsigned char*)memInfo.AllocationBase, pImgNtHdr->OptionalHeader.SizeOfImage, ::g_Sigs[::SIG_GIVENAMEDITEM].Signature, &Addr, true))
			::g_pDoDGiveNamedItem_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_GiveNamedItem signature not found!");
	}
	else if (!(::g_pDoDGiveNamedItem_Addr = (void*) ::GetProcAddress(pDoD, ::g_Sigs[::SIG_GIVENAMEDITEM].Symbol.c_str())))
		::MF_Log("::DoD_GiveNamedItem symbol not found!");

	if (::g_Sigs[::SIG_DROPPLAYERITEM].IsSymbol == false)
	{
		if (::findInMemory((unsigned char*)memInfo.AllocationBase, pImgNtHdr->OptionalHeader.SizeOfImage, ::g_Sigs[::SIG_DROPPLAYERITEM].Signature, &Addr, true))
			::g_pDoDDropPlayerItem_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_DropPlayerItem signature not found!");
	}
	else if (!(::g_pDoDDropPlayerItem_Addr = (void*) ::GetProcAddress(pDoD, ::g_Sigs[::SIG_DROPPLAYERITEM].Symbol.c_str())))
		::MF_Log("::DoD_DropPlayerItem symbol not found!");

	if (::g_Sigs[::SIG_GIVEAMMO].IsSymbol == false)
	{
		if (::findInMemory((unsigned char*)memInfo.AllocationBase, pImgNtHdr->OptionalHeader.SizeOfImage, ::g_Sigs[::SIG_GIVEAMMO].Signature, &Addr, true))
			::g_pDoDGiveAmmo_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_GiveAmmo signature not found!");
	}
	else if (!(::g_pDoDGiveAmmo_Addr = (void*) ::GetProcAddress(pDoD, ::g_Sigs[::SIG_GIVEAMMO].Symbol.c_str())))
		::MF_Log("::DoD_GiveAmmo symbol not found!");

	if (::g_Sigs[::SIG_UTILREMOVE].IsSymbol == false)
	{
		if (::findInMemory((unsigned char*)memInfo.AllocationBase, pImgNtHdr->OptionalHeader.SizeOfImage, ::g_Sigs[::SIG_UTILREMOVE].Signature, &Addr, true))
			::g_pDoDUtilRemove_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_UtilRemove signature not found!");
	}
	else if (!(::g_pDoDUtilRemove_Addr = (void*) ::GetProcAddress(pDoD, ::g_Sigs[::SIG_UTILREMOVE].Symbol.c_str())))
		::MF_Log("::DoD_UtilRemove symbol not found!");

	if (::g_Sigs[::SIG_SUBREMOVE].IsSymbol == false)
	{
		if (::findInMemory((unsigned char*)memInfo.AllocationBase, pImgNtHdr->OptionalHeader.SizeOfImage, ::g_Sigs[::SIG_SUBREMOVE].Signature, &Addr, true))
			::g_pDoDSubRemove_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_SubRemove signature not found!");
	}
	else if (!(::g_pDoDSubRemove_Addr = (void*) ::GetProcAddress(pDoD, ::g_Sigs[::SIG_SUBREMOVE].Symbol.c_str())))
		::MF_Log("::DoD_SubRemove symbol not found!");

	if (::g_Sigs[::SIG_WPNBOXKILL].IsSymbol == false)
	{
		if (::findInMemory((unsigned char*)memInfo.AllocationBase, pImgNtHdr->OptionalHeader.SizeOfImage, ::g_Sigs[::SIG_WPNBOXKILL].Signature, &Addr, true))
			::g_pDoDWpnBoxKill_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_WpnBoxKill signature not found!");
	}
	else if (!(::g_pDoDWpnBoxKill_Addr = (void*) ::GetProcAddress(pDoD, ::g_Sigs[::SIG_WPNBOXKILL].Symbol.c_str())))
		::MF_Log("::DoD_WpnBoxKill symbol not found!");

	if (::g_Sigs[::SIG_WPNBOXACTIVATETHINK].IsSymbol == false)
	{
		if (::findInMemory((unsigned char*)memInfo.AllocationBase, pImgNtHdr->OptionalHeader.SizeOfImage, ::g_Sigs[::SIG_WPNBOXACTIVATETHINK].Signature, &Addr, true))
			::g_pDoDWpnBoxActivateThink_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_WpnBoxActivateThink signature not found!");
	}
	else if (!(::g_pDoDWpnBoxActivateThink_Addr = (void*) ::GetProcAddress(pDoD, ::g_Sigs[::SIG_WPNBOXACTIVATETHINK].Symbol.c_str())))
		::MF_Log("::DoD_WpnBoxActivateThink symbol not found!");

	if (::g_Sigs[::SIG_CREATE].IsSymbol == false)
	{
		if (::findInMemory((unsigned char*)memInfo.AllocationBase, pImgNtHdr->OptionalHeader.SizeOfImage, ::g_Sigs[::SIG_CREATE].Signature, &Addr, true))
			::g_pDoDCreate_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_Create signature not found!");
	}
	else if (!(::g_pDoDCreate_Addr = (void*) ::GetProcAddress(pDoD, ::g_Sigs[::SIG_CREATE].Symbol.c_str())))
		::MF_Log("::DoD_Create symbol not found!");

	if (::g_Sigs[::SIG_PACKWEAPON].IsSymbol == false)
	{
		if (::findInMemory((unsigned char*)memInfo.AllocationBase, pImgNtHdr->OptionalHeader.SizeOfImage, ::g_Sigs[::SIG_PACKWEAPON].Signature, &Addr, true))
			::g_pDoDPackWeapon_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_PackWeapon signature not found!");
	}
	else if (!(::g_pDoDPackWeapon_Addr = (void*) ::GetProcAddress(pDoD, ::g_Sigs[::SIG_PACKWEAPON].Symbol.c_str())))
		::MF_Log("::DoD_PackWeapon symbol not found!");

	if (::g_Sigs[::SIG_DESTROYITEM].IsSymbol == false)
	{
		if (::findInMemory((unsigned char*)memInfo.AllocationBase, pImgNtHdr->OptionalHeader.SizeOfImage, ::g_Sigs[::SIG_DESTROYITEM].Signature, &Addr, true))
			::g_pDoDDestroyItem_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_DestroyItem signature not found!");
	}
	else if (!(::g_pDoDDestroyItem_Addr = (void*) ::GetProcAddress(pDoD, ::g_Sigs[::SIG_DESTROYITEM].Symbol.c_str())))
		::MF_Log("::DoD_DestroyItem symbol not found!");

	if (::g_Sigs[::SIG_SETWAVETIME].IsSymbol == false)
	{
		if (::findInMemory((unsigned char*)memInfo.AllocationBase, pImgNtHdr->OptionalHeader.SizeOfImage, ::g_Sigs[::SIG_SETWAVETIME].Signature, &Addr, true))
			::g_pDoDSetWaveTime_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_SetWaveTime signature not found!");
	}
	else if (!(::g_pDoDSetWaveTime_Addr = (void*) ::GetProcAddress(pDoD, ::g_Sigs[::SIG_SETWAVETIME].Symbol.c_str())))
		::MF_Log("::DoD_SetWaveTime symbol not found!");

	if (::g_Sigs[::SIG_GETWAVETIME].IsSymbol == false)
	{
		if (::findInMemory((unsigned char*)memInfo.AllocationBase, pImgNtHdr->OptionalHeader.SizeOfImage, ::g_Sigs[::SIG_GETWAVETIME].Signature, &Addr, true))
			::g_pDoDGetWaveTime_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_GetWaveTime signature not found!");
	}
	else if (!(::g_pDoDGetWaveTime_Addr = (void*) ::GetProcAddress(pDoD, ::g_Sigs[::SIG_GETWAVETIME].Symbol.c_str())))
		::MF_Log("::DoD_GetWaveTime symbol not found!");

	if (::g_Sigs[::SIG_REMOVEPLAYERITEM].IsSymbol == false)
	{
		if (::findInMemory((unsigned char*)memInfo.AllocationBase, pImgNtHdr->OptionalHeader.SizeOfImage, ::g_Sigs[::SIG_REMOVEPLAYERITEM].Signature, &Addr, true))
			::g_pDoDRemovePlayerItem_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_RemovePlayerItem signature not found!");
	}
	else if (!(::g_pDoDRemovePlayerItem_Addr = (void*) ::GetProcAddress(pDoD, ::g_Sigs[::SIG_REMOVEPLAYERITEM].Symbol.c_str())))
		::MF_Log("::DoD_RemovePlayerItem symbol not found!");

	if (::g_Sigs[::SIG_ADDPLAYERITEM].IsSymbol == false)
	{
		if (::findInMemory((unsigned char*)memInfo.AllocationBase, pImgNtHdr->OptionalHeader.SizeOfImage, ::g_Sigs[::SIG_ADDPLAYERITEM].Signature, &Addr, true))
			::g_pDoDAddPlayerItem_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_AddPlayerItem signature not found!");
	}
	else if (!(::g_pDoDAddPlayerItem_Addr = (void*) ::GetProcAddress(pDoD, ::g_Sigs[::SIG_ADDPLAYERITEM].Symbol.c_str())))
		::MF_Log("::DoD_AddPlayerItem symbol not found!");

	if (::g_Sigs[::SIG_REMOVEALLITEMS].IsSymbol == false)
	{
		if (::findInMemory((unsigned char*)memInfo.AllocationBase, pImgNtHdr->OptionalHeader.SizeOfImage, ::g_Sigs[::SIG_REMOVEALLITEMS].Signature, &Addr, true))
			::g_pDoDRemoveAllItems_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_RemoveAllItems signature not found!");
	}
	else if (!(::g_pDoDRemoveAllItems_Addr = (void*) ::GetProcAddress(pDoD, ::g_Sigs[::SIG_REMOVEALLITEMS].Symbol.c_str())))
		::MF_Log("::DoD_RemoveAllItems symbol not found!");

	if (::g_Sigs[::SIG_SETBODYGROUP].IsSymbol == false)
	{
		if (::findInMemory((unsigned char*)memInfo.AllocationBase, pImgNtHdr->OptionalHeader.SizeOfImage, ::g_Sigs[::SIG_SETBODYGROUP].Signature, &Addr, true))
			::g_pDoDSetBodygroup_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_SetBodygroup signature not found!");
	}
	else if (!(::g_pDoDSetBodygroup_Addr = (void*) ::GetProcAddress(pDoD, ::g_Sigs[::SIG_SETBODYGROUP].Symbol.c_str())))
		::MF_Log("::DoD_SetBodygroup symbol not found!");

	if (::g_Sigs[::SIG_INSTALLGAMERULES].IsSymbol == false)
	{
		if (::findInMemory((unsigned char*)memInfo.AllocationBase, pImgNtHdr->OptionalHeader.SizeOfImage, ::g_Sigs[::SIG_INSTALLGAMERULES].Signature, &Addr, true))
			::g_pDoDInstallGameRules_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_InstallGameRules signature not found!");
	}
	else if (!(::g_pDoDInstallGameRules_Addr = (void*) ::GetProcAddress(pDoD, ::g_Sigs[::SIG_INSTALLGAMERULES].Symbol.c_str())))
		::MF_Log("::DoD_InstallGameRules symbol not found!");

	if (::findInMemory((unsigned char*)memInfo.AllocationBase, pImgNtHdr->OptionalHeader.SizeOfImage, ::g_Sigs[::SIG_PATCHFG42].Signature, &Addr, true))
		::g_pAutoScopeFG42Addr = (unsigned char*)Addr;
	else
		::MF_Log("::DoD_PatchAutoScope(FG42) signature not found!");

	if (::findInMemory((unsigned char*)memInfo.AllocationBase, pImgNtHdr->OptionalHeader.SizeOfImage, ::g_Sigs[::SIG_PATCHENFIELD].Signature, &Addr, true))
		::g_pAutoScopeEnfieldAddr = (unsigned char*)Addr;
	else
		::MF_Log("::DoD_PatchAutoScope(Enfield) signature not found!");

	if (::g_pDoDInstallGameRules_Addr)
	{
		::DoD_InstallGameRules = (::DoD_InstallGameRules_Type) ::g_pDoDInstallGameRules_Addr;
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourAttach(&(void*&) ::DoD_InstallGameRules, ::DoD_InstallGameRules_Hook);
		::DetourTransactionCommit();
		::g_DoDInstallGameRules_Hook = true;
	}
#else
	if (!::ReadConfig(true))
	{
		::g_fn_AddNatives(::DoDHacks_Natives);
		return;
	}

	char Buffer[256];
	::g_fn_BuildPathnameR(Buffer, sizeof Buffer, "dlls/dod.so");
	void* pDoD = ::dlopen(Buffer, RTLD_LAZY | RTLD_NODELETE | RTLD_NOLOAD);
	if (!pDoD)
	{
		pDoD = ::dlopen(Buffer, RTLD_NOW);
		if (!pDoD)
		{
			::MF_Log("::dlopen failed!");
			::g_fn_AddNatives(::DoDHacks_Natives);
			return;
		}
	}

	void* pSym = ::dlsym(pDoD, "GiveFnptrsToDll");
	if (!pSym)
	{
		::MF_Log("::dlsym (::GiveFnptrsToDll) failed!");
		::dlclose(pDoD);
		::g_fn_AddNatives(::DoDHacks_Natives);
		return;
	}

	::Dl_info memInfo;
	if (!::dladdr(pSym, &memInfo))
	{
		::MF_Log("::dladdr failed!");
		::dlclose(pDoD);
		::g_fn_AddNatives(::DoDHacks_Natives);
		return;
	}

	struct ::stat memData;
	if (::stat(memInfo.dli_fname, &memData))
	{
		::MF_Log("::stat failed!");
		::dlclose(pDoD);
		::g_fn_AddNatives(::DoDHacks_Natives);
		return;
	}

	::size_t Addr = false;

	if (::g_Sigs[::SIG_PLAYERSPAWN].IsSymbol)
	{
		if (!(::g_pDoDPlayerSpawn_Addr = ::dlsym(pDoD, ::g_Sigs[::SIG_PLAYERSPAWN].Symbol.c_str())))
			::MF_Log("::DoD_PlayerSpawn symbol not found!");
	}
	else
	{
		if (::findInMemory((unsigned char*)memInfo.dli_fbase, memData.st_size, ::g_Sigs[::SIG_PLAYERSPAWN].Signature, &Addr, true))
			::g_pDoDPlayerSpawn_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_PlayerSpawn signature not found!");
	}

	if (::g_Sigs[::SIG_GIVENAMEDITEM].IsSymbol)
	{
		if (!(::g_pDoDGiveNamedItem_Addr = ::dlsym(pDoD, ::g_Sigs[::SIG_GIVENAMEDITEM].Symbol.c_str())))
			::MF_Log("::DoD_GiveNamedItem symbol not found!");
	}
	else
	{
		if (::findInMemory((unsigned char*)memInfo.dli_fbase, memData.st_size, ::g_Sigs[::SIG_GIVENAMEDITEM].Signature, &Addr, true))
			::g_pDoDGiveNamedItem_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_GiveNamedItem signature not found!");
	}

	if (::g_Sigs[::SIG_SUBREMOVE].IsSymbol)
	{
		if (!(::g_pDoDSubRemove_Addr = ::dlsym(pDoD, ::g_Sigs[::SIG_SUBREMOVE].Symbol.c_str())))
			::MF_Log("::DoD_SubRemove symbol not found!");
	}
	else
	{
		if (::findInMemory((unsigned char*)memInfo.dli_fbase, memData.st_size, ::g_Sigs[::SIG_SUBREMOVE].Signature, &Addr, true))
			::g_pDoDSubRemove_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_SubRemove signature not found!");
	}

	if (::g_Sigs[::SIG_WPNBOXKILL].IsSymbol)
	{
		if (!(::g_pDoDWpnBoxKill_Addr = ::dlsym(pDoD, ::g_Sigs[::SIG_WPNBOXKILL].Symbol.c_str())))
			::MF_Log("::DoD_WpnBoxKill symbol not found!");
	}
	else
	{
		if (::findInMemory((unsigned char*)memInfo.dli_fbase, memData.st_size, ::g_Sigs[::SIG_WPNBOXKILL].Signature, &Addr, true))
			::g_pDoDWpnBoxKill_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_WpnBoxKill signature not found!");
	}

	if (::g_Sigs[::SIG_WPNBOXACTIVATETHINK].IsSymbol)
	{
		if (!(::g_pDoDWpnBoxActivateThink_Addr = ::dlsym(pDoD, ::g_Sigs[::SIG_WPNBOXACTIVATETHINK].Symbol.c_str())))
			::MF_Log("::DoD_WpnBoxActivateThink symbol not found!");
	}
	else
	{
		if (::findInMemory((unsigned char*)memInfo.dli_fbase, memData.st_size, ::g_Sigs[::SIG_WPNBOXACTIVATETHINK].Signature, &Addr, true))
			::g_pDoDWpnBoxActivateThink_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_WpnBoxActivateThink signature not found!");
	}

	if (::g_Sigs[::SIG_CREATE].IsSymbol)
	{
		if (!(::g_pDoDCreate_Addr = ::dlsym(pDoD, ::g_Sigs[::SIG_CREATE].Symbol.c_str())))
			::MF_Log("::DoD_Create symbol not found!");
	}
	else
	{
		if (::findInMemory((unsigned char*)memInfo.dli_fbase, memData.st_size, ::g_Sigs[::SIG_CREATE].Signature, &Addr, true))
			::g_pDoDCreate_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_Create signature not found!");
	}

	if (::g_Sigs[::SIG_PACKWEAPON].IsSymbol)
	{
		if (!(::g_pDoDPackWeapon_Addr = ::dlsym(pDoD, ::g_Sigs[::SIG_PACKWEAPON].Symbol.c_str())))
			::MF_Log("::DoD_PackWeapon symbol not found!");
	}
	else
	{
		if (::findInMemory((unsigned char*)memInfo.dli_fbase, memData.st_size, ::g_Sigs[::SIG_PACKWEAPON].Signature, &Addr, true))
			::g_pDoDPackWeapon_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_PackWeapon signature not found!");
	}

	if (::g_Sigs[::SIG_UTILREMOVE].IsSymbol)
	{
		if (!(::g_pDoDUtilRemove_Addr = ::dlsym(pDoD, ::g_Sigs[::SIG_UTILREMOVE].Symbol.c_str())))
			::MF_Log("::DoD_UtilRemove symbol not found!");
	}
	else
	{
		if (::findInMemory((unsigned char*)memInfo.dli_fbase, memData.st_size, ::g_Sigs[::SIG_UTILREMOVE].Signature, &Addr, true))
			::g_pDoDUtilRemove_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_UtilRemove signature not found!");
	}

	if (::g_Sigs[::SIG_DESTROYITEM].IsSymbol)
	{
		if (!(::g_pDoDDestroyItem_Addr = ::dlsym(pDoD, ::g_Sigs[::SIG_DESTROYITEM].Symbol.c_str())))
			::MF_Log("::DoD_DestroyItem symbol not found!");
	}
	else
	{
		if (::findInMemory((unsigned char*)memInfo.dli_fbase, memData.st_size, ::g_Sigs[::SIG_DESTROYITEM].Signature, &Addr, true))
			::g_pDoDDestroyItem_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_DestroyItem signature not found!");
	}

	if (::g_Sigs[::SIG_DROPPLAYERITEM].IsSymbol)
	{
		if (!(::g_pDoDDropPlayerItem_Addr = ::dlsym(pDoD, ::g_Sigs[::SIG_DROPPLAYERITEM].Symbol.c_str())))
			::MF_Log("::DoD_DropPlayerItem symbol not found!");
	}
	else
	{
		if (::findInMemory((unsigned char*)memInfo.dli_fbase, memData.st_size, ::g_Sigs[::SIG_DROPPLAYERITEM].Signature, &Addr, true))
			::g_pDoDDropPlayerItem_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_DropPlayerItem signature not found!");
	}

	if (::g_Sigs[::SIG_GIVEAMMO].IsSymbol)
	{
		if (!(::g_pDoDGiveAmmo_Addr = ::dlsym(pDoD, ::g_Sigs[::SIG_GIVEAMMO].Symbol.c_str())))
			::MF_Log("::DoD_GiveAmmo symbol not found!");
	}
	else
	{
		if (::findInMemory((unsigned char*)memInfo.dli_fbase, memData.st_size, ::g_Sigs[::SIG_GIVEAMMO].Signature, &Addr, true))
			::g_pDoDGiveAmmo_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_GiveAmmo signature not found!");
	}

	if (::g_Sigs[::SIG_SETWAVETIME].IsSymbol)
	{
		if (!(::g_pDoDSetWaveTime_Addr = ::dlsym(pDoD, ::g_Sigs[::SIG_SETWAVETIME].Symbol.c_str())))
			::MF_Log("::DoD_SetWaveTime symbol not found!");
	}
	else
	{
		if (::findInMemory((unsigned char*)memInfo.dli_fbase, memData.st_size, ::g_Sigs[::SIG_SETWAVETIME].Signature, &Addr, true))
			::g_pDoDSetWaveTime_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_SetWaveTime signature not found!");
	}

	if (::g_Sigs[::SIG_GETWAVETIME].IsSymbol)
	{
		if (!(::g_pDoDGetWaveTime_Addr = ::dlsym(pDoD, ::g_Sigs[::SIG_GETWAVETIME].Symbol.c_str())))
			::MF_Log("::DoD_GetWaveTime symbol not found!");
	}
	else
	{
		if (::findInMemory((unsigned char*)memInfo.dli_fbase, memData.st_size, ::g_Sigs[::SIG_GETWAVETIME].Signature, &Addr, true))
			::g_pDoDGetWaveTime_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_GetWaveTime signature not found!");
	}

	if (::g_Sigs[::SIG_REMOVEPLAYERITEM].IsSymbol)
	{
		if (!(::g_pDoDRemovePlayerItem_Addr = ::dlsym(pDoD, ::g_Sigs[::SIG_REMOVEPLAYERITEM].Symbol.c_str())))
			::MF_Log("::DoD_RemovePlayerItem symbol not found!");
	}
	else
	{
		if (::findInMemory((unsigned char*)memInfo.dli_fbase, memData.st_size, ::g_Sigs[::SIG_REMOVEPLAYERITEM].Signature, &Addr, true))
			::g_pDoDRemovePlayerItem_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_RemovePlayerItem signature not found!");
	}

	if (::g_Sigs[::SIG_ADDPLAYERITEM].IsSymbol)
	{
		if (!(::g_pDoDAddPlayerItem_Addr = ::dlsym(pDoD, ::g_Sigs[::SIG_ADDPLAYERITEM].Symbol.c_str())))
			::MF_Log("::DoD_AddPlayerItem symbol not found!");
	}
	else
	{
		if (::findInMemory((unsigned char*)memInfo.dli_fbase, memData.st_size, ::g_Sigs[::SIG_ADDPLAYERITEM].Signature, &Addr, true))
			::g_pDoDAddPlayerItem_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_AddPlayerItem signature not found!");
	}

	if (::g_Sigs[::SIG_REMOVEALLITEMS].IsSymbol)
	{
		if (!(::g_pDoDRemoveAllItems_Addr = ::dlsym(pDoD, ::g_Sigs[::SIG_REMOVEALLITEMS].Symbol.c_str())))
			::MF_Log("::DoD_RemoveAllItems symbol not found!");
	}
	else
	{
		if (::findInMemory((unsigned char*)memInfo.dli_fbase, memData.st_size, ::g_Sigs[::SIG_REMOVEALLITEMS].Signature, &Addr, true))
			::g_pDoDRemoveAllItems_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_RemoveAllItems signature not found!");
	}

	if (::g_Sigs[::SIG_SETBODYGROUP].IsSymbol)
	{
		if (!(::g_pDoDSetBodygroup_Addr = ::dlsym(pDoD, ::g_Sigs[::SIG_SETBODYGROUP].Symbol.c_str())))
			::MF_Log("::DoD_SetBodygroup symbol not found!");
	}
	else
	{
		if (::findInMemory((unsigned char*)memInfo.dli_fbase, memData.st_size, ::g_Sigs[::SIG_SETBODYGROUP].Signature, &Addr, true))
			::g_pDoDSetBodygroup_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_SetBodygroup signature not found!");
	}

	if (::g_Sigs[::SIG_INSTALLGAMERULES].IsSymbol)
	{
		if (!(::g_pDoDInstallGameRules_Addr = ::dlsym(pDoD, ::g_Sigs[::SIG_INSTALLGAMERULES].Symbol.c_str())))
			::MF_Log("::DoD_InstallGameRules symbol not found!");
	}
	else
	{
		if (::findInMemory((unsigned char*)memInfo.dli_fbase, memData.st_size, ::g_Sigs[::SIG_INSTALLGAMERULES].Signature, &Addr, true))
			::g_pDoDInstallGameRules_Addr = (void*)Addr;
		else
			::MF_Log("::DoD_InstallGameRules signature not found!");
	}

	if (::findInMemory((unsigned char*)memInfo.dli_fbase, memData.st_size, ::g_Sigs[::SIG_PATCHFG42].Signature, &Addr, true))
	{
		::g_pAutoScopeFG42Addr = (unsigned char*)Addr;
		::mprotect(::g_pAutoScopeFG42Addr, true, PROT_READ | PROT_EXEC); /// Just in case, but it should already be RX (not X).
	}
	else
		::MF_Log("::DoD_PatchAutoScope(FG42) signature not found!");

	if (::findInMemory((unsigned char*)memInfo.dli_fbase, memData.st_size, ::g_Sigs[::SIG_PATCHENFIELD].Signature, &Addr, true))
	{
		::g_pAutoScopeEnfieldAddr = (unsigned char*)Addr;
		::mprotect(::g_pAutoScopeEnfieldAddr, true, PROT_READ | PROT_EXEC); /// Just in case, but it should already be RX (not X).
	}
	else
		::MF_Log("::DoD_PatchAutoScope(Enfield) signature not found!");

	if (::g_pDoDInstallGameRules_Addr)
	{
		::g_pDoDInstallGameRules = ::subhook_new(::g_pDoDInstallGameRules_Addr, ::DoD_InstallGameRules_Hook, ::SUBHOOK_TRAMPOLINE);
		::subhook_install(::g_pDoDInstallGameRules);
		::DoD_InstallGameRules = (::DoD_InstallGameRules_Type) ::subhook_get_trampoline(::g_pDoDInstallGameRules);
		::g_DoDInstallGameRules_Hook = true;
	}

	::dlclose(pDoD);
#endif

	::g_fn_AddNatives(::DoDHacks_Natives);
}

void OnAmxxDetach()
{
#ifndef __linux__
	if (::g_DoDPlayerSpawn_Hook)
	{
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourDetach(&(void*&) ::DoD_PlayerSpawn, ::DoD_PlayerSpawn_Hook);
		::DetourTransactionCommit();
		::g_DoDPlayerSpawn_Hook = false;
	}
	if (::g_DoDGiveNamedItem_Hook)
	{
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourDetach(&(void*&) ::DoD_GiveNamedItem, ::DoD_GiveNamedItem_Hook);
		::DetourTransactionCommit();
		::g_DoDGiveNamedItem_Hook = false;
	}
	if (::g_DoDDropPlayerItem_Hook)
	{
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourDetach(&(void*&) ::DoD_DropPlayerItem, ::DoD_DropPlayerItem_Hook);
		::DetourTransactionCommit();
		::g_DoDDropPlayerItem_Hook = false;
	}
	if (::g_DoDRemovePlayerItem_Hook)
	{
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourDetach(&(void*&) ::DoD_RemovePlayerItem, ::DoD_RemovePlayerItem_Hook);
		::DetourTransactionCommit();
		::g_DoDRemovePlayerItem_Hook = false;
	}
	if (::g_DoDAddPlayerItem_Hook)
	{
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourDetach(&(void*&) ::DoD_AddPlayerItem, ::DoD_AddPlayerItem_Hook);
		::DetourTransactionCommit();
		::g_DoDAddPlayerItem_Hook = false;
	}
	if (::g_DoDRemoveAllItems_Hook)
	{
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourDetach(&(void*&) ::DoD_RemoveAllItems, ::DoD_RemoveAllItems_Hook);
		::DetourTransactionCommit();
		::g_DoDRemoveAllItems_Hook = false;
	}
	if (::g_DoDSetWaveTime_Hook)
	{
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourDetach(&(void*&) ::DoD_SetWaveTime, ::DoD_SetWaveTime_Hook);
		::DetourTransactionCommit();
		::g_DoDSetWaveTime_Hook = false;
	}
	if (::g_DoDGetWaveTime_Hook)
	{
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourDetach(&(void*&) ::DoD_GetWaveTime, ::DoD_GetWaveTime_Hook);
		::DetourTransactionCommit();
		::g_DoDGetWaveTime_Hook = false;
	}
	if (::g_DoDGiveAmmo_Hook)
	{
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourDetach(&(void*&) ::DoD_GiveAmmo, ::DoD_GiveAmmo_Hook);
		::DetourTransactionCommit();
		::g_DoDGiveAmmo_Hook = false;
	}
	if (::g_DoDSetBodygroup_Hook)
	{
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourDetach(&(void*&) ::DoD_SetBodygroup, ::DoD_SetBodygroup_Hook);
		::DetourTransactionCommit();
		::g_DoDSetBodygroup_Hook = false;
	}
	if (::g_DoDInstallGameRules_Hook)
	{
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourDetach(&(void*&) ::DoD_InstallGameRules, ::DoD_InstallGameRules_Hook);
		::DetourTransactionCommit();
		::g_DoDInstallGameRules_Hook = false;
	}
	if (::g_DoDUtilRemove_Hook)
	{
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourDetach(&(void*&) ::DoD_UtilRemove, ::DoD_UtilRemove_Hook);
		::DetourTransactionCommit();
		::g_DoDUtilRemove_Hook = false;
	}
	if (::g_DoDSubRemove_Hook)
	{
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourDetach(&(void*&) ::DoD_SubRemove, ::DoD_SubRemove_Hook);
		::DetourTransactionCommit();
		::g_DoDSubRemove_Hook = false;
	}
	if (::g_DoDWpnBoxKill_Hook)
	{
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourDetach(&(void*&) ::DoD_WpnBoxKill, ::DoD_WpnBoxKill_Hook);
		::DetourTransactionCommit();
		::g_DoDWpnBoxKill_Hook = false;
	}
	if (::g_DoDWpnBoxActivateThink_Hook)
	{
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourDetach(&(void*&) ::DoD_WpnBoxActivateThink, ::DoD_WpnBoxActivateThink_Hook);
		::DetourTransactionCommit();
		::g_DoDWpnBoxActivateThink_Hook = false;
	}
	if (::g_DoDCreate_Hook)
	{
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourDetach(&(void*&) ::DoD_Create, ::DoD_Create_Hook);
		::DetourTransactionCommit();
		::g_DoDCreate_Hook = false;
	}
	if (::g_DoDPackWeapon_Hook)
	{
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourDetach(&(void*&) ::DoD_PackWeapon, ::DoD_PackWeapon_Hook);
		::DetourTransactionCommit();
		::g_DoDPackWeapon_Hook = false;
	}
	if (::g_DoDDestroyItem_Hook)
	{
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourDetach(&(void*&) ::DoD_DestroyItem, ::DoD_DestroyItem_Hook);
		::DetourTransactionCommit();
		::g_DoDDestroyItem_Hook = false;
	}
	if (::g_pAutoScopeFG42Addr)
	{
		unsigned long origProtection_FG42 = false;
		::VirtualProtect(::g_pAutoScopeFG42Addr, true, PAGE_EXECUTE_READWRITE, &origProtection_FG42);
		*::g_pAutoScopeFG42Addr = ::g_Sigs[::SIG_ORIGFG42_BYTE].Signature[0];
		unsigned long oldProtection_FG42 = false;
		::VirtualProtect(::g_pAutoScopeFG42Addr, true, origProtection_FG42, &oldProtection_FG42);
	}
	if (::g_pAutoScopeEnfieldAddr)
	{
		unsigned long origProtection_Enfield = false;
		::VirtualProtect(::g_pAutoScopeEnfieldAddr, true, PAGE_EXECUTE_READWRITE, &origProtection_Enfield);
		*::g_pAutoScopeEnfieldAddr = ::g_Sigs[::SIG_ORIGENFIELD_BYTE].Signature[0];
		unsigned long oldProtection_Enfield = false;
		::VirtualProtect(::g_pAutoScopeEnfieldAddr, true, origProtection_Enfield, &oldProtection_Enfield);
	}
#else
	if (::g_DoDPlayerSpawn_Hook)
	{
		::subhook_remove(::g_pDoDPlayerSpawn);
		::subhook_free(::g_pDoDPlayerSpawn);
		::g_DoDPlayerSpawn_Hook = false;
	}
	if (::g_DoDGiveNamedItem_Hook)
	{
		::subhook_remove(::g_pDoDGiveNamedItem);
		::subhook_free(::g_pDoDGiveNamedItem);
		::g_DoDGiveNamedItem_Hook = false;
	}
	if (::g_DoDDropPlayerItem_Hook)
	{
		::subhook_remove(::g_pDoDDropPlayerItem);
		::subhook_free(::g_pDoDDropPlayerItem);
		::g_DoDDropPlayerItem_Hook = false;
	}
	if (::g_DoDRemovePlayerItem_Hook)
	{
		::subhook_remove(::g_pDoDRemovePlayerItem);
		::subhook_free(::g_pDoDRemovePlayerItem);
		::g_DoDRemovePlayerItem_Hook = false;
	}
	if (::g_DoDAddPlayerItem_Hook)
	{
		::subhook_remove(::g_pDoDAddPlayerItem);
		::subhook_free(::g_pDoDAddPlayerItem);
		::g_DoDAddPlayerItem_Hook = false;
	}
	if (::g_DoDRemoveAllItems_Hook)
	{
		::subhook_remove(::g_pDoDRemoveAllItems);
		::subhook_free(::g_pDoDRemoveAllItems);
		::g_DoDRemoveAllItems_Hook = false;
	}
	if (::g_DoDSetWaveTime_Hook)
	{
		::subhook_remove(::g_pDoDSetWaveTime);
		::subhook_free(::g_pDoDSetWaveTime);
		::g_DoDSetWaveTime_Hook = false;
	}
	if (::g_DoDGetWaveTime_Hook)
	{
		::subhook_remove(::g_pDoDGetWaveTime);
		::subhook_free(::g_pDoDGetWaveTime);
		::g_DoDGetWaveTime_Hook = false;
	}
	if (::g_DoDGiveAmmo_Hook)
	{
		::subhook_remove(::g_pDoDGiveAmmo);
		::subhook_free(::g_pDoDGiveAmmo);
		::g_DoDGiveAmmo_Hook = false;
	}
	if (::g_DoDSetBodygroup_Hook)
	{
		::subhook_remove(::g_pDoDSetBodygroup);
		::subhook_free(::g_pDoDSetBodygroup);
		::g_DoDSetBodygroup_Hook = false;
	}
	if (::g_DoDInstallGameRules_Hook)
	{
		::subhook_remove(::g_pDoDInstallGameRules);
		::subhook_free(::g_pDoDInstallGameRules);
		::g_DoDInstallGameRules_Hook = false;
	}
	if (::g_DoDDestroyItem_Hook)
	{
		::subhook_remove(::g_pDoDDestroyItem);
		::subhook_free(::g_pDoDDestroyItem);
		::g_DoDDestroyItem_Hook = false;
	}
	if (::g_DoDSubRemove_Hook)
	{
		::subhook_remove(::g_pDoDSubRemove);
		::subhook_free(::g_pDoDSubRemove);
		::g_DoDSubRemove_Hook = false;
	}
	if (::g_DoDWpnBoxKill_Hook)
	{
		::subhook_remove(::g_pDoDWpnBoxKill);
		::subhook_free(::g_pDoDWpnBoxKill);
		::g_DoDWpnBoxKill_Hook = false;
	}
	if (::g_DoDWpnBoxActivateThink_Hook)
	{
		::subhook_remove(::g_pDoDWpnBoxActivateThink);
		::subhook_free(::g_pDoDWpnBoxActivateThink);
		::g_DoDWpnBoxActivateThink_Hook = false;
	}
	if (::g_DoDCreate_Hook)
	{
		::subhook_remove(::g_pDoDCreate);
		::subhook_free(::g_pDoDCreate);
		::g_DoDCreate_Hook = false;
	}
	if (::g_DoDPackWeapon_Hook)
	{
		::subhook_remove(::g_pDoDPackWeapon);
		::subhook_free(::g_pDoDPackWeapon);
		::g_DoDPackWeapon_Hook = false;
	}
	if (::g_DoDUtilRemove_Hook)
	{
		::subhook_remove(::g_pDoDUtilRemove);
		::subhook_free(::g_pDoDUtilRemove);
		::g_DoDUtilRemove_Hook = false;
	}
	if (::g_pAutoScopeFG42Addr)
	{
		::mprotect(::g_pAutoScopeFG42Addr, true, PROT_READ | PROT_WRITE | PROT_EXEC);
		*::g_pAutoScopeFG42Addr = ::g_Sigs[::SIG_ORIGFG42_BYTE].Signature[0];
		::mprotect(::g_pAutoScopeFG42Addr, true, PROT_READ | PROT_EXEC);
	}
	if (::g_pAutoScopeEnfieldAddr)
	{
		::mprotect(::g_pAutoScopeEnfieldAddr, true, PROT_READ | PROT_WRITE | PROT_EXEC);
		*::g_pAutoScopeEnfieldAddr = ::g_Sigs[::SIG_ORIGENFIELD_BYTE].Signature[0];
		::mprotect(::g_pAutoScopeEnfieldAddr, true, PROT_READ | PROT_EXEC);
	}
#endif

	::g_pAutoScopeFG42Addr = NULL;
	::g_pAutoScopeEnfieldAddr = NULL;

	::g_pDoDPlayerSpawn_Addr = NULL;
	::g_pDoDGiveNamedItem_Addr = NULL;
	::g_pDoDDropPlayerItem_Addr = NULL;
	::g_pDoDGiveAmmo_Addr = NULL;
	::g_pDoDSetWaveTime_Addr = NULL;
	::g_pDoDGetWaveTime_Addr = NULL;
	::g_pDoDRemovePlayerItem_Addr = NULL;
	::g_pDoDAddPlayerItem_Addr = NULL;
	::g_pDoDRemoveAllItems_Addr = NULL;
	::g_pDoDSetBodygroup_Addr = NULL;
	::g_pDoDDestroyItem_Addr = NULL;
	::g_pDoDSubRemove_Addr = NULL;
	::g_pDoDPackWeapon_Addr = NULL;
	::g_pDoDInstallGameRules_Addr = NULL;
	::g_pDoDUtilRemove_Addr = NULL;
	::g_pDoDCreate_Addr = NULL;
	::g_pDoDWpnBoxActivateThink_Addr = NULL;
	::g_pDoDWpnBoxKill_Addr = NULL;

	::g_Sigs.clear();
}

void OnPluginsLoaded()
{
	::g_fwPlayerSpawn = ::g_fn_RegisterForward("DoD_OnPlayerSpawn", ::ET_STOP, ::FP_CELL, ::FP_CELL_BYREF /** can be altered during exec */, ::FP_DONE);
	::g_fwGiveNamedItem = ::g_fn_RegisterForward("DoD_OnGiveNamedItem", ::ET_STOP, ::FP_CELL, ::FP_STRINGEX /** can be altered during exec */, ::FP_CELL, ::FP_DONE);
	::g_fwDropPlayerItem = ::g_fn_RegisterForward("DoD_OnDropPlayerItem", ::ET_STOP, ::FP_CELL, ::FP_STRINGEX /** can be altered during exec */, ::FP_CELL, ::FP_CELL_BYREF /** can be altered during exec */, ::FP_DONE);
	::g_fwAddPlayerItem = ::g_fn_RegisterForward("DoD_OnAddPlayerItem", ::ET_STOP, ::FP_CELL, ::FP_CELL_BYREF /** can be altered during exec */, ::FP_DONE);
	::g_fwRemovePlayerItem = ::g_fn_RegisterForward("DoD_OnRemovePlayerItem", ::ET_STOP, ::FP_CELL, ::FP_CELL_BYREF /** can be altered during exec */, ::FP_DONE);
	::g_fwRemoveAllItems = ::g_fn_RegisterForward("DoD_OnRemoveAllItems", ::ET_STOP, ::FP_CELL, ::FP_CELL_BYREF /** can be altered during exec */, ::FP_DONE);
	::g_fwGiveAmmo = ::g_fn_RegisterForward("DoD_OnGiveAmmo", ::ET_STOP, ::FP_CELL, ::FP_CELL_BYREF /** can be altered during exec */, ::FP_STRINGEX /** can be altered during exec */, ::FP_CELL_BYREF /** can be altered during exec */, ::FP_DONE);
	::g_fwGetWaveTime = ::g_fn_RegisterForward("DoD_OnGetWaveTime", ::ET_STOP, ::FP_CELL, ::FP_CELL_BYREF /** can be altered during exec */, ::FP_FLOAT_BYREF /** can be altered during exec */, ::FP_DONE);
	::g_fwSetWaveTime = ::g_fn_RegisterForward("DoD_OnSetWaveTime", ::ET_STOP, ::FP_CELL, ::FP_CELL_BYREF /** can be altered during exec */, ::FP_FLOAT_BYREF /** can be altered during exec */, ::FP_DONE);
	::g_fwSetBodygroup = ::g_fn_RegisterForward("DoD_OnSetBodygroup", ::ET_STOP, ::FP_CELL, ::FP_CELL_BYREF /** can be altered during exec */, ::FP_CELL_BYREF /** can be altered during exec */, ::FP_DONE);
	::g_fwInstallGameRules = ::g_fn_RegisterForward("DoD_OnInstallGameRules", ::ET_STOP, ::FP_DONE);
	::g_fwSubRemove = ::g_fn_RegisterForward("DoD_OnSubRemove", ::ET_STOP, ::FP_CELL, ::FP_DONE);
	::g_fwUtilRemove = ::g_fn_RegisterForward("DoD_OnUtilRemove", ::ET_STOP, ::FP_CELL_BYREF /** can be altered during exec */, ::FP_DONE);
	::g_fwDestroyItem = ::g_fn_RegisterForward("DoD_OnDestroyItem", ::ET_STOP, ::FP_CELL, ::FP_DONE);
	::g_fwPackWeapon = ::g_fn_RegisterForward("DoD_OnPackWeapon", ::ET_STOP, ::FP_CELL, ::FP_CELL_BYREF /** can be altered during exec */, ::FP_DONE);
	::g_fwWpnBoxKill = ::g_fn_RegisterForward("DoD_OnWpnBoxKill", ::ET_STOP, ::FP_CELL, ::FP_DONE);
	::g_fwWpnBoxActivateThink = ::g_fn_RegisterForward("DoD_OnWpnBoxActivateThink", ::ET_STOP, ::FP_CELL, ::FP_DONE);
	::g_fwCreate = ::g_fn_RegisterForward("DoD_OnCreate", ::ET_STOP, ::FP_STRINGEX /** can be altered during exec */, ::FP_CELL, ::FP_ARRAY, ::FP_ARRAY, ::FP_CELL_BYREF /** can be altered during exec */, ::FP_DONE);

	::g_fwPlayerSpawn_Post = ::g_fn_RegisterForward("DoD_OnPlayerSpawn_Post", ::ET_IGNORE, ::FP_CELL, ::FP_CELL, ::FP_DONE);
	::g_fwGiveNamedItem_Post = ::g_fn_RegisterForward("DoD_OnGiveNamedItem_Post", ::ET_IGNORE, ::FP_CELL, ::FP_STRING, ::FP_CELL, ::FP_DONE);
	::g_fwDropPlayerItem_Post = ::g_fn_RegisterForward("DoD_OnDropPlayerItem_Post", ::ET_IGNORE, ::FP_CELL, ::FP_STRING, ::FP_CELL, ::FP_DONE);
	::g_fwAddPlayerItem_Post = ::g_fn_RegisterForward("DoD_OnAddPlayerItem_Post", ::ET_IGNORE, ::FP_CELL, ::FP_CELL, ::FP_CELL, ::FP_DONE);
	::g_fwRemovePlayerItem_Post = ::g_fn_RegisterForward("DoD_OnRemovePlayerItem_Post", ::ET_IGNORE, ::FP_CELL, ::FP_CELL, ::FP_CELL, ::FP_DONE);
	::g_fwRemoveAllItems_Post = ::g_fn_RegisterForward("DoD_OnRemoveAllItems_Post", ::ET_IGNORE, ::FP_CELL, ::FP_CELL, ::FP_DONE);
	::g_fwGiveAmmo_Post = ::g_fn_RegisterForward("DoD_OnGiveAmmo_Post", ::ET_IGNORE, ::FP_CELL, ::FP_CELL, ::FP_STRING, ::FP_CELL, ::FP_CELL, ::FP_DONE);
	::g_fwGetWaveTime_Post = ::g_fn_RegisterForward("DoD_OnGetWaveTime_Post", ::ET_IGNORE, ::FP_CELL, ::FP_CELL, ::FP_FLOAT, ::FP_DONE);
	::g_fwSetWaveTime_Post = ::g_fn_RegisterForward("DoD_OnSetWaveTime_Post", ::ET_IGNORE, ::FP_CELL, ::FP_CELL, ::FP_FLOAT, ::FP_DONE);
	::g_fwSetBodygroup_Post = ::g_fn_RegisterForward("DoD_OnSetBodygroup_Post", ::ET_IGNORE, ::FP_CELL, ::FP_CELL, ::FP_CELL, ::FP_DONE);
	::g_fwInstallGameRules_Post = ::g_fn_RegisterForward("DoD_OnInstallGameRules_Post", ::ET_IGNORE, ::FP_CELL, ::FP_DONE);
	::g_fwSubRemove_Post = ::g_fn_RegisterForward("DoD_OnSubRemove_Post", ::ET_IGNORE, ::FP_CELL, ::FP_DONE);
	::g_fwUtilRemove_Post = ::g_fn_RegisterForward("DoD_OnUtilRemove_Post", ::ET_IGNORE, ::FP_CELL, ::FP_DONE);
	::g_fwDestroyItem_Post = ::g_fn_RegisterForward("DoD_OnDestroyItem_Post", ::ET_IGNORE, ::FP_CELL, ::FP_DONE);
	::g_fwPackWeapon_Post = ::g_fn_RegisterForward("DoD_OnPackWeapon_Post", ::ET_IGNORE, ::FP_CELL, ::FP_CELL, ::FP_CELL, ::FP_DONE);
	::g_fwWpnBoxKill_Post = ::g_fn_RegisterForward("DoD_OnWpnBoxKill_Post", ::ET_IGNORE, ::FP_CELL, ::FP_DONE);
	::g_fwWpnBoxActivateThink_Post = ::g_fn_RegisterForward("DoD_OnWpnBoxActivateThink_Post", ::ET_IGNORE, ::FP_CELL, ::FP_DONE);
	::g_fwCreate_Post = ::g_fn_RegisterForward("DoD_OnCreate_Post", ::ET_IGNORE, ::FP_STRING, ::FP_ARRAY, ::FP_ARRAY, ::FP_CELL, ::FP_CELL, ::FP_DONE);

	::tagAMX* pAmx;
	int Func, Iter = false;
	while (pAmx = ::g_fn_GetAmxScript(Iter++))
	{
#ifndef __linux__
		if (::g_pDoDPlayerSpawn_Addr && false == ::g_DoDPlayerSpawn_Hook &&
			(::g_fn_AmxFindPublic(pAmx, "DoD_OnPlayerSpawn", &Func) == ::AMX_ERR_NONE ||
				::g_fn_AmxFindPublic(pAmx, "DoD_OnPlayerSpawn_Post", &Func) == ::AMX_ERR_NONE))
		{
			::DoD_PlayerSpawn = (::DoD_PlayerSpawn_Type) ::g_pDoDPlayerSpawn_Addr;
			::DetourTransactionBegin();
			::DetourUpdateThread(::GetCurrentThread());
			::DetourAttach(&(void*&) ::DoD_PlayerSpawn, ::DoD_PlayerSpawn_Hook);
			::DetourTransactionCommit();
			::g_DoDPlayerSpawn_Hook = true;
		}

		if (::g_pDoDGiveNamedItem_Addr && false == ::g_DoDGiveNamedItem_Hook &&
			(::g_fn_AmxFindPublic(pAmx, "DoD_OnGiveNamedItem", &Func) == ::AMX_ERR_NONE ||
				::g_fn_AmxFindPublic(pAmx, "DoD_OnGiveNamedItem_Post", &Func) == ::AMX_ERR_NONE))
		{
			::DoD_GiveNamedItem = (::DoD_GiveNamedItem_Type) ::g_pDoDGiveNamedItem_Addr;
			::DetourTransactionBegin();
			::DetourUpdateThread(::GetCurrentThread());
			::DetourAttach(&(void*&) ::DoD_GiveNamedItem, ::DoD_GiveNamedItem_Hook);
			::DetourTransactionCommit();
			::g_DoDGiveNamedItem_Hook = true;
		}

		if (::g_pDoDDropPlayerItem_Addr && false == ::g_DoDDropPlayerItem_Hook &&
			(::g_fn_AmxFindPublic(pAmx, "DoD_OnDropPlayerItem", &Func) == ::AMX_ERR_NONE ||
				::g_fn_AmxFindPublic(pAmx, "DoD_OnDropPlayerItem_Post", &Func) == ::AMX_ERR_NONE))
		{
			::DoD_DropPlayerItem = (::DoD_DropPlayerItem_Type) ::g_pDoDDropPlayerItem_Addr;
			::DetourTransactionBegin();
			::DetourUpdateThread(::GetCurrentThread());
			::DetourAttach(&(void*&) ::DoD_DropPlayerItem, ::DoD_DropPlayerItem_Hook);
			::DetourTransactionCommit();
			::g_DoDDropPlayerItem_Hook = true;
		}

		if (::g_pDoDAddPlayerItem_Addr && false == ::g_DoDAddPlayerItem_Hook &&
			(::g_fn_AmxFindPublic(pAmx, "DoD_OnAddPlayerItem", &Func) == ::AMX_ERR_NONE ||
				::g_fn_AmxFindPublic(pAmx, "DoD_OnAddPlayerItem_Post", &Func) == ::AMX_ERR_NONE))
		{
			::DoD_AddPlayerItem = (::DoD_AddPlayerItem_Type) ::g_pDoDAddPlayerItem_Addr;
			::DetourTransactionBegin();
			::DetourUpdateThread(::GetCurrentThread());
			::DetourAttach(&(void*&) ::DoD_AddPlayerItem, ::DoD_AddPlayerItem_Hook);
			::DetourTransactionCommit();
			::g_DoDAddPlayerItem_Hook = true;
		}

		if (::g_pDoDRemovePlayerItem_Addr && false == ::g_DoDRemovePlayerItem_Hook &&
			(::g_fn_AmxFindPublic(pAmx, "DoD_OnRemovePlayerItem", &Func) == ::AMX_ERR_NONE ||
				::g_fn_AmxFindPublic(pAmx, "DoD_OnRemovePlayerItem_Post", &Func) == ::AMX_ERR_NONE))
		{
			::DoD_RemovePlayerItem = (::DoD_RemovePlayerItem_Type) ::g_pDoDRemovePlayerItem_Addr;
			::DetourTransactionBegin();
			::DetourUpdateThread(::GetCurrentThread());
			::DetourAttach(&(void*&) ::DoD_RemovePlayerItem, ::DoD_RemovePlayerItem_Hook);
			::DetourTransactionCommit();
			::g_DoDRemovePlayerItem_Hook = true;
		}

		if (::g_pDoDRemoveAllItems_Addr && false == ::g_DoDRemoveAllItems_Hook &&
			(::g_fn_AmxFindPublic(pAmx, "DoD_OnRemoveAllItems", &Func) == ::AMX_ERR_NONE ||
				::g_fn_AmxFindPublic(pAmx, "DoD_OnRemoveAllItems_Post", &Func) == ::AMX_ERR_NONE))
		{
			::DoD_RemoveAllItems = (::DoD_RemoveAllItems_Type) ::g_pDoDRemoveAllItems_Addr;
			::DetourTransactionBegin();
			::DetourUpdateThread(::GetCurrentThread());
			::DetourAttach(&(void*&) ::DoD_RemoveAllItems, ::DoD_RemoveAllItems_Hook);
			::DetourTransactionCommit();
			::g_DoDRemoveAllItems_Hook = true;
		}

		if (::g_pDoDGiveAmmo_Addr && false == ::g_DoDGiveAmmo_Hook &&
			(::g_fn_AmxFindPublic(pAmx, "DoD_OnGiveAmmo", &Func) == ::AMX_ERR_NONE ||
				::g_fn_AmxFindPublic(pAmx, "DoD_OnGiveAmmo_Post", &Func) == ::AMX_ERR_NONE))
		{
			::DoD_GiveAmmo = (::DoD_GiveAmmo_Type) ::g_pDoDGiveAmmo_Addr;
			::DetourTransactionBegin();
			::DetourUpdateThread(::GetCurrentThread());
			::DetourAttach(&(void*&) ::DoD_GiveAmmo, ::DoD_GiveAmmo_Hook);
			::DetourTransactionCommit();
			::g_DoDGiveAmmo_Hook = true;
		}

		if (::g_pDoDGetWaveTime_Addr && false == ::g_DoDGetWaveTime_Hook &&
			(::g_fn_AmxFindPublic(pAmx, "DoD_OnGetWaveTime", &Func) == ::AMX_ERR_NONE ||
				::g_fn_AmxFindPublic(pAmx, "DoD_OnGetWaveTime_Post", &Func) == ::AMX_ERR_NONE))
		{
			::DoD_GetWaveTime = (::DoD_GetWaveTime_Type) ::g_pDoDGetWaveTime_Addr;
			::DetourTransactionBegin();
			::DetourUpdateThread(::GetCurrentThread());
			::DetourAttach(&(void*&) ::DoD_GetWaveTime, ::DoD_GetWaveTime_Hook);
			::DetourTransactionCommit();
			::g_DoDGetWaveTime_Hook = true;
		}

		if (::g_pDoDSetWaveTime_Addr && false == ::g_DoDSetWaveTime_Hook &&
			(::g_fn_AmxFindPublic(pAmx, "DoD_OnSetWaveTime", &Func) == ::AMX_ERR_NONE ||
				::g_fn_AmxFindPublic(pAmx, "DoD_OnSetWaveTime_Post", &Func) == ::AMX_ERR_NONE))
		{
			::DoD_SetWaveTime = (::DoD_SetWaveTime_Type) ::g_pDoDSetWaveTime_Addr;
			::DetourTransactionBegin();
			::DetourUpdateThread(::GetCurrentThread());
			::DetourAttach(&(void*&) ::DoD_SetWaveTime, ::DoD_SetWaveTime_Hook);
			::DetourTransactionCommit();
			::g_DoDSetWaveTime_Hook = true;
		}

		if (::g_pDoDSetBodygroup_Addr && false == ::g_DoDSetBodygroup_Hook &&
			(::g_fn_AmxFindPublic(pAmx, "DoD_OnSetBodygroup", &Func) == ::AMX_ERR_NONE ||
				::g_fn_AmxFindPublic(pAmx, "DoD_OnSetBodygroup_Post", &Func) == ::AMX_ERR_NONE))
		{
			::DoD_SetBodygroup = (::DoD_SetBodygroup_Type) ::g_pDoDSetBodygroup_Addr;
			::DetourTransactionBegin();
			::DetourUpdateThread(::GetCurrentThread());
			::DetourAttach(&(void*&) ::DoD_SetBodygroup, ::DoD_SetBodygroup_Hook);
			::DetourTransactionCommit();
			::g_DoDSetBodygroup_Hook = true;
		}

		if (::g_pDoDSubRemove_Addr && false == ::g_DoDSubRemove_Hook &&
			(::g_fn_AmxFindPublic(pAmx, "DoD_OnSubRemove", &Func) == ::AMX_ERR_NONE ||
				::g_fn_AmxFindPublic(pAmx, "DoD_OnSubRemove_Post", &Func) == ::AMX_ERR_NONE))
		{
			::DoD_SubRemove = (::DoD_SubRemove_Type) ::g_pDoDSubRemove_Addr;
			::DetourTransactionBegin();
			::DetourUpdateThread(::GetCurrentThread());
			::DetourAttach(&(void*&) ::DoD_SubRemove, ::DoD_SubRemove_Hook);
			::DetourTransactionCommit();
			::g_DoDSubRemove_Hook = true;
		}

		if (::g_pDoDWpnBoxKill_Addr && false == ::g_DoDWpnBoxKill_Hook &&
			(::g_fn_AmxFindPublic(pAmx, "DoD_OnWpnBoxKill", &Func) == ::AMX_ERR_NONE ||
				::g_fn_AmxFindPublic(pAmx, "DoD_OnWpnBoxKill_Post", &Func) == ::AMX_ERR_NONE))
		{
			::DoD_WpnBoxKill = (::DoD_WpnBoxKill_Type) ::g_pDoDWpnBoxKill_Addr;
			::DetourTransactionBegin();
			::DetourUpdateThread(::GetCurrentThread());
			::DetourAttach(&(void*&) ::DoD_WpnBoxKill, ::DoD_WpnBoxKill_Hook);
			::DetourTransactionCommit();
			::g_DoDWpnBoxKill_Hook = true;
		}

		if (::g_pDoDWpnBoxActivateThink_Addr && false == ::g_DoDWpnBoxActivateThink_Hook &&
			(::g_fn_AmxFindPublic(pAmx, "DoD_OnWpnBoxActivateThink", &Func) == ::AMX_ERR_NONE ||
				::g_fn_AmxFindPublic(pAmx, "DoD_OnWpnBoxActivateThink_Post", &Func) == ::AMX_ERR_NONE))
		{
			::DoD_WpnBoxActivateThink = (::DoD_WpnBoxActivateThink_Type) ::g_pDoDWpnBoxActivateThink_Addr;
			::DetourTransactionBegin();
			::DetourUpdateThread(::GetCurrentThread());
			::DetourAttach(&(void*&) ::DoD_WpnBoxActivateThink, ::DoD_WpnBoxActivateThink_Hook);
			::DetourTransactionCommit();
			::g_DoDWpnBoxActivateThink_Hook = true;
		}

		if (::g_pDoDCreate_Addr && false == ::g_DoDCreate_Hook &&
			(::g_fn_AmxFindPublic(pAmx, "DoD_OnCreate", &Func) == ::AMX_ERR_NONE ||
				::g_fn_AmxFindPublic(pAmx, "DoD_OnCreate_Post", &Func) == ::AMX_ERR_NONE))
		{
			::DoD_Create = (::DoD_Create_Type) ::g_pDoDCreate_Addr;
			::DetourTransactionBegin();
			::DetourUpdateThread(::GetCurrentThread());
			::DetourAttach(&(void*&) ::DoD_Create, ::DoD_Create_Hook);
			::DetourTransactionCommit();
			::g_DoDCreate_Hook = true;
		}

		if (::g_pDoDPackWeapon_Addr && false == ::g_DoDPackWeapon_Hook &&
			(::g_fn_AmxFindPublic(pAmx, "DoD_OnPackWeapon", &Func) == ::AMX_ERR_NONE ||
				::g_fn_AmxFindPublic(pAmx, "DoD_OnPackWeapon_Post", &Func) == ::AMX_ERR_NONE))
		{
			::DoD_PackWeapon = (::DoD_PackWeapon_Type) ::g_pDoDPackWeapon_Addr;
			::DetourTransactionBegin();
			::DetourUpdateThread(::GetCurrentThread());
			::DetourAttach(&(void*&) ::DoD_PackWeapon, ::DoD_PackWeapon_Hook);
			::DetourTransactionCommit();
			::g_DoDPackWeapon_Hook = true;
		}

		if (::g_pDoDUtilRemove_Addr && false == ::g_DoDUtilRemove_Hook &&
			(::g_fn_AmxFindPublic(pAmx, "DoD_OnUtilRemove", &Func) == ::AMX_ERR_NONE ||
				::g_fn_AmxFindPublic(pAmx, "DoD_OnUtilRemove_Post", &Func) == ::AMX_ERR_NONE))
		{
			::DoD_UtilRemove = (::DoD_UtilRemove_Type) ::g_pDoDUtilRemove_Addr;
			::DetourTransactionBegin();
			::DetourUpdateThread(::GetCurrentThread());
			::DetourAttach(&(void*&) ::DoD_UtilRemove, ::DoD_UtilRemove_Hook);
			::DetourTransactionCommit();
			::g_DoDUtilRemove_Hook = true;
		}

		if (::g_pDoDDestroyItem_Addr && false == ::g_DoDDestroyItem_Hook &&
			(::g_fn_AmxFindPublic(pAmx, "DoD_OnDestroyItem", &Func) == ::AMX_ERR_NONE ||
				::g_fn_AmxFindPublic(pAmx, "DoD_OnDestroyItem_Post", &Func) == ::AMX_ERR_NONE))
		{
			::DoD_DestroyItem = (::DoD_DestroyItem_Type) ::g_pDoDDestroyItem_Addr;
			::DetourTransactionBegin();
			::DetourUpdateThread(::GetCurrentThread());
			::DetourAttach(&(void*&) ::DoD_DestroyItem, ::DoD_DestroyItem_Hook);
			::DetourTransactionCommit();
			::g_DoDDestroyItem_Hook = true;
		}
#else
		if (::g_pDoDPlayerSpawn_Addr && false == ::g_DoDPlayerSpawn_Hook &&
			(::g_fn_AmxFindPublic(pAmx, "DoD_OnPlayerSpawn", &Func) == ::AMX_ERR_NONE ||
				::g_fn_AmxFindPublic(pAmx, "DoD_OnPlayerSpawn_Post", &Func) == ::AMX_ERR_NONE))
		{
			::g_pDoDPlayerSpawn = ::subhook_new(::g_pDoDPlayerSpawn_Addr, ::DoD_PlayerSpawn_Hook, ::SUBHOOK_TRAMPOLINE);
			::subhook_install(::g_pDoDPlayerSpawn);
			::DoD_PlayerSpawn = (::DoD_PlayerSpawn_Type) ::subhook_get_trampoline(::g_pDoDPlayerSpawn);
			::g_DoDPlayerSpawn_Hook = true;
		}

		if (::g_pDoDGiveNamedItem_Addr && false == ::g_DoDGiveNamedItem_Hook &&
			(::g_fn_AmxFindPublic(pAmx, "DoD_OnGiveNamedItem", &Func) == ::AMX_ERR_NONE ||
				::g_fn_AmxFindPublic(pAmx, "DoD_OnGiveNamedItem_Post", &Func) == ::AMX_ERR_NONE))
		{
			::g_pDoDGiveNamedItem = ::subhook_new(::g_pDoDGiveNamedItem_Addr, ::DoD_GiveNamedItem_Hook, ::SUBHOOK_TRAMPOLINE);
			::subhook_install(::g_pDoDGiveNamedItem);
			::DoD_GiveNamedItem = (::DoD_GiveNamedItem_Type) ::subhook_get_trampoline(::g_pDoDGiveNamedItem);
			::g_DoDGiveNamedItem_Hook = true;
		}

		if (::g_pDoDDropPlayerItem_Addr && false == ::g_DoDDropPlayerItem_Hook &&
			(::g_fn_AmxFindPublic(pAmx, "DoD_OnDropPlayerItem", &Func) == ::AMX_ERR_NONE ||
				::g_fn_AmxFindPublic(pAmx, "DoD_OnDropPlayerItem_Post", &Func) == ::AMX_ERR_NONE))
		{
			::g_pDoDDropPlayerItem = ::subhook_new(::g_pDoDDropPlayerItem_Addr, ::DoD_DropPlayerItem_Hook, ::SUBHOOK_TRAMPOLINE);
			::subhook_install(::g_pDoDDropPlayerItem);
			::DoD_DropPlayerItem = (::DoD_DropPlayerItem_Type) ::subhook_get_trampoline(::g_pDoDDropPlayerItem);
			::g_DoDDropPlayerItem_Hook = true;
		}

		if (::g_pDoDAddPlayerItem_Addr && false == ::g_DoDAddPlayerItem_Hook &&
			(::g_fn_AmxFindPublic(pAmx, "DoD_OnAddPlayerItem", &Func) == ::AMX_ERR_NONE ||
				::g_fn_AmxFindPublic(pAmx, "DoD_OnAddPlayerItem_Post", &Func) == ::AMX_ERR_NONE))
		{
			::g_pDoDAddPlayerItem = ::subhook_new(::g_pDoDAddPlayerItem_Addr, ::DoD_AddPlayerItem_Hook, ::SUBHOOK_TRAMPOLINE);
			::subhook_install(::g_pDoDAddPlayerItem);
			::DoD_AddPlayerItem = (::DoD_AddPlayerItem_Type) ::subhook_get_trampoline(::g_pDoDAddPlayerItem);
			::g_DoDAddPlayerItem_Hook = true;
		}

		if (::g_pDoDRemovePlayerItem_Addr && false == ::g_DoDRemovePlayerItem_Hook &&
			(::g_fn_AmxFindPublic(pAmx, "DoD_OnRemovePlayerItem", &Func) == ::AMX_ERR_NONE ||
				::g_fn_AmxFindPublic(pAmx, "DoD_OnRemovePlayerItem_Post", &Func) == ::AMX_ERR_NONE))
		{
			::g_pDoDRemovePlayerItem = ::subhook_new(::g_pDoDRemovePlayerItem_Addr, ::DoD_RemovePlayerItem_Hook, ::SUBHOOK_TRAMPOLINE);
			::subhook_install(::g_pDoDRemovePlayerItem);
			::DoD_RemovePlayerItem = (::DoD_RemovePlayerItem_Type) ::subhook_get_trampoline(::g_pDoDRemovePlayerItem);
			::g_DoDRemovePlayerItem_Hook = true;
		}

		if (::g_pDoDRemoveAllItems_Addr && false == ::g_DoDRemoveAllItems_Hook &&
			(::g_fn_AmxFindPublic(pAmx, "DoD_OnRemoveAllItems", &Func) == ::AMX_ERR_NONE ||
				::g_fn_AmxFindPublic(pAmx, "DoD_OnRemoveAllItems_Post", &Func) == ::AMX_ERR_NONE))
		{
			::g_pDoDRemoveAllItems = ::subhook_new(::g_pDoDRemoveAllItems_Addr, ::DoD_RemoveAllItems_Hook, ::SUBHOOK_TRAMPOLINE);
			::subhook_install(::g_pDoDRemoveAllItems);
			::DoD_RemoveAllItems = (::DoD_RemoveAllItems_Type) ::subhook_get_trampoline(::g_pDoDRemoveAllItems);
			::g_DoDRemoveAllItems_Hook = true;
		}

		if (::g_pDoDGiveAmmo_Addr && false == ::g_DoDGiveAmmo_Hook &&
			(::g_fn_AmxFindPublic(pAmx, "DoD_OnGiveAmmo", &Func) == ::AMX_ERR_NONE ||
				::g_fn_AmxFindPublic(pAmx, "DoD_OnGiveAmmo_Post", &Func) == ::AMX_ERR_NONE))
		{
			::g_pDoDGiveAmmo = ::subhook_new(::g_pDoDGiveAmmo_Addr, ::DoD_GiveAmmo_Hook, ::SUBHOOK_TRAMPOLINE);
			::subhook_install(::g_pDoDGiveAmmo);
			::DoD_GiveAmmo = (::DoD_GiveAmmo_Type) ::subhook_get_trampoline(::g_pDoDGiveAmmo);
			::g_DoDGiveAmmo_Hook = true;
		}

		if (::g_pDoDGetWaveTime_Addr && false == ::g_DoDGetWaveTime_Hook &&
			(::g_fn_AmxFindPublic(pAmx, "DoD_OnGetWaveTime", &Func) == ::AMX_ERR_NONE ||
				::g_fn_AmxFindPublic(pAmx, "DoD_OnGetWaveTime_Post", &Func) == ::AMX_ERR_NONE))
		{
			::g_pDoDGetWaveTime = ::subhook_new(::g_pDoDGetWaveTime_Addr, ::DoD_GetWaveTime_Hook, ::SUBHOOK_TRAMPOLINE);
			::subhook_install(::g_pDoDGetWaveTime);
			::DoD_GetWaveTime = (::DoD_GetWaveTime_Type) ::subhook_get_trampoline(::g_pDoDGetWaveTime);
			::g_DoDGetWaveTime_Hook = true;
		}

		if (::g_pDoDSetWaveTime_Addr && false == ::g_DoDSetWaveTime_Hook &&
			(::g_fn_AmxFindPublic(pAmx, "DoD_OnSetWaveTime", &Func) == ::AMX_ERR_NONE ||
				::g_fn_AmxFindPublic(pAmx, "DoD_OnSetWaveTime_Post", &Func) == ::AMX_ERR_NONE))
		{
			::g_pDoDSetWaveTime = ::subhook_new(::g_pDoDSetWaveTime_Addr, ::DoD_SetWaveTime_Hook, ::SUBHOOK_TRAMPOLINE);
			::subhook_install(::g_pDoDSetWaveTime);
			::DoD_SetWaveTime = (::DoD_SetWaveTime_Type) ::subhook_get_trampoline(::g_pDoDSetWaveTime);
			::g_DoDSetWaveTime_Hook = true;
		}

		if (::g_pDoDSetBodygroup_Addr && false == ::g_DoDSetBodygroup_Hook &&
			(::g_fn_AmxFindPublic(pAmx, "DoD_OnSetBodygroup", &Func) == ::AMX_ERR_NONE ||
				::g_fn_AmxFindPublic(pAmx, "DoD_OnSetBodygroup_Post", &Func) == ::AMX_ERR_NONE))
		{
			::g_pDoDSetBodygroup = ::subhook_new(::g_pDoDSetBodygroup_Addr, ::DoD_SetBodygroup_Hook, ::SUBHOOK_TRAMPOLINE);
			::subhook_install(::g_pDoDSetBodygroup);
			::DoD_SetBodygroup = (::DoD_SetBodygroup_Type) ::subhook_get_trampoline(::g_pDoDSetBodygroup);
			::g_DoDSetBodygroup_Hook = true;
		}

		if (::g_pDoDSubRemove_Addr && false == ::g_DoDSubRemove_Hook &&
			(::g_fn_AmxFindPublic(pAmx, "DoD_OnSubRemove", &Func) == ::AMX_ERR_NONE ||
				::g_fn_AmxFindPublic(pAmx, "DoD_OnSubRemove_Post", &Func) == ::AMX_ERR_NONE))
		{
			::g_pDoDSubRemove = ::subhook_new(::g_pDoDSubRemove_Addr, ::DoD_SubRemove_Hook, ::SUBHOOK_TRAMPOLINE);
			::subhook_install(::g_pDoDSubRemove);
			::DoD_SubRemove = (::DoD_SubRemove_Type) ::subhook_get_trampoline(::g_pDoDSubRemove);
			::g_DoDSubRemove_Hook = true;
		}

		if (::g_pDoDCreate_Addr && false == ::g_DoDCreate_Hook &&
			(::g_fn_AmxFindPublic(pAmx, "DoD_OnCreate", &Func) == ::AMX_ERR_NONE ||
				::g_fn_AmxFindPublic(pAmx, "DoD_OnCreate_Post", &Func) == ::AMX_ERR_NONE))
		{
			::g_pDoDCreate = ::subhook_new(::g_pDoDCreate_Addr, ::DoD_Create_Hook, ::SUBHOOK_TRAMPOLINE);
			::subhook_install(::g_pDoDCreate);
			::DoD_Create = (::DoD_Create_Type) ::subhook_get_trampoline(::g_pDoDCreate);
			::g_DoDCreate_Hook = true;
		}

		if (::g_pDoDWpnBoxKill_Addr && false == ::g_DoDWpnBoxKill_Hook &&
			(::g_fn_AmxFindPublic(pAmx, "DoD_OnWpnBoxKill", &Func) == ::AMX_ERR_NONE ||
				::g_fn_AmxFindPublic(pAmx, "DoD_OnWpnBoxKill_Post", &Func) == ::AMX_ERR_NONE))
		{
			::g_pDoDWpnBoxKill = ::subhook_new(::g_pDoDWpnBoxKill_Addr, ::DoD_WpnBoxKill_Hook, ::SUBHOOK_TRAMPOLINE);
			::subhook_install(::g_pDoDWpnBoxKill);
			::DoD_WpnBoxKill = (::DoD_WpnBoxKill_Type) ::subhook_get_trampoline(::g_pDoDWpnBoxKill);
			::g_DoDWpnBoxKill_Hook = true;
		}

		if (::g_pDoDWpnBoxActivateThink_Addr && false == ::g_DoDWpnBoxActivateThink_Hook &&
			(::g_fn_AmxFindPublic(pAmx, "DoD_OnWpnBoxActivateThink", &Func) == ::AMX_ERR_NONE ||
				::g_fn_AmxFindPublic(pAmx, "DoD_OnWpnBoxActivateThink_Post", &Func) == ::AMX_ERR_NONE))
		{
			::g_pDoDWpnBoxActivateThink = ::subhook_new(::g_pDoDWpnBoxActivateThink_Addr, ::DoD_WpnBoxActivateThink_Hook, ::SUBHOOK_TRAMPOLINE);
			::subhook_install(::g_pDoDWpnBoxActivateThink);
			::DoD_WpnBoxActivateThink = (::DoD_WpnBoxActivateThink_Type) ::subhook_get_trampoline(::g_pDoDWpnBoxActivateThink);
			::g_DoDWpnBoxActivateThink_Hook = true;
		}

		if (::g_pDoDPackWeapon_Addr && false == ::g_DoDPackWeapon_Hook &&
			(::g_fn_AmxFindPublic(pAmx, "DoD_OnPackWeapon", &Func) == ::AMX_ERR_NONE ||
				::g_fn_AmxFindPublic(pAmx, "DoD_OnPackWeapon_Post", &Func) == ::AMX_ERR_NONE))
		{
			::g_pDoDPackWeapon = ::subhook_new(::g_pDoDPackWeapon_Addr, ::DoD_PackWeapon_Hook, ::SUBHOOK_TRAMPOLINE);
			::subhook_install(::g_pDoDPackWeapon);
			::DoD_PackWeapon = (::DoD_PackWeapon_Type) ::subhook_get_trampoline(::g_pDoDPackWeapon);
			::g_DoDPackWeapon_Hook = true;
		}

		if (::g_pDoDUtilRemove_Addr && false == ::g_DoDUtilRemove_Hook &&
			(::g_fn_AmxFindPublic(pAmx, "DoD_OnUtilRemove", &Func) == ::AMX_ERR_NONE ||
				::g_fn_AmxFindPublic(pAmx, "DoD_OnUtilRemove_Post", &Func) == ::AMX_ERR_NONE))
		{
			::g_pDoDUtilRemove = ::subhook_new(::g_pDoDUtilRemove_Addr, ::DoD_UtilRemove_Hook, ::SUBHOOK_TRAMPOLINE);
			::subhook_install(::g_pDoDUtilRemove);
			::DoD_UtilRemove = (::DoD_UtilRemove_Type) ::subhook_get_trampoline(::g_pDoDUtilRemove);
			::g_DoDUtilRemove_Hook = true;
		}

		if (::g_pDoDDestroyItem_Addr && false == ::g_DoDDestroyItem_Hook &&
			(::g_fn_AmxFindPublic(pAmx, "DoD_OnDestroyItem", &Func) == ::AMX_ERR_NONE ||
				::g_fn_AmxFindPublic(pAmx, "DoD_OnDestroyItem_Post", &Func) == ::AMX_ERR_NONE))
		{
			::g_pDoDDestroyItem = ::subhook_new(::g_pDoDDestroyItem_Addr, ::DoD_DestroyItem_Hook, ::SUBHOOK_TRAMPOLINE);
			::subhook_install(::g_pDoDDestroyItem);
			::DoD_DestroyItem = (::DoD_DestroyItem_Type) ::subhook_get_trampoline(::g_pDoDDestroyItem);
			::g_DoDDestroyItem_Hook = true;
		}
#endif
	}
}

void OnPluginsUnloaded()
{
#ifndef __linux__
	if (::g_DoDPlayerSpawn_Hook)
	{
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourDetach(&(void*&) ::DoD_PlayerSpawn, ::DoD_PlayerSpawn_Hook);
		::DetourTransactionCommit();
		::g_DoDPlayerSpawn_Hook = false;
	}
	if (::g_DoDGiveNamedItem_Hook)
	{
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourDetach(&(void*&) ::DoD_GiveNamedItem, ::DoD_GiveNamedItem_Hook);
		::DetourTransactionCommit();
		::g_DoDGiveNamedItem_Hook = false;
	}
	if (::g_DoDDropPlayerItem_Hook)
	{
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourDetach(&(void*&) ::DoD_DropPlayerItem, ::DoD_DropPlayerItem_Hook);
		::DetourTransactionCommit();
		::g_DoDDropPlayerItem_Hook = false;
	}
	if (::g_DoDRemovePlayerItem_Hook)
	{
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourDetach(&(void*&) ::DoD_RemovePlayerItem, ::DoD_RemovePlayerItem_Hook);
		::DetourTransactionCommit();
		::g_DoDRemovePlayerItem_Hook = false;
	}
	if (::g_DoDAddPlayerItem_Hook)
	{
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourDetach(&(void*&) ::DoD_AddPlayerItem, ::DoD_AddPlayerItem_Hook);
		::DetourTransactionCommit();
		::g_DoDAddPlayerItem_Hook = false;
	}
	if (::g_DoDRemoveAllItems_Hook)
	{
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourDetach(&(void*&) ::DoD_RemoveAllItems, ::DoD_RemoveAllItems_Hook);
		::DetourTransactionCommit();
		::g_DoDRemoveAllItems_Hook = false;
	}
	if (::g_DoDSetWaveTime_Hook)
	{
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourDetach(&(void*&) ::DoD_SetWaveTime, ::DoD_SetWaveTime_Hook);
		::DetourTransactionCommit();
		::g_DoDSetWaveTime_Hook = false;
	}
	if (::g_DoDGetWaveTime_Hook)
	{
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourDetach(&(void*&) ::DoD_GetWaveTime, ::DoD_GetWaveTime_Hook);
		::DetourTransactionCommit();
		::g_DoDGetWaveTime_Hook = false;
	}
	if (::g_DoDGiveAmmo_Hook)
	{
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourDetach(&(void*&) ::DoD_GiveAmmo, ::DoD_GiveAmmo_Hook);
		::DetourTransactionCommit();
		::g_DoDGiveAmmo_Hook = false;
	}
	if (::g_DoDSetBodygroup_Hook)
	{
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourDetach(&(void*&) ::DoD_SetBodygroup, ::DoD_SetBodygroup_Hook);
		::DetourTransactionCommit();
		::g_DoDSetBodygroup_Hook = false;
	}
	if (::g_DoDUtilRemove_Hook)
	{
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourDetach(&(void*&) ::DoD_UtilRemove, ::DoD_UtilRemove_Hook);
		::DetourTransactionCommit();
		::g_DoDUtilRemove_Hook = false;
	}
	if (::g_DoDSubRemove_Hook)
	{
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourDetach(&(void*&) ::DoD_SubRemove, ::DoD_SubRemove_Hook);
		::DetourTransactionCommit();
		::g_DoDSubRemove_Hook = false;
	}
	if (::g_DoDWpnBoxKill_Hook)
	{
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourDetach(&(void*&) ::DoD_WpnBoxKill, ::DoD_WpnBoxKill_Hook);
		::DetourTransactionCommit();
		::g_DoDWpnBoxKill_Hook = false;
	}
	if (::g_DoDCreate_Hook)
	{
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourDetach(&(void*&) ::DoD_Create, ::DoD_Create_Hook);
		::DetourTransactionCommit();
		::g_DoDCreate_Hook = false;
	}
	if (::g_DoDWpnBoxActivateThink_Hook)
	{
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourDetach(&(void*&) ::DoD_WpnBoxActivateThink, ::DoD_WpnBoxActivateThink_Hook);
		::DetourTransactionCommit();
		::g_DoDWpnBoxActivateThink_Hook = false;
	}
	if (::g_DoDPackWeapon_Hook)
	{
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourDetach(&(void*&) ::DoD_PackWeapon, ::DoD_PackWeapon_Hook);
		::DetourTransactionCommit();
		::g_DoDPackWeapon_Hook = false;
	}
	if (::g_DoDDestroyItem_Hook)
	{
		::DetourTransactionBegin();
		::DetourUpdateThread(::GetCurrentThread());
		::DetourDetach(&(void*&) ::DoD_DestroyItem, ::DoD_DestroyItem_Hook);
		::DetourTransactionCommit();
		::g_DoDDestroyItem_Hook = false;
	}
#else
	if (::g_DoDPlayerSpawn_Hook)
	{
		::subhook_remove(::g_pDoDPlayerSpawn);
		::subhook_free(::g_pDoDPlayerSpawn);
		::g_DoDPlayerSpawn_Hook = false;
	}
	if (::g_DoDGiveNamedItem_Hook)
	{
		::subhook_remove(::g_pDoDGiveNamedItem);
		::subhook_free(::g_pDoDGiveNamedItem);
		::g_DoDGiveNamedItem_Hook = false;
	}
	if (::g_DoDDropPlayerItem_Hook)
	{
		::subhook_remove(::g_pDoDDropPlayerItem);
		::subhook_free(::g_pDoDDropPlayerItem);
		::g_DoDDropPlayerItem_Hook = false;
	}
	if (::g_DoDRemovePlayerItem_Hook)
	{
		::subhook_remove(::g_pDoDRemovePlayerItem);
		::subhook_free(::g_pDoDRemovePlayerItem);
		::g_DoDRemovePlayerItem_Hook = false;
	}
	if (::g_DoDAddPlayerItem_Hook)
	{
		::subhook_remove(::g_pDoDAddPlayerItem);
		::subhook_free(::g_pDoDAddPlayerItem);
		::g_DoDAddPlayerItem_Hook = false;
	}
	if (::g_DoDRemoveAllItems_Hook)
	{
		::subhook_remove(::g_pDoDRemoveAllItems);
		::subhook_free(::g_pDoDRemoveAllItems);
		::g_DoDRemoveAllItems_Hook = false;
	}
	if (::g_DoDSetWaveTime_Hook)
	{
		::subhook_remove(::g_pDoDSetWaveTime);
		::subhook_free(::g_pDoDSetWaveTime);
		::g_DoDSetWaveTime_Hook = false;
	}
	if (::g_DoDGetWaveTime_Hook)
	{
		::subhook_remove(::g_pDoDGetWaveTime);
		::subhook_free(::g_pDoDGetWaveTime);
		::g_DoDGetWaveTime_Hook = false;
	}
	if (::g_DoDGiveAmmo_Hook)
	{
		::subhook_remove(::g_pDoDGiveAmmo);
		::subhook_free(::g_pDoDGiveAmmo);
		::g_DoDGiveAmmo_Hook = false;
	}
	if (::g_DoDSetBodygroup_Hook)
	{
		::subhook_remove(::g_pDoDSetBodygroup);
		::subhook_free(::g_pDoDSetBodygroup);
		::g_DoDSetBodygroup_Hook = false;
	}
	if (::g_DoDDestroyItem_Hook)
	{
		::subhook_remove(::g_pDoDDestroyItem);
		::subhook_free(::g_pDoDDestroyItem);
		::g_DoDDestroyItem_Hook = false;
	}
	if (::g_DoDSubRemove_Hook)
	{
		::subhook_remove(::g_pDoDSubRemove);
		::subhook_free(::g_pDoDSubRemove);
		::g_DoDSubRemove_Hook = false;
	}
	if (::g_DoDCreate_Hook)
	{
		::subhook_remove(::g_pDoDCreate);
		::subhook_free(::g_pDoDCreate);
		::g_DoDCreate_Hook = false;
	}
	if (::g_DoDWpnBoxKill_Hook)
	{
		::subhook_remove(::g_pDoDWpnBoxKill);
		::subhook_free(::g_pDoDWpnBoxKill);
		::g_DoDWpnBoxKill_Hook = false;
	}
	if (::g_DoDWpnBoxActivateThink_Hook)
	{
		::subhook_remove(::g_pDoDWpnBoxActivateThink);
		::subhook_free(::g_pDoDWpnBoxActivateThink);
		::g_DoDWpnBoxActivateThink_Hook = false;
	}
	if (::g_DoDPackWeapon_Hook)
	{
		::subhook_remove(::g_pDoDPackWeapon);
		::subhook_free(::g_pDoDPackWeapon);
		::g_DoDPackWeapon_Hook = false;
	}
	if (::g_DoDUtilRemove_Hook)
	{
		::subhook_remove(::g_pDoDUtilRemove);
		::subhook_free(::g_pDoDUtilRemove);
		::g_DoDUtilRemove_Hook = false;
	}
#endif
}

int AmxxCheckGame(const char* pGame)
{
	if (false == ::_stricmp("DoD", pGame))
	{
		return false; /// OK
	}
	return true; /// ERROR
}
