local engine = require('engine')

local player = {}

function player.update(eid, dt)
    local pos = engine.entities:get_component(eid, component.position)
    local r = pos.pos.y
    local c = pos.pos.x

    local stack = board[r][c]

    for _,e in ipairs(stack) do
        if engine.entities:has_component(e, component.body) then
            local body = engine.entities:get_component(e, component.body)
            if body.coin then
                coins = coins + 1
                engine.entities:destroy_entity(e)
            end
        end
    end
end

function player.on_hurt(eid, other)
    if coins <= 0 then
        engine.entities:destroy_entity(eid)
    else
        coins = coins - 1
    end
end

function player.controller(eid, keys, dt)
    local pos = engine.entities:get_component(eid, component.position)
    local con = engine.entities:get_component(eid, component.controller)

    local delay = 1

    if not engine.entities:has_component(eid, component.motion) then
        local dir_r = 0
        local dir_c = 0

        if keys.down_pressed then dir_r = -1 end
        if keys.up_pressed then dir_r = 1 end

        if keys.left_pressed then dir_c = -1 end
        if keys.right_pressed then dir_c = 1 end

        if dir_r ~= 0 or dir_c ~= 0 then
            local motion = component.motion.new()
            motion.dir_r = dir_r
            motion.dir_c = dir_r == 0 and dir_c or 0
            motion.delay = delay
            motion.next = 0
            engine.entities:add_component(eid, motion)
        end
    end
end

return player
