local engine = require('engine')
local visitor = require('visitor')

local controller = {}

function controller.visit(dt)
    local do_reset = false

    visitor.visit(
        {component.motion, component.position},
        function (eid, motion, position)
            if motion.next <= 0 then
                local cur_c = position.pos.x
                local cur_r = position.pos.y
                local next_c = cur_c + motion.dir_c
                local next_r = cur_r + motion.dir_r

                for i,v in ipairs(board[cur_r][cur_c]) do
                    if v:get_index() == eid:get_index() then
                        table.remove(board[cur_r][cur_c], i)
                        break
                    end
                end

                if next_r < 1 or next_r > 15 or next_c < 1 or next_c > 15 then
                    engine.entities:destroy_entity(eid)
                    return
                end

                for _,v in ipairs(board[next_r][next_c]) do
                    if engine.entities:has_component(v, component.body) then
                        local body = engine.entities:get_component(v, component.body)
                        if body.stops then
                            engine.entities:remove_component(eid, component.motion)
                            next_r = cur_r
                            next_c = cur_c
                        end
                        if body.hurts then
                            if engine.entities:has_component(eid, component.script) then
                                local script = engine.entities:get_component(eid, component.script)
                                local script_impl = require('actors.' .. script.name)
                                if script_impl.on_hurt then
                                    local alive = script_impl.on_hurt(eid, v)
                                    if not alive then
                                        return
                                    end
                                end
                            end
                        end
                        if body.exit then
                            if engine.entities:has_component(eid, component.script) then
                                local script = engine.entities:get_component(eid, component.script)
                                local script_impl = require('actors.' .. script.name)
                                if script_impl.on_exit then
                                    local done = script_impl.on_exit(eid, v)
                                    if done then
                                        engine.entities:destroy_entity(eid)
                                        current_stage = current_stage + 1
                                        do_reset = true
                                        return
                                    end
                                end
                            end
                        end
                    end
                end

                table.insert(board[next_r][next_c], eid)

                position.pos.x = next_c
                position.pos.y = next_r

                if engine.entities:has_component(eid, component.motion) then
                    motion.next = motion.delay
                end
            else
                motion.next = motion.next - 1
            end
        end
    )

    if do_reset then
        play_sfx('portal')
        reset()
    end
end

return controller
