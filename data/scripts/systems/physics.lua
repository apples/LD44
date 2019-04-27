local engine = require('engine')
local visitor = require('visitor')

local physics = {}

function physics.visit(dt)
    visitor.visit(
        {component.physics, component.position},
        function (eid, physics, position)
            position.x = position.x + physics.vel.x
            position.y = position.y + physics.vel.y
        end
    )
end

return physics
