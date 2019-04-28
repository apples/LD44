local engine = require('engine')

config = {
}

do
    local player = engine.entities:create_entity()

    local position = component.position.new()
    position.pos.x = 0
    position.pos.y = 0

    local sprite = component.sprite.new()
    sprite.r = 1
    sprite.c = 0

    local script = component.script.new()
    script.next_tick = -1
    script.name = 'player'

    local controller = component.controller.new()
    controller.data = {
    }

    engine.entities:add_component(player, position)
    engine.entities:add_component(player, sprite)
    engine.entities:add_component(player, script)
    engine.entities:add_component(player, controller)
end

