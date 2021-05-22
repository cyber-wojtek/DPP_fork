/************************************************************************************
 *
 * D++, A Lightweight C++ library for Discord
 *
 * Copyright 2021 Craig Edwards and D++ contributors 
 * (https://github.com/brainboxdotcc/DPP/graphs/contributors)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ************************************************************************************/
#include <dpp/discord.h>
#include <dpp/event.h>
#include <string>
#include <iostream>
#include <fstream>
#include <dpp/discordclient.h>
#include <dpp/discordevents.h>
#include <dpp/discord.h>
#include <dpp/cache.h>
#include <dpp/stringops.h>
#include <dpp/nlohmann/json.hpp>

using json = nlohmann::json;

namespace dpp { namespace events {

using namespace dpp;

/**
 * @brief Handle event
 * 
 * @param client Websocket client (current shard)
 * @param j JSON data for the event
 * @param raw Raw JSON string
 */
void guild_delete::handle(discord_client* client, json &j, const std::string &raw) {
	json& d = j["d"];
	dpp::guild* g = dpp::find_guild(SnowflakeNotNull(&d, "id"));
	if (g) {
		if (!BoolNotNull(&d, "unavailable")) {
			dpp::get_guild_cache()->remove(g);
			for (auto & ee : g->emojis) {
				dpp::emoji* fe = dpp::find_emoji(ee);
				if (fe) {
					dpp::get_emoji_cache()->remove(fe);
				}
			}
			for (auto & rr : g->roles) {
				dpp::role* role = dpp::find_role(rr);
				if (role) {
					dpp::get_role_cache()->remove(role);
				}
			}
			for (auto & cc : g->channels) {
				dpp::channel* ch = dpp::find_channel(cc);
				if (ch) {
					dpp::get_channel_cache()->remove(ch);
				}
			}
			for (auto gm = g->members.begin(); gm != g->members.end(); ++gm) {
				dpp::user* u = dpp::find_user(gm->second->user_id);
				if (u) {
					u->refcount--;
					if (u->refcount < 1) {
						dpp::get_user_cache()->remove(u);
					}
				}
				delete gm->second;
			}
			g->members.clear();
		} else {
			g->flags |= dpp::g_unavailable;
		}

		if (client->creator->dispatch.guild_delete) {
			dpp::guild_delete_t gd(client, raw);
			gd.deleted = g;
			client->creator->dispatch.guild_delete(gd);
		}
	}
}

}};