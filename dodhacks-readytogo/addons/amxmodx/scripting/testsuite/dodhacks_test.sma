
#include <amxmodx>
#include <fakemeta>
#include <hamsandwich>
#include <dodhacks>

#if !defined client_disconnected
#define DOD_ON_PLAYER_DISCONNECTED client_disconnect(Player) /** OLD AMX MOD X */
#else
#define DOD_ON_PLAYER_DISCONNECTED client_disconnected(Player, bool: Drop, Msg[], Size) /** NEW AMX MOD X */
#endif

new bool: g_InSpawn[33];
new bool: g_InServer[33];

new Array: g_Items[33];
new Array: g_Entities[33];

public plugin_init()
{
    register_plugin("DoD Hacks: Testing", "1.0.0.2", "Hattrick HKS (claudiuhks)");
    if (DoD_IsAutoScopingEnabled())
    {
        DoD_DisableAutoScoping();
        server_print("@@@ Disabled Auto Scoping");
    }
    else
        server_print("@@@ Auto Scoping Already Disabled");
#if defined amxclient_cmd && defined RegisterHamPlayer
    RegisterHamPlayer(Ham_Killed, "OnPlayerKilled_Post", true);
#else
    RegisterHam(Ham_Killed, "player", "OnPlayerKilled_Post", true);
#endif
    for (new Iter = 1; Iter <= get_maxplayers(); Iter++)
    {
        g_Items[Iter] = ArrayCreate(32);
        g_Entities[Iter] = ArrayCreate();
    }
    register_clcmd("dht_givek43", "OnPlayerConCmd_GiveK43", ADMIN_ALL, "- gives a K43");
    register_clcmd("dht_givek43_hooks", "OnPlayerConCmd_GiveK43_Hooks", ADMIN_ALL, "- gives a K43 and calls all related hooks");
    register_clcmd("dht_givekar_hooks", "OnPlayerConCmd_GiveKar_Hooks", ADMIN_ALL, "- gives a Kar and calls all related hooks");
    register_clcmd("dht_showweapons", "OnPlayerConCmd_ShowWeapons", ADMIN_ALL, "- prints all owned weapons");
}

public client_connect(Player)
{
    g_InSpawn[Player] = false;
    g_InServer[Player] = false;
}

public DOD_ON_PLAYER_DISCONNECTED
{
    g_InSpawn[Player] = false;
    g_InServer[Player] = false;

    ArrayClear(g_Items[Player]);
    ArrayClear(g_Entities[Player]);
}

public client_putinserver(Player)
{
    g_InServer[Player] = true;

    ArrayClear(g_Items[Player]);
    ArrayClear(g_Entities[Player]);
}

public DoD_OnPlayerSpawn(DoD_Address: CDoDTeamPlay, &Player)
{
    if (g_InServer[Player])
    {
        server_print("@@@ DoD_OnPlayerSpawn(%N) [In Server]", Player);
        g_InSpawn[Player] = true;
    }
    else
        server_print("@@@ DoD_OnPlayerSpawn(%N) [Not In Server Yet]", Player);
}

public DoD_OnPlayerSpawn_Post(DoD_Address: CDoDTeamPlay, Player)
{
    if (g_InServer[Player])
    {
        g_InSpawn[Player] = false;
        server_print("@@@ DoD_OnPlayerSpawn_Post(%N) [In Server]", Player);
    }
    else
        server_print("@@@ DoD_OnPlayerSpawn_Post(%N) [Not In Server Yet]", Player);
}

public DoD_OnGiveNamedItem(Player, Item[], ItemSize /** = 64 */)
{
    if (g_InSpawn[Player])
    {
        server_print("@@@ DoD_OnGiveNamedItem(%N, %s, %d) [In Spawn]", Player, Item, ItemSize);
        if (equali(Item, "weapon_webley") || equali(Item, "weapon_colt"))
        {
            copy(Item, ItemSize - 1, "weapon_luger");
            server_print("@@@ Changed %N's Hand Gun To %s", Player, Item);
        }
        else if (equali(Item, "weapon_luger"))
        {
            copy(Item, ItemSize - 1, DoD_AreAlliesBritish() ? "weapon_webley" : "weapon_colt");
            server_print("@@@ Changed %N's Hand Gun To %s", Player, Item);
        }
    }
    else
    {
        server_print("@@@ DoD_OnGiveNamedItem(%N, %s, %d) [Not In Spawn]", Player, Item, ItemSize);
        if (equali(Item, "weapon_kar"))
        {
            server_print("@@@ Blocking weapon_kar From Being Given [Outside Of Spawn]");
            return PLUGIN_HANDLED;
        }
    }
    return PLUGIN_CONTINUE;
}

