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

struct controller {
    sol::table data;
};

REGISTER(controller,
         MEMBER(data))

struct physics {
    glm::vec2 vel;
};

REGISTER(physics,
         MEMBER(vel))

struct aabb {
    glm::vec2 bottomleft;
    glm::vec2 topright;
};

REGISTER(aabb,
         MEMBER(bottomleft),
         MEMBER(topright))

struct collider {
    std::function<void(ember_database::ent_id othereid)> collide;
};

REGISTER(collider,
         MEMBER(collide))

} //namespace component

#undef TAG
#undef MEMBER
#undef REGISTER
