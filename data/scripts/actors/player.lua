local engine = require('engine')

local player = {}

function player.controller(eid, keys, dt)
    local pos = engine.entities:get_component(eid, component.position)
    local con = engine.entities:get_component(eid, component.controller)

    local speed = 15 * dt

    if keys.down then pos.pos.y = pos.pos.y - speed end
    if keys.up then pos.pos.y = pos.pos.y + speed end
end

return player
