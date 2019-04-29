#pragma once

#include "entities.hpp"
#include "json_serializers.hpp"

#include <Meta.h>
#include <glm/glm.hpp>
#include <sushi/texture.hpp>
#include <sushi/mesh.hpp>
#include <sol_forward.hpp>

#include <functional>
#include <string>
#include <type_traits>

#define REGISTER(NAME, ...)                                       \
    }                                                             \
    namespace meta {                                              \
    template <> constexpr auto registerName<component::NAME>() {  \
        return #NAME;                                             \
    }                                                             \
    template <> inline auto registerMembers<component::NAME>() {  \
        using comtype = component::NAME;                          \
        return members(__VA_ARGS__);                              \
    }                                                             \
    }                                                             \
    namespace component {
#define MEMBER(FIELD) member(#FIELD, &comtype::FIELD)

#define TAG(name) using name = ginseng::tag<struct name##_t>; REGISTER(name)

namespace component {

using json_serializers::basic::from_json;
using json_serializers::basic::to_json;

void register_all_components(sol::table& table);

struct net_id {
    ember_database::net_id id;
};

REGISTER(net_id,
         MEMBER(id))

struct position {
    glm::vec2 pos;
};

REGISTER(position,
         MEMBER(pos))

struct script {
    std::string name;
    int next_tick;
};

REGISTER(script,
         MEMBER(name),
         MEMBER(next_tick))

struct model {
    std::shared_ptr<sushi::static_mesh> mesh;
    std::shared_ptr<sushi::texture_2d> texture;
    float scale = 1;
};

REGISTER(model,
         MEMBER(mesh),
         MEMBER(texture),
         MEMBER(scale))

struct sprite {
    int r;
    int c;
};

REGISTER(sprite,
         MEMBER(r),
         MEMBER(c))

struct controller {
    sol::table data;
};

REGISTER(controller,
         MEMBER(data))

struct motion {
    int dir_r = 0;
    int dir_c = 0;
    int delay = 0;
    int next = 0;
    std::function<void(ember_database::ent_id eid)> on_hit;
};

REGISTER(motion,
         MEMBER(dir_r),
         MEMBER(dir_c),
         MEMBER(delay),
         MEMBER(next),
         MEMBER(on_hit))

struct body {
    bool stops = false;
    bool coin = false;
    bool hurts = false;
    bool exit = false;
};

REGISTER(body,
         MEMBER(stops),
         MEMBER(coin),
         MEMBER(hurts),
         MEMBER(exit))

} //namespace component

#undef TAG
#undef MEMBER
#undef REGISTER
