#include "components.hpp"

#include "component_scripting.hpp"

namespace component {

void register_all_components(sol::table& table) {
    using scripting::register_type;

    register_type<net_id>(table);
    register_type<position>(table);
    register_type<script>(table);
    register_type<model>(table);
    register_type<sprite>(table);
    register_type<controller>(table);
}

} //namespace component
