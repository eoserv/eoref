#include "eo_packets.hpp"

#include "data/eo_stream.hpp"

#include <cstdio>
#include <memory>
#include <type_traits>

namespace eo_protocol
{


std::shared_ptr<Server_Packet> unserialize(EO_Stream_Reader& reader)
{
	if (reader.remaining() < 2)
		return nullptr;

	auto action = PacketAction(reader.get_byte());
	auto family = PacketFamily(reader.get_byte());

	auto id = packet_id_hash(PacketID{family, action});

#define case_packet(type) \
	case packet_id_hash(server::type::id): \
	{ \
		auto p = std::make_shared<server::type>(); \
		p->unserialize(reader); \
		return p; \
	}

	switch (id)
	{
#include "eo_protocol/server_packets.tpp"
	}

#undef packet

	return nullptr;
}

static const char* family_names[] = {
	"0",             "Connection",    "Account",       "Character",
	"Login",         "Welcome",       "Walk",          "Face",
	"Chair",         "Emote",         "10",            "Attack",
	"Spell",         "Shop",          "Item",          "15",
	"StatSkill",     "Global",        "Talk",          "Warp",
	"20",            "Jukebox",       "Players",       "Avatar",
    "Party",         "Refresh",       "NPC",           "PlayerRefresh",
    "NPCRefresh",    "Appear",        "Paperdoll",     "Effect",
    "Trade",         "Chest",         "Door",          "Message",
	"Bank",          "Locker",        "Barber",        "Guild",
	"Music",         "Sit",           "Recover",       "Board",
	"Cast",          "Arena",         "Priest",        "Marriage",
	"AdminInteract", "Citizen",       "Quest",         "Book"
};

static const char* action_names[] = {
	"0",            "Request",      "Accept",       "Reply",
	"Remove",       "Agree",        "Create",       "Add",
	"Player",       "Take",         "Use",          "Buy",
	"Sell",         "Open",         "Close",        "Msg",
	"Spec",         "Admin",        "List",         "19",
	"Tell",         "Report",       "Announce",     "Server",
	"Drop",         "Junk",         "Obtain",       "Get",
	"Kick",         "Rank",         "Target_Self",  "Target_Other",
	"32",           "Target_Group", "Dialog",       "35"
};

static char family_name_buf[4];
static char action_name_buf[4];

const char* name(PacketFamily family)
{
	if (family == PacketFamily::Init)
	{
		return "Init";
	}
	else if (eo_byte(family) > std::extent_v<decltype(family_names)>)
	{
		std::snprintf(family_name_buf, sizeof family_name_buf, "%d", eo_byte(family));
		return family_name_buf;
	}
	else
	{
		return family_names[eo_byte(family)];
	}
}

const char* name(PacketAction action)
{
	if (action == PacketAction::Init)
	{
		return "Init";
	}
	else if (eo_byte(action) > std::extent_v<decltype(action_names)>)
	{
		std::snprintf(family_name_buf, sizeof family_name_buf, "%d", eo_byte(action));
		return action_name_buf;
	}
	else
	{
		return action_names[eo_byte(action)];
	}
}


}
