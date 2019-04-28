local engine = require('engine')

return function(r, c)
    local ent = engine.entities:create_entity()

    local position = component.position.new()
    position.pos.x = c
    position.pos.y = r

    local sprite = component.sprite.new()
    sprite.r = 2
    sprite.c = 0

    local body = component.body.new()
    body.stops = true

    engine.entities:add_component(ent, position)
    engine.entities:add_component(ent, sprite)
    engine.entities:add_component(ent, body)

    return ent
end