public DoD_OnGiveNamedItem_Post(Player, const Item[])
{
    if (g_InSpawn[Player])
        server_print("@@@ DoD_OnGiveNamedItem_Post(%N, %s) [In Spawn]", Player, Item);
    else
        server_print("@@@ DoD_OnGiveNamedItem_Post(%N, %s) [Not In Spawn]", Player, Item);
}

public DoD_OnCreate(Name[], NameSize /** = 64 */, Float: Origin[3], Float: Angles[3], &Owner)
{
    server_print("@@@ DoD_OnCreate(%s, %d, %f, %f, %f, %f, %f, %f, %d)",
        Name, NameSize, Origin[0], Origin[1], Origin[2], Angles[0], Angles[1], Angles[2], Owner);

    Origin[2] += 96.0;
    server_print("@@@ Origin[2] Increased With 96.0 Units");
}

public DoD_OnCreate_Post(const Name[], const Float: Origin[3], const Float: Angles[3], Owner, ItemRes)
{
    server_print("@@@ DoD_OnCreate_Post(%s, %f, %f, %f, %f, %f, %f, %d, %d)",
        Name, Origin[0], Origin[1], Origin[2], Angles[0], Angles[1], Angles[2], Owner, ItemRes);
}

public OnPlayerConCmd_GiveK43_Hooks(Player)
{
    if (g_InServer[Player])
    {
        static Item, Weapon[32];
        if (F_HasPrimaryWeapon(Player, Weapon, charsmax(Weapon), Item))
        {
            DoD_DropPlayerItem(Player, Weapon, true);
            Item = ArrayFindString(g_Items[Player], Weapon);
            if (Item > -1)
            {
                ArrayDeleteItem(g_Items[Player], Item);
                ArrayDeleteItem(g_Entities[Player], Item);
            }
        }
        if (DoD_GiveNamedItem(Player, "weapon_k43", Item, true) && Item > 0)
            client_print(Player, print_console, "K43 given (%d)!", Item);
        else
            client_print(Player, print_console, "Failed giving K43!");
    }
    return PLUGIN_HANDLED;
}

public OnPlayerConCmd_GiveKar_Hooks(Player)
{
    if (g_InServer[Player])
    {
        static Item, Weapon[32];
        if (F_HasPrimaryWeapon(Player, Weapon, charsmax(Weapon), Item))
        {
            DoD_DropPlayerItem(Player, Weapon, true);
            Item = ArrayFindString(g_Items[Player], Weapon);
            if (Item > -1)
            {
                ArrayDeleteItem(g_Items[Player], Item);
                ArrayDeleteItem(g_Entities[Player], Item);
            }
        }
        if (DoD_GiveNamedItem(Player, "weapon_kar", Item, true) && Item > 0)
            client_print(Player, print_console, "Kar given (%d)!", Item);
        else
            client_print(Player, print_console, "Failed giving Kar!");
    }
    return PLUGIN_HANDLED;
}

public OnPlayerConCmd_GiveK43(Player)
{
    if (g_InServer[Player])
    {
        static Item, Weapon[32];
        if (F_HasPrimaryWeapon(Player, Weapon, charsmax(Weapon), Item))
        {
            DoD_DropPlayerItem(Player, Weapon, true);
            Item = ArrayFindString(g_Items[Player], Weapon);
            if (Item > -1)
            {
                ArrayDeleteItem(g_Items[Player], Item);
                ArrayDeleteItem(g_Entities[Player], Item);
            }
        }
        if (DoD_GiveNamedItem(Player, "weapon_k43", Item) && Item > 0)
            client_print(Player, print_console, "K43 given (%d)!", Item);
        else
            client_print(Player, print_console, "Failed giving K43!");
    }
    return PLUGIN_HANDLED;
}

