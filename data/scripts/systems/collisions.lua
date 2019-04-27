local engine = require('engine')
local visitor = require('visitor')

local physics = {}

function physics.visit(dt)
    visitor.visit(
        {component.position, component.aabb},
        function (eid, position, aabb)
            visitor.visit(
                {component.position, component.aabb},
                function (othereid, otherposition, otheraabb)
                    if (
                        position.pos.x + aabb.topright.x >= otherposition.pos.x + otheraabb.bottomleft.x and
                        position.pos.x + aabb.bottomleft.x <= otherposition.pos.x + otheraabb.topright.x and
                        position.pos.y + aabb.topright.y >= otherposition.pos.y + otheraabb.bottomleft.y and
                        position.pos.y + aabb.bottomleft.y <= otherposition.pos.y + otheraabb.topright.y
                    ) then
                        if (engine.entities:has_component(eid, component.collider)) then
                            engine.entities:get_component(eid, component.collider).collide(othereid)
                        end
                    end
                end
            )
        end
    )
end

return physics
