local engine = require('engine')

return function(r, c)
    local player = engine.entities:create_entity()

    local position = component.position.new()
    position.pos.x = c
    position.pos.y = r

    local sprite = component.sprite.new()
    sprite.r = 1
    sprite.c = 0

    local script = component.script.new()
    script.next_tick = -1
    script.name = 'player'

    local controller = component.controller.new()
    controller.data = {
    }

    local body = component.body.new()
    body.stops = true

    engine.entities:add_component(player, position)
    engine.entities:add_component(player, sprite)
    engine.entities:add_component(player, script)
    engine.entities:add_component(player, controller)
    engine.entities:add_component(player, body)

    return player
end