public OnPlayerConCmd_ShowWeapons(Player)
{
    if (g_InServer[Player])
    {
        static Item, Size, Weapon[32], Buffer[256], Index[16];
        Size = ArraySize(g_Items[Player]);
        Buffer[0] = EOS;
        for (Item = 0; Item < Size; Item++)
        {
            ArrayGetString(g_Items[Player], Item, Weapon, charsmax(Weapon));
#if !defined replace_stringex
            replace(Weapon, charsmax(Weapon), "weapon_", "");
#else
            replace_stringex(Weapon, charsmax(Weapon), "weapon_", "", 7, 0, false);
#endif
            add(Buffer, charsmax(Buffer), Weapon);
            num_to_str(ArrayGetCell(g_Entities[Player], Item), Index, charsmax(Index));
            add(Buffer, charsmax(Buffer), "(");
            add(Buffer, charsmax(Buffer), Index);
            add(Buffer, charsmax(Buffer), ((Item == Size - 1) ? ")" : "),"));
        }
        client_print(Player, print_console, Buffer[0] != EOS ? Buffer : "Your inventory is empty right now!");
    }
    return PLUGIN_HANDLED;
}

public DoD_OnAddPlayerItem(Player, &Item)
{
    server_print("@@@ DoD_OnAddPlayerItem(%N, %d)", Player, Item);
}

public DoD_OnAddPlayerItem_Post(Player, Item, Res)
{
    if (Res > 0)
    {
        static Class[32];
        pev(Item, pev_classname, Class, charsmax(Class));
        if (ArrayFindString(g_Items[Player], Class) < 0)
        {
            ArrayPushString(g_Items[Player], Class);
            ArrayPushCell(g_Entities[Player], Item);
        }
        server_print("@@@ DoD_OnAddPlayerItem_Post(%N, %s, %d, %d)", Player, Class, Item, Res);
    }
    else
        server_print("@@@ DoD_OnAddPlayerItem_Post(%N, %d, %d)", Player, Item, Res);
}

public DoD_OnRemovePlayerItem(Player, &Item)
{
    server_print("@@@ DoD_OnRemovePlayerItem(%N, %d)", Player, Item);
}

public DoD_OnRemovePlayerItem_Post(Player, Item, Res)
{
    if (Res > 0)
    {
        static Index, Class[32];
        pev(Item, pev_classname, Class, charsmax(Class));
        Index = ArrayFindString(g_Items[Player], Class);
        if (Index > -1)
        {
            ArrayDeleteItem(g_Items[Player], Index);
            ArrayDeleteItem(g_Entities[Player], Index);
        }
        server_print("@@@ DoD_OnRemovePlayerItem_Post(%N, %s, %d, %d)", Player, Class, Item, Res);
    }
    else
        server_print("@@@ DoD_OnRemovePlayerItem_Post(%N, %d, %d)", Player, Item, Res);
}

public DoD_OnRemoveAllItems(Player, &RemoveSuit)
{
    if (g_InServer[Player])
        server_print("@@@ DoD_OnRemoveAllItems(%N, %d)", Player, RemoveSuit);
    else
        server_print("@@@ DoD_OnRemoveAllItems(%d, %d) [disconnected]", Player, RemoveSuit);
}

public DoD_OnRemoveAllItems_Post(Player, RemoveSuit)
{
    ArrayClear(g_Items[Player]);
    ArrayClear(g_Entities[Player]);

    if (g_InServer[Player])
        server_print("@@@ DoD_OnRemoveAllItems_Post(%N, %d)", Player, RemoveSuit);
    else
        server_print("@@@ DoD_OnRemoveAllItems_Post(%d, %d) [disconnected]", Player, RemoveSuit);
}

bool: F_HasPrimaryWeapon(Player, Weapon[], Size, &Entity)
{
    static Item[32], Iter;
    for (Iter = 0; Iter < ArraySize(g_Items[Player]); Iter++)
    {
        ArrayGetString(g_Items[Player], Iter, Item, charsmax(Item));
        if (DoD_IsWeaponPrimary(Item))
        {
            copy(Weapon, Size, Item);
            Entity = ArrayGetCell(g_Entities[Player], Iter);
            return true;
        }
    }
    return false;
}

public OnPlayerKilled_Post(Player)
{
    if (g_InServer[Player])
    {
        ArrayClear(g_Items[Player]);
        ArrayClear(g_Entities[Player]);
    }
}
